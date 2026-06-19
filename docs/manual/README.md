# Harmony User Manual

**HARMONic stabilitY assessment of PE-penetrated power systems**

Version: draft (June 2025)  
License: GPL v3  
Project: [CRESYM/Harmony](https://github.com/CRESYM/Harmony)

---

## About this manual

This manual explains how to **install**, **build**, and **use** Harmony for stability and analysis studies on AC–DC power systems with high power-electronics penetration. It is written for researchers and engineers who run studies rather than developers extending the C++ library.

For API-level documentation (classes, methods), see the planned Doxygen reference described in [`../documentation-plan.md`](../documentation-plan.md).

For detailed installation steps with screenshots, see [`../installation.md`](../installation.md).

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

---

## Quick reference

| Task | Where to look |
|------|----------------|
| First-time install | [Chapter 2](02-getting-started.md), [`installation.md`](../installation.md) |
| Run a hybrid AC–DC stability study | [Chapter 7](07-analysis-workflows.md) |
| Run optimal power flow | [Chapter 7](07-analysis-workflows.md) |
| Define a case without C++ | [Chapter 5](05-json-input.md) |
| MMC converter parameters | [Chapter 6 § Converters](06-component-reference.md#62-modular-multilevel-converter-mmc) |
| Run unit tests | [Chapter 2 § Tests](02-getting-started.md#running-the-test-suite) |

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
│   └── main.cpp          Entry point — uncomment one example to run
├── input_file/           JSON-driven network builder (separate executable)
├── tests/                GoogleTest unit tests
└── docs/                 Installation guide and this manual
```

---

## Support and citation

Harmony was developed at Delft University of Technology with support from the [Digital Competence Centre](https://www.tudelft.nl/en/2021/tu-delft/organisatie/faculteiten/electrical-engineering-mathematics-and-computer-science/departments/intelligent-electrical-power-grids/electa/digital-competence-centre).

When publishing results obtained with Harmony, cite the project repository and the underlying methods referenced in the [README](../../README.md#references).
