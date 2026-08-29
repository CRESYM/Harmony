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

- PowerImpedance.jl example suite mapping and Y/Z comparisons:
  see [`powerimpedance_examples/README.md`](powerimpedance_examples/README.md)
  and the standalone MMC match in [`mmc_powerimpedance/`](mmc_powerimpedance/).

- PowerImpedance.jl example suite mapping and Y/Z comparisons:
  see [`powerimpedance_examples/README.md`](powerimpedance_examples/README.md)
  and the standalone MMC match in [`mmc_powerimpedance/`](mmc_powerimpedance/).

### PowerImpedance.jl parity suite

See [`powerimpedance_examples/`](powerimpedance_examples/README.md) for Julia exports, Harmony `--cpp pi_*` cases, and Python compare scripts.

## Fit summary (this tree)

| Case | Metric | Result |
|------|--------|--------|
| MMC Y (`mmc_powerimpedance`) | mean ‖Y_H−Y_PI‖_F / ‖Y_PI‖_F | ≈ **0.0008** (~0.1%) |
| Branch–shunt Z∈{8,10} / SmallSignal | max \|Z_H−Z_ref\|/\|Z_ref\| over log grid | ≈ **1e−12** (machine precision) |
| P2P README cable geometry | Harmony Y(f) 10–1000 Hz, 100 pts | Produced (`results/dc_line.csv`); PI layered-cable numeric export needs full `PowerImpedance` |
| P2P network (`pi_p2p_hvdc`) | StabilityEstimate TF @ c2 AC | Runs; fd OHL/cable — compare via `compare_p2p_z.py` |
| Connection DSL DC | Topology build | Matches Connection_DSL.jl DC graph |
| IEEE39 soil ρ | PI network overlays vs Harmony T8_9 line | Different scope — see README |
