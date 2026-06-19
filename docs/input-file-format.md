# JSON Input File Format

Harmony can build a `Network` from a JSON simulation file using the main executable.

## Usage

Build Harmony from the repository root (see [`installation.md`](installation.md) or [User Manual Chapter 2](manual/02-getting-started.md)). Then run from the **repository root**:

```bash
conda activate harmony

# Windows
build\Release\Harmony.exe --json path/to/simulation.json
build\Release\Harmony.exe --json example.json --verbose
build\Release\Harmony.exe --list-json

# Linux / macOS
./build/Harmony --json path/to/simulation.json
./build/Harmony --json example.json --verbose
```

Full run instructions: [**Running Harmony**](running-harmony.md). By default Harmony searches `src/examples`, `src/examples/json`, `src/json`, `examples`, and `.`. Override with the `HARMONY_JSON_PATH` environment variable or `--json-path`; append with `--search-path`. CLI details: [User Manual Chapter 10](manual/10-command-line.md).

## Top-level structure

| Section | Required | Description |
|---------|----------|-------------|
| `simulation` | yes | Metadata and default analysis settings |
| `buses` | yes | Electrical nodes |
| `components` | yes | Devices connected to buses |
| `computations` | no | Post-build analysis steps |

See [`src/examples/example.json`](../src/examples/example.json) for a working minimal case.

## `simulation` object

| Field | Type | Description |
|-------|------|-------------|
| `title` | string | Simulation name |
| `description` | string | Free-text description |
| `output_directory` | string | Directory for CSV/plot output (default: `./files`) |
| `frequency_range` | object | Default `{start, end, step}` or `{start, end, points}` for Y-matrix sweeps |
| `nominal_power` | number | Base MVA for OPF (default 100) |
| `nominal_voltage` | number | Base kV for OPF (default 345) |
| `dc_nominal_voltage` | number | DC base kV for hybrid OPF |

## `buses` array

Each bus entry:

```json
{
  "id": "bus1",
  "location": "AC1",
  "pins": 3,
  "enabled": true
}
```

- `location` — grid area label (`AC1`, `DC1`, …) used by stability area detection
- `pins` — number of phases / conductors
- `enabled: false` skips the bus

## `components` array

Common fields for all components:

| Field | Required | Description |
|-------|----------|-------------|
| `id` | yes | Unique designator |
| `type` | yes | See supported types below |
| `location` | yes | Area tag (`AC1`, `AC1_DC1` for converters, …) |
| `pins` | yes | Pin count (must match connected bus pin counts) |
| `values` | usually | Numeric array or object (transformers) |
| `connected_bus` | * | Single `{bus_id, terminal}` connection |
| `connected_buses` | * | Array of connections (two-terminal devices) |
| `enabled` | no | Default `true` |

At least one connection form is required for the component to be wired into the network.

### Supported component types

| `type` | Notes |
|--------|-------|
| `load`, `load_pq` | `values`: `[P, Q, …]` per phase |
| `ac_source`, `generator` | Requires `voltage` + `values` |
| `resistor`, `inductor`, `capacitor` | `values`: per-phase parameters |
| `transformer_real` | `values`: `{R_primary, L_primary, R_secondary, L_secondary, turns_ratio, phase_shift}` |
| `transformer_classic`, `transformer_yy`, `transformer_deltay`, … | `values`: `{R_primary, L_primary, R_secondary, L_secondary, M}` |
| `dc_source` | `voltage` (number or array); optional `resistance` / `values` |
| `impedance`, `admittance` | Numeric `values` array |
| `switch` | `state` (bool array) or `closed` (bool) |
| `transmission_line` | `values`: `[R, L, G, C, length]` |
| `cable` | `cable_type`, `length`, `earth`, `conductors`, `insulators`, `positions` |
| `overhead_line` | `length_km`, `earth`, `conductor`, `groundwire` |
| `mmc` | `converter_params`; optional `controller_params`, `filter_params` |
| `wt_type_3`, `wt_type_4`, `pv_plant` | `parameters` array |
| `wp_plant` | `turbine_type`, `number_wt`, `parameters` |

Unknown keys are rejected (strict validation at load time).

## `computations` array

Each entry has a `type` field (case-insensitive).

| Type | Description |
|------|-------------|
| `print_connections` | Print bus–element wiring |
| `network_summary` | Print AC/DC area summary |
| `y_matrix` (`y_matrx`) | Frequency sweep to CSV. Optional `component_id`, `frequency_range` |
| `stability_assessment` | `StabilityEstimate` after `add_areas`. Optional: `converter_id`, `location`, `frequency_range` |
| `power_flow` / `opf` | Requires `case_name` matching CSV prefix under `src/data/` |
| `time_domain` / `dqsym` | DQsym time-domain run (`dt`, `t_end`, `frequency`, `output_bus_ids`, …) |
| `equivalent_impedance` | Not wired — use C++ API |

## See also

- [User Manual — Command-line interface](manual/10-command-line.md)
- [User Manual — JSON workflow](manual/05-json-input.md)
- Build and install: [`installation.md`](installation.md)
