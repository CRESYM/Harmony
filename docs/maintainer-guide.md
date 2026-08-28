# Guide for Maintainers

Audience: people who own **build system**, **CI**, **dependencies**, and **releases**.

Docs map: [`docs/README.md`](README.md).  
Contributor-facing extension tips: [Developer guide](developer-guide.md).

## CMake structure

| Entry | Purpose |
|-------|---------|
| Root `CMakeLists.txt` | `Harmony` and `HarmonyUI` targets |
| `sources.cmake` | Shared source/header lists |
| `cmake/dependencies.cmake` | `link_eigen`, `link_symengine`, `link_sundials`, `link_implot`, `link_gurobi`, … |
| `tests/CMakeLists.txt` | Separate test project (`testharmony`) |
| `-DBUILD_DOCS=ON` | Doxygen HTML ([doxygen/README.md](doxygen/README.md)) |

Gurobi path is always passed as `-DGUROBI_PATH=…` (see [installation.md](installation.md)).

## Adding a dependency on a third-party library

1. Prefer **conda-forge** and add the package to [`environment.yml`](../environment.yml).
2. Add a `link_*` helper in `cmake/dependencies.cmake` and call it from the root / tests CMake files.
3. Document install impact in [installation.md](installation.md) if users must take extra steps (e.g. system OpenGL on Linux).
4. Update CI setup under `.github/actions/setup-env/` if the env change is required on runners.

Prefer documenting the conda path in [installation.md](installation.md); do not revive vcpkg-only flows without an explicit maintainer decision.

## ImPlot / ImGui

Plotting links through `link_implot` in `cmake/dependencies.cmake` (imgui, implot, glfw, OpenGL). Visualization entry points are under `src/Solver/Helper_Functions/Visualization.*` and HarmonyUI (`src/ui/`). User-facing plot behaviour: [Manual Ch. 12](manual/12-harmony-ui.md).

## GitHub Actions pipelines

Workflows under `.github/workflows/`:

| Workflow | Role |
|----------|------|
| `*_build_and_test.yml` | Build + `ctest` excluding `TestExamples` |
| `*_run_examples.yml` | Build + `ctest -R TestExamples` |
| `docs.yml` | Doxygen API docs artifact |

Shared steps: `.github/actions/setup-env`, `build-harmony`, `run-ctest`.

## Updating Gurobi license for GitHub Actions

Secrets used by `.github/actions/setup-env`:

- `GUROBI_WLSACCESSID`
- `GUROBI_WLSSECRET`
- `GUROBI_LICENSEID`

Rotate them in the GitHub repo **Settings → Secrets and variables → Actions**. Keep the installed Gurobi major version in sync with the download URLs in that action and with docs examples (`gurobi1301`, etc.).

## Related documentation

- [Docs index](README.md)
- [Installation](installation.md) · [Running Harmony](running-harmony.md)
- [Developer guide](developer-guide.md) · [User Manual](manual/README.md)
- [Doxygen](doxygen/README.md) · [Documentation roadmap](documentation-plan.md)
