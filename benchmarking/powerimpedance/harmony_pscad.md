# PowerImpedance, Harmony, and PSCAD

PowerImpedance.jl tag `v0.4.0` (examples and component tests) against Harmony.
Admittance numbers below were measured on 0.3.0. The 0.4.0 network API
(`NetworkBuilder`) is not what `run_pi.jl` calls, so those overlays have not
been repeated on 0.4.0. The device list did not change: the same models are
present, and the same ones are absent.

Equations of the models Harmony cannot run are in
[missing_models.pdf](missing_models.pdf).

## Park

PSCAD, and the PowerImpedance checks that reproduce it, use a positive-sine q axis. Peak amplitude is unchanged:

$$
v_d = \frac{2}{3}\sum_p v_p\cos\theta_p, \qquad
v_q = \frac{2}{3}\sum_p v_p\sin\theta_p, \qquad
v_0 = \frac{1}{3}\sum_p v_p
$$

with $\theta_a=\omega t$, $\theta_b=\omega t-2\pi/3$, $\theta_c=\omega t+2\pi/3$.
Reactive power on that frame is $Q = v_d i_q - v_q i_d$ (the $3/2$ sits in the
peak base). A series RL branch is

$$
Z_{dq}=+\omega L, \qquad Z_{qd}=-\omega L.
$$

Harmony uses the same d axis and the same $2/3$ scaling, with the opposite q axis: $v_q=-V_m\sin\theta$. Its reactive power is $Q=\frac{3}{2}(v_q i_d-v_d i_q)$, so $Q>0$ gives $i_q<0$ when $v_q=0$. The frequency-domain Park of a passive network then has $Z_{dq}\approx-\omega L$ and $Z_{qd}\approx+\omega L$.

That is a frame reflection, not a different plant. The overlays already undo it:

- Network impedance: $S=\mathrm{diag}(1,-1)$ on the q voltage and the q current (`qalign` in `p2p/run_pi_networks.jl`).
- MMC admittance: PowerImpedance AC current rows are multiplied by $-1$, because its port power is positive into the device and Harmony's $P>0$ is AC export.

After those two maps, the grid-following MMC cases sit between $4.3\times 10^{-4}$ and $7.2\times 10^{-4}$ mean relative Frobenius error. Passive lines and the Y-Y transformer are the same or tighter. Park is consistent for every case Harmony can run. The source still uses the negative-sine q; the comparison carries the sign, the models do not.

Version 0.4.0 also changes $\varepsilon_0$ to the CODATA value, the cable earth-return constant, and the order of the line $B$ and $D$ blocks. Passive overlays should be repeated after the Julia scripts move to `NetworkBuilder`. That is not a Park change.

## Cases Harmony can run

Relative error is $\|Y_H-Y_{PI}\|/\|Y_{PI}\|$ unless noted. MMC rows use the current-sign flip above. Network $Z$ uses `qalign`.

### Already compared

| Case | What it is | Samples | Mean | Max |
|------|------------|---------|------|-----|
| Resistor | 10 Ω shunt | 11 | 0 | 0 |
| Y-Y transformer | series R, L | 41 | $1.11\times 10^{-6}$ | $3.17\times 10^{-6}$ |
| Aerial cable | example geometry | 41 | $1.64\times 10^{-4}$ | $1.60\times 10^{-3}$ |
| Overhead line | example geometry | 41 | $2.71\times 10^{-4}$ | $2.93\times 10^{-3}$ |
| GFL MMC, $P+Q$ | example plant, PLL, OCC, CCSC | 81 | $7.21\times 10^{-4}$ | $1.43\times 10^{-3}$ |
| GFL MMC, $V_{dc}+Q$ | same plant, DC-voltage outer loop | 81 | $5.18\times 10^{-4}$ | $1.41\times 10^{-3}$ |
| c1, $V_{dc}+Q$ | P2P rectifier | 81 | $4.29\times 10^{-4}$ | $1.10\times 10^{-3}$ |
| c2, $P+Q$ | P2P inverter | 81 | $6.74\times 10^{-4}$ | $1.19\times 10^{-3}$ |
| P2P DC cable $Y$ | 100 km, phase 4×4 | 41 | $2.39\times 10^{-4}$ | $1.65\times 10^{-3}$ |
| P2P $Y_n$ at B6 | c2 AC admittance | 41 | $2.76\times 10^{-4}$ | $3.43\times 10^{-3}$ |
| P2P $Z_{eq}$ at B6 | rest of the link | 41 | $4.59\times 10^{-3}$ | $1.05\times 10^{-2}$ |
| P2P $H=Y_n Z_{eq}$ | loop gain | 41 | $4.63\times 10^{-3}$ | $1.05\times 10^{-2}$ |
| P2P $Z_{in}$ | $(Y_n+Y_{eq})^{-1}$ | 41 | $5.52\times 10^{-2}$ | 0.376 |

The $Z_{in}$ maximum is one bin where the two admittances nearly cancel. $H$ and $Z_{eq}$ stay near $10^{-2}$.

Named line and cable layouts, 21 points from 10 Hz to 1 kHz. Harmony matches PowerImpedance on these geometries. Two of them (`vertical` with 3 bundles, `delta` with 3 bundles) share centres that are not the PSCAD 3V and 3D towers. See [lines/geometry.md](lines/geometry.md).

| Layout | Mean | Max |
|--------|------|-----|
| `ohl_flat2`, twin bundle | $2.19\times 10^{-5}$ | $1.78\times 10^{-4}$ |
| `ohl_flat3` | $3.62\times 10^{-3}$ | $3.80\times 10^{-3}$ |
| `ohl_flat3`, no ground wires | $8.75\times 10^{-3}$ | $1.02\times 10^{-2}$ |
| `ohl_flat6` | $1.61\times 10^{-2}$ | $2.00\times 10^{-2}$ |
| `ohl_vert3` | $2.16\times 10^{-2}$ | $3.81\times 10^{-2}$ |
| `ohl_vert6` | $1.78\times 10^{-2}$ | $2.66\times 10^{-2}$ |
| `ohl_delta3` | $1.34\times 10^{-2}$ | $2.39\times 10^{-2}$ |
| `ohl_delta6` | $1.28\times 10^{-2}$ | $1.99\times 10^{-2}$ |
| `ohl_conc3`, `ohl_off3` | $1.41\times 10^{-2}$ | $2.35\times 10^{-2}$ |
| `ohl_conc6` | $1.18\times 10^{-2}$ | $1.73\times 10^{-2}$ |
| `ohl_vert3`, twin bundle | $3.29\times 10^{-2}$ | $5.79\times 10^{-2}$ |
| Aerial and underground cables | $2.42\times 10^{-4}$ | $1.41\times 10^{-3}$ |

PowerImpedance component tests that exercise the same devices, and that Harmony can rebuild: `Source_test`, `Impedance_test`, `Transformer_test` for Y-Y, `adm_OHL_test`, `adm_DC_cable_test`, the uncompensated GFL part of the MMC tests, and `imp_test` (the point-to-point link). Counts on the 0.3.0 run: Source 2, Impedance 6284, Transformer 3142, OHL 516, DC cable 336, P2P 512.

### Examples that use only models Harmony has

| Example | Runnable piece |
|---------|----------------|
| `Line_and_cable_models.jl` | The overhead-line and cable geometries, including a two-core DC cable as a line |
| `P2P_HVDC_stability.jl` | The full link. Already compared |
| `P2P_HVDC_ALT.jl` | One length. The length sweep is a parameter study, not a new device |
| `MMC_harmonic_stability.jl` | Both MMCs. Same grid-following stack (PLL, $P$ or $V_{dc}$, $Q$, output current, circulating-current suppression, uncompensated modulation), with a 116 µs delay on the inverter |
| `IEEE9_bus_impedance.jl` | Ideal sources, flat overhead lines, series RL loads, and the Y-Y transformers. The 7 nF and 12 nF winding capacitances are not in Harmony's transformer, so the high-frequency peaks of that example will not match. The MMC added at bus 7 is the same grid-following stack |
| `IEEE39bus.jl` | Lines, loads, and the ideal voltage sources that stand in for the generators |
| `Energy_hub_MTDC.jl` | MMC1 and MMC2 ($P+Q$), MMC3 ($V_{dc}+Q$), the two-core DC cables, the overhead lines, and the series grid impedances. Not the offshore wind two-level converter, so not the closed hub |

The energy-hub impedance in that script is one number at the central DC node. Each MMC still has a single DC port. The two cable positions are a symmetric bipolar geometry reduced to that one port. Running the hub does not test independent positive and negative poles.

`Connection_DSL.jl` and `Network_Diagram.jl` are wiring and plotting. They are not admittance cases.

## Cases Harmony cannot run

PowerImpedance against its own PSCAD / Z-tool files. Harmony has no component to put on the same plot.

| Case | Samples | Mean Frob | Max | Missing piece |
|------|---------|-----------|-----|----------------|
| GFM MMC, total energy, uncompensated | 58 | $5.69\times 10^{-4}$ | $4.28\times 10^{-3}$ | Virtual swing equation and current-controlled virtual impedance |
| GFM MMC, leg energy, compensated modulation | 192 | $5.93\times 10^{-3}$ | $1.17\times 10^{-2}$ | Leg-energy notch filters and compensated insertion |
| Two-level converter, $V_{dc}$ and QV lag | 28 | $1.51\times 10^{-3}$ | $2.50\times 10^{-3}$ | Two-level converter |
| Two-level converter, $P+Q$ | 36 | $1.60\times 10^{-3}$ | $3.02\times 10^{-3}$ | Two-level converter |
| Synchronous machine | 240 | $3.43\times 10^{-3}$ | 0.204 | Field, dampers, AVR, governor |
| Induction machine | 80 | $9.44\times 10^{-4}$ | $4.88\times 10^{-3}$ | Inverse-Γ induction machine |

The synchronous-machine maximum is one frequency bin. The package test still passes at `rtol = 9\times 10^{-3}` on the whole sweep.

| Example or test | Why it stops |
|-----------------|--------------|
| `GFM_weak_grid.jl` | Virtual swing equation, virtual impedance, and two-level wind farms |
| `Getting_started_SMIB.jl` | Synchronous machine with AVR and governor |
| `Converter_machine_interaction.jl` | The machine. The MMC stations alone are the grid-following model Harmony has |
| `IEEE39bus.jl` STATCOM | Two-level converter with DC voltage and QV support |
| `Energy_hub_MTDC.jl` as a system | Offshore two-level converter, 700 MW |
| `adm_TLC_test`, `adm_SM_test`, `adm_IM_test` | The three devices above |
| `adm_MMC_test`, `adm_MMC_compensated_test` | Grid-forming control and, in the second file, compensated modulation |
| `blackbox_mmc_test` | Measured frequency-response data as a component |
| `data_MMC_bipolar.txt` | 192 frequencies, ports $d$, $q$, positive, negative. No constructor emits it. PowerImpedance 0.4.0 still has one DC terminal (`DC_CM`). Harmony's MMC admittance is $3\times 3$ |
| `transformer_dy_test` | $\Delta$Y is Dyn11. Harmony's compared transformer is Y-Y |
| `IEEE39bus_Gridspace.jl`, `SmallSignal_Gridspace.jl`, `Gridspace_uncertainty.jl`, `P2P_HVDC_Gridspace.jl` | Parameter sampling on top of the devices above |
| `PSCAD_tutorial.jl` | Builds a project and runs PSCAD and Z-tool |

Harmony's generator is one field winding per phase, $Y_{11}=Z_f/D$ and $Y_{12}=-j X_m/D$, with $R_f$, $T_f$, and $X_d$. It is not the synchronous machine. Harmony's type-3 model is a DFIG with converters. It is not the induction machine. Harmony's grid-forming path is a filtered $P$ and $Q$ droop plus an optional virtual $R$ and $L$, and it cannot be combined with the output-current loop.

## PSCAD, Z-tool, and IMTB

PSCAD does not contain Z-tool. Z-tool ([Electa-Git/Z-tool](https://github.com/Electa-Git/Z-tool), package `ztoolacdc`) is a separate Python toolbox. It drops scan blocks into a PSCAD project, perturbs the voltage, and builds the admittance from the recorded currents. It drives PSCAD through the Automation Library (`mhi.pscad`). PowerImpedance does the same: `src/PSCAD/remote/runner.jl` launches or attaches to PSCAD with `mhi.pscad` and then reads Z-tool result files. The pinned automation package in that runner is the one Z-tool 0.1.52 expects.

The Automation Library is Python. It starts PSCAD, loads a workspace, sets parameters, and runs the project. PSCAD V5 also embeds that Python inside its Scripts pane. There is no supported C++ API for the same control. A C++ program can call the Python library, or talk to a PSCAD instance that a Python script already started. It cannot link against a Manitoba C++ automation SDK. The older Fortran EMTDC interface is for writing component models, not for driving the impedance scan.

IMTB ([Energinet-SimTools/IMTB](https://github.com/Energinet-SimTools/IMTB)) is the same kind of tool: a PSCAD library plus Python that injects a perturbation at a point of connection and writes immittance tables. It is the scanner this project uses. It is not inside PSCAD either, and it is not Z-tool. Both are perturbation scanners. Z-tool is what PowerImpedance's shipped PSCAD files were produced with. IMTB is the one to keep for Harmony checks against an EMT model.

Integrating IMTB into Harmony means reading IMTB's CSV (or xlsx) admittance and comparing it with Harmony's $Y(f)$, the same way `controls/run_pi_vs_pscad.jl` already compares PowerImpedance with Z-tool text files. It does not mean embedding PSCAD. Harmony stays the analytic state-space model. IMTB stays the EMT measurement. That comparison is worth doing for every device both sides can represent: the grid-following MMC, lines, and cables. It does not create the missing two-level converter or the machines.

MATLAB and Simulink are a separate path. Harmony already overlays PV, type-3, and type-4 admittances on MATLAB scripts (`benchmarking/matlab_pv`, `matlab_wt3`, `matlab_wt4`), at about $10^{-6}$ relative Frobenius error. Simulink, or Simulink with an EMT export, is useful when the controller under test lives there and not in Harmony. It does not replace IMTB, and it does not need the PSCAD automation layer. Call MATLAB from the existing Python benchmark runner, the same way Julia is called today. Do not add a C++ link into PSCAD to reach Simulink.
