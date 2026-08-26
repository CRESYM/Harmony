# Harmony documentation

This folder holds all project documentation. Start from the role that matches what you need.

| Role | Start here |
|------|------------|
| **New user** — install, build, first run | [User Manual](manual/README.md) → [Chapter 2](manual/02-getting-started.md) |
| **Run studies** (GUI / CLI) | [Running Harmony](running-harmony.md) · [HarmonyUI](manual/11-harmony-ui.md) · [CLI](manual/10-command-line.md) |
| **JSON cases** | [Manual Ch. 5](manual/05-json-input.md) · [JSON field reference](input-file-format.md) |
| **Workflows** (OPF, stability, certificates, …) | [Manual Ch. 7](manual/07-analysis-workflows.md) · [Certificates](certificates.md) |
| **C++ API** | [Doxygen](doxygen/README.md) |
| **Extend Harmony** | [Developer guide](developer-guide.md) |
| **CI / packaging / deps** | [Maintainer guide](maintainer-guide.md) |

Repository overview and quick build: [README](../README.md) at the project root.

---

## How the pieces fit together

```text
README.md (repo)          short overview + link here
    │
docs/README.md            ← you are here (index)
    │
    ├── manual/           narrative user guide (chapters 1–11)
    │     └── points to → installation, running, JSON reference, certificates
    │
    ├── installation.md   detailed install / build / test (screenshots)
    ├── running-harmony.md  conda, paths, GUI + CLI how-to
    ├── input-file-format.md  full JSON field reference
    ├── certificates.md   passivity / geometric certificate APIs
    │
    ├── developer-guide.md
    ├── maintainer-guide.md
    └── doxygen/          API HTML (build with -DBUILD_DOCS=ON)
```

**Division of labour**

| Document | Purpose | Avoids duplicating |
|----------|---------|-------------------|
| [User Manual](manual/README.md) | Tutorials, concepts, workflows, examples | Deep platform install detail |
| [installation.md](installation.md) | Prerequisites, cmake, conda, tests | Study workflows |
| [running-harmony.md](running-harmony.md) | Day-to-day run recipes | Component theory |
| [input-file-format.md](input-file-format.md) | Schema / every JSON key | Why / when to use JSON |
| [certificates.md](certificates.md) | Certificate APIs, metrics, tests | Full stability tutorial |
| [doxygen/](doxygen/README.md) | Class / method reference | User tutorials |

---

## User Manual chapters

| Ch. | Topic |
|-----|--------|
| [1](manual/01-introduction.md) | Introduction |
| [2](manual/02-getting-started.md) | Getting started |
| [3](manual/03-core-concepts.md) | Core concepts |
| [4](manual/04-building-networks.md) | Building networks in C++ |
| [5](manual/05-json-input.md) | JSON input workflow |
| [6](manual/06-component-reference.md) | Component reference |
| [7](manual/07-analysis-workflows.md) | Analysis workflows |
| [8](manual/08-examples-catalog.md) | Examples catalog |
| [9](manual/09-troubleshooting.md) | Troubleshooting |
| [10](manual/10-command-line.md) | Command-line interface |
| [11](manual/11-harmony-ui.md) | HarmonyUI |

Full TOC and quick-reference table: [manual/README.md](manual/README.md).

---

## Related elsewhere in the repo

| Path | Content |
|------|---------|
| [`benchmarking/`](../benchmarking/) | MMC / external-tool comparison notes |
| [`documentation-plan.md`](documentation-plan.md) | Short roadmap of remaining doc gaps |
