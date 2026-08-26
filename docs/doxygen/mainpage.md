# Harmony API Reference {#mainpage}

**HARMONic stabilitY assessment of PE-penetrated power systems**

Harmony is a C++17 framework for modeling and analyzing AC–DC hybrid power systems
with high power-electronics penetration.

## Modules

- @ref network — Graph model (`Network`, `Bus`, `SubNetwork`)
- @ref elements — Device library (`Element` hierarchy)
- @ref solvers — Analysis engines (OPF, DQsym, state-space, stability)
- @ref control — Controllers and filters for converters
- @ref input — JSON-driven network builder

## User documentation

- [**Documentation index**](../README.md) — map of all docs
- [**Running Harmony**](../running-harmony.md) — command-line usage (`--cpp`, `--json`)
- [User Manual](../manual/README.md) — installation, workflows, and examples
- [JSON input format](../input-file-format.md) — schema for `--json`
- [Certificates](../certificates.md) — passivity / geometric screens

## License

GPL v3 — see LICENSE in the repository root.
