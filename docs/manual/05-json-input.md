# Chapter 5 — JSON Input Workflow

[← Building networks](04-building-networks.md) | [Manual index](README.md) | [Next: Component reference →](06-component-reference.md)

---

## 5.1 Purpose

The **`src/json/`** module lets you define a study in **JSON** and run it with:

```bash
Harmony --json src/examples/example.json
```

Full run instructions: [`../running-harmony.md`](../running-harmony.md). It is suitable for:

- Parameter sweeps on passives and transformers
- Automated batch runs
- Sharing case definitions with non-developers

Full field-level specification: [`../input-file-format.md`](../input-file-format.md).

---

## 5.2 Run a JSON case

Build the main executable once ([Chapter 2 § Build](02-getting-started.md#24-build-the-main-executable)), then from the **repository root**:

```bash
conda activate harmony
# Windows
build\Release\Harmony.exe --json src/examples/example.json
build\Release\Harmony.exe --json example.json --verbose
build\Release\Harmony.exe --search-path /path/to/cases --json my_case.json

# Linux / macOS
./build/Harmony --json src/examples/example.json
./build/Harmony --json example.json --verbose
```

Full run guide: [`../running-harmony.md`](../running-harmony.md). CLI reference: [Chapter 10](10-command-line.md).

---

## 5.3 File structure

```json
{
  "simulation": { … },
  "buses": [ … ],
  "components": [ … ],
  "computations": [ … ]
}
```

| Section | Required | Role |
|---------|----------|------|
| `simulation` | Yes | Title, output directory, default frequency range, base values |
| `buses` | Yes | Nodes with `id`, `location`, `pins` |
| `components` | Yes | Devices with `type`, parameters, and bus connections |
| `computations` | No | Analysis steps run after the network is built |

---

## 5.4 Minimal working example

See `src/examples/example.json`:

- Two three-phase AC buses
- A load on `bus2`
- A real transformer between `bus1` and `bus2`
- Computations: print connections, Y-matrix sweep on the load, network summary

---

## 5.5 Connecting components to buses

**Single connection** (loads, single-terminal devices):

```json
"connected_bus": {
  "bus_id": "bus2",
  "terminal": 1
}
```

**Multiple connections** (transformers, branches):

```json
"connected_buses": [
  { "bus_id": "bus1", "terminal": 1 },
  { "bus_id": "bus2", "terminal": 2 }
]
```

---

## 5.6 Supported component types (JSON)

| `type` | Notes |
|--------|-------|
| `load`, `load_pq` | `"values": [P, Q, …]` |
| `ac_source`, `generator` | Requires `"voltage"` and `"values"` |
| `resistor`, `inductor`, `capacitor` | Per-phase `"values"` array |
| `transformer_real` | Object `"values"` with R, L, turns ratio, phase shift |
| `transformer_classic`, `transformer_yy`, `transformer_deltay`, … | Object `"values"` with R, L, M |
| `dc_source` | `"voltage"` (scalar or array), optional `"resistance"` / `"values"` |
| `impedance`, `admittance` | Per-phase or matrix `"values"` |
| `switch` | `"state"` (bool array) or `"closed"` (bool) |
| `transmission_line` | `"values"`: `[R, L, G, C, length]` (5 numbers) |
| `cable` | `"cable_type"`, `"length"`, `"earth"`, `"conductors"`, `"insulators"`, `"positions"` |
| `overhead_line` | `"length_km"`, `"earth"`, `"conductor"`, `"groundwire"` |
| `mmc` | `"converter_params"`; optional `"controller_params"`, `"filter_params"` |
| `wt_type_3`, `wt_type_4`, `pv_plant` | `"parameters"` numeric array |
| `wp_plant` | `"turbine_type"`, `"number_wt"`, `"parameters"` |

JSON is validated strictly at load time: unknown keys are rejected. See [`../input-file-format.md`](../input-file-format.md).

---

## 5.7 Computations block

After the network is built, the runner executes each entry in `computations`:

| `type` | Description |
|--------|-------------|
| `print_connections` | Print wiring table |
| `network_summary` | AC/DC area breakdown |
| `y_matrix` | CSV frequency sweep; optional `"component_id"` |
| `stability_assessment` | Requires converters in model; optional `"converter_id"`, `"location"` |
| `power_flow` / `opf` | Requires `"case_name"` matching CSV prefix in `src/data/` |
| `dqsym` / `time_domain` | Time-domain DQsym on the built network (`dt`, `t_end`, `output_bus_ids`, …) |
| `equivalent_impedance` | Not wired — use C++ API |

Example:

```json
"computations": [
  { "type": "print_connections" },
  {
    "type": "y_matrix",
    "component_id": "l1",
    "frequency_range": { "start": 10, "end": 1000, "step": 50 }
  }
]
```

Output CSV files go to `./files/` (or `"output_directory"` in `simulation`).

---

## 5.8 Validation errors

The builder validates:

- Required top-level sections
- Unique bus and component IDs
- Required fields per component type
- Bus existence when connecting

Errors print to stderr and terminate with a non-zero exit code.

---

## 5.9 When to use JSON vs C++

| Use JSON | Use C++ examples |
|----------|------------------|
| Repeatable Y-matrix sweeps | MMC with custom controllers (fine-tune in C++) |
| Batch runs via `--json` | Complex breaker schedules in DQsym (C++ for now) |
| Teaching / case sharing | Full hybrid OPF cases with RES when JSON is insufficient |

[← Building networks](04-building-networks.md) | [Manual index](README.md) | [Next: Component reference →](06-component-reference.md)
