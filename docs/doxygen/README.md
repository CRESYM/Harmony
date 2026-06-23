# Building API Documentation (Doxygen)

Harmony API reference is generated with [Doxygen](https://www.doxygen.org/).

## Prerequisites

With the Harmony conda environment active, Doxygen and Graphviz are already installed via [`environment.yml`](../../environment.yml):

```bash
conda activate harmony
```

To add them to an existing environment without recreating it:

```bash
conda install -c conda-forge doxygen graphviz
```

| Platform | Alternative install |
|----------|---------------------|
| Ubuntu/Debian | `sudo apt install doxygen graphviz` |
| macOS | `brew install doxygen graphviz` |

Graphviz is optional but enables class inheritance diagrams (`HAVE_DOT = YES` in `Doxyfile.in`).

## Generate HTML docs

```bash
conda activate harmony
mkdir build && cd build
cmake .. -DGUROBI_PATH="/path/to/gurobi" -DBUILD_DOCS=ON
cmake --build . --target harmony-docs
```

Open the result in a browser:

```
build/docs/html/index.html
```

## What is documented

| Input | Coverage |
|-------|----------|
| `src/**/*.h` | All public headers — classes, methods, modules |
| `src/**/*.cpp` | `@file` / `@brief` on every translation unit |
| `src/json/` | JSON builder API |
| `docs/doxygen/` | Main page and `@defgroup` module index |

Excluded: `stb_image_write.h`, `tests/`, `build/`.

## Module groups

Defined in `docs/doxygen/groups.dox`:

- `network` — `Network`, `Bus`, `SubNetwork`
- `elements` — device library (`Element` hierarchy)
- `solvers` — OPF, DQsym, state-space, stability, helpers
- `control` — controllers and filters
- `input` — JSON simulation builder
- `examples` — runnable demos

## Related docs

- [**Running Harmony**](running-harmony.md) — command-line usage after build
- [User Manual](manual/README.md) — workflows and tutorials
- [Input file format](input-file-format.md) — JSON schema
