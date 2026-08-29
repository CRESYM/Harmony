# Documentation roadmap

The live documentation map is **[`docs/README.md`](README.md)**. Use that page (and the [User Manual](manual/README.md)) as the entry point.

## Done

| Item | Location |
|------|----------|
| Docs hub / index | [`README.md`](README.md) |
| User manual (Ch. 1–12) | [`manual/`](manual/README.md) |
| Installation guide | [`installation.md`](installation.md) |
| Running guide (GUI + CLI) | [`running-harmony.md`](running-harmony.md) |
| JSON field reference | [`input-file-format.md`](input-file-format.md) |
| Certificates reference | [`manual/08-certificates.md`](manual/08-certificates.md) |
| Doxygen API docs | [`doxygen/README.md`](doxygen/README.md) · CI: `.github/workflows/docs.yml` |

## Remaining gaps

1. Flesh out [developer-guide.md](developer-guide.md) (add element / solver / test checklists).
2. Flesh out [maintainer-guide.md](maintainer-guide.md) (CMake, Gurobi Actions secrets, ImPlot notes).
3. Optional: JSON Schema file for editor validation; PDF export of the manual.
4. Expand manual figures / MMC parameter tables where examples still rely on source only.

Current installs use conda — see [`installation.md`](installation.md).
