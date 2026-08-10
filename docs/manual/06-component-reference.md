# Chapter 6 — Component Reference

[← JSON input](05-json-input.md) | [Manual index](README.md) | [Next: Analysis workflows →](07-analysis-workflows.md)

---

This chapter summarizes major device types, parameters, and construction patterns. Units are **SI** unless noted. For exact constructor signatures, see source headers under `src/Elements/`.

To **run** studies that use these components, see [`../running-harmony.md`](../running-harmony.md) (`Harmony --cpp …` or `--json …`).

---

## 6.1 Passive elements

### Resistor, inductor, capacitor

```cpp
Resistor("R1", "AC1", pins, { R_per_phase, … });
Inductor("L1", "AC1", pins, { L_per_phase, … });
Capacitor("C1", "AC1", pins, { C_per_phase, … });
```

| Parameter | Unit | Description |
|-----------|------|-------------|
| `pins` | — | Number of phases (1 or 3 typical) |
| `values[i]` | Ω, H, or F | Per-phase element value |

### Generic impedance / admittance

```cpp
Impedance("br1", "AC1", 3, std::complex<double>(R, X));
Impedance("br_dc", "DC1", 2, Rdc);  // DC resistive branch
```

Use for AC series branches and DC links when a single R or R+jX suffices.

---

## 6.2 Sources and loads

### AC source

Ideal voltage source with series impedance:

```cpp
AC_source("SRC01", "AC1", 3, V_nominal, Z_series);
// or
AC_source("SRC01", "AC1", 3, 345e3, std::vector<double>{R, X, …});
```

Attach OPF limits via `setOPFInfo` (see [Chapter 4](04-building-networks.md)).

### Generator

Machine model with internal impedance parameters:

```cpp
Generator("GEN1", "AC1", 3, V, { R_f, L_f, X_d, T_f, X_m, … });
```

### Load / LoadPQ

```cpp
Load("LOAD1", "AC1", 3, { P, Q, … });      // per-phase or aggregated per model
LoadPQ("LOAD1", "AC1", 3, { P, Q, … });
```

---

## 6.3 Transformers

All transformers take a **pin count** (phases per winding) and a **values vector**.

### Real transformer (`Transformer_real`)

Six values: `[R_p, L_p, R_s, L_s, turns_ratio, phase_shift_rad]`

```cpp
Transformer_real("T1", "AC1", 3, { 1.0, 0.04, 1.0, 0.04, 1.0, 0.0 });
```

### Classic transformer (`Transformer_classic` and variants)

Five values: `[R_p, L_p, R_s, L_s, M]`

Topology variants: `TransformerYY`, `TransformerDeltaY`, `TransformerYDelta`, `TransformerDeltaDelta`, plus `_real` counterparts with turns ratio and phase shift.

See `src/examples/example_transformer.cpp` for wiring comparisons.

---

## 6.4 Transmission lines

### Overhead line (`Overhead_Line`)

Requires bundle geometry, conductor data, and ground-wire parameters. See `example_OHL.cpp`.

Key inputs: line length [km], earth `(μ_r, ε_r, ρ)`, conductor organization (flat, vertical, delta, …), ground-wire count and spacing.

### Cable (`Cable`)

Multi-layer underground/aerial cable with conductors and insulators per layer. See `example_cable.cpp`.

### Transmission line (lumped)

`TransmissionLine` — lumped-parameter line model for simpler cases.

---

## 6.5 Switches

```cpp
Switch("SW1", "AC1", pins, parameters);
```

Used primarily in **DQsym** time-domain studies for breaker opening/closing sequences.

---

## 6.6 Modular multilevel converter (MMC)

The most detailed converter model. Constructor variants accept:

1. **Converter parameters** — physical and operating-point values
2. **Controller parameters** — PI gains and limits for each control loop
3. **Filter parameters** — optional measurement filters

### Converter parameter vector (typical order)

| Index | Quantity | Unit | Description |
|-------|----------|------|-------------|
| 0 | ω | rad/s | Nominal angular frequency (e.g. 2π·50) |
| 1 | P | W | Active power setpoint |
| 2 | Q | var | Reactive power setpoint |
| 3 | θ | rad | AC voltage angle |
| 4 | V_m | V | AC voltage magnitude |
| 5 | P_dc | W | DC power |
| 6 | V_dc | V | DC voltage |
| 7 | L_arm | H | Arm inductance |
| 8 | R_arm | Ω | Arm resistance |
| 9 | C_arm | F | Submodule capacitance |
| 10 | N | — | Submodule count |
| 11 | L_reactor | H | Interface reactor inductance |
| 12 | R_reactor | Ω | Interface reactor resistance |
| 13 | t_delay | s | Control delay |

### Controller blocks

Named controllers (in order): `pll`, `dc_voltage`, `active_power`, `ac_voltage`, `reactive_power`, `energy`, `zcc`, `occ`, `ccc`, `droop`.

Each slot begins with an enable flag (`0` = off, `1` = on). Disabled controllers occupy one number; enabled controllers add type (`0` = PI, `1` = P), gains, output dimension, and reference value(s). In JSON, use per-controller enable names (e.g. `"pll_enable": 0.0`, `"active_power_enable": 1.0`) so it is clear which blocks are off — see [`mmc_named_params.json`](../../src/examples/json/mmc_named_params.json).

Each enabled controller consumes a fixed-length parameter sub-vector in `controller_params`. See `example_MMC.cpp` and `example_stability_check.cpp` for tuned sets.

### Typical workflow

```cpp
MMC* mmc = new MMC("MMC1", "AC1_DC1", converter_params, controller_params);
mmc->solveEquilibrium();
mmc->computeABCD();
mmc->checkStability();
mmc->compute_y_parameters(50.0);  // Hz
```

---

## 6.7 Renewable energy sources

| Class | Description | Example |
|-------|-------------|---------|
| `WTtype3` | Doubly-fed induction generator wind turbine | `example_WT_type_3.cpp` |
| `WTtype4` | Full converter wind turbine | `example_WT_type_4.cpp` |
| `PVplant` | PV inverter plant | `example_PV_plant.cpp` |
| `WPplant` | Wind park aggregation | Wraps type 3 or 4 turbine |

All inherit from `RES_base` and support OPF hooks for renewable dispatch studies (`example_OPF_PV.cpp`, `example_OPF_WT.cpp`).

---

## 6.8 Location string conventions

| Pattern | Meaning |
|---------|---------|
| `AC1`, `AC2`, … | AC grid area index |
| `DC1`, `DC2`, … | DC grid area index |
| `AC1_DC1` | Converter linking AC area 1 to DC area 1 |
| `gnd` | Ground reference bus |

Consistency between bus locations and element locations is required for correct area decomposition.

---

## 6.9 Symbolic vs numerical parameters

Many elements store a **symbolic Y-matrix** (SymEngine) internally. At runtime:

- `compute_y_parameters(f_Hz)` substitutes ω = 2πf and returns complex numbers
- `writeFile` sweeps frequency logarithmically
- State-space formation uses MNA stamping from symbolic matrices

[← JSON input](05-json-input.md) | [Manual index](README.md) | [Next: Analysis workflows →](07-analysis-workflows.md)
