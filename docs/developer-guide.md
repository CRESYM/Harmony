# Guides for Developers

Audience: people adding **elements**, **solvers**, **tests**, or **examples** to Harmony.

Start here for the wider docs map: [`docs/README.md`](README.md).  
For class/method detail: [Doxygen API docs](doxygen/README.md).  
For end-user workflows: [User Manual](manual/README.md).

## Repository structure

| Path | Role |
|------|------|
| `src/` | Library, CLI (`main.cpp`, `cli.cpp`), UI (`src/ui/`), examples |
| `src/Elements/` | Device models (`Element` hierarchy) |
| `src/Solver/` | OPF, DQsym, state-space, stability, certificates, visualization |
| `src/Control/` | Controllers / filters used by converters |
| `src/json/` | JSON builders, validation, computation runner |
| `sources.cmake` | Lists of sources / headers for CMake and VS filters |
| `cmake/dependencies.cmake` | Third-party link helpers (Eigen, SymEngine, …) |
| `tests/` | GoogleTest suite (separate CMake project) |
| `docs/` | User + developer documentation |

## Adding new elements and solvers

1. Implement under `src/Elements/` or `src/Solver/` following nearby types.
2. Register headers/sources in [`sources.cmake`](../sources.cmake).
3. If the type should be loadable from JSON, extend `src/json/` builders + [`input-file-format.md`](input-file-format.md).
4. Prefer a small example under `src/examples/` and a unit test under `tests/`.
5. Document the workflow in the [User Manual](manual/README.md) when it is user-facing.

API comments: use Doxygen `/** … */` so they appear in the generated reference ([doxygen/README.md](doxygen/README.md)).

## Adding tests

1. Add `tests/test*.cpp` and list the file in `tests/CMakeLists.txt`.
2. Build from `tests/build` as in [installation.md — Testing](installation.md#testing) / [Manual Ch. 2](manual/02-getting-started.md#27-running-the-test-suite).
3. Keep examples gated behind `TestExamples` where CI splits unit vs example runs.

## Adding examples

1. Add `src/examples/example_*.cpp` and declare it in [`sources.cmake`](../sources.cmake) + `Examples.h`.
2. Register the CLI name in `src/cli.cpp` (see existing `--cpp` entries).
3. Optionally add a mirror JSON under `src/examples/json/`.
4. List it in [Manual Ch. 8](manual/08-examples-catalog.md).

## Related documentation

- [Docs index](README.md)
- [Installation](installation.md) · [Running](running-harmony.md)
- [Maintainer guide](maintainer-guide.md) — CMake, CI, dependencies
