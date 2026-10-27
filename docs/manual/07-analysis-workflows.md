# Chapter 7 — Analysis Workflows

[← Component reference](06-component-reference.md) | [Manual index](README.md) | [Docs map](../README.md) | [Next: Certificates →](08-certificates.md)

---

## 7.1 Overview

After building a `Network`, you invoke a **solver** appropriate to the study type. This chapter describes the main workflows and points to example code.

**How to run:** [`../running-harmony.md`](../running-harmony.md). Use **HarmonyUI** for interactive runs with optional embedded plots ([Chapter 12](12-harmony-ui.md)), or the **`Harmony` CLI** for scripts and CI:

```bash
HarmonyUI                             # graphical launcher (build target HarmonyUI)
Harmony --cpp <example_name>          # C++ bundled study
Harmony --json path/to/case.json      # JSON-defined study
Harmony --json case.json --no-plot    # headless / CI
```

In **HarmonyUI**, enable **Plot** on the Launcher tab to see Bode/Nyquist, OPF network, and DQsym charts under **Plots**. JSON plot flags apply only when **Plot** is checked.

---

## 7.2 Y-parameter / frequency sweep

**Goal:** Obtain admittance vs frequency for one element or the whole network interface.

```cpp
element->writeFile(1.0, 1e4, 1000);           // 1 Hz – 10 kHz, 1000 points
element->plotYParameters(1.0, 1e4, 1000);     // interactive Bode plot
```

Output: `./files/<element_id>.csv`

**Examples:** `Harmony --cpp mmc`, `Harmony --cpp transformer`, `Harmony --cpp cable`, `Harmony --cpp ohl`

**JSON:** `"type": "y_matrix"` in computations block.

---

## 7.3 State-space analysis

**Goal:** Linearized A, B, C, D matrices for eigenvalue and input–output analysis.

```cpp
StateSpaceModel model;
model.formState(&network, { outputBus1, outputBus2 });

Eigen::MatrixXd A = model.getA();
Eigen::MatrixXd B = model.getB();
// …
```

Use `SSMMode::DQsym` when preparing matrices for dynamic phasor coupling.

**Examples:** `Harmony --cpp state_space`

**Tests:** `tests/teststatespacemodel.cpp`, `tests/testABCDforming.cpp`

---

## 7.4 DQsym — dynamic phasor time domain

**Goal:** Simulate electromagnetic transients in the dynamic phasor domain with optional breaker switching.

```cpp
DQsym solver;
solver.initialize(&network);

Config cfg;
cfg.dt = 2e-5;
cfg.t_start = 0.0;
cfg.t_end = 0.1;
cfg.f = 50.0;
cfg.outputBuses = { bus1, bus2 };

DQsymResult result = solver.run(cfg);
solver.exportCSV("output.csv");
solver.plot();

// Linearize MMCs at a stored instant (no Newton): packed x,u → computeABCD
solver.analyzeAtTime(cfg.t_end);                    // plots + CSV
solver.analyzeAtTime(0.05, /*plotResults=*/false);  // headless: eigenvalues + TF CSV
```

On the abc plot, **Ctrl+click** a time (or type `t*`) and click **Analyze at t\*** to linearize at that sample. **Continue from t\*** resumes the same DQsym run with the original setpoints. Linearization uses the frozen snapshot (`setEquilibriumState`); it does not re-solve Newton equilibrium.

**Examples:** `Harmony --cpp dqsym_rlc`, `Harmony --cpp dqsym_simple_mmc`, `Harmony --cpp dqsym_mmc_controlled`, `Harmony --cpp dqsym_dsss2`, `Harmony --cpp dqsym_math_operations`

**JSON:** `"type": "dqsym"` or `"time_domain"` in the `computations` block (see [Chapter 5](05-json-input.md)). Optional `"snapshot_time"` (seconds) linearizes every MMC at the nearest stored sample after the run — with `"plot": true` this opens eigenvalue, participation, and Bode/Nyquist tabs; with `--no-plot` it still prints stability/eigenvalues and writes transfer-function CSV. Optional `"frequency_range"` (or the simulation-level range) sets the Bode/Nyquist/CSV sweep.

---

## 7.5 Optimal power flow (OPF)

**Goal:** Compute AC–DC dispatch, voltages, and converter setpoints for a given case.

### CSV case files

Harmony's CSV workflow follows the same preparation convention as
[CRESYM/ACDC_OPF `Prepare_Data`](https://github.com/CRESYM/ACDC_OPF/tree/main/Prepare_Data):
start from one or more MATPOWER case structures, normalize their bus numbering,
append an AC-grid identifier when cases are combined, and write each numeric
matrix to a separate headerless CSV file. The reference implementation is useful
when preparing a new Harmony case:

- [`mpc_sorted.m`](https://github.com/CRESYM/ACDC_OPF/blob/main/Prepare_Data/mpc_sorted.m)
  renumbers the buses of one MATPOWER case and updates the branch and generator
  bus references.
- [`mpc_merged.m`](https://github.com/CRESYM/ACDC_OPF/blob/main/Prepare_Data/mpc_merged.m)
  combines multiple AC cases that use the same `baseMVA` and appends a `grid`
  column to the AC matrices.
- [`save_csv.m`](https://github.com/CRESYM/ACDC_OPF/blob/main/Prepare_Data/save_csv.m)
  writes `baseMVA`, `bus`, `branch`, `gen`, and `gencost` to separate CSV files.

Harmony ships prepared cases under `src/data/`. All files belonging to one AC
or DC case use a common prefix:

```
<case>_bus_ac.csv
<case>_branch_ac.csv
<case>_gen_ac.csv
<case>_gencost_ac.csv
<case>_res_ac.csv
<case>_baseMVA_ac.csv

<case>_bus_dc.csv
<case>_branch_dc.csv
<case>_conv_dc.csv
<case>_baseMW_dc.csv
<case>_pol_dc.csv
```

#### Bundled cases

| Prefix | Contents | Origin |
|---|---|---|
| `ac5` | 5 AC buses | PJM/Stagg 5-bus |
| `SLO9` | 9 AC buses, 400 kV | Slovenian 9-bus |
| `ieee9` | 9 AC buses, 345 kV | MATPOWER `case9` (WSCC 3-machine) |
| `ieee39` | 39 AC buses | MATPOWER `case39` (New England) |
| `ac9ac14`, `ac14ac57`, `ac57ac118` | merged multi-grid AC cases | MATPOWER cases merged pairwise |
| `mtdc3`, `mtdc3slack_a` | 3 DC buses, 3 converters | MatACDC 3-terminal MTDC |
| `hvdc2ptp` | 4 DC buses, 2 point-to-point links, 4 converters | Built for Harmony; pair with the `ieee9` AC case |
| `ieee39hvdc` | 39 AC buses + 10 DC buses, 12 DC branches | pglib-opf-hvdc `case39_10_he` |
| `rts24hvdc` | 3 AC grids (50 buses) joined only by DC + 7 DC buses | pglib-opf-hvdc `case24_7_jb` |

#### Importing a MATPOWER or MatACDC case

`tools/matpower_to_harmony.py` converts `.m` case files into this CSV layout,
including MatACDC's `dcbus` / `dcconv` / `dcbranch` matrices:

```bash
python tools/matpower_to_harmony.py convert case39.m --prefix ieee39 --ac-only
python tools/matpower_to_harmony.py convert case39_10_he.m --prefix ieee39hvdc
python tools/matpower_to_harmony.py convert case24_7_jb.m --prefix rts24hvdc --grid-from-island
python tools/matpower_to_harmony.py extract-grid ac9ac14 --grid 1 --prefix ieee9
```

Use `--grid-from-island` when a case holds several electrically separate AC
systems that are tied together only by HVDC; each island becomes its own Harmony
AC grid. The converter renumbers buses to `1..N` per grid, enforces one converter
per DC bus, and requires a slack bus in every AC grid.

Source cases are available from
[MATPOWER](https://github.com/MATPOWER/matpower/tree/master/data),
[MatACDC](https://www.esat.kuleuven.be/electa/teaching/matacdc), and
[pglib-opf-hvdc](https://github.com/power-grid-lib/pglib-opf-hvdc).

#### AC file 

The AC matrices are the standard MATPOWER matrices with one final `grid` column.
The `grid` value is `1` for a single AC grid and `1, 2, ...` for merged grids.

`<case>_baseMVA_ac.csv` contains one positive scalar: the common AC power base
in MVA.

`<case>_bus_ac.csv` has 14 columns:

| # | Field | Meaning / unit |
|---:|---|---|
| 1 | `BUS_I` | Local bus number |
| 2 | `BUS_TYPE` | 1 PQ, 2 PV, 3 reference/swing, 4 isolated |
| 3–4 | `PD`, `QD` | Demand [MW, Mvar] |
| 5–6 | `GS`, `BS` | Shunt conductance/susceptance at 1 pu [MW, Mvar] |
| 7 | `BUS_AREA` | MATPOWER area number |
| 8–9 | `VM`, `VA` | Initial voltage magnitude [pu] and angle [degree] |
| 10 | `BASE_KV` | Nominal line-to-line voltage [kV] |
| 11 | `ZONE` | Loss-zone number |
| 12–13 | `VMAX`, `VMIN` | Voltage limits [pu] |
| 14 | `grid` | Harmony AC-grid identifier |

`<case>_branch_ac.csv` has 14 columns:

| # | Field | Meaning / unit |
|---:|---|---|
| 1–2 | `F_BUS`, `T_BUS` | From/to local bus numbers |
| 3–5 | `BR_R`, `BR_X`, `BR_B` | Series R/X and total charging B [pu] |
| 6–8 | `RATE_A`, `RATE_B`, `RATE_C` | Long/short/emergency ratings [MVA]; zero means unrestricted |
| 9 | `TAP` | Off-nominal tap ratio; zero means 1.0 |
| 10 | `SHIFT` | Transformer phase shift [degree] |
| 11 | `BR_STATUS` | 1 in service, 0 out of service |
| 12–13 | `ANGMIN`, `ANGMAX` | Branch angle-difference limits [degree] |
| 14 | `grid` | Harmony AC-grid identifier |

`<case>_gen_ac.csv` has the 21 MATPOWER generator columns followed by `grid`
(22 columns total):

```text
GEN_BUS, PG, QG, QMAX, QMIN, VG, MBASE, GEN_STATUS, PMAX, PMIN,
PC1, PC2, QC1MIN, QC1MAX, QC2MIN, QC2MAX,
RAMP_AGC, RAMP_10, RAMP_30, RAMP_Q, APF, grid
```

`VG` is the generator voltage setpoint [pu]. `PG/QG` are starting values;
`PMIN/PMAX`, `QMIN/QMAX`, bus voltage limits, and network constraints determine
the feasible dispatch.

`<case>_gencost_ac.csv` normally has eight columns for the polynomial model used
by Harmony:

```text
MODEL, STARTUP, SHUTDOWN, NCOST, C2, C1, C0, grid
```

Use `MODEL=2` and `NCOST=3` for the quadratic cost
`C2*PG^2 + C1*PG + C0`. Keep generator-cost rows aligned with generator rows
within each grid.

`<case>_res_ac.csv` uses Harmony's 12-column renewable-source extension:

```text
BUS, PRESMAX, SRESMAX, MODEL, STARTUP, SHUTDOWN, NCOST,
C2, C1, C0, VM, grid
```

`PRESMAX` and `SRESMAX` are in MW/MVA and `VM` is the voltage target [pu]. This
matrix is not part of the basic MATPOWER case, so it must be prepared separately
when renewable generation is modeled.

#### Merging several AC grids

The ACDC_OPF preparation utilities implement the following procedure:

1. Load each MATPOWER case (`case9`, `case14`, etc.).
2. Verify that all cases use the same `baseMVA`; convert them to a common base
   before merging if they do not.
3. For each case, sort its original bus IDs and map them to `1..N`.
4. Apply that mapping consistently to `BUS_I`, `F_BUS`, `T_BUS`, and `GEN_BUS`
   (and to `BUS` in the RES table, if present).
5. Append the grid identifier to every AC row: `1` for the first case, `2` for
   the second, and so on.
6. Concatenate like matrices vertically and write one AC CSV set.

Bus IDs are therefore **local to an AC grid**. The pair `(grid, BUS_I)` is the
unique identifier in a merged case. Do not renumber only the bus table without
updating branches, generators, converters, and RES references.

MATLAB example, matching the reference repository:

```matlab
addpath('Prepare_Data');
addpath('Prepare_Data/Matpower_Cases');

merged_ac = mpc_merged('case9', 'case14');
save_csv(merged_ac, 'path/to/Harmony/src/data');
% Enter a prefix such as ac9ac14 when prompted.
```

After export, create or copy the corresponding `<prefix>_res_ac.csv` because
the reference `save_csv.m` writes the five basic AC files but does not create
the RES extension.

#### DC and converter files

DC data are prepared as a separate case. `<case>_baseMW_dc.csv` contains the DC
power base and `<case>_pol_dc.csv` contains the pole factor. The remaining files
are numeric, headerless matrices:

```text
bus_dc (13):
BUSDC_I, BUS_TYPE, PD, QD, GS, BS, AREA, VM, VA, BASE_KV, ZONE, VMAX, VMIN

branch_dc (13):
F_BUSDC, T_BUSDC, R, X, B, RATE_A, RATE_B, RATE_C,
TAP, SHIFT, STATUS, ANGMIN, ANGMAX

conv_dc (26):
BUSDC_I, BUSAC_I, GRIDAC, TYPE_DC, TYPE_AC, P_G, Q_G, VTAR,
RTF, XTF, BF, RC, XC, BASE_KVAC, VMMAX, VMMIN, IMAX, STATUS,
LOSS_A, LOSS_B, LOSS_CREC, LOSS_CINV, DROOP, PDCSET, VDCSET, DVDCSET
```

`BUSAC_I` is interpreted together with `GRIDAC`; both must match an AC bus in
the selected AC CSV case. Converter control-type codes and signs must be kept
consistent with the ACDC_OPF case from which the data are taken. In particular,
do not independently reverse `P_G`, `PDCSET`, or branch orientations during CSV
conversion.

**Power sign conventions (MatACDC OPF vs MMC dynamics):**

| Quantity | MatACDC / OPF CSV | MMC `Pac`/`Pdc` / Y-matrix |
|---|---|---|
| Inverter | `P_G > 0` ⇒ `ps,pn ≤ 0` (network injections) | `Pac,Pdc > 0` (AC export / DC import) |
| Rectifier | `P_G < 0` ⇒ `ps,pn ≥ 0` | `Pac,Pdc < 0` |

For component-built cases, see [Mapping between OPF and converter models](#mapping-between-opf-and-converter-models) below for the equivalent impedance, loss defaults, control settings, and conversion back to MMC operating quantities.

The OPF pairs converter row `i` with DC bus row `i`. `make_OPF` sorts
converter rows (and `conv_point`) by `BUSDC_I` so this holds even when
elements come from an unordered map. `conv_dc` must still have exactly one
row per DC bus.

`TYPE_DC` selects the DC-side control: `1` constant power (`P_G`), `2` constant
DC voltage (`VTAR`), anything else power-voltage droop (`DROOP`, `PDCSET`,
`VDCSET`). `TYPE_AC` selects `1` constant reactive power (`Q_G`) or AC voltage
control (`VTAR`).

### Run a prepared CSV case

CSV mode calls `solve_opf` directly. A null data pointer tells Harmony to load
the matrices from `src/data/` using the supplied prefixes:

```cpp
PowerFlow pf;
pf.solve_opf(/*dc_name=*/"mtdc3",
             /*ac_name=*/"ac5",
             /*dataOPF=*/nullptr,
             /*vscControl=*/true,
             /*writeTxt=*/false,
             /*plotResult=*/false,
             /*print_info=*/true);
```

For an AC-only case, pass an empty DC prefix:

```cpp
pf.solve_opf("", "SLO9", nullptr, false, false, false, true);
```

Do not combine this pattern with `make_OPF(&network, ...)`: `make_OPF` is the
other input path, which constructs fresh OPF matrices from Harmony `Network`
components and their `OPFInfo`. See `example_OPF_SLO.cpp` for a component-built
counterpart to the CSV-loaded SLO9 case.

**Examples:** `Harmony --cpp opf_ac` (CSV-loaded SLO9),
`Harmony --cpp opf_slo` (the same case built from components),
`Harmony --cpp opf_csv`, `Harmony --cpp opf_csv_1`,
`Harmony --cpp point2point_case`, `Harmony --cpp opf_pv`, and
`Harmony --cpp opf_wt`.

IEEE benchmark cases: `Harmony --cpp opf_ieee9`, `opf_ieee9_hvdc`
(9-bus AC grid plus two point-to-point HVDC links), `opf_ieee39`,
`opf_ieee39_hvdc`, and `opf_rts24_hvdc`. The AC-only solves reproduce the
published MATPOWER optima to within the second-order-cone relaxation gap
($5296.67 against 5296.69 for `case9`, $41854.59 against 41864.18 for `case39`).

**JSON:** `"type": "opf"` with `"case_name"` in `computations`.

**Requirements:** A valid Gurobi license.

### Mapping between OPF and converter models

For a network built from Harmony components, `PowerFlow::make_OPF` constructs
the steady-state OPF representation from the shared `Network` and its element
parameters. After a successful solve, it converts the converter terminal
quantities and updates the associated MMC objects. The MMC models then determine
their internal converter and controller states before small-signal analysis.

This automatic MMC update belongs to the component-built `make_OPF` workflow.
Calling `solve_opf` directly with CSV case files supplies an OPF case; it does
not by itself create or initialize corresponding dynamic MMC objects.

#### Shared parameters and model representations

**Loads.** A series RLC `Load` retains its impedance characteristics in the
frequency-domain model. `Load::computePowerFlow()` uses the same parameters to
calculate fixed three-phase active and reactive demands at the AC base voltage
and nominal angular frequency. For a balanced load,

$$
P_{\mathrm{load}}=
\frac{3V_{\mathrm{ph},b}^{2}R}{R^{2}+X^{2}},
\qquad
Q_{\mathrm{load}}=
\frac{3V_{\mathrm{ph},b}^{2}X}{R^{2}+X^{2}},
\qquad
V_{\mathrm{ph},b}=\frac{V_{b,\mathrm{LL,rms}}}{\sqrt{3}}.
$$

Here, $V_{b,\mathrm{LL,rms}}$ is `ACbaseKV * 1e3` in volts, and
$X=\omega_0L-1/(\omega_0C)$ when the corresponding elements are present.
In the implementation, `C == 0` omits the capacitive term and `L == 0` omits
the inductive term. The resulting W/var demands are divided by $10^6$ for
storage as MW/Mvar in the OPF data. These representations match the load powers
at the reference voltage and frequency; the fixed OPF demand does not reproduce
the RLC load's voltage and frequency dependence away from those conditions.

For a directly specified `LoadPQ`, `LoadPQ::computePowerFlow()` stores the given
P and Q after converting W/var to MW/Mvar. See
[Sources and loads](06-component-reference.md#62-sources-and-loads) for the
component constructors.

**Converter impedance and losses.** When the OPF converter row is generated
from an MMC object, `MMC::computePowerFlow()` uses

$$
R_{\mathrm{eq}}=\frac{R_{\mathrm{arm}}}{2}+R_{\mathrm{reactor}},
\qquad
L_{\mathrm{eq}}=\frac{L_{\mathrm{arm}}}{2}+L_{\mathrm{reactor}},
$$

$$
r_{\mathrm{eq}}=\frac{R_{\mathrm{eq}}}{Z_{b,\mathrm{ac}}},
\qquad
x_{\mathrm{eq}}=\frac{\omega_0L_{\mathrm{eq}}}{Z_{b,\mathrm{ac}}}.
$$

The OPF fields `rc` and `xc` are therefore per-unit quantities on `ACZbase`.
This mapping retains the equivalent series impedance and its resistive losses.
Unless explicitly supplied through `setOPFInfo`, `bf`, `rtf`, `xtf`, `LossA`,
`LossB`, `LossCrec`, and `LossCinv` are set to zero. This avoids adding a filter,
transformer, or empirical loss term without a corresponding specification in
the selected MMC model. Explicit `rc` and `xc` settings also override the
equivalent-impedance defaults.

CSV converter cases are a separate input path and may specify additional
interface elements and empirical internal losses of the form
$p_{\mathrm{loss}}=a+bI+cI^2$, where $I$ is the converter current magnitude and
$a$, $b$, and $c$ are the constant, linear, and quadratic loss coefficients.
Those data must be matched to the intended dynamic converter model when its
initialization is required; they are not automatically equivalent to the
defaults derived from an MMC object.

#### Control modes and reference updates

The MMC retains its selected controllers and their parameters.
`MMC::computePowerFlow()` infers the OPF control types from the enabled
controllers unless explicit `type_dc` or `type_ac` settings are supplied:

| Enabled controller | Inferred OPF setting | Steady-state control |
|---|---|---|
| `active_power` | `type_dc = 1` | Scheduled DC-side active power |
| `dc_voltage` | `type_dc = 2` | DC voltage |
| `droop` | `type_dc = 3` | DC power-voltage droop |
| `ac_voltage` | `type_ac = 2` | AC voltage magnitude |
| `reactive_power` | `type_ac = 1` | Reactive power |

For type inference, the DC-side check order is `active_power`, `dc_voltage`,
then `droop`; the AC-side order is `ac_voltage`, then `reactive_power`.
Explicit OPF type settings take priority. When `vscControl` is enabled, the OPF
adds the corresponding steady-state control constraints. When it is disabled,
the OPF can choose new converter setpoints within the remaining constraints.
The type-1 OPF active-power constraint acts on the DC port; it is not a direct
constraint fixing the MMC's AC-side active-power reference.

After OPF, `update_MMC()` updates references from the converted terminal
quantities:

| Enabled controller | Updated reference |
|---|---|
| `dc_voltage` | $V_{\mathrm{dc}}$; takes priority over `active_power` |
| `active_power` | $P_{\mathrm{ac}}$, when `dc_voltage` is absent |
| `reactive_power` | $Q_{\mathrm{ac}}$; takes priority over `ac_voltage` |
| `ac_voltage` | $(V_d,V_q)$, when `reactive_power` is absent |
| `occ` | $(I_d,I_q)$ calculated from terminal voltage and power |

These reference-update priorities differ from the type-inference order when
both controllers in a pair are enabled. The selected OPF types and controller
configuration should therefore be considered together. Other enabled internal
loops receive the references calculated by `update_MMC()` for the new operating
point. The controller structure and gains are retained.

#### Units, power signs, and reference frames

The OPF uses per-unit quantities, while the MMC operating-point interface uses
V, W, var, and radians. With $`S_b=10^6\,\texttt{baseMW\_dc}`$ as the physical
power base used for the converter/DC OPF quantities, `make_OPF` converts

$$
P_{\mathrm{ac}}=-S_b p_s^{\mathrm{VSC}},
\qquad
Q_{\mathrm{ac}}=-S_b q_s^{\mathrm{VSC}},
\qquad
P_{\mathrm{dc}}=-S_b p_j^{\mathrm{DC}}.
$$

Here, $s$ is the converter PCC and $j$ is its associated DC bus. The minus signs
map the OPF port convention to the MMC machine convention: positive
$P_{\mathrm{ac}}$ denotes export to the AC network and positive
$P_{\mathrm{dc}}$ denotes import from the DC network. The reactive-power sign
follows the dq equations below.

The AC voltage is converted from line-to-line RMS to phase-voltage peak:

$$
V_m=\sqrt{\frac{2}{3}}\,V_{b,\mathrm{LL,rms}}v_s^{\mathrm{VSC}},
\qquad
V_{\mathrm{dc}}=V_{b,\mathrm{dc}}v_j^{\mathrm{DC}}.
$$

The voltage bases are `ACbaseKV * 1e3` and `DCbaseKV * 1e3` in volts.
The implementation takes the square root of the optimized squared-voltage
variables before scaling them. The recovered PCC angle is passed in **radians**.
In `update_MMC()`, the grid-frame voltage components are

$$
V_d=V_m\cos\theta_s^{\mathrm{VSC}},
\qquad
V_q=-V_m\sin\theta_s^{\mathrm{VSC}}.
$$

The power and current conventions satisfy

$$
P_{\mathrm{ac}}=\frac{3}{2}(V_dI_d+V_qI_q),
\qquad
Q_{\mathrm{ac}}=\frac{3}{2}(V_qI_d-V_dI_q),
$$

which give the current references

$$
I_d=\frac{2}{3}\frac{V_dP_{\mathrm{ac}}+V_qQ_{\mathrm{ac}}}{V_d^2+V_q^2},
\qquad
I_q=\frac{2}{3}\frac{V_qP_{\mathrm{ac}}-V_dQ_{\mathrm{ac}}}{V_d^2+V_q^2}.
$$

For PLL-based control, the nonlinear model retains electrical states in the
grid frame and rotates the voltage and current copies used by the controllers
into the converter frame. For the terminal voltage, the implemented rotation is

```math
\begin{bmatrix}
V_d^{\mathrm{ctrl}} \\
V_q^{\mathrm{ctrl}}
\end{bmatrix}
=
\begin{bmatrix}
\cos\theta_c & -\sin\theta_c \\
\sin\theta_c & \cos\theta_c
\end{bmatrix}
\begin{bmatrix}
V_d \\
V_q
\end{bmatrix}.
```

where $\theta_c$ is the controller angle state. This transformation is part of
the nonlinear state-derivative function and is included in its linearization.

#### Equilibrium initialization and linearization

The OPF supplies terminal operating quantities, not the internal MMC and
controller states. After the conversions above, the component-built workflow
uses the following sequence (arguments are physical quantities):

```cpp
mmc->update_MMC(Vm_V, theta_rad, Pac_W, Qac_Var, Vdc_V, Pdc_W);
mmc->solveEquilibrium();
mmc->computeABCD();
```

`solveEquilibrium()` solves the nonlinear steady-state equations for the
converter and enabled controllers. `computeABCD()` obtains the small-signal
matrices at that state. Both use `MMC::computeStateDerivatives()`; the MMC
controller updates during DQsym simulation also call this function. Thus the
controller equations and their frame transformations are shared between these
calculations. For linearization at a stored DQsym sample instead of a solved
steady state, see [§7.4](#74-dqsym--dynamic-phasor-time-domain).

The resulting converter model can be used for
[stability assessment](#76-stability-assessment) and
[converter eigenvalue analysis](#77-eigenvalues-and-participation-converters).
OPF success alone does not establish a feasible nonlinear converter equilibrium
or system stability. `make_OPF` skips MMC updates after an unsuccessful OPF
solve and reports exceptions from equilibrium calculation or linearization.
These checks are not an automatic nonlinear feasibility-restoration procedure.

**Implementation references:**
[load mapping](../../src/Elements/Load/Load.cpp),
[PQ load mapping](../../src/Elements/Load/Load_PQ.cpp),
[MMC-to-OPF mapping](../../src/Elements/Converter/MMC.h),
[OPF control constraints](../../src/Solver/OPF/Powerflow_solver.cpp),
[OPF-to-MMC mapping](../../src/Solver/OPF/Powerflow_network_params.cpp), and
[MMC references, equilibrium, and linearization](../../src/Elements/Converter/MMC.cpp).

---

## 7.6 Stability assessment

**Goal:** Impedance-based stability analysis at converter interfaces in a hybrid AC–DC system.

When using an OPF operating point, first follow the [OPF-to-converter mapping](#mapping-between-opf-and-converter-models) in §7.5, including terminal-unit conversions, controller-reference updates, and equilibrium initialization.

### Workflow

```cpp
Network net;
// … build network with converters at ACx_DCy locations …

PowerFlow pf;
// … optional: OPF for operating point …

net.add_areas();

StabilityEstimate stability;
stability.add_areas(&net);
stability.print_summary();

// Transfer function at converter terminal
stability.compute_transfer_function("MMC2", "DC", 1000.0);

// Frequency sweep plots
stability.bodeplotTF("MMC2", "DC", 0.1, 10000, 1000);
stability.nyquistplotTF("MMC2", "DC", 10, 2000, 500);
stability.writeFileTF("MMC2", "DC", 10, 2000, 500);
```

**Run:** `Harmony --cpp stability_check`

**JSON:** `"type": "stability_assessment"` in `computations` (requires converters in the model).

**Location argument:** `"AC"` or `"DC"` selects which converter terminal is analyzed.

**JSON:** `"type": "stability_assessment"` with `"converter_id"` and `"location"` when converters exist in the JSON-built model (MMC JSON support pending).

---

## 7.7 Eigenvalues and participation (converters)

For a linearized converter at an operating point:

```cpp
mmc->solveEquilibrium();
mmc->computeABCD();
mmc->checkStability();
mmc->printEigenvalues();
mmc->plotEigenvalues();
mmc->plotParticipationFactors();
```

**Run:** `Harmony --cpp mmc`

---

## 7.8 Equivalent impedance (advanced)

`StabilityEstimate` also provides:

- `compute_equivalent_admittance_parameters_num` — numerical Y at a SubNetwork port
- `compute_closing_impedance` — impedance with other ports shorted

These require programmatic bus/element selection; not yet exposed in JSON.

**Run:** `Harmony --cpp admittance_parameters`

---

## 7.9 Recommended study sequences

### Harmonic stability screening

1. Build network with converters and grid equivalents
2. Run OPF for operating point (optional but recommended)
3. `add_areas()` → `StabilityEstimate`
4. Bode/Nyquist at each converter interface
5. Compare peaks against criteria

### Converter control tuning

1. Single MMC in isolation (`Harmony --cpp mmc`)
2. Tune `controller_params`, re-run `computeABCD` and eigenvalue plots
3. Integrate into full network stability check

### Time-domain validation

1. Match steady state from OPF or equilibrium solve
2. Configure DQsym with appropriate `dt` and output buses
3. Apply breaker function if modeling switching events

---

## 7.10 Device certificates

Decentralized **passivity / phase / geometric** screens on top of `Y(jω)` and `H = Y Z_eq`:

| Workflow | Purpose |
|----------|---------|
| Device gate | Local allow/block before interconnect |
| PnP library | Catalog of gate results (JSON/CSV) |
| Operating region | (P,Q) / post-OPF setpoint check |
| GFM droop tune | Search droops to restore certificates |
| Local vs system | Local Y vs system `Her(I+H)` |

**Run:** `Harmony --cpp certificate_figures`

Full API, metrics, and test filters: [Chapter 8 — Certificates](08-certificates.md).

---

See [`../running-harmony.md`](../running-harmony.md) for CLI usage. Docs map: [`../README.md`](../README.md).

[← Component reference](06-component-reference.md) | [Manual index](README.md) | [Docs map](../README.md) | [Next: Certificates →](08-certificates.md)
