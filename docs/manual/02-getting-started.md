# Chapter 2 — Getting Started

[← Introduction](01-introduction.md) | [Manual index](README.md) | [Next: Core concepts →](03-core-concepts.md)

---

## 2.1 Overview

This chapter walks through installing Harmony, building it, running an example, and running the test suite. Detailed platform-specific instructions with screenshots are in [`../installation.md`](../installation.md).

Supported platforms: **Windows**, **Linux** (Ubuntu 22.04+ tested), **macOS** (Sequoia tested).

---

## 2.2 Prerequisites

| Requirement | Purpose |
|-------------|---------|
| C++17 compiler | MSVC 2022+ (Windows), GCC 11+ (Linux), Clang (macOS) |
| [miniconda](https://docs.anaconda.com/miniconda/) | Dependency management via `environment.yml` |
| [CMake](https://cmake.org/) 4.0+ | Build system |
| [Gurobi Optimizer](https://www.gurobi.com/) | OPF solver (license required) |
| Eigen, SymEngine, SUNDIALS, ImPlot | Installed via conda (see `environment.yml`) |

> **Tip:** When installing miniconda, enable **Add to PATH** so Visual Studio and terminals find `conda`.

---

## 2.3 Clone and create the environment

```bash
git clone https://github.com/CRESYM/Harmony.git
cd Harmony

conda env create -f environment.yml
conda activate harmony
```

Verify the prompt shows `(harmony)`.

---

## 2.4 Build the main executable

```bash
mkdir build
cd build
cmake .. -DGUROBI_PATH="/path/to/gurobi"
cmake --build . --config Release -j 4
```

**Gurobi path examples:**

| OS | Typical path |
|----|--------------|
| Windows | `C:\gurobi1202\win64` |
| Linux | `/opt/gurobi1301/linux64` |
| macOS | `/Library/gurobi1200/macos_universal2` |

On Windows you may also open `build/Harmony.sln` in Visual Studio, set **Harmony** as startup project, and build Release.

---

## 2.5 Run your first example

Harmony does not yet expose a command-line interface for analysis selection on the main executable. You choose an example in source code:

1. Open `src/main.cpp`.
2. Uncomment one example function. For a full hybrid stability demo:

   ```cpp
   example_stability_check(true);
   ```

3. Rebuild and run:

   ```bash
   cd build/Release    # Windows
   ./Harmony
   ```

The `true` argument enables plotting windows where the example supports it.

**Expected behavior for `example_stability_check`:**

- Builds an AC–DC network with loads, branches, and two MMC converters
- Runs OPF to obtain an operating point
- Computes transfer functions and optionally shows Bode/Nyquist plots
- Writes Y-parameter data under `./files/` when applicable

---

## 2.6 Output files

Many examples write CSV frequency sweeps to:

```
./files/<element_id>.csv
```

Create this folder if an example fails to write (the test CMake target creates it automatically). Each row contains frequency and complex Y-matrix entries.

---

## 2.7 Running the test suite

Unit tests use GoogleTest and link the full library (excluding `main.cpp`):

```bash
cd tests
mkdir build && cd build
cmake .. -DGUROBI_PATH="/path/to/gurobi"
cmake --build . --config Release
ctest
```

Tests cover network wiring, individual elements, MMC Y-matrices, state-space formation, and stability estimation.

---

## 2.8 Build the JSON input executable (optional)

For file-driven cases without editing `main.cpp`:

```bash
cd input_file
mkdir build && cd build
cmake .. -DGUROBI_PATH="/path/to/gurobi"
cmake --build . --config Release
./Release/Harmony.exe ../src/examples/example.json
```

See [Chapter 5](05-json-input.md).

---

## 2.9 Recommended learning path

| Step | Action |
|------|--------|
| 1 | Build successfully and run `example_constructors` or `example_transformer` |
| 2 | Read [Chapter 3 — Core concepts](03-core-concepts.md) |
| 3 | Run `example_MMC` to inspect a single converter |
| 4 | Run `example_stability_check` for a full AC–DC study |
| 5 | Try JSON input with `src/examples/example.json` |
| 6 | Explore OPF examples if you need operating points |

---

## 2.10 Regenerating the build after CMake changes

Regenerate the build directory when you:

- Add or remove `.cpp` / `.h` files (update `sources.cmake`)
- Change `CMakeLists.txt` or dependencies

```bash
rm -rf build    # or delete the folder on Windows
mkdir build && cd build
cmake .. -DGUROBI_PATH="..."
cmake --build . --config Release
```

---

## 2.11 Generate API documentation (optional)

Harmony source includes Doxygen comments on all headers and `.cpp` files. Build HTML reference:

```bash
cmake .. -DGUROBI_PATH="/path/to/gurobi" -DBUILD_DOCS=ON
cmake --build . --target harmony-docs
```

Open `build/docs/html/index.html`. Requires Doxygen and optionally Graphviz — both are listed in [`environment.yml`](../../environment.yml). See [`../doxygen/README.md`](../doxygen/README.md).

[← Introduction](01-introduction.md) | [Manual index](README.md) | [Next: Core concepts →](03-core-concepts.md)
