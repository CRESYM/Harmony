# Running Harmony

After [building](installation.md) the `Harmony` executable, use the **command-line interface** to run C++ examples or JSON simulation files. You do **not** need to edit `src/main.cpp` or rebuild to switch studies.

---

## Before you run

1. **Activate the conda environment** (required so DLLs and libraries are found):

   ```bash
   conda activate harmony
   ```

   The prompt should show `(harmony)`.

2. **Use the repository root as the working directory** when possible. Harmony searches for JSON files relative to the current directory and default search paths under the repo.

3. **Locate the executable:**

   | Platform | Typical path (after Release build) |
   |----------|----------------------------------|
   | Windows | `build/Release/Harmony.exe` |
   | Linux / macOS | `build/Harmony` or `build/Release/Harmony` |

---

## Quick start

From the **Harmony repository root**:

```bash
conda activate harmony

# Windows
build\Release\Harmony.exe --help
build\Release\Harmony.exe --list-cpp
build\Release\Harmony.exe --cpp stability_check
build\Release\Harmony.exe --json src/examples/example.json --verbose

# Linux / macOS
./build/Harmony --help
./build/Harmony --cpp stability_check
./build/Harmony --json src/examples/example.json
```

---

## Command-line interface

### Modes

| Flag | Description |
|------|-------------|
| `--help`, `-h` | Print usage |
| `--list-cpp` | List available C++ examples |
| `--list-json` | List JSON files in search paths |
| `--cpp <name>` | Run a C++ example (e.g. `stability_check`, `mmc`, `opf`) |
| `--json <file>` | Run a JSON simulation file |

### Options

| Flag | Description |
|------|-------------|
| `--no-plot` | Disable GUI plots (headless / CI) |
| `--verbose`, `-v` | Extra console output |
| `--search-path <dir>` | Add directory for resolving JSON filenames (repeatable) |

Example names for `--cpp` omit the `example_` prefix: `example_stability_check` → `--cpp stability_check`.

### Default JSON search paths

When `--json` receives a filename without a full path, Harmony looks in:

1. `src/examples`
2. `src/json`
3. `examples`
4. `.` (current directory)

Example:

```bash
Harmony --json example.json
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
| `opf`, `opf_csv` | Optimal power flow |
| `cable`, `ohl` | Line/cable Y-parameter sweeps |
| `wt_type_3`, `pv_plant` | RES models |
| `dqsym_rlc`, `dqsym_simple_mmc` | Dynamic phasor (DQsym) |

Full catalog: [User Manual — Examples](manual/08-examples-catalog.md).

---

## JSON simulations

Define buses, components, and computations in a JSON file. See [input file format](input-file-format.md) and [User Manual Chapter 5](manual/05-json-input.md).

```bash
Harmony --json src/examples/example.json
Harmony --json src/examples/example.json --verbose
```

The file can include a `computations` array for post-build steps (`y_matrix`, `opf`, `dqsym`, `stability_assessment`, etc.).

---

## Visual Studio

1. Set **Harmony** as the startup project.
2. **Project → Properties → Debugging → Command Arguments**, for example:
   - `--cpp stability_check --no-plot`
   - `--json src/examples/example.json --verbose`
3. Set **Working Directory** to the **repository root** (not `build/Release`).

You can also run from **View → Terminal** at the repo root using the commands above.

---

## Output files

Many studies write CSV frequency data to:

```
./files/<element_id>.csv
```

Create `./files` if needed, or set `"output_directory"` in the JSON `simulation` section.

---

## Troubleshooting

| Problem | What to do |
|---------|------------|
| Executable exits immediately (Windows `0xC0000135`) | Run `conda activate harmony` first; dependencies must be on `PATH`. |
| `Unknown C++ example` | Run `--list-cpp` for valid names. |
| JSON file not found | Use a full path, or `--search-path`; run `--list-json`. |
| Plots block the terminal | Use `--no-plot`. |
| OPF fails | Ensure Gurobi is installed and `GUROBI_PATH` was set at configure time. |

More detail: [User Manual — Troubleshooting](manual/09-troubleshooting.md) and [Chapter 10 — CLI](manual/10-command-line.md).

---

## Related documentation

- [Installation](installation.md) — build the executable
- [User Manual](manual/README.md) — workflows and components
- [JSON input format](input-file-format.md) — schema for `--json`
- [Command-line reference (Chapter 10)](manual/10-command-line.md)
