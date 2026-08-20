# Chapter 7 — Analysis Workflows

[← Component reference](06-component-reference.md) | [Manual index](README.md) | [Next: Examples catalog →](08-examples-catalog.md)

---

## 7.1 Overview

After building a `Network`, you invoke a **solver** appropriate to the study type. This chapter describes the main workflows and points to example code.

**How to run:** [`../running-harmony.md`](../running-harmony.md). Use **HarmonyUI** for interactive runs with optional embedded plots ([Chapter 11](11-harmony-ui.md)), or the **`Harmony` CLI** for scripts and CI:

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
```

**Examples:** `Harmony --cpp dqsym_rlc`, `Harmony --cpp dqsym_simple_mmc`, `Harmony --cpp dqsym_dsss2`, `Harmony --cpp dqsym_math_operations`

**JSON:** `"type": "dqsym"` or `"time_domain"` in the `computations` block (see [Chapter 5](05-json-input.md)).

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

`MMC::computePowerFlow` writes `P_G = Pac` (MW) so that OPF’s `pn = −P_G`
matches. After OPF, `make_OPF` maps results back with
`Pac = −ps`, `Qac = −qs`, `Pdc = −pn` before `update_MMC`. Seed MMC examples
in machine signs (`Pac`/`Pdc` same sign), not raw OPF injections.

The OPF pairs converter row `i` with DC bus row `i`, so `conv_dc` must have
exactly one row per DC bus, ordered by `BUSDC_I`.

`TYPE_DC` selects the DC-side control: `1` constant power (`P_G`), `2` constant
DC voltage (`VTAR`), anything else power-voltage droop (`DROOP`, `PDCSET`,
`VDCSET`). `TYPE_AC` selects `1` constant reactive power (`Q_G`) or AC voltage
control (`VTAR`). These setpoints are only enforced when `solve_opf` is called
with `vscControl = true`; with `false` the optimiser chooses the setpoints, which
is the right choice for a benchmark where no converter controls DC voltage.

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

---

## 7.6 Stability assessment

**Goal:** Impedance-based stability analysis at converter interfaces in a hybrid AC–DC system.

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

See [`../running-harmony.md`](../running-harmony.md) for CLI usage.

[← Component reference](06-component-reference.md) | [Manual index](README.md) | [Next: Examples catalog →](08-examples-catalog.md)
