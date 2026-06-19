# Chapter 5 — JSON Input Workflow

[← Building networks](04-building-networks.md) | [Manual index](README.md) | [Next: Component reference →](06-component-reference.md)

---

## 5.1 Purpose

The `input_file/` module lets you define a study in **JSON** instead of editing C++ and recompiling. It is suitable for:

- Parameter sweeps on passives and transformers
- Automated batch runs
- Sharing case definitions with non-developers

Full field-level specification: [`../input-file-format.md`](../input-file-format.md).

---

## 5.2 Build and run

```bash
conda activate harmony
cd input_file
mkdir build && cd build
cmake .. -DGUROBI_PATH="/path/to/gurobi"
cmake --build . --config Release

# Run with explicit path
./Release/Harmony.exe ../../src/examples/example.json

# Or omit path to use the default example.json
./Release/Harmony.exe
```

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

---

## 5.7 Not yet available via JSON

These require C++ examples today:

- MMC and generic converters
- Cables, overhead lines, transmission lines
- Switches
- Generic impedance/admittance branches
- RES plants (PV, wind types 3/4)

---

## 5.8 Computations block

After the network is built, the runner executes each entry in `computations`:

| `type` | Description |
|--------|-------------|
| `print_connections` | Print wiring table |
| `network_summary` | AC/DC area breakdown |
| `y_matrix` | CSV frequency sweep; optional `"component_id"` |
| `stability_assessment` | Requires converters in model; optional `"converter_id"`, `"location"` |
| `power_flow` | Requires `"case_name"` matching CSV prefix in `src/data/` |
| `equivalent_impedance`, `dqsym` | Not wired — use C++ API |

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

## 5.9 Validation errors

The builder validates:

- Required top-level sections
- Unique bus and component IDs
- Required fields per component type
- Bus existence when connecting

Errors print to stderr and terminate with a non-zero exit code.

---

## 5.10 When to use JSON vs C++

| Use JSON | Use C++ examples |
|----------|------------------|
| Passive networks and transformers | MMC with custom controllers |
| Repeatable Y-matrix sweeps | DQsym time-domain with breakers |
| Teaching / batch preprocessing | Full OPF hybrid cases with RES |
| Early case prototyping | Stability studies with detailed OPF setup |

[← Building networks](04-building-networks.md) | [Manual index](README.md) | [Next: Component reference →](06-component-reference.md)
