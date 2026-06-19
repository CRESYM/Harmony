# Chapter 4 — Building Networks in C++

[← Core concepts](03-core-concepts.md) | [Manual index](README.md) | [Next: JSON input →](05-json-input.md)

---

## 4.1 Basic pattern

Every programmatic study follows the same steps:

```cpp
#include "network.h"
#include "Bus.h"
#include "Include_components.h"

Network net;

// 1. Create buses (heap-allocated — network takes ownership when added)
Bus* bus1 = new Bus("ACBUS01", "AC1", 3);
Bus* bus2 = new Bus("ACBUS02", "AC1", 3);

// 2. Create elements
Impedance* line = new Impedance("line1", "AC1", 3, std::complex<double>(1.0, 40.0));

// 3. Register and connect
net.connectElementToBus(line, 1, bus1);
net.connectElementToBus(line, 2, bus2);

// 4. Run analysis (see Chapter 7)
```

`connectElementToBus` automatically registers buses and elements if not already present.

---

## 4.2 Two-terminal devices

Branches, transformers, and lines connect **terminal 1** and **terminal 2** to different buses:

```cpp
net.connectElementToBus(transformer, 1, primaryBus);
net.connectElementToBus(transformer, 2, secondaryBus);
```

---

## 4.3 Converters (AC + DC)

Converters use terminal 1 for the **AC side** and terminal 2 for the **DC side**. Set location as `ACx_DCy` (e.g. `AC1_DC1`) so area detection links the converter to both grids:

```cpp
MMC* mmc = new MMC("MMC1", "AC1_DC1", converter_params, controller_params);
net.connectElementToBus(mmc, 1, acBus);
net.connectElementToBus(mmc, 2, dcBus);
```

---

## 4.4 OPF metadata on elements

For optimal power flow, attach economic and limit data before calling `PowerFlow::make_OPF`:

```cpp
std::map<std::string, double> genInfo = {
    {"Pmax", 250.0}, {"Pmin", 0.0},
    {"Qmax", 10.0},  {"Qmin", -10.0},
    {"Vmax", 1.06},  {"Vmin", 0.94},
    {"Ref", 1}       // slack/reference flag
};
source->setOPFInfo(genInfo);
```

Loads, branches, and converters have analogous OPF hooks via `computePowerFlow` overrides.

---

## 4.5 Inspecting the model

```cpp
net.printConnections();   // bus ↔ element list
net.printBuses();
net.printElements();
net.add_areas();
net.print_summary();      // AC/DC areas and converters
```

---

## 4.6 Minimal single-phase RLC example

See `src/examples/example_state_space.cpp` for a grounded AC source, resistor, and capacitor. The pattern:

1. Ground bus `gnd`
2. Source between live bus and ground
3. R and C forming a node between live bus and capacitor bus

After wiring, pass output buses to `StateSpaceModel::formState`.

---

## 4.7 Full hybrid reference case

`src/examples/example_stability_check.cpp` is the **reference template** for:

- Multi-area AC network with generator, loads, and AC branches
- DC network with DC branch
- Two MMC converters coupling AC1↔DC and AC2↔DC
- OPF followed by stability transfer-function analysis

Copy and modify this file when starting a new hybrid study.

---

## 4.8 Common assembly mistakes

| Mistake | Symptom | Fix |
|---------|---------|-----|
| Pin count mismatch | `[WARNING] Pin mismatch` | Match bus `pins` to element phase count |
| Missing second terminal | Open circuit in Y-matrix | Connect terminal 2 for two-port devices |
| Wrong converter location | Converter not in area summary | Use `ACi_DCj` location string |
| Element not in network map | Solver cannot find device | Call `connectElementToBus` or `addElement` |
| Double-delete | Crash on exit | Let `Network` own all `new` objects |

---

## 4.9 Running your network

After assembly, invoke solvers ([Chapter 7](07-analysis-workflows.md)) or run a bundled example that matches your study:

```bash
Harmony --cpp stability_check    # hybrid AC–DC with OPF + stability
Harmony --cpp mmc --no-plot      # MMC Y-matrix sweep
Harmony --json my_case.json      # JSON-defined case
```

See [`../running-harmony.md`](../running-harmony.md).

[← Core concepts](03-core-concepts.md) | [Manual index](README.md) | [Next: JSON input →](05-json-input.md)
