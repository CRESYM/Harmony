# Chapter 8 — Examples Catalog

[← Analysis workflows](07-analysis-workflows.md) | [Manual index](README.md) | [Next: Troubleshooting →](09-troubleshooting.md)

---

All examples are declared in `src/examples/Examples.h`. **Run from the terminal** (recommended):

```bash
conda activate harmony
Harmony --list-cpp
Harmony --cpp <name> [--no-plot] [--verbose]
```

See [`../running-harmony.md`](../running-harmony.md) and [Chapter 10](10-command-line.md). Example names drop the `example_` prefix (`example_MMC` → `--cpp mmc`).

Most plotting examples accept `bool plotting_enabled = true`. Use `--no-plot` in CI or headless environments.

---

## 8.1 Component demonstrations

| Example | `--cpp` name | File |
|---------|--------------|------|
| `example_constructors()` | `constructors` | `example_constructors.cpp` |
| `example_transformer()` | `transformer` | `example_transformer.cpp` |
| `example_generator()` | `generator` | `example_generator.cpp` |
| `example_cable()` | `cable` | `example_cable.cpp` |
| `example_OHL()` | `ohl` | `example_OHL.cpp` |
| `example_MMC()` | `mmc` | `example_MMC.cpp` |
| `example_WT_type_3()` | `wt_type_3` | `example_WT_type_3.cpp` |
| `example_WT_type_4()` | `wt_type_4` | `example_WT_type_4.cpp` |
| `example_PV_plant()` | `pv_plant` | `example_PV_plant.cpp` |
| `example_visuals()` | `visuals` | `example_visuals.cpp` |

---

## 8.2 Solver examples — OPF

| Example | `--cpp` name | File | Description |
|---------|--------------|------|-------------|
| `example_OPF()` | `opf` | `example_OPF.cpp` | Basic hybrid OPF |
| `example_OPF_1()` | `opf_1` | `example_OPF_1.cpp` | Extended OPF case |
| `example_OPF_csv()` | `opf_csv` | `example_OPF_csv.cpp` | OPF from CSV data |
| `example_OPF_csv_1()` | `opf_csv_1` | `example_OPF_csv_1.cpp` | Variant CSV OPF |
| `example_point2point_case()` | `point2point_case` | `example_point2point_case.cpp` | Point-to-point HVDC-style case |
| `example_OPF_PV()` | `opf_pv` | `example_OPF_PV.cpp` | OPF with PV plant |
| `example_OPF_WT()` | `opf_wt` | `example_OPF_WT.cpp` | OPF with wind plant |

**Requires:** Gurobi, CSV files in `src/data/`.

---

## 8.3 Solver examples — DQsym

| Example | `--cpp` name | File | Description |
|---------|--------------|------|-------------|
| `example_DQsym_math_operations()` | `dqsym_math_operations` | `example_DQsym_math_operations.cpp` | Core math validation |
| `example_DQsym_RLC()` | `dqsym_rlc` | `example_DQsym_RLC.cpp` | Simple RLC circuit |
| `example_DQsym_Simple_MMC()` | `dqsym_simple_mmc` | `example_DQsym_Simple_MMC.cpp` | Simplified MMC |
| `example_DQsym_DSSS2()` | `dqsym_dsss2` | `example_DQsym_DSSS2.cpp` | Switching / DSSS demo |
| `example_DQsym_MMC_controlled()` | *(not in CLI yet)* | `example_DQsym_MMC_controlled.cpp` | MMC with controls |
| `example_DQsym_validation()` | *(header only)* | — | Validation routines |

---

## 8.4 Solver examples — state-space and stability

| Example | `--cpp` name | File | Description |
|---------|--------------|------|-------------|
| `example_state_space()` | `state_space` | `example_state_space.cpp` | Form A,B,C,D for RLC network |
| `example_stability_check()` | `stability_check` | `example_stability_check.cpp` | **Full AC–DC hybrid stability study** |
| `example_admittance_parameters()` | `admittance_parameters` | `example_admittance_parameters.cpp` | Equivalent admittance utilities |

---

## 8.5 How to switch examples

From the **repository root** with `(harmony)` active:

```bash
conda activate harmony

# Windows
build\Release\Harmony.exe --list-cpp
build\Release\Harmony.exe --cpp stability_check
build\Release\Harmony.exe --cpp mmc --no-plot

# Linux / macOS
./build/Harmony --list-cpp
./build/Harmony --cpp stability_check
```

CLI names drop the `example_` prefix (`example_stability_check` → `stability_check`). See [`../running-harmony.md`](../running-harmony.md).

---

## 8.6 JSON examples (from C++ examples)

JSON equivalents of bundled C++ demos are in `src/examples/json/`:

| JSON file | C++ example | Notes |
|-----------|-------------|-------|
| `example.json` | (minimal) | Load + transformer |
| `constructors.json` | `example_constructors` | Transmission line |
| `transformer.json` | `example_transformer` | Delta-Y real transformer |
| `generator.json` | `example_generator` | Generator Y-matrix |
| `cable.json` | `example_cable` | Cable model |
| `ohl.json` | `example_OHL` | Overhead line |
| `mmc.json` | `example_MMC` | MMC (MMC1 parameters) |
| `pv_plant.json` | `example_PV_plant` | PV plant |
| `wt_type_3.json` | `example_WT_type_3` | Type-3 wind turbine |
| `wt_type_4.json` | `example_WT_type_4` | Type-4 wind turbine |
| `state_space.json` | `example_state_space` | RLC network (build only) |
| `passives_rlc.json` | — | R/L/C passives demo |
| `opf_csv.json` | `example_OPF_csv` | CSV OPF via `solve_opf` (`ac5` + `mtdc3`); stub network is ignored |
| `dqsym_mmc.json` | `example_DQsym_Simple_MMC` | DQsym time-domain MMC with sources |
| `stability_check.json` | `example_stability_check` | Full hybrid OPF + stability + Y-matrix plots |

Run with:

```bash
Harmony --json src/examples/json/mmc.json --no-plot
Harmony --json src/examples/json/stability_check.json --no-plot
Harmony --json src/examples/json/dqsym_mmc.json --no-plot
Harmony --json src/examples/json/opf_csv.json --no-plot
Harmony --list-json
```

CSV OPF reads `src/data/` relative to the repo (auto-detected from the working directory). Built-network OPF/stability cases use components in the JSON file.

---

## 8.7 CI coverage

GitHub Actions workflows build and run examples on Windows, Linux, and macOS (`windows_run_examples.yml`, etc.). Use CI logs to confirm an example runs cleanly on your target platform.

---

## 8.8 Suggested example by user goal

| Goal | Start with |
|------|------------|
| Learn network assembly | `--cpp constructors`, `--cpp state_space` |
| Understand MMC | `--cpp mmc` |
| Full hybrid stability | `--cpp stability_check` |
| OPF from files | `--cpp opf_csv` |
| Time-domain transients | `--cpp dqsym_rlc` → `--cpp dqsym_simple_mmc` |
| RES integration | `--cpp pv_plant`, `--cpp opf_pv` |
| Lines and cables | `--cpp cable`, `--cpp ohl` |

[← Analysis workflows](07-analysis-workflows.md) | [Manual index](README.md) | [Next: Troubleshooting →](09-troubleshooting.md)
