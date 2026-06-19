# Chapter 3 — Core Concepts

[← Getting started](02-getting-started.md) | [Manual index](README.md) | [Next: Building networks →](04-building-networks.md)

---

## 3.1 The network graph

Every Harmony study centers on a **`Network`** object:

- **`Bus`** — electrical node with a name, location tag, and pin count (phases or conductors)
- **`Element`** — device connected to one or more buses at specific terminals
- **Connections** — stored internally when you call `connectElementToBus`

```
Network
 ├── buses:     { "ACBUS01" → Bus*, "DCBUS01" → Bus*, … }
 ├── elements:  { "MMC1" → Element*, "br1_ac" → Element*, … }
 └── connections: { Bus* → [Element*, …] }
```

The `Network` **owns** buses and elements allocated with `new`. When the network is destroyed, it deletes them. Always transfer ownership to the network via `addBus` / `addElement` / `connectElementToBus`.

---

## 3.2 Bus locations and areas

Each bus carries a **location string** such as `AC1`, `AC2`, or `DC1`. This tag:

- Identifies which **AC or DC area** the bus belongs to
- Is used by `Network::add_areas()` to build **SubNetwork** views for stability analysis
- Should match the location of elements connected to that bus (pin-count warnings appear on mismatch)

A bus named `gnd` (case-insensitive) is treated as **ground** and skips pin-count validation.

---

## 3.3 Element pins

Elements declare **input** and **output** pin counts:

| Device class | Typical pins | Notes |
|--------------|--------------|-------|
| Three-phase AC branch | 3 in, 3 out | R, L, C, line, transformer |
| DC branch | 2 in, 2 out | DC impedance |
| Converter (MMC) | 3 AC, 2 DC | Terminal 1 = AC side, terminal 2 = DC side |
| Load | 3 | Often single-terminal connection |

Pin count on each bus must match the element terminal unless the bus is ground.

---

## 3.4 Y-parameter matrices

Most elements expose a symbolic or numerical **admittance matrix** `Y(ω)`:

- Computed via `compute_y_parameters(frequency)` in Hz
- Written to CSV via `writeFile(start_Hz, end_Hz, num_points)`
- Plotted via `plotYParameters(...)` when ImPlot is available

For AC elements in dq-frame studies, a **transformation flag** enables dynamic-phasor sideband modeling (see DQsym workflow in [Chapter 7](07-analysis-workflows.md)).

---

## 3.5 SubNetworks and hierarchical areas

Calling `network.add_areas()`:

1. Groups buses by location (`AC1`, `DC1`, …)
2. Creates **SubNetwork** objects (non-owning views of the same buses/elements)
3. Detects **converters** and registers their AC/DC interface buses as SubNetwork outputs

This hierarchy feeds **StabilityEstimate**, which computes equivalent admittances and transfer functions at converter boundaries.

---

## 3.6 Solvers overview

| Solver | Class | Input | Output |
|--------|-------|-------|--------|
| State-space | `StateSpaceModel` | Network + output buses | A, B, C, D matrices |
| Dynamic phasor | `DQsym` | Network + time config | Time histories |
| Stability | `StabilityEstimate` | Network with areas | TF, Bode, Nyquist |
| OPF | `PowerFlow` | Network + CSV case data | Dispatch, voltages |

Solvers read the same `Network`; they do not duplicate component data.

---

## 3.7 Control blocks (converters)

Converters — especially **MMC** — embed **controllers** and **filters**:

- PI controllers for PLL, active/reactive power, DC voltage, etc.
- Low-pass and band-pass filters on measured quantities

Controller parameters are passed as numeric vectors at construction time. See [Chapter 6](06-component-reference.md).

---

## 3.8 Ownership rules (important)

| Object | Owner |
|--------|-------|
| `Bus*`, `Element*` in a `Network` | `Network` destructor |
| `SubNetwork*` from `add_areas()` | Parent `Network` (SubNetworks are non-owning views) |
| Controllers/filters inside MMC | Parent `Converter` / `MMC` |
| `StabilityEstimate`, `PowerFlow`, `DQsym` | Stack or user scope; hold non-owning pointers to network |

Do not `delete` a bus or element still registered in a network. Use `deleteElement` / `deleteBus` if you need to remove items during assembly.

[← Getting started](02-getting-started.md) | [Manual index](README.md) | [Next: Building networks →](04-building-networks.md)
