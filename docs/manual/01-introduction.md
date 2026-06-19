# Chapter 1 — Introduction

[← Manual index](README.md) | [Next: Getting started →](02-getting-started.md)

---

## 1.1 What is Harmony?

Harmony is a C++ simulation and analysis framework for **AC–DC hybrid power systems** where **power electronic (PE) converters** play a dominant role — for example PV and wind plants, batteries, HVDC links, STATCOMs, and modular multilevel converters (MMC).

The name stands for **HARMONic stabilitY assessment of PE-penetrated power systems**.

Traditional phasor tools often struggle with:

- Detailed converter control dynamics
- Harmonic and wideband interactions between converters
- Unified treatment of AC and DC grids in one model
- Impedance-based stability assessment at converter terminals

Harmony addresses these gaps with a component-based model that supports **frequency-domain**, **time-domain (dynamic phasor)**, **state-space**, and **optimization (OPF)** analysis on the same network description.

---

## 1.2 What you can do with Harmony

| Analysis type | Purpose | Typical use |
|---------------|---------|-------------|
| **Y-parameters / admittance** | Frequency response of components and networks | Harmonic stability, resonance screening |
| **Stability assessment** | Equivalent impedance, transfer functions at converter interfaces | Interaction studies between grids and converters |
| **Optimal power flow (OPF)** | AC–DC dispatch with converter constraints | Operating point for dynamic studies |
| **State-space model** | A, B, C, D matrices from network topology | Eigenvalue analysis, participation factors |
| **DQsym (dynamic phasor)** | Time-domain simulation with switching | Transient events, breaker operations |
| **Visualization** | Bode, Nyquist, eigenvalue plots | Result interpretation |

---

## 1.3 System scope

Harmony models:

- **AC grids** — three-phase (or reduced pin count) buses and branches
- **DC grids** — two-conductor buses typical of HVDC networks
- **Converters** — especially MMC with full control chain (PLL, power/voltage controllers, filters)
- **Renewable plants** — wind (type 3 and 4), PV, wind-power plant aggregations
- **Passives** — R, L, C, transformers (ideal and real), transmission lines, cables, overhead lines
- **Sources and loads** — AC/DC sources, generators, PQ loads

Studies are built as a **graph**: buses connected by **elements**. Solvers operate on this shared `Network` object.

---

## 1.4 Two ways to define a study

### A. C++ examples (command line)

Bundled studies live under `src/examples/`. Run them **without recompiling**:

```bash
conda activate harmony
Harmony --list-cpp
Harmony --cpp stability_check
Harmony --cpp mmc --no-plot
```

See [`../running-harmony.md`](../running-harmony.md) and [Chapter 10](10-command-line.md).

### B. JSON input file

Define buses, components, and computations in JSON under `src/json/` and `src/examples/`:

```bash
Harmony --json src/examples/example.json
```

See [Chapter 5](05-json-input.md) and [`../input-file-format.md`](../input-file-format.md).

---

## 1.5 Technical foundations

Harmony builds on prior research and tools:

- Transmission line and MMC base models from PowerImpedanceACDC.jl
- State-space formulation from dc_dc_simulator
- AC–DC OPF from ACDC-OpFlow
- Dynamic phasor core from DQsym

Analysis solvers and stability methods in this repository are original developments integrated into one framework.

---

## 1.6 Prerequisites for users

Before running studies you need:

- A working **Harmony build** (see [Chapter 2](02-getting-started.md))
- **`conda activate harmony`** before each run (see [`../running-harmony.md`](../running-harmony.md))
- **Gurobi** license for OPF studies (free academic licenses available)
- Basic familiarity with power-system concepts (buses, per-unit, converters)
- For plotting examples: display/OpenGL support (ImPlot/GLFW)

You do **not** need to modify C++ code for introductory admittance sweeps if you use the JSON input path with supported component types.

---

## 1.7 Document conventions

- Paths are relative to the repository root unless stated otherwise.
- `Harmony` denotes the main executable (built from `src/main.cpp`; typically `build/Release/Harmony.exe` on Windows or `build/Harmony` on Linux/macOS).
- Run from the **repository root** with `conda activate harmony`. See [`../running-harmony.md`](../running-harmony.md).
- Code blocks show bash commands for Linux/macOS; on Windows use `build\Release\Harmony.exe` and backslashes where needed.

[← Manual index](README.md) | [Next: Getting started →](02-getting-started.md)
