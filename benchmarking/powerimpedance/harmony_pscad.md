# PowerImpedance, Harmony, and PSCAD

PowerImpedance.jl tag `v0.4.0` against Harmony. The tables below are that rerun. The device list is unchanged: the same models are present, and the same ones are absent.

Equations of the models Harmony cannot run are in
[missing_models.pdf](missing_models.pdf).

## Park

The network kernel, the MMC, and the two-level model use the Park matrix in the design report. Peak amplitude is unchanged:

$$
v_d = V_m\cos\theta, \qquad v_q = +V_m\sin\theta
$$

The operating-point line in the MMC section now has that positive sine. The frequency-domain kernel (`makeParkKernels`) puts $+j$ on the q row, which is the Fourier form of that matrix. A passive series inductor, the MMC reactor, and the two-level reactor are

$$
Z_{dq}=-\omega L, \qquad Z_{qd}=+\omega L.
$$

Circulating current, capacitor speed terms, current-controller feedforward, and the grid-forming virtual inductance follow that reactor. Reactive power is $Q=\frac{3}{2}(v_q i_d-v_d i_q)$. The AC–DC power flow writes $P_{ac}=-p_s$, $Q_{ac}=-q_s$, $P_{dc}=-p_n$, and the equilibrium currents use the operating-point formulas with this $v_q$.

Harmony uses the PowerImpedance frame: $v_q=-V_m\sin\theta$ and $Z_{dq}=+\omega L$. PowerImpedance's PSCAD abc/dq block is the same pair (`qaxis=1`, q from $+\sin$). Overlays negate both AC current rows because PowerImpedance reports load current. The MATLAB PV, type-3, and type-4 models are unchanged.

The MATLAB PV, type-3, and type-4 plants are unchanged. They were derived to match those scripts. Do not drop their $Y_{dq}$ onto a Park-transformed line without the reflection $S=\mathrm{diag}(1,-1)$.

## Cases Harmony can run

Relative error is $\|Y_H-Y_{PI}\|/\|Y_{PI}\|$ unless noted. These overlays are the PowerImpedance 0.4.0 rerun. MMC rows negate both AC current rows. The 2LC device sweeps later on this page use the same relative Frobenius norm, reported as \(20\log_{10}\) of that number, in dB.

### Already compared

| Case | What it is | Samples | Mean | Max |
|------|------------|---------|------|-----|
| Resistor | 10 Ω shunt | 11 | 0 | 0 |
| Y-Y transformer | series R, L | 41 | $1.11\times 10^{-6}$ | $3.17\times 10^{-6}$ |
| Aerial cable | example geometry | 41 | $8.54\times 10^{-6}$ | $7.51\times 10^{-5}$ |
| Overhead line | example geometry | 41 | $1.33\times 10^{-5}$ | $1.37\times 10^{-4}$ |
| GFL MMC, $P+Q$ | example plant, PLL, OCC, CCSC | 81 | $5.30\times 10^{-4}$ | $1.58\times 10^{-3}$ |
| GFL MMC, $V_{dc}+Q$ | same plant, DC-voltage outer loop | 81 | $4.90\times 10^{-4}$ | $1.39\times 10^{-3}$ |
| c1, $V_{dc}+Q$ | P2P rectifier | 81 | $1.11\times 10^{-5}$ | $5.61\times 10^{-5}$ |
| c2, $P+Q$ | P2P inverter | 81 | $1.59\times 10^{-5}$ | $5.94\times 10^{-5}$ |
| P2P DC cable $Y$ | 100 km, phase 4×4 | 41 | $1.16\times 10^{-5}$ | $7.74\times 10^{-5}$ |
| P2P $Y_n$ at B6 | c2 AC admittance | 41 | $6.83\times 10^{-5}$ | $1.53\times 10^{-4}$ |
| P2P $Z_{eq}$ at B6 | rest of the link | 41 | $4.38\times 10^{-3}$ | $5.45\times 10^{-3}$ |
| P2P $H=Y_n Z_{eq}$ | loop gain | 41 | $4.35\times 10^{-3}$ | $5.45\times 10^{-3}$ |
| P2P $Z_{in}$ | $(Y_n+Y_{eq})^{-1}$ | 41 | $7.28\times 10^{-2}$ | $0.455$ |

$Z_{eq}$, $Z_{in}$, $Y_n$, and $H$ are compared in the same frame, with no q-axis reflection.

Named line and cable layouts, 21 points from 10 Hz to 1 kHz. Harmony matches PowerImpedance on these geometries. Two of them (`vertical` with 3 bundles, `delta` with 3 bundles) share centres that are not the PSCAD 3V and 3D towers. See [lines/geometry.md](lines/geometry.md).

| Layout | Mean | Max |
|--------|------|-----|
| `ohl_flat2`, twin bundle | $2.02\times 10^{-6}$ | $9.22\times 10^{-6}$ |
| `ohl_flat3` | $1.90\times 10^{-6}$ | $8.77\times 10^{-6}$ |
| `ohl_flat3`, no ground wires | $1.85\times 10^{-6}$ | $8.95\times 10^{-6}$ |
| `ohl_flat6` | $1.31\times 10^{-2}$ | $1.55\times 10^{-2}$ |
| `ohl_vert3` | $1.79\times 10^{-6}$ | $8.51\times 10^{-6}$ |
| `ohl_vert6` | $8.16\times 10^{-3}$ | $1.11\times 10^{-2}$ |
| `ohl_delta3` | $1.76\times 10^{-6}$ | $8.62\times 10^{-6}$ |
| `ohl_delta6` | $7.47\times 10^{-3}$ | $1.03\times 10^{-2}$ |
| `ohl_conc3`, `ohl_off3` | $1.89\times 10^{-6}$ | $7.79\times 10^{-6}$ |
| `ohl_conc6` | $1.12\times 10^{-3}$ | $1.74\times 10^{-3}$ |
| `ohl_vert3`, twin bundle | $1.95\times 10^{-6}$ | $7.49\times 10^{-6}$ |
| Aerial and underground cables | $1.18\times 10^{-5}$ | $6.56\times 10^{-5}$ |

### Overlays

Harmony is blue, PowerImpedance is orange. Each figure is magnitude, phase, and the entry-wise relative error of the run that produced the tables above. Where a third trace appears, it is Harmony's block \(A_0\).

#### Standalone

10 Ω shunt.

![Resistor](standalone/results/overlay_resistor.svg)

Y-Y transformer, series R and L.

![Y-Y transformer](standalone/results/overlay_transformer_yy.svg)

Aerial coaxial cable, example layers.

![Aerial cable](standalone/results/overlay_cable.svg)

Two-bundle flat overhead line, example geometry.

![Overhead line](standalone/results/overlay_ohl.svg)

Grid-following MMC, \(P+Q\). Example plant, PLL, output-current loop, circulating-current suppression.

![GFL MMC P+Q](standalone/results/overlay_mmc_gfl.svg)

Same plant, DC-voltage outer loop.

![GFL MMC Vdc+Q](standalone/results/overlay_mmc_gfl_vdc.svg)

Point-to-point rectifier c1, \(V_{dc}+Q\).

![c1 Vdc+Q](standalone/results/overlay_mmc_c1.svg)

Point-to-point inverter c2, \(P+Q\).

![c2 P+Q](standalone/results/overlay_mmc_c2.svg)

#### Line and cable layouts

`ohl_flat2`, twin bundle.

![ohl_flat2](lines/results/overlay_ohl_flat2.svg)

`ohl_flat3`.

![ohl_flat3](lines/results/overlay_ohl_flat3.svg)

`ohl_flat3`, no ground wires.

![ohl_flat3 no ground wires](lines/results/overlay_ohl_flat3_nogw.svg)

`ohl_flat6`.

![ohl_flat6](lines/results/overlay_ohl_flat6.svg)

`ohl_vert3`.

![ohl_vert3](lines/results/overlay_ohl_vert3.svg)

`ohl_vert3`, twin bundle.

![ohl_vert3 twin](lines/results/overlay_ohl_vert3_twin.svg)

`ohl_vert6`.

![ohl_vert6](lines/results/overlay_ohl_vert6.svg)

`ohl_delta3`.

![ohl_delta3](lines/results/overlay_ohl_delta3.svg)

`ohl_delta6`.

![ohl_delta6](lines/results/overlay_ohl_delta6.svg)

`ohl_conc3`.

![ohl_conc3](lines/results/overlay_ohl_conc3.svg)

`ohl_off3`.

![ohl_off3](lines/results/overlay_ohl_off3.svg)

`ohl_conc6`.

![ohl_conc6](lines/results/overlay_ohl_conc6.svg)

Aerial cable.

![Aerial cable layout](lines/results/overlay_cable_aerial.svg)

Aerial bipolar cable.

![Aerial bipolar cable](lines/results/overlay_cable_aerial_bipolar.svg)

Underground single cable.

![Underground single cable](lines/results/overlay_cable_ug_single.svg)

Underground staggered cable.

![Underground staggered cable](lines/results/overlay_cable_ug_staggered.svg)

Underground bipolar cable.

![Underground bipolar cable](lines/results/overlay_cable_ug_bipolar.svg)

#### Point-to-point link

DC cable admittance, 100 km, phase \(4\times 4\).

![P2P DC cable](p2p/results/overlay_p2p_cable.svg)

\(Y_n\) at B6, c2 AC admittance.

![P2P Yn](p2p/results/overlay_p2p_yn.svg)

\(Z_{eq}\) at B6, the rest of the link.

![P2P Zeq](p2p/results/overlay_p2p_zeq.svg)

\(H=Y_n Z_{eq}\) at B6.

![P2P H](p2p/results/overlay_p2p_tf.svg)

\(Z_{in}=(Y_n+Y_{eq})^{-1}\) at B6. The spike is the bin where the two admittances nearly cancel.

![P2P Zin](p2p/results/overlay_p2p_zin.svg)

#### MATLAB plants

These three stay on the MATLAB frame. Harmony is blue, MATLAB is orange. Mean relative Frobenius error is about \(10^{-6}\).

PV plant.

![MATLAB PV](../matlab_pv/results/overlay_Y.svg)

Type-3 DFIG.

![MATLAB WT3](../matlab_wt3/results/overlay_Y.svg)

Type-4 wind turbine.

![MATLAB WT4](../matlab_wt4/results/overlay_Y.svg)

### Examples that use only models Harmony has

These sweeps use PowerImpedance 0.4 `NetworkBuilder` and the current Harmony build, on one shared log grid. Harmony is blue and PowerImpedance is orange. The error is the relative Frobenius norm of the matrix that the example plots. This pass repeated them; the numbers are unchanged.

| Example quantity | Samples | Mean Frob | Max |
|------------------|---------|-----------|-----|
| 200 km AC overhead line, phase sending \(Z\) | 61 | \(2.03\times 10^{-4}\) | \(2.59\times 10^{-3}\) |
| Same line, \(dq\) sending \(Z\) | 61 | \(8.87\times 10^{-3}\) | \(6.05\times 10^{-2}\) |
| 227 km DC overhead line, differential sending \(Z\) | 61 | \(1.96\times 10^{-4}\) | \(2.06\times 10^{-3}\) |
| 60 km three-core cable, 1 mΩ far end | 61 | \(3.73\times 10^{-5}\) | \(2.21\times 10^{-4}\) |
| IEEE 9, Bus 2 \(dq\) \(Z\), no winding capacitance | 41 | \(3.25\times 10^{-3}\) | \(1.83\times 10^{-2}\) |
| Same Harmony result against the example with 7 nF / 12 nF | 41 | \(3.46\times 10^{-2}\) | 0.174 |
| IEEE 39, Bus 9 \(dq\) \(Z\), STATCOM removed | 31 | \(5.04\times 10^{-3}\) | \(2.06\times 10^{-2}\) |

The phase and DC lines match at a few \(10^{-4}\). The \(dq\) line in that table was inverted from the previous positive-sine Park map.

IEEE 9 without the winding capacitances matches at a few \(10^{-3}\). Putting the example's 7 nF and 12 nF on the PowerImpedance side only raises the error to a few \(10^{-2}\), which is the transformer capacitance Harmony does not have. IEEE 39 is the passive grid at Bus 9. That network impedance leaves the STATCOM out on both sides, and it does not depend on the STATCOM operating point. The STATCOM device admittance is in the 2LC section.

The 60 km cable now matches at \(3.7\times 10^{-5}\). Both tools Kron-reduce the sheath and the armour onto the three cores and terminate the far end with 1 mΩ. The first peak sits near 630 Hz on both sides, which is the peak the example text places near 650 Hz. Before the correction the same sweep had mean error 0.52 and a Harmony peak near 400 Hz. Two mistakes caused that. The semiconducting-screen correction ran inside the conductor loop, so the three layers cubed it. The earth-return logarithm used \(\gamma_E \approx 0.577\) where the formula uses \(e^{\gamma_E} \approx 1.781\). A third term, an insulation permeability factor with \(N=1.4\), is not part of this screen model. The screen thicknesses only rescale \(\varepsilon_r\) by \(\ln(r_o/r_i)/\ln(b/a)\).

`P2P_HVDC_stability.jl` is the point-to-point link already plotted above (DC cable, \(Y_n\), \(Z_{eq}\), \(H\), \(Z_{in}\)). `P2P_HVDC_ALT.jl` is that corridor with a varying underground share and a Padé delay of order 5. Harmony builds the delay at orders 2 and 3. `MMC_harmonic_stability.jl` and the MMC stations in `Energy_hub_MTDC.jl` use the same grid-following MMCs as the standalone \(c_1\) and \(c_2\) overlays, on lines and cables of the kind measured here. Their assembled network impedance was not recomputed in this sweep. The offshore two-level device in that hub, 700 MW, is the 2LC section. The hub impedance in that script is still one number at the central DC node. Each MMC has a single DC port, so the hub does not test independent positive and negative poles.

`Connection_DSL.jl` and `Network_Diagram.jl` are wiring and plotting. They are not admittance cases.

200 km overhead line, phase sending impedance. Far end grounded.

![200 km AC line, phase](examples/results/overlay_ac_ohl_phase.svg)

Same line in \(dq\).

![200 km AC line, dq](examples/results/overlay_ac_ohl_dq.svg)

227 km DC overhead line, differential sending impedance.

![227 km DC line](examples/results/overlay_dc_ohl.svg)

60 km cable with semiconducting screens and a 1 mΩ far-end termination.

![60 km cable](examples/results/overlay_cable.svg)

IEEE 9 Bus 2, both sides without the 7 nF and 12 nF winding capacitances.

![IEEE 9 Bus 2](examples/results/overlay_ieee9_bus2.svg)

The same Harmony network against the example as written, with those capacitances on the PowerImpedance transformers.

![IEEE 9 Bus 2 with winding capacitance](examples/results/overlay_ieee9_bus2_with_capacitance.svg)

IEEE 39 Bus 9. Lines, series loads, transformers, and the generator source impedances. No STATCOM.

![IEEE 39 Bus 9](examples/results/overlay_ieee39_bus9.svg)

### 2LC

`TwoLC` is the two-level converter. The JSON type is `2lc`. The plant is the MMC series reactor. The controls are the same PLL, outer \(P\) or \(V_{dc}\), outer \(Q\), and output-current loop. Modulation is \(m=2v_M/V_{dc}\). A nonzero delay is a Padé of order 3, then a rotation by \(\omega_0\tau\).

Five standalone device admittances use that stack. Both sides omit the Butterworth measurement filters and the voltage or frequency support lags, so the comparison is the shared loops. These sweeps are separate from `adm_TLC_test` against the archived PSCAD files, and separate from the assembled IEEE 39, energy-hub, and weak-grid networks. Harmony has no network stamp for 2LC, so each device is linearized on its own. The grid is 31 log points from 1 Hz to 1 kHz. The comparison negates both AC current rows.

Magnitude in the figures is \(20\log_{10}|Y|\) in dB, and phase is in degrees. The table is \(20\log_{10}\) of the relative Frobenius norm, also in dB. A more negative number is a closer match. The maximum is the worst frequency bin.

| Case | What it is | Samples | Mean | Max |
|------|------------|---------|------|-----|
| 2LC, \(P+Q\) | 100 MVA, 220 kV / 640 kV, 70 MW and 20 Mvar into the AC port, 200 µs delay | 31 | \(-54.63\,\mathrm{dB}\) | \(-46.22\,\mathrm{dB}\) |
| 2LC, \(V_{dc}+Q\) | Same plant, DC-voltage outer loop, 70 MW out of the DC port | 31 | \(-64.18\,\mathrm{dB}\) | \(-60.50\,\mathrm{dB}\) |
| 2LC STATCOM | IEEE 39 ratings, 269 MVA, 345 kV / 110 kV, \(V_{dc}\) and \(Q=-134.5\) Mvar, 200 µs delay | 31 | \(-65.69\,\mathrm{dB}\) | \(-59.84\,\mathrm{dB}\) |
| 2LC energy-hub wind | 1000 MVA, 220 kV / 640 kV, 700 MW and 150 Mvar into the AC grid, no delay | 31 | \(-49.21\,\mathrm{dB}\) | \(-44.75\,\mathrm{dB}\) |
| 2LC weak-grid wind | 600 MVA, 380 kV / 640 kV, 600 MW into the AC grid, \(Q=0\), no delay. One admittance covers WF1, WF2, and WF3 | 31 | \(-57.54\,\mathrm{dB}\) | \(-44.64\,\mathrm{dB}\) |

These two-level errors are the design-report reactor against PowerImpedance's \(Z_{dq}=+\omega L\).

\(P+Q\), 100 MVA rectifier.

![2LC P+Q](twolevel/results/overlay_p_q.svg)

\(V_{dc}+Q\), same rectifier.

![2LC Vdc+Q](twolevel/results/overlay_vdc_q.svg)

IEEE 39 STATCOM device. The QV support lag and the measurement filters are omitted on both sides.

![2LC STATCOM](twolevel/results/overlay_statcom.svg)

Energy-hub offshore wind, 700 MW. Measurement filters omitted on both sides.

![2LC energy-hub wind](twolevel/results/overlay_owf.svg)

Weak-grid wind farm, 600 MW. Measurement filters and the PLL filter omitted on both sides.

![2LC weak-grid wind](twolevel/results/overlay_wind.svg)

## Cases Harmony cannot run

PowerImpedance against its own PSCAD / Z-tool files. The 2LC comparison above leaves out the filters and support lags in the two-level files. The other rows have no Harmony component.

| Case | Samples | Mean Frob | Max | Missing piece |
|------|---------|-----------|-----|----------------|
| GFM MMC, total energy, uncompensated | 58 | $5.69\times 10^{-4}$ | $4.28\times 10^{-3}$ | Virtual swing equation and current-controlled virtual impedance |
| GFM MMC, leg energy, compensated modulation | 23 | $5.31\times 10^{-4}$ | $1.16\times 10^{-3}$ | Leg-energy notch filters and compensated insertion. The 0.4 file is the AC $2\times 2$ block |
| Two-level converter, $V_{dc}$ and QV lag | 28 | $1.51\times 10^{-3}$ | $2.50\times 10^{-3}$ | Voltage-support lag and measurement filters |
| Two-level converter, $P+Q$ | 36 | $1.60\times 10^{-3}$ | $3.02\times 10^{-3}$ | Frequency-support lag and measurement filters |
| Synchronous machine | 240 | $3.39\times 10^{-3}$ | 0.203 | Field, dampers, AVR, governor |
| Induction machine | 80 | $9.44\times 10^{-4}$ | $4.88\times 10^{-3}$ | Inverse-Γ induction machine |

The synchronous-machine maximum is one frequency bin. The package test still passes at `rtol = 9\times 10^{-3}` on the whole sweep.

| Example or test | Why it stops |
|-----------------|--------------|
| `GFM_weak_grid.jl` | Virtual swing equation and current-controlled virtual impedance on the GFM MMC, and the assembled Bus 7 impedance. The two-level wind-farm device is the 2LC section |
| `Getting_started_SMIB.jl` | Synchronous machine with AVR and governor |
| `Converter_machine_interaction.jl` | The machine. The MMC stations alone are the grid-following model Harmony has |
| `IEEE39bus.jl` STATCOM in the network | Assembled network. The device admittance, without the QV lag and the measurement filters, is the 2LC section |
| `adm_SM_test`, `adm_IM_test` | Synchronous machine and induction machine |
| `Energy_hub_MTDC.jl` as a system | Assembled MTDC. The offshore two-level device, 700 MW, is the 2LC section |
| `adm_TLC_test` | Measurement filters and the voltage or frequency support lag. The shared loops are the 2LC section above |
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

Integrating IMTB into Harmony means reading IMTB's CSV (or xlsx) admittance and comparing it with Harmony's $Y(f)$, the same way `controls/run_pi_vs_pscad.jl` already compares PowerImpedance with Z-tool text files. It does not mean embedding PSCAD. Harmony stays the analytic state-space model. IMTB stays the EMT measurement. That comparison is worth doing for every device both sides can represent: the grid-following MMC, the 2LC, lines, and cables. It does not create the machines.

The one archived PSCAD scan already plotted in this repository is the Python averaged GFL MMC against that scan, 160 points from 1 Hz to 1 kHz. Mean relative Frobenius error is 3.6%, maximum 8.0% at 323 Hz. That figure checks the Python plant in `benchmarking/pscad_mmc`. It is separate from the C++ MMC overlays above.

![Python GFL MMC versus the archived PSCAD scan](../pscad_mmc/results/overlay_Y.svg)

MATLAB and Simulink are a separate path. Harmony already overlays PV, type-3, and type-4 admittances on MATLAB scripts (`benchmarking/matlab_pv`, `matlab_wt3`, `matlab_wt4`), at about $10^{-6}$ relative Frobenius error. Those three figures are in the overlays section above. Simulink, or Simulink with an EMT export, is useful when the controller under test lives there and not in Harmony. It does not replace IMTB, and it does not need the PSCAD automation layer. Call MATLAB from the existing Python benchmark runner, the same way Julia is called today. Do not add a C++ link into PSCAD to reach Simulink.
