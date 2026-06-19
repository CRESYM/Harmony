# Running Harmony

After [building](installation.md), you can run studies in two ways:

- **`HarmonyUI`** — graphical launcher (recommended for interactive use): pick examples, optional plots in the same window, log panel.
- **`Harmony`** — command-line interface for developers, scripts, and CI.

Both use the same solvers and JSON pipeline. Activate the conda environment before either executable.

---

## Before you run

1. **Activate the conda environment** (required so DLLs and libraries are found):

   ```bash
   conda activate harmony
   ```

   The prompt should show `(harmony)`.

2. **Use the repository root as the working directory** when possible. Harmony also **auto-detects the repo root** from the executable location (e.g. `build/Release/Harmony.exe` → searches `../../src/examples/…`), so JSON files often work even when the shell cwd is `build/Release`.

3. **Locate the executables:**

   | Platform | HarmonyUI | Harmony (CLI) |
   |----------|-----------|---------------|
   | Windows | `build/Release/HarmonyUI.exe` | `build/Release/Harmony.exe` |
   | Linux / macOS | `build/HarmonyUI` | `build/Harmony` or `build/Release/Harmony` |

   Build HarmonyUI:

   ```bash
   cmake --build build --config Release --target HarmonyUI
   ```

---

## Quick start — HarmonyUI

From the **repository root**:

```bash
conda activate harmony
cmake --build build --config Release --target HarmonyUI

# Windows
build\Release\HarmonyUI.exe

# Linux / macOS
./build/HarmonyUI
```

1. On **Launcher**, pick a **C++ example** or **JSON file** (OPF cases are grouped at the top).
2. Check **Plot** only if you want charts.
3. Click **Run** — output goes to **Log**; charts appear under **Plots** when enabled.

Full UI guide: [User Manual — Chapter 11](manual/11-harmony-ui.md).

---

## Quick start — CLI (`Harmony`)

```bash
conda activate harmony

# Windows
build\Release\Harmony.exe --help
build\Release\Harmony.exe --list-cpp
build\Release\Harmony.exe --cpp stability_check
build\Release\Harmony.exe --json src/examples/json/stability_check.json --verbose

# Linux / macOS
./build/Harmony --help
./build/Harmony --cpp stability_check
./build/Harmony --json src/examples/json/stability_check.json
```

---

## Command-line interface (`Harmony`)

### Modes

| Flag | Description |
|------|-------------|
| `--help`, `-h` | Print usage (includes ASCII banner) |
| `--list-cpp` | List available C++ examples |
| `--list-json` | List JSON files in search paths |
| `--cpp <name>` | Run a C++ example (e.g. `stability_check`, `mmc`, `opf`) |
| `--json <file>` | Run a JSON simulation file |

### Options

| Flag | Description |
|------|-------------|
| `--no-plot` | Disable GUI plots for C++ examples and JSON `plot` / `plot_result` flags |
| `--verbose`, `-v` | Extra console output |
| `--json-path <dir>` | Replace default JSON search directories (repeatable) |
| `--search-path <dir>` | Append directory for resolving JSON filenames (repeatable) |

Example names for `--cpp` omit the `example_` prefix: `example_stability_check` → `--cpp stability_check`.

### Default JSON search paths

When `HARMONY_JSON_PATH` is **unset**, Harmony looks for bare filenames in this order:

1. `src/examples`
2. `src/examples/json`
3. `src/json`
4. `examples`
5. `.` (current directory)

**Change defaults permanently** (environment variable):

```bash
# Windows
set HARMONY_JSON_PATH=D:\my_cases;D:\shared\harmony-json

# Linux / macOS
export HARMONY_JSON_PATH=/home/me/cases:/opt/harmony/json
```

**Change defaults for one run** (`--json-path` replaces built-in / env defaults):

```bash
Harmony --json-path D:\cases --json my_network.json
Harmony --json example.json
```

**Append** without replacing defaults:

```bash
Harmony --search-path D:\cases --json my_network.json
```

---

## C++ examples

List and run bundled demonstrations:

```bash
Harmony --list-cpp
Harmony --cpp mmc
Harmony --cpp opf --no-plot
Harmony --cpp dqsym_simple_mmc --verbose
```

Common examples:

| `--cpp` name | Study |
|--------------|--------|
| `stability_check` | Full AC–DC hybrid stability |
| `mmc` | MMC equilibrium and Y-matrix |
| `opf`, `opf_csv`, `opf_1`, `opf_csv_1`, `opf_pv`, `opf_wt` | Optimal power flow |
| `cable`, `ohl` | Line/cable Y-parameter sweeps |
| `wt_type_3`, `pv_plant` | RES models |
| `dqsym_rlc`, `dqsym_simple_mmc` | Dynamic phasor (DQsym) |

Full catalog: [User Manual — Examples](manual/08-examples-catalog.md).

---

## JSON simulations

Define buses, components, and computations in a JSON file. See [input file format](input-file-format.md) and [User Manual Chapter 5](manual/05-json-input.md).

```bash
Harmony --json src/examples/json/mmc.json
Harmony --json src/examples/json/stability_check.json --verbose
Harmony --json src/examples/json/opf_csv.json --no-plot
```

The file can include a `computations` array for post-build steps (`y_matrix`, `opf`, `dqsym`, `stability_assessment`, etc.). Enable plots in JSON with `"plot": true` or `"plot_result": true`; disable from CLI with `--no-plot`.

**HarmonyUI:** Check **Plot** on the Launcher tab (equivalent to omitting `--no-plot`).

---

## Visual Studio

### HarmonyUI (GUI)

1. Set **HarmonyUI** as the startup project.
2. Set **Working Directory** to the **repository root**.
3. Press Run (no command-line arguments needed).

### Harmony (CLI)

1. Set **Harmony** as the startup project.
2. **Project → Properties → Debugging → Command Arguments**, for example:
   - `--cpp stability_check --no-plot`
   - `--json src/examples/json/stability_check.json --verbose`
3. Set **Working Directory** to the **repository root**.

You can also run from **View → Terminal** at the repo root using the commands above.

---

## Output files

Many studies write CSV frequency data to:

```
./files/<element_id>.csv
```

Create `./files` if needed, or set `"output_directory"` in the JSON `simulation` section.

DQsym JSON runs may write debug text files to the current directory (`state_space_output*.txt`).

---

## Troubleshooting

| Problem | What to do |
|---------|------------|
| Executable exits immediately (Windows `0xC0000135`) | Run `conda activate harmony` first; dependencies must be on `PATH`. |
| `Unknown C++ example` | Run `--list-cpp` for valid names. |
| JSON file not found | Run `Harmony --list-json`. Use repo root as cwd, a full path, or `--json-path`. |
| Plots block the terminal (CLI) | Use `--no-plot`. In HarmonyUI, leave **Plot** unchecked. |
| OPF fails | Ensure Gurobi is installed and `GUROBI_PATH` was set at configure time. |
| HarmonyUI: no charts | Check **Plot** before Run; JSON needs `"plot": true` or `"plot_result": true`. |
| ImGui focus / plot crash (old builds) | Use current HarmonyUI with embedded **Plots** tab (single window). |

More detail: [User Manual — Troubleshooting](manual/09-troubleshooting.md), [Chapter 10 — CLI](manual/10-command-line.md), [Chapter 11 — HarmonyUI](manual/11-harmony-ui.md).

---

## Related documentation

- [Installation](installation.md) — build the executables
- [HarmonyUI (Chapter 11)](manual/11-harmony-ui.md) — graphical launcher
- [User Manual](manual/README.md) — workflows and components
- [JSON input format](input-file-format.md) — schema for `--json`
- [Command-line reference (Chapter 10)](manual/10-command-line.md)
