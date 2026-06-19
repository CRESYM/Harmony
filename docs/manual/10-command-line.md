# Chapter 10 — Command-Line Interface

[← Troubleshooting](09-troubleshooting.md) | [Manual index](README.md)

---

> **Canonical run guide:** [`../running-harmony.md`](../running-harmony.md) (conda, paths, Visual Studio, troubleshooting). This chapter is the detailed CLI reference.

After you have built Harmony (see [Chapter 2 § Build](02-getting-started.md#24-build-the-main-executable)), use the same executable to run **C++ examples** or **JSON simulation files**. No recompile is needed to switch studies.

Run from the **repository root** so default JSON search paths resolve correctly:

```bash
conda activate harmony

# Windows
build\Release\Harmony.exe --help

# Linux / macOS
./build/Harmony --help
```

---

## 10.1 Overview

| Mode | Flag | Purpose |
|------|------|---------|
| C++ example | `--cpp <name>` | Run a bundled demonstration program |
| JSON case | `--json <file>` | Build a network from JSON and run configured computations |
| Discover | `--list-cpp`, `--list-json` | List available examples or JSON files |
| Help | `--help`, `-h` | Print usage |

Common options:

| Flag | Purpose |
|------|---------|
| `--no-plot` | Disable GUI plotting in examples that support it |
| `--verbose`, `-v` | Extra console output |
| `--json-path <dir>` | Replace default JSON search directories (repeatable) |
| `--search-path <dir>` | Append a directory when resolving JSON filenames (repeatable) |

---

## 10.2 Help and discovery

```bash
./build/Harmony --help
./build/Harmony --list-cpp
./build/Harmony --list-json
```

Example names in `--list-cpp` omit the `example_` prefix (e.g. `stability_check` for `example_stability_check()`).

---

## 10.3 Run a C++ example

```bash
./build/Harmony --cpp stability_check
./build/Harmony --cpp mmc --no-plot
./build/Harmony --cpp opf --verbose
```

Examples that open plot windows respect `--no-plot` (useful on headless machines or in CI).

**Visual Studio:** Set **Harmony** as the startup project. In **Project → Properties → Debugging → Command Arguments**, enter for example:

```text
--cpp stability_check --no-plot
```

Set the **working directory** to the repository root.

---

## 10.4 Run a JSON simulation

```bash
./build/Harmony --json example.json
./build/Harmony --json src/examples/example.json --verbose
```

If `<file>` is not found in the current directory, Harmony searches configured paths **in order** (see below).

Built-in defaults (when `HARMONY_JSON_PATH` is unset):

1. `src/examples`
2. `src/examples/json`
3. `src/json`
4. `examples`
5. `.` (current directory)

Override defaults for one run:

```bash
./build/Harmony --json-path /path/to/my_cases --json case_01.json
```

Set defaults for all runs in the shell:

```bash
export HARMONY_JSON_PATH=/path/to/my_cases:/path/to/other
```

Append a directory without replacing defaults:

```bash
./build/Harmony --search-path /path/to/my_cases --json case_01.json
```

JSON format and component types: [Chapter 5](05-json-input.md) and [`../input-file-format.md`](../input-file-format.md).

---

## 10.5 Example names (C++)

| CLI name | Topic |
|----------|--------|
| `stability_check` | Full AC–DC hybrid stability study |
| `mmc` | MMC equilibrium, ABCD, Y-matrix |
| `opf`, `opf_csv`, `opf_pv`, `opf_wt` | Optimal power flow cases |
| `dqsym_rlc`, `dqsym_simple_mmc`, `dqsym_dsss2` | Dynamic phasor (DQsym) demos |
| `cable`, `ohl` | Cable and overhead line Y sweeps |
| `wt_type_3`, `wt_type_4`, `pv_plant` | RES models |
| `state_space`, `constructors`, `transformer` | Learning / component demos |

Run `./build/Harmony --list-cpp` (or `build\Release\Harmony.exe --list-cpp` on Windows) for the full list registered in `src/cli.cpp`.

---

## 10.6 JSON computations from the CLI

When using `--json`, the `computations` array in the file runs after the network is built. Supported types include:

| `type` | Description |
|--------|-------------|
| `print_connections`, `network_summary` | Wiring and area summary |
| `y_matrix` | Y-parameter frequency sweep to CSV |
| `stability_assessment` | Impedance-based stability (needs converters in model) |
| `opf`, `power_flow` | OPF; requires `"case_name"` (CSV prefix under `src/data/`) |
| `dqsym`, `time_domain` | DQsym time-domain run on the built network |

Example DQsym block:

```json
{
  "type": "dqsym",
  "dt": 2.0e-5,
  "t_start": 0.0,
  "t_end": 0.1,
  "frequency": 50,
  "n_keep": 5,
  "output_bus_ids": ["bus1"]
}
```

---

## 10.7 Runtime issues

### Executable exits immediately (Windows)

Exit code `0xC0000135` usually means a dependency DLL is not on `PATH`. Activate the conda environment before running:

```bash
conda activate harmony
./build/Harmony --help
```

### JSON file not found

Use an explicit path, or add `--search-path`. Confirm the file exists:

```bash
./build/Harmony --list-json
```

### Plots block the terminal

Some examples wait for Enter after plotting. Use `--no-plot`, or run non-interactive examples via `--json`.

More build and environment issues: [Chapter 9](09-troubleshooting.md).

---

## 10.8 Adding new examples (developers)

To expose a new C++ example on the command line, register it in `src/cli.cpp`. You do not need to edit `src/main.cpp` or rebuild to switch between already-registered examples.

[← Troubleshooting](09-troubleshooting.md) | [Manual index](README.md)
