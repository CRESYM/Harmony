# Chapter 8 — Examples Catalog

[← Analysis workflows](07-analysis-workflows.md) | [Manual index](README.md) | [Next: Troubleshooting →](09-troubleshooting.md)

---

All examples are declared in `src/examples/Examples.h` and invoked from `src/main.cpp` by uncommenting one function call.

Most plotting examples accept `bool plotting_enabled = true`. Pass `false` in CI or headless environments.

---

## 8.1 Component demonstrations

| Example | File | Description |
|---------|------|-------------|
| `example_constructors()` | `example_constructors.cpp` | Instantiate basic element types |
| `example_transformer()` | `example_transformer.cpp` | Transformer topologies and Y-parameters |
| `example_generator()` | `example_generator.cpp` | Generator model |
| `example_cable()` | `example_cable.cpp` | Underground cable layers and Y sweep |
| `example_OHL()` | `example_OHL.cpp` | Overhead line geometry and Y sweep |
| `example_MMC()` | `example_MMC.cpp` | MMC equilibrium, ABCD, Y-matrix, eigenvalues |
| `example_WT_type_3()` | `example_WT_type_3.cpp` | Type 3 wind turbine |
| `example_WT_type_4()` | `example_WT_type_4.cpp` | Type 4 wind turbine |
| `example_PV_plant()` | `example_PV_plant.cpp` | PV plant model |
| `example_visuals()` | `example_visuals.cpp` | Plotting utilities |

---

## 8.2 Solver examples — OPF

| Example | File | Description |
|---------|------|-------------|
| `example_OPF()` | `example_OPF.cpp` | Basic hybrid OPF |
| `example_OPF_1()` | `example_OPF_1.cpp` | Extended OPF case |
| `example_OPF_csv()` | `example_OPF_csv.cpp` | OPF from CSV data |
| `example_OPF_csv_1()` | `example_OPF_csv_1.cpp` | Variant CSV OPF |
| `example_point2point_case()` | `example_point2point_case.cpp` | Point-to-point HVDC-style case |
| `example_OPF_PV()` | `example_OPF_PV.cpp` | OPF with PV plant |
| `example_OPF_WT()` | `example_OPF_WT.cpp` | OPF with wind plant |

**Requires:** Gurobi, CSV files in `src/data/`.

---

## 8.3 Solver examples — DQsym

| Example | File | Description |
|---------|------|-------------|
| `example_DQsym_math_operations()` | `example_DQsym_math_operations.cpp` | Core math validation |
| `example_DQsym_RLC()` | `example_DQsym_RLC.cpp` | Simple RLC circuit |
| `example_DQsym_Simple_MMC()` | `example_DQsym_Simple_MMC.cpp` | Simplified MMC |
| `example_DQsym_DSSS2()` | `example_DQsym_DSSS2.cpp` | Switching / DSSS demo |
| `example_DQsym_MMC_controlled()` | `example_DQsym_MMC_controlled.cpp` | MMC with controls |
| `example_DQsym_validation()` | (header only) | Validation routines |

---

## 8.4 Solver examples — state-space and stability

| Example | File | Description |
|---------|------|-------------|
| `example_state_space()` | `example_state_space.cpp` | Form A,B,C,D for RLC network |
| `example_stability_check()` | `example_stability_check.cpp` | **Full AC–DC hybrid stability study** |
| `example_admittance_parameters()` | `example_admittance_parameters.cpp` | Equivalent admittance utilities |

---

## 8.5 How to switch examples

1. Edit `src/main.cpp`
2. Comment out the currently active example
3. Uncomment the desired `example_*()` call
4. Rebuild: `cmake --build . --config Release`
5. Run from `build/Release/`

There is no runtime flag on the main executable to select examples.

---

## 8.6 CI coverage

GitHub Actions workflows build and run examples on Windows, Linux, and macOS (`windows_run_examples.yml`, etc.). Use CI logs to confirm an example runs cleanly on your target platform.

---

## 8.7 Suggested example by user goal

| Goal | Start with |
|------|------------|
| Learn network assembly | `example_constructors`, `example_state_space` |
| Understand MMC | `example_MMC` |
| Full hybrid stability | `example_stability_check` |
| OPF from files | `example_OPF_csv` |
| Time-domain transients | `example_DQsym_RLC` → `example_DQsym_Simple_MMC` |
| RES integration | `example_PV_plant`, `example_OPF_PV` |
| Lines and cables | `example_cable`, `example_OHL` |

[← Analysis workflows](07-analysis-workflows.md) | [Manual index](README.md) | [Next: Troubleshooting →](09-troubleshooting.md)
