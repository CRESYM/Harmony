# PowerImpedance.jl examples → Harmony

Maps every script under
[Electa-Git/PowerImpedance.jl/examples](https://github.com/Electa-Git/PowerImpedance.jl/tree/main/examples)
to a Harmony counterpart, and records how results compare.

## Julia environment (PowerImpedance 0.3.x)

An **isolated** Julia project lives in this folder (`Project.toml` / `Manifest.toml`).
It installs full `PowerImpedance` from GitHub (not the older global `PowerImpedanceACDC` 0.1).

```bash
cd benchmarking/powerimpedance_examples
julia install_powerimpedance.jl          # first run: resolves deps (~5–15 min)
julia run_pi_exports.jl                  # cable Y, P2P Z, IEEE39 soil, branch||shunt
```

| PowerImpedance example | Harmony counterpart | Comparable quantity | Fit status |
|------------------------|---------------------|---------------------|------------|
| `Connection_DSL.jl` | `json/connection_dsl_dc.json` | DC topology | Structural |
| `Gridspace_uncertainty.jl` | `pi_branch_shunt` | Driving-point Z(f) | Numeric ≈ 1e−12 |
| `SmallSignal_Gridspace.jl` | same + Nyquist | Fixed Z Nyquist | Partial |
| `P2P README` / `P2P_HVDC_ALT.jl` | `pi_p2p_hvdc` + `json/p2p_cable_readme.json` | fd OHL/cable + Z/H | Compare via scripts |
| `P2P_HVDC_Gridspace.jl` | discrete UGC in `pi_p2p_hvdc` notes | Peak \|Z\| vs share | Partial |
| `IEEE39bus.jl` | `opf_ieee39` | OPF only | Not impedance-parity |
| `IEEE39bus_Gridspace.jl` | `pi_ieee39_soil` + `export_pi_ieee39_soil.jl` | Soil-ρ overlays | PI full network; Harmony line sanity |

## How to run (full pipeline)

```bash
# 1) Julia reference (isolated env)
cd benchmarking/powerimpedance_examples
julia install_powerimpedance.jl
julia run_pi_exports.jl

# 2) Harmony (Release, conda DLLs on PATH)
Harmony --cpp pi_branch_shunt --no-plot
Harmony --cpp pi_p2p_hvdc --no-plot
Harmony --cpp pi_ieee39_soil --no-plot
Harmony --json benchmarking/powerimpedance_examples/json/p2p_cable_readme.json --no-plot

# 3) Compare
python compare_branch_shunt.py
python compare_cable_y.py
python compare_p2p_z.py
python compare_ieee39_soil.py
```

Results land in `benchmarking/powerimpedance_examples/results/`.

## Fit notes

| Case | PI export | Harmony export | Compare script |
|------|-----------|----------------|----------------|
| Branch–shunt | `pi_branch_shunt_*.csv` | `harmony_branch_shunt_*.csv` | `compare_branch_shunt.py` |
| P2P cable Y | `pi_dc_line.csv` | `dc_line.csv` | `compare_cable_y.py` |
| P2P network Z | `pi_p2p_Zdd.csv` (`determine_impedance`) | `harmony_p2p_Zdd.csv` (StabilityEstimate H₀₀) | `compare_p2p_z.py` |
| IEEE39 soil ρ | `pi_ieee39_bus{9,16,29}_rho*.csv` | `harmony_ieee39_t89_rho*.csv` (isolated T8_9 OHL) | `compare_ieee39_soil.py` |

**IEEE39:** PowerImpedance uses the ~1k-line `NetworkBuilder_test.jl` fixture. Harmony does not port that full model; `pi_ieee39_soil` validates the **earth-resistivity** sensitivity on the representative `T8_9` line geometry while Julia exports the **network-level** driving-point overlays at buses 9, 16, and 29.

Standalone MMC plant match remains in [`../mmc_powerimpedance/`](../mmc_powerimpedance/) (~0.1% Frobenius Y).
