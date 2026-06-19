# Harmony User Manual

**HARMONic stabilitY assessment of PE-penetrated power systems**

Version: draft (June 2025)  
License: GPL v3  
Project: [CRESYM/Harmony](https://github.com/CRESYM/Harmony)

---

## About this manual

This manual explains how to **install**, **build**, and **use** Harmony for stability and analysis studies on AC–DC power systems with high power-electronics penetration. It is written for researchers and engineers who run studies rather than developers extending the C++ library.

For API-level documentation, see [Building API Documentation (Doxygen)](../doxygen/README.md).

For detailed installation steps with screenshots, see [`../installation.md`](../installation.md).

For **how to run** Harmony after building, see [`../running-harmony.md`](../running-harmony.md), [Chapter 11 — HarmonyUI](11-harmony-ui.md) (graphical launcher), and [Chapter 10 — Command-line interface](10-command-line.md) (CLI).

For the JSON input file specification, see [`../input-file-format.md`](../input-file-format.md).

---

## Table of contents

| Chapter | Title | Description |
|---------|-------|-------------|
| [1](01-introduction.md) | Introduction | Scope, capabilities, and analysis types |
| [2](02-getting-started.md) | Getting started | Install, build, run your first example |
| [3](03-core-concepts.md) | Core concepts | Network, buses, elements, areas |
| [4](04-building-networks.md) | Building networks in C++ | Programmatic model assembly |
| [5](05-json-input.md) | JSON input workflow | File-driven simulations |
| [6](06-component-reference.md) | Component reference | Parameters for major device types |
| [7](07-analysis-workflows.md) | Analysis workflows | OPF, stability, state-space, DQsym |
| [8](08-examples-catalog.md) | Examples catalog | Guide to all bundled examples |
| [9](09-troubleshooting.md) | Troubleshooting | Common errors and fixes |
| [10](10-command-line.md) | Command-line interface | Run examples and JSON cases with `--cpp` / `--json` |
| [11](11-harmony-ui.md) | HarmonyUI | Graphical launcher, embedded plots, PNG export |

---

## Quick reference

| Task | Where to look |
|------|----------------|
| First-time install | [Chapter 2](02-getting-started.md), [`installation.md`](../installation.md) |
| **Run Harmony (GUI)** | [**HarmonyUI**](11-harmony-ui.md), [`running-harmony.md`](../running-harmony.md) |
| **Run Harmony (CLI)** | [**`running-harmony.md`**](../running-harmony.md), [Chapter 10](10-command-line.md) |
| Run a hybrid AC–DC stability study | `Harmony --cpp stability_check`, [Chapter 7](07-analysis-workflows.md) |
| Run optimal power flow | `Harmony --cpp opf`, [Chapter 7](07-analysis-workflows.md) |
| Define a case without C++ | `Harmony --json …`, [Chapter 5](05-json-input.md) |
| MMC converter parameters | [Chapter 6 § Converters](06-component-reference.md#62-modular-multilevel-converter-mmc) |
| Run unit tests | [Chapter 2 § Tests](02-getting-started.md#27-running-the-test-suite) |

---

## How Harmony is organized

```
Harmony/
├── src/                  Main library and examples
│   ├── Elements/         Device models (R, L, C, MMC, transformers, RES, …)
│   ├── Solver/           OPF, DQsym, state-space, stability
│   ├── Control/          PI controllers, filters (used by converters)
│   ├── examples/         Runnable demonstration programs
│   ├── data/             CSV case files for OPF
│   ├── main.cpp          CLI entry point (--cpp / --json)
│   └── ui/               HarmonyUI launcher (harmony_ui_main.cpp)
├── src/json/             JSON builders, validation, computation runner
├── tests/                GoogleTest unit tests
└── docs/                 Installation, running guide, and this manual
    ├── running-harmony.md
    └── installation.md
```

---

## Support and citation

Harmony was developed at Delft University of Technology with support from the [Digital Competence Centre](https://www.tudelft.nl/en/2021/tu-delft/organisatie/faculteiten/electrical-engineering-mathematics-and-computer-science/departments/intelligent-electrical-power-grids/electa/digital-competence-centre).

When publishing results obtained with Harmony, cite the project repository and the underlying methods referenced in the [README](../../README.md#references).
