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

Graphviz is required for class inheritance diagrams (`HAVE_DOT = YES` in `Doxyfile.in`).

## Generate HTML docs locally

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

Equivalent without CMake (same output as CI):

```bash
conda activate harmony
root="$(pwd)"   # Harmony repository root
out="${root}/build/docs"
mkdir -p build
sed -e "s|@CMAKE_SOURCE_DIR@|${root}|g" \
    -e "s|@DOXYGEN_OUTPUT_DIR@|${out}|g" \
    docs/Doxyfile.in > build/Doxyfile
doxygen build/Doxyfile
```

## CI build (this repository)

Workflow [`.github/workflows/docs.yml`](../../.github/workflows/docs.yml) runs on pushes to `main` / `develop`, on pull requests, and manually (**Actions → Build API docs**).

It verifies that Doxygen + Graphviz succeed and uploads the HTML folder as artifact **`harmony-api-docs`** (30 days). **This repo does not deploy GitHub Pages** — the public site lives in your separate Pages project.

## Publishing to your GitHub Pages project

Keep your existing Pages repository unchanged. When you want to refresh the online API reference:

1. **Get fresh HTML** — either:
   - Download **`harmony-api-docs`** from a green [Build API docs](https://github.com/CRESYM/Harmony/actions/workflows/docs.yml) run, or
   - Build locally (`build/docs/html/` as above).
2. **Copy into the Pages repo** — replace the API section (or the whole site tree) with the contents of `html/` (include hidden `.nojekyll` if your host requires it).
3. **Commit and push** the Pages repository as you do today.

Typical layout (adjust to match your Pages repo):

```text
your-pages-repo/
  index.html          # your existing landing page
  manual/             # user manual, if hosted there
  api/                # copy contents of build/docs/html/ here
      index.html
      ...
```

If the site is served from a subpath (e.g. `https://example.github.io/Harmony/api/`), ensure links still work: Doxygen uses relative URLs by default, so copying the whole `html/` tree into that folder is usually enough.

Optional: add a workflow **in the Pages repo** that checks out Harmony, runs the same `doxygen` step, and commits to `gh-pages` — only if you want fully automated sync; manual copy from the CI artifact is fine.

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
