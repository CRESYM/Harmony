# Certificates in Harmony

Decentralized **passivity / phase / geometric** device certificates on top of Harmony’s `Y(jω)` and `H = Y Z_eq` pipelines.

Narrative context: [User Manual — Chapter 7 § Certificates](manual/07-analysis-workflows.md#710-device-certificates). Docs map: [`docs/README.md`](README.md).

## Workflows

| # | API | Purpose |
|---|-----|---------|
| 1 | `evaluateDeviceGate` | Local allow/block before interconnect |
| 2 | `PnPCertificateLibrary` | Catalog of gate results (JSON/CSV) |
| 3 | `certifyOperatingRegion` | (P,Q) region / post-OPF setpoint check |
| 4 | `tuneGfmDroops` | GFM droop search to restore certificates |
| 5 | `compareLocalVsSystem` | Local Y vs system `Her(I+H)` benchmark |

Paper-style ImPlot figures: `plot_certificate_*` in `Stability_certificate.h`.

## Geometric metrics

Implemented in `Geometric_certificates.h` and always filled into each `CertificateSweep`:

| Metric | Meaning | Spec flag |
|--------|---------|-----------|
| Excess / shortage passivity | `ν = λ_min(Her Y)`, shortage `max(0,-ν)` | `require_passivity` + `passivity_eps` |
| Eigenphase (heuristic) | `max \|arg λ(Y)\|` | `require_phase` |
| **Numerical-range phase** | `max \|arg z\|` for `z ∈ W(Y)` (small-phase) | `require_nr_phase` |
| **0 ∉ W(Y)** | support-function zero margin | `require_nr_zero_outside` |
| **Small-gain** | `σ_max(Y) ≤ gain_limit` | `require_small_gain` |
| **DW-lite** | 0∉W and NR phase OK (local shell screen) | `require_dw` |
| **Sector / loop-shift** | passivity of `(Y-αI)(βI-Y)^{-1}` | `require_sector` |

Plots: `plot_certificate_numerical_range`, `plot_certificate_dw_shell` (xz projection of sampled DW shell), `plot_certificate_geometric_sweep`.

## Run the demo

```bash
Harmony --cpp certificate_figures
Harmony --cpp certificate_figures --no-plot
```

## Unit tests (geometric certificates)

Build `testharmony` from `tests/`, then:

```bash
testharmony.exe --gtest_filter=GeometricCertificates*
testharmony.exe --gtest_filter=CertificateMetrics*:CertificateWorkflows*:GeometricCertificates*
```

| GTest | Covers |
|-------|--------|
| `GeometricCertificates.*` | NR, DW-lite, small-gain, sector |
| `CertificateMetrics.*` | Classical passivity / shifted |
| `CertificateWorkflows.*` | Device gate + PnP + sweep vectors |

End-to-end demo case (GFM local FAIL vs `I+H` PASS): `certificate_figures`.

Outputs under `files/`: device/system sweeps, operating region CSV, PnP library JSON, tuning before/after.

## Spec

`CertificateSpec` controls frequency grid, AC dq block, phase limit, passivity ε, shifted passivity δ, gain limit, sector `[α,β]`, and which gates are enforced.

## Honesty / scope

- Device checks are **sufficient local screens**, not a replacement for full eigenvalue / Nyquist studies.
- GFM MMCs often **fail** strict passivity; NR phase / shortage maps make that visible vs `H`.
- DW support here is a **sampled shell + NR projection** (DW-lite). It is **not** a full projected-shell interconnection theorem between converter and network DW shells.
- Full SRG composition and dynamic multipliers are still future work.

## Related documentation

- [Docs index](README.md)
- [User Manual Ch. 7 — Analysis workflows](manual/07-analysis-workflows.md)
- [Running Harmony](running-harmony.md) — how to launch `certificate_figures`
- [Examples catalog](manual/08-examples-catalog.md)
- [Installation](installation.md) · [Developer guide](developer-guide.md) · [Doxygen](doxygen/README.md)
