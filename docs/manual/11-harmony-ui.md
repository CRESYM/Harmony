# Chapter 11 — HarmonyUI (graphical launcher)

[← Command-line interface](10-command-line.md) | [Manual index](README.md)

---

Harmony ships two executables after a normal build:

| Executable | Audience | Purpose |
|------------|----------|---------|
| **`HarmonyUI`** | Interactive use | Pick examples or JSON files, run studies, view plots in-app |
| **`Harmony`** | Developers, CI, scripts | Full CLI (`--cpp`, `--json`, `--list-cpp`, …) |

Both link the same solvers and JSON pipeline. The CLI is unchanged for automation and batch runs.

---

## 11.1 Build and launch

From the repository root with `(harmony)` active:

```bash
conda activate harmony
cmake --build build --config Release --target HarmonyUI

# Windows
build\Release\HarmonyUI.exe

# Linux / macOS
./build/HarmonyUI
```

Build both targets:

```bash
cmake --build build --config Release --target Harmony --target HarmonyUI
```

Typical output paths:

| Platform | HarmonyUI | Harmony (CLI) |
|----------|-----------|---------------|
| Windows | `build/Release/HarmonyUI.exe` | `build/Release/Harmony.exe` |
| Linux / macOS | `build/HarmonyUI` | `build/Harmony` |

**Visual Studio:** Set **HarmonyUI** as the startup project to launch the GUI. Set **Working Directory** to the **repository root** so JSON examples and `src/data/` OPF cases resolve correctly.

---

## 11.2 Main window layout

HarmonyUI uses a single window with three sections (top buttons):

| Section | Contents |
|---------|----------|
| **Launcher** | HARMONY banner, example/JSON picker, **Plot** checkbox, **Run**, **Validate JSON**, PNG output directory |
| **Plots** | Interactive charts (Bode, Nyquist, OPF network, DQsym waveforms, …) when **Plot** was checked for the run |
| **Log** | Live stdout/stderr from the solver |

After a successful run with **Plot** enabled, HarmonyUI switches to the **Plots** section automatically.

---

## 11.3 Running a study

1. Open **Launcher**.
2. Choose **C++ example** or **JSON file**.
3. Select a case from the dropdown (OPF examples are grouped at the top).
4. Optionally check **Verbose log**.
5. Check **Plot** only if you want charts (default is off — headless-friendly).
6. Click **Run**.

Runs execute on a **background thread** so the window stays responsive. Status and solver output appear in **Log**.

### Plot checkbox

| **Plot** | Behaviour |
|----------|-----------|
| Unchecked | Solvers run; no charts (same idea as `Harmony --no-plot`) |
| Checked | When the JSON computation sets `"plot": true` or `"plot_result": true`, charts appear under **Plots** |

JSON plot flags are still honoured only when **Plot** is checked in HarmonyUI.

---

## 11.4 OPF examples

All bundled OPF cases are listed in the dropdowns (not hidden when Gurobi is missing):

**C++ (Launcher → C++ example → OPF group):**

- `opf`, `opf_csv`, `opf_1`, `opf_csv_1`, `opf_pv`, `opf_wt`

**JSON (Launcher → JSON file → OPF group):**

- `opf_csv.json` — CSV OPF (`ac5` + `mtdc3` under `src/data/`)
- Other JSON files whose names contain `opf` (e.g. built-network OPF inside `stability_check.json`)

If Gurobi is not installed or licensed, OPF runs fail with an error in **Log**; other studies still work.

---

## 11.5 Saving plots as PNG

- **Per tab:** In **Plots**, each chart tab has a **Save PNG** button (top-right of the tab).
- **All tabs:** **Save all tabs as PNG** on the Plots toolbar saves every open chart.
- **Output directory:** Set **PNG output directory** on the Launcher tab (default: `.`).

PNG capture uses the HarmonyUI window; plots are embedded in the same process (no separate plot window).

---

## 11.6 Validate JSON

On the Launcher tab, with **JSON file** selected, click **Validate JSON** to parse and schema-check the file without running solvers. Errors are written to **Log**.

---

## 11.7 CLI vs HarmonyUI (plotting)

| Aspect | HarmonyUI | `Harmony` CLI |
|--------|-----------|---------------|
| Plot window | **Plots** tab (same process) | Separate **Harmony Visualization** window (background thread) |
| Disable plots | Leave **Plot** unchecked | `--no-plot` |
| Wait for plots | Window stays open; switch tabs freely | JSON runs block until the plot window closes (unless scripted) |
| Automation | Manual launcher | Scripts, CI, `--verbose`, env paths |

Developers and CI should keep using:

```bash
Harmony --json src/examples/json/stability_check.json --no-plot
Harmony --cpp opf_csv --verbose
```

---

## 11.8 Troubleshooting

| Problem | What to do |
|---------|------------|
| Examples / JSON not found | Run HarmonyUI from the **repo root**, or type a full path in **Or path / filename**. Click **Refresh list**. |
| OPF fails | Install Gurobi; reconfigure with `-DGUROBI_PATH=...`. Check **Log** for license errors. |
| No **Plots** tab content | Enable **Plot** before **Run**; JSON must include `"plot": true` or `"plot_result": true` on computations. |
| DLL error on start (Windows `0xC0000135`) | `conda activate harmony` before launching. |
| ImGui / focus errors (older builds) | Update to a build with **embedded plots** (single window). Do not run HarmonyUI and the old separate visualization window together. |

More: [Chapter 9 — Troubleshooting](09-troubleshooting.md), [`../running-harmony.md`](../running-harmony.md).

---

## 11.9 Related documentation

- [`../running-harmony.md`](../running-harmony.md) — conda, paths, CLI flags
- [Chapter 10 — Command-line interface](10-command-line.md) — `Harmony` executable reference
- [Chapter 8 — Examples catalog](08-examples-catalog.md) — JSON files in `src/examples/json/`
- [Chapter 5 — JSON input](05-json-input.md) — `computations` and plot flags

[← Command-line interface](10-command-line.md) | [Manual index](README.md)
