# MMC admittance: Harmony vs PowerImpedanceACDC

Standalone MMC comparison used to check whether Harmony’s MMC linearisation
matches [PowerImpedanceACDC.jl](https://github.com/Electa-Git/PowerImpedance.jl)
(the model Harmony’s README attributes as the MMC/transmission-line source).

## Case

Identical plant and operating point (Harmony `example_MMC` plant):

| Quantity | Value |
|----------|-------|
| \(L_\mathrm{arm}, R_\mathrm{arm}, C_\mathrm{arm}, N\) | 50 mH, 1.07 Ω, 10 mF, 400 |
| \(L_r, R_r\) | 60 mH, 0.535 Ω |
| \(V_m\) (peak phase), \(V_\mathrm{dc}\) | 100 kV, 200 kV |
| \(P, Q\) | 100 MW, 0 |
| Controls | PowerImpedance default GFL: PLL, P, Q, OCC, CCC |

Harmony gains are the PowerImpedance per-unit defaults converted with
\(Z_\mathrm{ac}=150\,\Omega\), \(Z_\mathrm{dc}=400\,\Omega\),
\(I_\mathrm{ac}=666.67\,\mathrm{A}\), \(\omega_0=100\pi\).

## How to reproduce

```bash
# PowerImpedance side (needs Julia + PowerImpedanceACDC)
julia benchmarking/mmc_powerimpedance/compare_mmc_y.jl

# Harmony side (after building Release)
Harmony --json benchmarking/mmc_powerimpedance/mmc_matched.json --no-plot
copy files\MMC1.csv benchmarking\mmc_powerimpedance\results\MMC1.csv

python benchmarking/mmc_powerimpedance/compare_results.py
```

## Result (this tree)

After fixing Harmony’s PLL/plant frame mixing, `Vac_mag`, CCC `T_2θ` refs, and
removing the incorrect AC-diagonal sign flips in `MMC::compute_y_parameters`:

| Metric | Value |
|---|---|
| Mean Frobenius \(\|Y_H-Y_{PI}\|_F/\|Y_{PI}\|_F\) | ≈ 0.26 |
| Median Frobenius | ≈ 0.22 |
| At 1 Hz (`Ydd`, `Yddc`) | ≈ exact |
| At 500 Hz (plant-dominated) | ≈ 0.02 |
| Mean \(\|Y_{\mathrm{dc,dc}}\|\) relative error | ≈ 0.10 |

`Ydd` / DC paths agree closely. Residual mid-band / `Yqq` differences remain
(outer Q-loop / linearisation details still differ from PowerImpedance).

## Related Harmony fix

JSON `"type": "y_matrix"` previously called `writeFile` without
`solveEquilibrium` / `computeABCD`, so converter Y was written as zeros.
`computation_runner.cpp` now linearises converters first.
