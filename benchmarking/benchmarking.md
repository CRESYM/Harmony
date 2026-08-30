## PowerImpedance.jl

Standalone Y(f) comparisons against the newest
[PowerImpedance.jl](https://github.com/Electa-Git/PowerImpedance.jl) package
(resistor, Y-Y / Δ-Y transformers, aerial cable, overhead line, GFL MMC) live in
[`powerimpedance/`](powerimpedance/). Reproduce with:

```bash
python benchmarking/powerimpedance/run_all.py
```

Latest PowerImpedance.jl (v0.3.0) results on this tree:

| Case | Mean Frobenius rel | Notes |
|------|--------------------|-------|
| 10 Ω resistor | 0 | exact |
| Y-Y transformer | 1.1e-6 | same windings as `example_transformer` |
| Aerial cable | 1.6e-4 | `example_cable` layers |
| OHL | 2.7e-4 | two-bundle flat `example_OHL` |
| GFL MMC | 7.2e-4 | composable API; AC-row sign aligned |
| P2P c1 / c2 $Y_{\mathrm{ac}}$ | 7.4e-4 / 9.4e-4 | Vdc+Q and P+Q |
| P2P $Z_{\mathrm{in}}$ / $H$ at B6 | 5.2e-2 / 0.20 | OPF $V_{\mathrm{ac}}$ still differs |

Overlays (SVG only) and how to regenerate them:
[`powerimpedance/README.md`](powerimpedance/README.md).

## Benchmarking examples:
- Three-phase transformer models as published here:
```
@ARTICLE{11629608,
  author={Lekić, Aleksandra and Alsarayreh, Saif and Dimoulias, Stelios C. and Malamaki, Kyriaki - Nefeli D.},
  journal={IEEE Transactions on Power Delivery}, 
  title={Admittance-Based Modeling and Validation of Three-Phase Transformers in Converter-Dominated Power Systems}, 
  year={2026},
  volume={},
  number={},
  pages={1-4},
  keywords={Modeling;Transformers;Admittance;Matrices;Titanium;Power systems;Voltage;Current;TV;IP networks;Admittance matrix;frequency-domain modeling;harmonic analysis;three-phase transformer modeling},
  doi={10.1109/TPWRD.2026.3718749}}
```
