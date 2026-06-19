# Documentation Plan for Harmony

## Current state

- **Installation guide** — [`installation.md`](installation.md)
- **JSON input format** — [`input-file-format.md`](input-file-format.md)
- **Doxygen-style comments** — partial (`@brief` in solvers, some elements); no `Doxyfile`
- **User manual PDF** — referenced in README/installer but **not in this repository**
- **Class diagram** — [`Harmony.cd`](Harmony.cd)

## Documentation types I can add

### Doxygen API reference

Best for developers integrating or extending Harmony.

- Class/method reference from `src/` and `input_file/`
- Module pages: Elements, Solvers, Control, Network
- Inheritance diagrams for `Element` hierarchy
- CMake `doxygen_add_docs` target + optional GitHub Pages CI

### User manual (Markdown → PDF or web)

Best for end users running studies.

Suggested chapters:

1. Introduction and scope
2. Installation (link to `installation.md`)
3. Running examples and tests
4. JSON input workflow (`input-file-format.md`)
5. OPF from CSV case files
6. Stability assessment workflow
7. DQsym time-domain simulation
8. Component parameter reference (MMC, transformers, RES)
9. Troubleshooting (Gurobi, conda, pin mismatches)

### Other useful docs

- **JSON Schema** — validate input files at parse time
- **Example gallery** — one page per `example_*.cpp`
- **CONTRIBUTING.md** — build, test, code style
- **CHANGELOG.md** — release history

## Recommended priority

1. JSON input docs — [`input-file-format.md`](input-file-format.md) (done)
2. **User manual draft** — [`manual/README.md`](manual/README.md) (done)
3. **Doxygen API docs** — [`doxygen/README.md`](doxygen/README.md) (done — build with `-DBUILD_DOCS=ON`)
4. Expand manual (figures, MMC parameter tables, PDF export)
5. JSON Schema + MMC/RES JSON builders

Ask which items to implement next.
