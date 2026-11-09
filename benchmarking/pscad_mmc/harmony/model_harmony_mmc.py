"""Corrected averaged MMC GFL benchmark model (V3).

PSCAD switch resistance is per cell, while Rarm is the total arm resistance.
The state representation retains selected arm harmonics; PWM and individual
cell sorting are not represented. auto freeze is a nonlinear hybrid law, and
a frequency response describes only the selected local operating branch.

Harmony MMC GFL State-Space and Admittance Engine.

Models the Grid-Following (GFL) MMC with control and averaged topology based on PSCAD
(VSCConverter.f, VSCControl1.f, CCSC.f, MMC_Pole_PWM.f):
- Phase-Locked Loop (PLL, master:tvekta):
    e_pll = -Vq_raw_pu
    d(x_pll)/dt = Gi_pll * e_pll
    w_pll = w0 + Gp_pll * e_pll + x_pll
    d(theta_pll)/dt = w_pll - w0
- PCC Voltage Feedforward Filter (master:realpole, Tf_v = 0.02 s):
    d(Vd_f_pu)/dt = (Vd_raw_pu - Vd_f_pu) / Tf_v
    d(Vq_f_pu)/dt = (Vq_raw_pu - Vq_f_pu) / Tf_v
- Multimeter MM1 Power Measurement Filter (P3PH3/Q3PH3, Tf_pq = 0.02 s):
    d(Ppu)/dt = (P_raw_pu - Ppu) / Tf_pq
    d(Qpu)/dt = (Q_raw_pu - Qpu) / Tf_pq
- Outer Active and Reactive Power PI controllers (PIwithFreeze):
    e_P = Pref_pu - Ppu,  d(x_P)/dt = e_P / Ti_P,  Id_ref_pu = Kp_P * e_P + x_P
    e_Q = Qref_pu - Qpu,  d(x_Q)/dt = e_Q / Ti_Q,  Iq_ref_pu = Kp_Q * e_Q + x_Q
- Inner Decoupled Current Loop (VSCControl1.f):
    idpu = id_pll / Ibase_pk,   iqpu = iq_pll / Ibase_pk
    iderr = Id_ref_pu - idpu,   iqerr = Iq_ref_pu - iqpu
    d(x_id)/dt = iderr / Tid,   d(x_iq)/dt = iqerr / Tiq
    vd1_pu = Vd_f_pu - wLpu * iqpu - (Kpd * iderr + x_id)
    vq1_pu = Vq_f_pu + wLpu * idpu - (Kpq * iqerr + x_iq)
- Circulating Current Suppression Controller (CCSC.f, 2*w0 frame, Kp=1.0, Ti=0.02 s)
  and constant DC insertion mSigma_z = 1.0 (stiff 240 kV DC source, MMC_Pole_PWM.f).
- Full 12-state nonlinear MMC arm dynamics (iDelta, iSigma, vCDelta, vCSigma).
"""
from __future__ import annotations

import math
import numpy as np
from pathlib import Path
from scipy.optimize import root


def _clip(value, lower, upper):
    if isinstance(value, np.ndarray):
        return np.clip(value, lower, upper)
    return min(max(value, lower), upper)


class HarmonyMmcGfl:
    def __init__(
        self,
        f0: float = 50.0,
        Vdc: float = 240e3,
        Vm: float = 100e3,        # 100 kV peak phase (= 122.4745 kV RMS L-L)
        Pac: float = 100e6,       # Positive AC power entering the converter
        Qac: float = 0.0,         # 0 MVAr
        Sbase: float = 100e6,
        Larm: float = 50e-3,
        Rarm: float | None = None,
        Lf: float = 60e-3,
        Rf: float = 0.535,
        Carm: float = 0.01,
        N: int = 50,
        # PLL parameters (master:tvekta)
        Gp_pll: float = 3000.0,
        Gi_pll: float = 2000.0,
        # Measurement & feedforward filter time constants (PSCAD MM1 & realpole)
        Tf_pq: float = 0.02,
        Tf_v: float = 0.02,
        # Outer Power PI gains
        Kp_P: float = 0.5,
        Ti_P: float = 0.1,
        Kp_Q: float = 0.1,
        Ti_Q: float = 0.05,
        # Inner Current PI gains
        Kpd: float = 0.55,
        Tid: float = 0.01,
        Kpq: float = 0.55,
        Tiq: float = 0.01,
        # CCSC 2*w0 PI gains (CCSC.f)
        Kp_ccsc: float = 1.0,
        Ti_ccsc: float = 0.02,
        Rcell: float = 1.07,
    ):
        positive = [f0, Vdc, Vm, Sbase, Larm, Lf, Carm, Tf_pq, Tf_v,
                    Ti_P, Ti_Q, Tid, Tiq, Ti_ccsc]
        if any(not math.isfinite(v) or v <= 0 for v in positive):
            raise ValueError("Frequencies, ratings, inductances, capacitance and time constants must be positive and finite")
        if not isinstance(N, int) or N < 1 or not math.isfinite(Rcell) or Rcell < 0 or (Rarm is not None and (not math.isfinite(Rarm) or Rarm < 0)):
            raise ValueError("N must be a positive integer and resistances must be nonnegative")
        self.f0 = f0
        self.w0 = 2.0 * math.pi * f0
        self.Vdc = Vdc
        self.Vm = Vm
        self.Pac_ref = Pac
        self.Qac_ref = Qac
        self.Sbase = Sbase
        self.Larm = Larm
        self.Rcell = Rcell
        self.Rarm = N * Rcell if Rarm is None else Rarm
        Rarm = self.Rarm
        self.outer_mode = "auto"
        self.modulation_mode = "closed_loop"
        self.Imax = 1.5
        self.freeze_threshold = 0.99999
        self._held_modulation = None
        self.Lf = Lf
        self.Rf = Rf
        self.Carm = Carm  # Cell capacitance; equivalent series arm value is Carm/N
        self.N = N

        self.Gp_pll = Gp_pll
        self.Gi_pll = Gi_pll
        self.Tf_pq = Tf_pq
        self.Tf_v = Tf_v

        self.Kp_P = Kp_P
        self.Ti_P = Ti_P
        self.Kp_Q = Kp_Q
        self.Ti_Q = Ti_Q

        self.Kpd = Kpd
        self.Tid = Tid
        self.Kpq = Kpq
        self.Tiq = Tiq

        self.Kp_ccsc = Kp_ccsc
        self.Ti_ccsc = Ti_ccsc

        self.Reqac = Rf + Rarm / 2.0
        self.Leqac = Lf + Larm / 2.0

        # Base values for per-unit conversion (matching VSCControl1.f and CCSC.f)
        self.Vbase_pk = Vm                                # 100 kV peak phase
        self.Ibase_pk = (2.0 / 3.0) * Sbase / Vm          # 666.667 A peak
        self.Ibase_rms = self.Ibase_pk / math.sqrt(2.0)   # 471.4045 A RMS (used in CCSC.f)
        self.Zbase = (Vm * math.sqrt(1.5)) ** 2 / Sbase   # 150 Ohm
        # V2 FIX: VSCControl1.f line 818: wLpu = Xt + w*(Lf + Larm/2)/Zbase, Xt = 0.1256637 pu
        self.Xt = 0.1256637
        self.wLpu = self.Xt + (self.w0 * self.Leqac) / self.Zbase   # 0.3037 pu

        self.iSigma_z_ref = Pac / (3.0 * Vdc)

        self.u0 = np.array([Vdc, Vm, 0.0], dtype=float)
        self.x_eq = None
        self.A = None
        self.B = None
        self.C = None
        self.D = None

    def derivatives(self, x: np.ndarray, u: np.ndarray, for_equilibrium: bool = False) -> np.ndarray:
        batch = x.ndim == 2
        cos = np.cos if batch else math.cos
        sin = np.sin if batch else math.sin
        hypot = np.hypot if batch else math.hypot
        Vdc, Vgd, Vgq = u[0], u[1], u[2]
        w_g = self.w0

        # Unpack states (24 states):
        # 0: theta_pll (rad)
        # 1: x_pll (rad/s)
        # 2: Ppu (filtered active power pu, MM1 Tf_pq = 0.02 s)
        # 3: Qpu (filtered reactive power pu, MM1 Tf_pq = 0.02 s)
        # 4: Vd_f_pu (filtered d-axis PCC voltage pu, realpole Tf_v = 0.02 s)
        # 5: Vq_f_pu (filtered q-axis PCC voltage pu, realpole Tf_v = 0.02 s)
        # 6: x_P (outer active power PI integrator)
        # 7: x_Q (outer reactive power PI integrator)
        # 8: x_id (inner d-axis current PI integrator)
        # 9: x_iq (inner q-axis current PI integrator)
        # 10: x_ccsc_d (CCSC d-axis 2*w0 PI integrator)
        # 11: x_ccsc_q (CCSC q-axis 2*w0 PI integrator)
        # 12: iDelta_d (A, generator convention: leaving converter into PCC)
        # 13: iDelta_q (A, generator convention: leaving converter into PCC)
        # 14: iSigma_z (A)
        # 15: iSigma_d (A)
        # 16: iSigma_q (A)
        # 17..20: vCDelta_d, vCDelta_q, vCDelta_Zd, vCDelta_Zq (V)
        # 21..23: vCSigma_d, vCSigma_q, vCSigma_z (V)
        theta_pll = x[0]
        x_pll = x[1]
        Ppu = x[2]
        Qpu = x[3]
        Vd_f_pu = x[4]
        Vq_f_pu = x[5]
        x_P = x[6]
        x_Q = x[7]
        x_id = x[8]
        x_iq = x[9]
        x_ccsc_d = x[10]
        x_ccsc_q = x[11]

        iDelta_d = x[12]
        iDelta_q = x[13]
        iSigma_z = x[14]
        iSigma_d = x[15]
        iSigma_q = x[16]

        vCDelta_d = x[17]
        vCDelta_q = x[18]
        vCDelta_Zd = x[19]
        vCDelta_Zq = x[20]

        vCSigma_d = x[21]
        vCSigma_q = x[22]
        vCSigma_z = x[23]

        # 1. Coordinate Transformation into PLL Frame (PSCAD TRDQO dqorder=1)
        # In PSCAD TRDQO(..., theta, 1):
        # X_a(t) = X_d * cos(w0*t + theta_pll) + X_q * sin(w0*t + theta_pll)
        # Expanding cos(w0*t + theta_pll) and sin(w0*t + theta_pll) in terms of the global
        # synchronous frame (where X_a(t) = X_gd * cos(w0*t) + X_gq * sin(w0*t)):
        #   X_d,pll =  cos(theta_pll) * X_gd - sin(theta_pll) * X_gq
        #   X_q,pll =  sin(theta_pll) * X_gd + cos(theta_pll) * X_gq
        cos_pll = cos(theta_pll)
        sin_pll = sin(theta_pll)

        Vd_pll = cos_pll * Vgd - sin_pll * Vgq
        Vq_pll = sin_pll * Vgd + cos_pll * Vgq

        Vd_raw_pu = Vd_pll / self.Vbase_pk
        Vq_raw_pu = Vq_pll / self.Vbase_pk

        # Converter currents in VSCConverter.f (Iabc) and Z-tool (IDUTacA2) are measured
        # ENTERING the converter from the PCC (load convention: I_load = -iDelta):
        i_load_d = -iDelta_d
        i_load_q = -iDelta_q
        id_pll = cos_pll * i_load_d - sin_pll * i_load_q
        iq_pll = sin_pll * i_load_d + cos_pll * i_load_q

        idpu = id_pll / self.Ibase_pk
        iqpu = iq_pll / self.Ibase_pk

        # 2. PLL Dynamics (master:tvekta locks Vq_raw_pu to 0; negative Vq_raw_pu means
        # grid voltage leads PLL angle, so e_pll = -Vq_raw_pu drives theta_pll positive)
        e_pll = -Vq_raw_pu
        dx_pll = self.Gi_pll * e_pll
        w_pll = self.w0 + self.Gp_pll * e_pll + x_pll
        dtheta_pll = w_pll - self.w0

        # 3. Voltage Feedforward LPF (master:realpole, Tf_v = 0.02 s)
        dVd_f_pu = (Vd_raw_pu - Vd_f_pu) / self.Tf_v
        dVq_f_pu = (Vq_raw_pu - Vq_f_pu) / self.Tf_v

        # 4. Multimeter MM1 Power Measurements (P3PH3/Q3PH3 at PCC, Tf_pq = 0.02 s)
        # In PSCAD TRDQO (dqorder=1, q-axis lagging d-axis) with load current i_load = -iDelta:
        P_raw = 1.5 * (Vgd * i_load_d + Vgq * i_load_q)
        Q_raw = 1.5 * (Vgd * i_load_q - Vgq * i_load_d)
        P_raw_pu = P_raw / self.Sbase
        Q_raw_pu = Q_raw / self.Sbase

        dPpu = (P_raw_pu - Ppu) / self.Tf_pq
        dQpu = (Q_raw_pu - Qpu) / self.Tf_pq

        Pref_pu = self.Pac_ref / self.Sbase
        Qref_pu = self.Qac_ref / self.Sbase

        # 5. Outer Power Control Loops (VSCControl1.f lines 821, 711, 937, 974)
        e_P = Pref_pu - Ppu
        e_Q = Qref_pu - Qpu

        dx_P = (1.0 / self.Ti_P) * e_P
        dx_Q = (1.0 / self.Ti_Q) * e_Q
        id_raw = self.Kp_P * e_P + x_P
        iq_raw = self.Kp_Q * e_Q + x_Q
        id_outer = _clip(id_raw, -self.Imax, self.Imax)
        iq_outer = _clip(iq_raw, -0.5, 0.5)
        requested_magnitude = hypot(id_outer, iq_outer)
        mode = "frozen" if getattr(self, "freeze_outer", False) else self.outer_mode
        if mode not in ("auto", "frozen", "unfrozen"):
            raise ValueError(f"Unknown outer_mode: {mode}")
        freeze = mode == "frozen" or (mode == "auto" and
                  _clip(requested_magnitude, 0, self.Imax) > self.freeze_threshold)
        if mode == "auto" and hasattr(self, "_sampled_freeze"):
            freeze = self._sampled_freeze
        if not for_equilibrium:
            if batch:
                dx_P = np.where(freeze | (id_raw <= -self.Imax) | (id_raw >= self.Imax), 0, dx_P)
                dx_Q = np.where(freeze | (iq_raw <= -0.5) | (iq_raw >= 0.5), 0, dx_Q)
            elif freeze or not -self.Imax < id_raw < self.Imax:
                dx_P = 0.0
            if not batch and (freeze or not -0.5 < iq_raw < 0.5):
                dx_Q = 0.0
        # PSCAD limits the current-reference vector after the outer PI limits.
        factor = np.minimum(1.0, self.Imax / np.maximum(requested_magnitude, 1e-30)) if batch else min(1.0, self.Imax / max(requested_magnitude, 1e-30))
        id_ref_pu = factor * id_outer
        iq_ref_pu = factor * iq_outer

        # 6. Inner Current Control Loops (matching VSCControl1.f lines 997-1016)
        e_id = id_ref_pu - idpu
        e_iq = iq_ref_pu - iqpu

        dx_id = (1.0 / self.Tid) * e_id
        dx_iq = (1.0 / self.Tiq) * e_iq

        pi_d_raw = self.Kpd * e_id + x_id
        pi_q_raw = self.Kpq * e_iq + x_iq
        if not for_equilibrium:
            if batch:
                dx_id = np.where((pi_d_raw <= -1) | (pi_d_raw >= 1), 0, dx_id)
                dx_iq = np.where((pi_q_raw <= -1) | (pi_q_raw >= 1), 0, dx_iq)
            elif not -1.0 < pi_d_raw < 1.0:
                dx_id = 0.0
            if not batch and not -1.0 < pi_q_raw < 1.0:
                dx_iq = 0.0
        vd1_pu = Vd_f_pu - self.wLpu * iqpu - _clip(pi_d_raw, -1.0, 1.0)
        vq1_pu = Vq_f_pu + self.wLpu * idpu - _clip(pi_q_raw, -1.0, 1.0)

        # Scale from per-unit to physical Volts:
        vd1_ref_pll = vd1_pu * self.Vbase_pk
        vq1_ref_pll = vq1_pu * self.Vbase_pk

        # Rotate internal voltage reference back from PLL frame to global frame
        # (inverse of TRDQO(..., theta, 1) rotation):
        vMDelta_d_ref = cos_pll * vd1_ref_pll + sin_pll * vq1_ref_pll
        vMDelta_q_ref = -sin_pll * vd1_ref_pll + cos_pll * vq1_ref_pll

        # 7. Circulating Current Suppression Controller (CCSC.f, 2*w0 PLL frame)
        # Note: CCSC.f swaps phases B and C in TRDQO(Iza, Izc, Izb, ..., Theta_2, 1) because
        # 2*w0 circulating current is negative sequence:
        #   I_za(t) = iSigma_d * cos(2*w0*t) - iSigma_q * sin(2*w0*t)
        # With Theta_2 = 2*w0*t + 2*theta_pll, TRDQO(Iza, Izc, Izb, ..., Theta_2, 1) gives:
        #   izd =  cos(2*theta_pll) * iSigma_d + sin(2*theta_pll) * iSigma_q
        #   izq =  sin(2*theta_pll) * iSigma_d - cos(2*theta_pll) * iSigma_q
        cos_2pll = cos(2.0 * theta_pll)
        sin_2pll = sin(2.0 * theta_pll)
        izd_pll = cos_2pll * iSigma_d + sin_2pll * iSigma_q
        izq_pll = sin_2pll * iSigma_d - cos_2pll * iSigma_q

        e_ccsc_d = -izd_pll / self.Ibase_rms
        e_ccsc_q = -izq_pll / self.Ibase_rms

        dx_ccsc_d = (1.0 / self.Ti_ccsc) * e_ccsc_d
        dx_ccsc_q = (1.0 / self.Ti_ccsc) * e_ccsc_q

        Vzd_pll = self.Kp_ccsc * e_ccsc_d + x_ccsc_d
        Vzq_pll = self.Kp_ccsc * e_ccsc_q + x_ccsc_q
        if not for_equilibrium:
            if batch:
                dx_ccsc_d = np.where((Vzd_pll <= -1) | (Vzd_pll >= 1), 0, dx_ccsc_d)
                dx_ccsc_q = np.where((Vzq_pll <= -1) | (Vzq_pll >= 1), 0, dx_ccsc_q)
            elif not -1.0 < Vzd_pll < 1.0:
                dx_ccsc_d = 0.0
            if not batch and not -1.0 < Vzq_pll < 1.0:
                dx_ccsc_q = 0.0
        Vzd_pll = _clip(Vzd_pll, -1.0, 1.0)
        Vzq_pll = _clip(Vzq_pll, -1.0, 1.0)

        # Inverse TRDQO(Vza, Vzc, Vzb, Vzd, Vzq, 0, Theta_2, -1) into (mSigma_d, mSigma_q):
        # Since Vza(t) = Vzd_pll * cos(2*w0*t + 2*theta_pll) + Vzq_pll * sin(2*w0*t + 2*theta_pll)
        #              = Vzd * cos(2*w0*t) + Vzq * sin(2*w0*t):
        # V2 FIX: CCSC.f uses the SAME swapped-phase TRDQO(a,c,b) for the forward and
        # inverse transform. The forward map [[c, s], [s, -c]] is a reflection and is its
        # own inverse. The old code used a rotation inverse, which flipped the sign of the
        # q-axis CCSC loop (positive feedback -> unstable pole at +5077 1/s).
        Vzd = cos_2pll * Vzd_pll + sin_2pll * Vzq_pll
        Vzq = sin_2pll * Vzd_pll - cos_2pll * Vzq_pll

        # 8. MMC Modulation Indices & Internal Voltages (MMC_Pole_PWM.f)
        # In MMC_Pole_PWM.f:
        #   VrefT = -(Vz + Vref), VrefB = -Vz + Vref
        #   n_u = (1 + VrefT)/2 = (1 - Vref - Vz)/2, n_l = (1 + VrefB)/2 = (1 + Vref - Vz)/2
        #   mDelta = n_u - n_l = -Vref,  mSigma = n_u + n_l = 1 - Vz
        mDelta_d = -2.0 * vMDelta_d_ref / Vdc
        mDelta_q = -2.0 * vMDelta_q_ref / Vdc
        mDelta_Zd = 0.0
        mDelta_Zq = 0.0
        mSigma_d = -Vzd
        mSigma_q = -Vzq
        mSigma_z = 1.0  # Fixed DC insertion index (MMC_Pole_PWM.f lines 186-198)

        modulation = (mDelta_d, mDelta_q, mSigma_d, mSigma_q)
        if getattr(self, "_capture_modulation", False):
            self._captured_modulation = modulation
        if for_equilibrium:
            self._equilibrium_modulation = modulation
        elif getattr(self, "_sampled_modulation", None) is not None:
            mDelta_d, mDelta_q, mSigma_d, mSigma_q = self._sampled_modulation
        elif self.modulation_mode == "frozen":
            if self._held_modulation is None:
                raise RuntimeError("Solve the equilibrium before freezing modulation")
            mDelta_d, mDelta_q, mSigma_d, mSigma_q = self._held_modulation
        elif self.modulation_mode != "closed_loop":
            raise ValueError(f"Unknown modulation_mode: {self.modulation_mode}")

        vMDelta_d = ((mDelta_q * vCSigma_q) / 4.0 - (mDelta_d * vCSigma_z) / 2.0 - (mDelta_d * vCSigma_d) / 4.0
            - (mDelta_Zd * vCSigma_d) / 4.0 + (mDelta_Zq * vCSigma_q) / 4.0 - (mSigma_d * vCDelta_d) / 4.0
            - (mSigma_z * vCDelta_d) / 2.0 + (mSigma_q * vCDelta_q) / 4.0 - (mSigma_d * vCDelta_Zd) / 4.0
            + (mSigma_q * vCDelta_Zq) / 4.0)

        vMDelta_q = ((mDelta_d * vCSigma_q) / 4.0 + (mDelta_q * vCSigma_d) / 4.0 - (mDelta_q * vCSigma_z) / 2.0
            - (mDelta_Zd * vCSigma_q) / 4.0 - (mDelta_Zq * vCSigma_d) / 4.0 + (mSigma_d * vCDelta_q) / 4.0
            + (mSigma_q * vCDelta_d) / 4.0 - (mSigma_z * vCDelta_q) / 2.0 - (mSigma_d * vCDelta_Zq) / 4.0
            - (mSigma_q * vCDelta_Zd) / 4.0)

        vMSigma_d = ((mDelta_d * vCDelta_d) / 4.0 - (mDelta_q * vCDelta_q) / 4.0 + (mDelta_d * vCDelta_Zd) / 4.0
            + (mDelta_Zd * vCDelta_d) / 4.0 + (mDelta_q * vCDelta_Zq) / 4.0 + (mDelta_Zq * vCDelta_q) / 4.0
            + (mSigma_d * vCSigma_z) / 2.0 + (mSigma_z * vCSigma_d) / 2.0)

        vMSigma_q = ((mDelta_q * vCDelta_Zd) / 4.0 - (mDelta_q * vCDelta_d) / 4.0 - (mDelta_d * vCDelta_Zq) / 4.0
            - (mDelta_d * vCDelta_q) / 4.0 + (mDelta_Zd * vCDelta_q) / 4.0 - (mDelta_Zq * vCDelta_d) / 4.0
            + (mSigma_q * vCSigma_z) / 2.0 + (mSigma_z * vCSigma_q) / 2.0)

        vMSigma_z = ((mDelta_d * vCDelta_d) / 4.0 + (mDelta_q * vCDelta_q) / 4.0 + (mDelta_Zd * vCDelta_Zd) / 4.0
            + (mDelta_Zq * vCDelta_Zq) / 4.0 + (mSigma_d * vCSigma_d) / 4.0 + (mSigma_q * vCSigma_q) / 4.0
            + (mSigma_z * vCSigma_z) / 2.0)

        # 9. Plant Dynamics (12 retained arm states). Same positive-sine frame as
        # Harmony's MMC: vMd = Vd + R id − ωL iq, vMq = Vq + R iq + ωL id.
        dx = np.zeros_like(x, dtype=float)
        dx[0] = dtheta_pll
        dx[1] = dx_pll
        dx[2] = dPpu
        dx[3] = dQpu
        dx[4] = dVd_f_pu
        dx[5] = dVq_f_pu
        dx[6] = dx_P
        dx[7] = dx_Q
        dx[8] = dx_id
        dx[9] = dx_iq
        dx[10] = dx_ccsc_d
        dx[11] = dx_ccsc_q

        dx[12] = -(Vgd - vMDelta_d + self.Reqac * iDelta_d - self.Leqac * iDelta_q * w_g) / self.Leqac
        dx[13] = -(Vgq - vMDelta_q + self.Reqac * iDelta_q + self.Leqac * iDelta_d * w_g) / self.Leqac

        dx[14] = -(vMSigma_z - Vdc / 2.0 + self.Rarm * iSigma_z) / self.Larm
        dx[15] = -(vMSigma_d + self.Rarm * iSigma_d + 2.0 * self.Larm * iSigma_q * w_g) / self.Larm
        dx[16] = -(vMSigma_q + self.Rarm * iSigma_q - 2.0 * self.Larm * iSigma_d * w_g) / self.Larm

        Ca = self.Carm
        N = self.N

        dx[17] = (N * (iSigma_z * mDelta_d - (iDelta_q * mSigma_q) / 4.0 + iSigma_d * (mDelta_d / 2.0 + mDelta_Zd / 2.0)
            - iSigma_q * (mDelta_q / 2.0 + mDelta_Zq / 2.0) + iDelta_d * (mSigma_d / 4.0 + mSigma_z / 2.0)
            + (2.0 * Ca * vCDelta_q * w_g) / N)) / (2.0 * Ca)

        dx[18] = -(N * ((iDelta_d * mSigma_q) / 4.0 - iSigma_z * mDelta_q + iSigma_q * (mDelta_d / 2.0 - mDelta_Zd / 2.0)
            + iSigma_d * (mDelta_q / 2.0 - mDelta_Zq / 2.0) + iDelta_q * (mSigma_d / 4.0 - mSigma_z / 2.0)
            + (2.0 * Ca * vCDelta_d * w_g) / N)) / (2.0 * Ca)

        dx[19] = (N * (iDelta_d * mSigma_d + 2.0 * iSigma_d * mDelta_d + iDelta_q * mSigma_q
            + 2.0 * iSigma_q * mDelta_q + 4.0 * iSigma_z * mDelta_Zd)) / (8.0 * Ca) + 3.0 * vCDelta_Zq * w_g

        dx[20] = -3.0 * vCDelta_Zd * w_g + (N * (iDelta_q * mSigma_d - iDelta_d * mSigma_q
            + 2.0 * iSigma_d * mDelta_q - 2.0 * iSigma_q * mDelta_d + 4.0 * iSigma_z * mDelta_Zq)) / (8.0 * Ca)

        dx[21] = (N * (iSigma_d * mSigma_z + iSigma_z * mSigma_d + iDelta_d * (mDelta_d / 4.0 + mDelta_Zd / 4.0)
            - iDelta_q * (mDelta_q / 4.0 - mDelta_Zq / 4.0) - (4.0 * Ca * vCSigma_q * w_g) / N)) / (2.0 * Ca)

        dx[22] = -(N * (iDelta_q * (mDelta_d / 4.0 - mDelta_Zd / 4.0) - iSigma_z * mSigma_q - iSigma_q * mSigma_z
            + iDelta_d * (mDelta_q / 4.0 + mDelta_Zq / 4.0) - (4.0 * Ca * vCSigma_d * w_g) / N)) / (2.0 * Ca)

        dx[23] = (N * (iDelta_d * mDelta_d + iDelta_q * mDelta_q + 2.0 * iSigma_d * mSigma_d
            + 2.0 * iSigma_q * mSigma_q + 4.0 * iSigma_z * mSigma_z)) / (8.0 * Ca)

        if self.modulation_mode == "frozen" and not for_equilibrium:
            # Hold the actual modulation, including proportional feedback paths.
            dx[:12] = 0.0
        return dx

    def solve_equilibrium(self) -> np.ndarray:
        x0 = np.zeros(24, dtype=float)
        x0[0] = 0.0
        x0[1] = 0.0
        x0[2] = 1.0  # Ppu = 1.0 pu
        x0[3] = 0.0  # Qpu = 0.0 pu
        x0[4] = 1.0  # Vd_f_pu = 1.0 pu
        x0[5] = 0.0  # Vq_f_pu = 0.0 pu
        x0[6] = 1.0  # x_P = Id_ref_pu ~ 1.0 pu
        x0[7] = 0.0  # x_Q = Iq_ref_pu ~ 0.0 pu
        x0[8] = -0.01  # x_id
        x0[9] = 0.0    # x_iq
        x0[10] = 0.0
        x0[11] = 0.0

        x0[12] = -self.Ibase_pk     # -666.67 A (load current +666.67 A entering converter)
        x0[13] = 0.0
        x0[14] = -self.iSigma_z_ref # -138.89 A
        x0[23] = self.Vdc

        # Solve dimensionless equations: currents, voltages and pu controller
        # states differ by five orders of magnitude in physical coordinates.
        scale = np.array([1, self.w0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          self.Ibase_pk, self.Ibase_pk, self.Ibase_pk,
                          self.Ibase_pk, self.Ibase_pk] + [self.Vdc] * 7)
        equation = lambda z: self.derivatives(z * scale, self.u0,
                                              for_equilibrium=True) / scale
        res = root(equation, x0 / scale, method="hybr", tol=1e-10)
        if not res.success or np.max(np.abs(equation(res.x))) > 1e-7:
            from scipy.optimize import least_squares
            res = least_squares(equation, res.x, xtol=1e-12, ftol=1e-12,
                                gtol=1e-12, max_nfev=3000)
        residual = float(np.max(np.abs(equation(res.x))))
        if not res.success or not np.isfinite(residual) or residual > 1e-7:
            raise RuntimeError(f"MMC equilibrium failed: {res.message}; residual={residual}")
        self.x_eq = res.x * scale
        self.derivatives(self.x_eq, self.u0, for_equilibrium=True)
        self._held_modulation = self._equilibrium_modulation
        self.equilibrium_residual = residual
        self.A = self.B = self.C = self.D = None
        return self.x_eq

    def compute_abcd(self, eps: float = 1e-7):
        if self.x_eq is None:
            self.solve_equilibrium()
        x0 = self.x_eq
        u0 = self.u0
        nominal_residual = self.derivatives(x0, u0)
        if np.max(np.abs(nominal_residual)) > 1e-4:
            raise RuntimeError("Linearization point is not an equilibrium")
        nx = len(x0)
        nu = len(u0)

        A = np.zeros((nx, nx), dtype=float)
        for j in range(nx):
            dx_step = np.zeros(nx)
            h = eps * max(abs(x0[j]), 1.0)
            dx_step[j] = h
            f_plus = self.derivatives(x0 + dx_step, u0, for_equilibrium=False)
            f_minus = self.derivatives(x0 - dx_step, u0, for_equilibrium=False)
            A[:, j] = (f_plus - f_minus) / (2.0 * h)

        B = np.zeros((nx, nu), dtype=float)
        for j in range(nu):
            du_step = np.zeros(nu)
            h = eps * max(abs(u0[j]), 1.0)
            du_step[j] = h
            f_plus = self.derivatives(x0, u0 + du_step, for_equilibrium=False)
            f_minus = self.derivatives(x0, u0 - du_step, for_equilibrium=False)
            B[:, j] = (f_plus - f_minus) / (2.0 * h)

        # Output currents: [i_dc, i_d, i_q]^T = [3*iSigma_z, iDelta_d, iDelta_q]^T
        C = np.zeros((3, nx), dtype=float)
        C[0, 14] = 3.0  # 3 * iSigma_z
        C[1, 12] = 1.0  # iDelta_d
        C[2, 13] = 1.0  # iDelta_q

        D = np.zeros((3, nu), dtype=float)

        self.A = A
        self.B = B
        self.C = C
        self.D = D
        return A, B, C, D

    def compute_y(self, freq_hz: float) -> np.ndarray:
        if self.A is None:
            self.compute_abcd()
        s = 1j * 2.0 * math.pi * freq_hz
        nx = self.A.shape[0]
        sI_minus_A = s * np.eye(nx, dtype=complex) - self.A
        inv_part = np.linalg.solve(sI_minus_A, self.B.astype(complex))
        Y = self.C.astype(complex) @ inv_part + self.D
        return Y

    def simulate(self, duration, input_function, timestep=20e-6,
                 output_stride=13, initial_state=None,
                 controller_timestep=20e-6, sample_modulation=True):
        """Integrate the hybrid averaged model with a sampled freeze comparator.

        PSCAD evaluates the comparator on solver steps. A discontinuous auto
        comparator inside an adaptive ODE solver can chatter at the threshold;
        latch it once per controller step instead. RK4 integrates the continuous
        states while that branch and the modulation command are held. A finer
        integration step preserves the specified controller sampling period.
        timestep is in seconds, not microseconds.
        PWM and cell sorting remain outside this averaged model.
        """
        if timestep <= 0 or output_stride < 1 or duration <= 0:
            raise ValueError("Duration, timestep and output_stride must be positive")
        control_stride = int(round(controller_timestep / timestep))
        if control_stride < 1 or not np.isclose(control_stride * timestep, controller_timestep, rtol=0, atol=1e-12):
            raise ValueError("Controller timestep must be an integer multiple of integration timestep")
        if self.x_eq is None:
            self.solve_equilibrium()
        x = self.x_eq.copy() if initial_state is None else np.array(initial_state, dtype=float).copy()
        count = int(round(duration / timestep))
        if not np.isclose(count * timestep, duration, rtol=0, atol=1e-10):
            raise ValueError("Duration must be a multiple of timestep")
        times, states, freezes = [], [], []
        try:
            for step in range(count + 1):
                t = step * timestep
                if step % control_stride == 0:
                    idref = _clip(self.Kp_P * (self.Pac_ref / self.Sbase - x[2]) + x[6], -self.Imax, self.Imax)
                    iqref = _clip(self.Kp_Q * (self.Qac_ref / self.Sbase - x[3]) + x[7], -0.5, 0.5)
                    magnitude = np.hypot(idref, iqref) if x.ndim == 2 else math.hypot(idref, iqref)
                    self._sampled_freeze = _clip(magnitude, 0, self.Imax) > self.freeze_threshold
                    if sample_modulation and self.modulation_mode == "closed_loop":
                        self._sampled_modulation = None
                        self._capture_modulation = True
                        self.derivatives(x, input_function(t))
                        self._sampled_modulation = self._captured_modulation
                        self._capture_modulation = False
                if step % output_stride == 0:
                    times.append(t)
                    states.append(x.copy())
                    freezes.append(self._sampled_freeze)
                if step == count:
                    break
                k1 = self.derivatives(x, input_function(t))
                k2 = self.derivatives(x + timestep * k1 / 2, input_function(t + timestep / 2))
                k3 = self.derivatives(x + timestep * k2 / 2, input_function(t + timestep / 2))
                k4 = self.derivatives(x + timestep * k3, input_function(t + timestep))
                x += timestep * (k1 + 2 * k2 + 2 * k3 + k4) / 6
                if not np.all(np.isfinite(x)):
                    raise RuntimeError(f"Nonfinite transient state at t={t}")
        finally:
            for attribute in ["_sampled_freeze", "_sampled_modulation", "_capture_modulation", "_captured_modulation"]:
                if hasattr(self, attribute):
                    delattr(self, attribute)
        return np.array(times), np.array(states), np.array(freezes)

    def compute_y_sweep(self, freqs: list[float] | np.ndarray) -> np.ndarray:
        if self.A is None:
            self.compute_abcd()
        Y_list = []
        for f in freqs:
            Y_list.append(self.compute_y(f))
        return np.array(Y_list)

    def export_harmony_csv(self, freqs: np.ndarray, out_path: Path):
        Y_sweep = self.compute_y_sweep(freqs)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        with out_path.open("w", encoding="utf-8") as f:
            for idx, freq in enumerate(freqs):
                Y = Y_sweep[idx]
                entries = []
                for r in range(3):
                    for c in range(3):
                        val = Y[r, c]
                        entries.append(f"{val.real:+.16e}+1i*({val.imag:+.16e})")
                f.write(f"{freq:.16e}, " + ", ".join(entries) + "\n")


if __name__ == "__main__":
    mmc = HarmonyMmcGfl()
    x_eq = mmc.solve_equilibrium()
    print("=== Harmony MMC GFL Steady-State Equilibrium ===")
    print(f"Id = {x_eq[12]:.6f} A (generator current convention)")
    print(f"Iq = {x_eq[13]:.6f} A (Target: ~0.0 A)")
    print(f"iSigma_z = {x_eq[14]:.6f} A")
    print(f"vCSigma_z = {x_eq[23]:.2f} V")
    print(f"theta_pll = {x_eq[0]:.6f} rad")

    A, B, C, D = mmc.compute_abcd()
    print(f"\nA matrix: {A.shape}, B matrix: {B.shape}")
    print("Eigenvalues max real part:", np.max(np.real(np.linalg.eigvals(A))))

