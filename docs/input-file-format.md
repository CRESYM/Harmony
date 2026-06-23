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
| `parameters` | no | Named numeric constants referenced from components |

See [`src/examples/example.json`](../src/examples/example.json) for a working minimal case. See [`src/examples/json/mmc_named_params.json`](../src/examples/json/mmc_named_params.json) for named MMC/ controller parameters.

## Named parameters (Type A)

Define reusable numbers once at the top of the file:

```json
"parameters": {
  "omega_0": 314.159,
  "V_dc": 200000.0,
  "zcc_kp": 19.93,
  "zcc_ki": 4500.0
}
```

Reference them anywhere a **numeric array** or scalar is accepted — passives `"values"`, MMC `converter_params`, `controller_params`, `filter_params`, RES `"parameters"`, transformer object fields, `voltage`, etc.:

```json
"values": ["R_ohm"],
"converter_params": ["omega_0", "P_nom", "Q_nom", ...],
"controller_params": ["pll_enable", "dc_voltage_enable", "active_power_enable", "pi_type", "pac_kp", "pac_ki", "n_outputs_1", "pac_ref", "ac_voltage_enable", "reactive_power_enable", ...]
```

MMC controllers are fixed-order slots: `pll`, `dc_voltage`, `active_power`, `ac_voltage`, `reactive_power`, `energy`, `zcc`, `occ`, `ccc`, `droop`. Each slot starts with an enable flag (`0`/`1` or a named parameter such as `"ac_voltage_enable": 0.0`). Disabled slots use a single entry; enabled slots continue with controller type, gains, output count, and references. See [`mmc_named_params.json`](../src/examples/json/mmc_named_params.json).

Rules:

- Parameter names are strings; array entries are either a **number** or a **parameter name**.
- Unknown names are rejected at validation.
- Optional per-component overrides: `"local_parameters": { "R_val": 99.0 }` (merged over root `parameters` for that component only).
- RES components keep the field name `"parameters"` for the plant vector; root definitions use the top-level `"parameters"` object (different JSON levels, no conflict).

Literal numbers still work — existing JSON files are unchanged.

## SymEngine expressions (Type B)

Define frequency-domain admittance or impedance with SymEngine strings. Numeric names from `"parameters"` are substituted; **`w`**, **`omega`**, and **`s`** stay symbolic for Y-matrix sweeps (`s = j*w` in Harmony).

| Field | Component types | Meaning |
|-------|-----------------|--------|
| `y_expr` | `resistor`, `inductor`, `capacitor`, `admittance` | Admittance \(Y(s)\) — passives are built as symbolic `admittance` internally |
| `y_exprs` | `admittance` | Array of \(Y(s)\) expressions (same layouts as numeric `values`) |
| `z_expr` | `impedance` | Impedance \(Z(s)\); element stores \(Y = 1/Z\) |

Examples:

```json
"parameters": { "R_ohm": 10.0, "L_H": 0.001, "C_F": 1e-6 },
"components": [
  { "type": "resistor", "y_expr": "1/R_ohm", ... },
  { "type": "inductor", "y_expr": "1/(s*L_H)", ... },
  { "type": "capacitor", "y_expr": "s*C_F", ... },
  { "type": "impedance", "z_expr": "R_ohm + s*L_H", ... },
  { "type": "admittance", "y_expr": "s*C_F + 1/R_ohm", ... }
]
```

Rules:

- Use **`values`** *or* expression fields — not both on the same component.
- Allowed symbols after substitution: **`w`**, **`omega`**, **`s`** only.
- Parser constants: `w`, `omega`, `s`, `j`, `pi`, `I` (imaginary unit).
- See [`src/examples/json/passives_rlc_expr.json`](../src/examples/json/passives_rlc_expr.json).

MMC / RES / controller vectors remain **Type A** (numeric arrays and name references). Structured controller objects are not part of Type B.

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
| `location` | yes* | Area tag (`AC1`, `AC1_DC1` for converters, …) |
| `pins` | yes* | Phase/conductor count; must match connected bus `pins` when required |
| `values` | usually | Numeric array or object (transformers) |
| `connected_bus` | * | Single `{bus_id, terminal}` connection |
| `connected_buses` | * | Array of connections (two-terminal devices) |
| `enabled` | no | Default `true` |

At least one connection form is required for the component to be wired into the network.

\* **`pins` not used in JSON** for types whose C++ model fixes the pin count internally: `overhead_line` (1), `mmc` (3 AC / 2 DC), `wt_type_3`, `wt_type_4`, `pv_plant`, `wp_plant` (3). **`cable`**: optional (`pins` defaults to `1`). **`location`**: optional for `overhead_line` (defaults to `AC1`).

For all other types, `pins` on the component must match the connected bus pin count.

### Supported component types

| `type` | Notes |
|--------|-------|
| `load`, `load_pq` | `values`: `[P, Q, …]` per phase |
| `ac_source`, `generator` | Requires `voltage` + `values`; optional `opf_info` for built-network OPF |
| `resistor`, `inductor`, `capacitor` | `values`: per-phase parameters |
| `transformer_real` | `values`: `{R_primary, L_primary, R_secondary, L_secondary, turns_ratio, phase_shift}` |
| `transformer_classic`, `transformer_yy`, `transformer_deltay`, … | `values`: `{R_primary, L_primary, R_secondary, L_secondary, M}` |
| `dc_source` | `voltage` (number or array); optional `resistance` / `values` |
| `impedance`, `admittance` | Numeric `values` array |
| `switch` | `state` (bool array) or `closed` (bool) |
| `transmission_line` | `values`: `[R, L, G, C, length]` |
| `cable` | `cable_type`, `length`, `earth`, `conductors`, `insulators`, `positions`; optional `pins` (default 1) |
| `overhead_line` | `length_km`, `earth`, `conductor`, `groundwire` — no `pins` |
| `mmc` | `converter_params`; optional `controller_params`, `filter_params` — no `pins` |
| `wt_type_3`, `wt_type_4`, `pv_plant` | `parameters` — no `pins` |
| `wp_plant` | `turbine_type`, `number_wt`, `parameters` |

Unknown keys are rejected (strict validation at load time).

## `computations` array

Each entry has a `type` field (case-insensitive).

| Type | Description |
|------|-------------|
| `print_connections` | Print bus–element wiring |
| `network_summary` | Print AC/DC area summary |
| `y_matrix` (`y_matrx`) | Frequency sweep to CSV. Optional `component_id`, `frequency_range`, `"plot": true` for Bode GUI |
| `stability_assessment` | `StabilityEstimate` after `add_areas`. Optional: `converter_id`, `location`, `frequency_range`, `"plot": true`, `"plot_type": "bode"` or `"nyquist"` |
| `power_flow` / `opf` | **CSV mode:** `"case_name"` (AC prefix) + optional `"dc_case_name"` → calls `solve_opf` like `example_OPF_csv` (network components ignored). **Built-network mode:** omit `case_name` → `make_OPF` on JSON components (like `stability_check`). Optional `"plot_result": true` |
| `time_domain` / `dqsym` | DQsym time-domain run (`dt`, `t_end`, `frequency`, `output_bus_ids`, …). Optional `"plot": true` for ABC waveforms |
| `equivalent_impedance` | Not wired — use C++ API |

JSON plot flags are honored only when the CLI is run without `--no-plot`.

## See also

- [User Manual — Command-line interface](manual/10-command-line.md)
- [User Manual — JSON workflow](manual/05-json-input.md)
- Build and install: [`installation.md`](installation.md)
