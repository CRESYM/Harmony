# Chapter 9 — Troubleshooting

[← Examples catalog](08-examples-catalog.md) | [Manual index](README.md) | [Next: Command-line interface →](10-command-line.md) | [HarmonyUI →](11-harmony-ui.md)

---

> **Running Harmony:** [`../running-harmony.md`](../running-harmony.md) — conda activation, working directory, CLI flags, and common run failures.

## 9.1 Build and environment

### `(harmony)` not shown in terminal

Conda environment is not activated.

```bash
conda activate harmony
```

On Windows in Visual Studio, ensure miniconda was installed with **Add to PATH**. See [`../installation.md`](../installation.md).

---

### CMake cannot find Gurobi

Pass the installation path explicitly:

```bash
cmake .. -DGUROBI_PATH="C:/gurobi1202/win64"
```

Verify `include/` and `lib/` exist under that path.

---

### CMake cannot find SymEngine / Eigen / SUNDIALS

Recreate the conda environment:

```bash
conda env remove -n harmony
conda env create -f environment.yml
conda activate harmony
```

Build always from an activated `(harmony)` environment.

---

### Link errors on Windows (Debug vs Release)

Gurobi provides separate C++ wrapper libraries for Debug (`gurobi_c++mdd*.lib`) and Release (`gurobi_c++md*.lib`). Harmony's CMake selects them by configuration. Do not mix Debug Harmony with Release Gurobi libraries.

---

## 9.2 Runtime errors

### `[WARNING] Pin mismatch`

An element terminal's pin count does not match the connected bus.

**Fix:** Set bus `pins` equal to element phases for that terminal. Converters: terminal 1 = AC pins (3), terminal 2 = DC pins (2).

---

### Gurobi license error at OPF

Obtain or refresh a Gurobi license (`grbgetkey` for academic licenses). OPF examples fail without a valid license; Y-matrix and stability examples that skip OPF may still run.

---

### `./files/` write failure

The output directory does not exist.

```bash
mkdir files
```

Or set `"output_directory"` in JSON simulation section.

---

### Plot window does not appear (CLI)

- Plots are enabled by default in most C++ examples
- Headless/remote sessions may lack OpenGL/GLFW display
- Disable plots from the CLI: `Harmony --cpp stability_check --no-plot`
- JSON plots need `"plot": true` or `"plot_result": true` in the computation block

### HarmonyUI / ImGui blank window or immediate exit (Linux / macOS)

- Install system OpenGL/GLFW packages (see [`../installation.md`](../installation.md)): on Ubuntu, `libgl1-mesa-dev`, `libx11-dev`, `libglfw3-dev`, and `libglew-dev`.
- Run from a local desktop session with a GPU/display (`echo $DISPLAY` set on Linux; not SSH without X forwarding).
- Rebuild after updating Harmony — Linux/macOS need OpenGL 3.x context hints and `glewInit()` on non-Apple platforms; macOS must not use the GLEW ImGui loader with a core-profile context.
- Use `--no-plot` on headless CI machines; GitHub Actions only **compile** HarmonyUI, they do not run the GUI.

### No charts in HarmonyUI

- Check **Plot** on the Launcher tab before clicking **Run** (default is off)
- JSON must include `"plot": true` or `"plot_result": true` on the relevant computation
- After a run with plots, open the **Plots** section (HarmonyUI switches there automatically)

### HarmonyUI crash or ImGui focus errors

Older builds opened plots in a second GLFW/ImGui window, which could trigger `PopFocusScope()` errors. Current **HarmonyUI** embeds plots in the **Plots** tab (single window). Rebuild **HarmonyUI** from an up-to-date source tree.

### Save PNG button missing or clipped

Rebuild **HarmonyUI** — plot toolbar sizing was adjusted for larger fonts. Use **Save all tabs as PNG** on the Plots toolbar if a per-tab button is off-screen.

---

### JSON input: `unable to initialize simulation`

Check stderr for the first validation error (missing field, unknown type, duplicate id). Compare your file to `src/examples/example.json`.

---

### JSON input: `bus 'X' not found`

Component references a bus id not defined in `"buses"`, or the bus has `"enabled": false`.

---

## 9.3 Model assembly

### Converter missing from area summary

- Location must match `ACi_DCj` pattern (e.g. `AC1_DC1`)
- Both AC and DC terminals must be connected to buses in the respective areas

---

### OPF infeasible or unexpected dispatch

- Verify CSV case name matches files in `src/data/`
- Check `setOPFInfo` on generators and converter limits
- Inspect `print_info = true` output from `make_OPF`

---

### MMC equilibrium fails to converge

- Review converter parameter vector order ([Chapter 6](06-component-reference.md))
- Compare against working vectors in `example_stability_check.cpp` or `tests/testMMC.cpp`
- Reduce controller gains if eigenvalues indicate instability

---

## 9.4 Memory and crashes on exit

Harmony expects the `Network` to own all heap-allocated buses and elements. Do not delete objects after handing them to `connectElementToBus`.

If extending the code:

- SubNetworks from `add_areas()` are non-owning views — do not delete their buses separately
- MMC controllers are freed by the `Converter` destructor when the element is destroyed with the network

---

## 9.5 Getting help

1. Check GitHub [Issues](https://github.com/CRESYM/Harmony/issues) for known problems
2. Run `ctest` to isolate library vs example issues
3. Compare against the closest bundled example from [Chapter 8](08-examples-catalog.md)
4. Include Harmony commit hash, OS, conda environment export, and minimal reproduction when reporting bugs

---

## 9.6 Known limitations (current release)

| Limitation | Workaround |
|------------|------------|
| Complex DQsym breaker schedules | C++ DQsym examples; basic `dqsym` in JSON |
| `equivalent_impedance` not in JSON | C++ `StabilityEstimate` API |
| User manual PDF not auto-generated from this draft | Export Markdown via Pandoc or publish to GitHub Pages |

[← Examples catalog](08-examples-catalog.md) | [Manual index](README.md) | [Next: Command-line interface →](10-command-line.md) | [HarmonyUI →](11-harmony-ui.md)
