## PowerImpedance.jl

Standalone $Y(f)$, OHL/cable topologies, and P2P HVDC vs
[PowerImpedance.jl](https://github.com/Electa-Git/PowerImpedance.jl) 0.3.0:
[`powerimpedance/`](powerimpedance/).

```
powerimpedance/standalone/   resistor, transformer, cable, OHL, MMC
powerimpedance/lines/        named towers and coaxial layouts
powerimpedance/p2p/          Zin, Zeq, Yn, H at c2 AC
```

```bash
python benchmarking/powerimpedance/run_all.py
```

Typical wall on this machine: **5–8 min** for the full suite (Julia compile
included). See [`powerimpedance/README.md`](powerimpedance/README.md).

| Case | Mean Frobenius rel |
|------|--------------------|
| 10 Ω resistor | 0 |
| Y-Y transformer | $1.1\times 10^{-6}$ |
| Aerial cable / OHL | $1.6\times 10^{-4}$ / $2.7\times 10^{-4}$ |
| GFL / c1 / c2 MMC $Y_{\mathrm{ac}}$ | $7.2$ / $7.4$ / $6.7\times 10^{-4}$ |
| P2P $Y_n$ / $Z_{\mathrm{eq}}$ / $H$ | $2.8\times 10^{-4}$ / $4.6\times 10^{-3}$ / $4.6\times 10^{-3}$ |

## Unbalanced AC $Y_{\mathrm{eff}}$

Block Park $A_0$ vs $Y_{\mathrm{eff}}$ (eq. 18): [`unbalanced_yeff/`](unbalanced_yeff/).

```
unbalanced_yeff/rlc/    lab and 2 GW RLC
unbalanced_yeff/ohl/    2 GW untransposed OHL
```

```bash
python benchmarking/unbalanced_yeff/run.py
```

Typical wall: **1–2 min** for all five cases.
[`unbalanced_yeff/README.md`](unbalanced_yeff/README.md).

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
