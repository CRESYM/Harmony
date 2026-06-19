# Chapter 7 — Analysis Workflows

[← Component reference](06-component-reference.md) | [Manual index](README.md) | [Next: Examples catalog →](08-examples-catalog.md)

---

## 7.1 Overview

After building a `Network`, you invoke a **solver** appropriate to the study type. This chapter describes the main workflows and points to example code.

---

## 7.2 Y-parameter / frequency sweep

**Goal:** Obtain admittance vs frequency for one element or the whole network interface.

```cpp
element->writeFile(1.0, 1e4, 1000);           // 1 Hz – 10 kHz, 1000 points
element->plotYParameters(1.0, 1e4, 1000);     // interactive Bode plot
```

Output: `./files/<element_id>.csv`

**Examples:** `example_MMC.cpp`, `example_transformer.cpp`, `example_cable.cpp`, `example_OHL.cpp`

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

**Examples:** `example_state_space.cpp`

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

**Examples:**

| Example | Focus |
|---------|-------|
| `example_DQsym_RLC` | Basic RLC circuit |
| `example_DQsym_Simple_MMC` | MMC without full control |
| `example_DQsym_DSSS2` | Switching sequence |
| `example_DQsym_math_operations` | Library validation |
| `example_DQsym_MMC_controlled` | Controlled MMC |

**Note:** JSON runner does not yet expose DQsym; use C++ examples.

---

## 7.5 Optimal power flow (OPF)

**Goal:** Compute AC–DC dispatch, voltages, and converter setpoints for a given case.

### CSV case files

Harmony ships MATPOWER-style CSV sets under `src/data/`. File naming:

```
<case>_bus_ac.csv
<case>_branch_ac.csv
<case>_gen_ac.csv
<case>_gencost_ac.csv
…
<case>_bus_dc.csv
<case>_branch_dc.csv
<case>_conv_dc.csv
```

Example case prefixes: `ac5`, `ac14ac57`, `mtdc3`, `mtdc3slack_a`.

### Typical code pattern

```cpp
PowerFlow pf;
auto dataAc = pf.create_ac("ac5");
auto dataDc = pf.create_dc("mtdc3");
pf.load_params_ac("AC1", dataAc);
pf.load_params_dc("DC1", dataDc);

std::map<std::string, double> globalParams;
globalParams["baseMVA"] = 100.0;
globalParams["ACbaseKV"] = 345.0;
globalParams["DCbaseKV"] = 400.0;

pf.make_OPF(&network, globalParams, /*vscControl=*/true,
            /*writeTxt=*/false, /*plotResult=*/false, /*print_info=*/true);
```

**Examples:** `example_OPF.cpp`, `example_OPF_csv.cpp`, `example_OPF_1.cpp`, `example_point2point_case.cpp`, `example_OPF_PV.cpp`, `example_OPF_WT.cpp`

**Requirements:** Gurobi license; network elements must carry OPF metadata where needed.

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

**Reference example:** `example_stability_check.cpp` (also the default in `src/main.cpp`).

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

**Example:** `example_MMC.cpp`

---

## 7.8 Equivalent impedance (advanced)

`StabilityEstimate` also provides:

- `compute_equivalent_admittance_parameters_num` — numerical Y at a SubNetwork port
- `compute_closing_impedance` — impedance with other ports shorted

These require programmatic bus/element selection; not yet exposed in JSON.

**Example:** `example_admittance_parameters.cpp`

---

## 7.9 Recommended study sequences

### Harmonic stability screening

1. Build network with converters and grid equivalents
2. Run OPF for operating point (optional but recommended)
3. `add_areas()` → `StabilityEstimate`
4. Bode/Nyquist at each converter interface
5. Compare peaks against criteria

### Converter control tuning

1. Single MMC in isolation (`example_MMC.cpp`)
2. Tune `controller_params`, re-run `computeABCD` and eigenvalue plots
3. Integrate into full network stability check

### Time-domain validation

1. Match steady state from OPF or equilibrium solve
2. Configure DQsym with appropriate `dt` and output buses
3. Apply breaker function if modeling switching events

[← Component reference](06-component-reference.md) | [Manual index](README.md) | [Next: Examples catalog →](08-examples-catalog.md)
