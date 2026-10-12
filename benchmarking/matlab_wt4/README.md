# Harmony WT4 $Y_{dq}$ vs MATLAB RTDS reduced-$L$ GSC

Standalone plant admittance for
`WT4_RTDS_Implementation_v2_EquationConsistent.m`
(2.5 MW, 690 V, 50 Hz, $Y=BA^{-1}$). Harmony uses the SI pack in
`example_WT_type_4.cpp`. The optional $C_f+R_{\mathrm{damp}}$ shunt is
not part of this overlay.

```
matlab_wt4/
  harmony/wt4_2p5mw.json
  export_wt4_y.m
  run.py  compare.py
  results/overlay_Y.svg
```

Overlay: Harmony **blue solid**, MATLAB **orange dashed**.

```bash
conda run -n harmony python benchmarking/matlab_wt4/run.py
```

Checked-in artefacts are the **SVG overlay only**.

## Result (this machine, 2026-09-08)

200 log-spaced points, $1$–$1000\,\mathrm{Hz}$. Wall **~26 s**.

| Metric | Value |
|--------|--------|
| mean $\\|Y_H-Y_M\\|_F/\\|Y_M\\|_F$ | $3.0\times 10^{-6}$ |
| max | $1.6\times 10^{-5}$ @ $48.8\,\mathrm{Hz}$ |

Residual is Harmony CSV print precision. All four $Y_{dq}$ entries overlay.
