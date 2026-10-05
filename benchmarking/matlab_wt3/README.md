# Harmony WT3 $Y_{dq}$ vs MATLAB 2.5 MW DFIG

Standalone plant admittance for
`WT3_DFIG_SequenceImpedance_2p5MW_v6_directEval.m`
(2.5 MW, 690 V, 50 Hz, slip $-0.35$). Harmony uses the SI pack in
`example_WT_type_3.cpp`.

```
matlab_wt3/
  harmony/wt3_2p5mw.json
  export_wt3_y.m
  run.py  compare.py
  results/overlay_Y.svg
```

Overlay: Harmony **blue solid**, MATLAB **orange dashed**.

```bash
conda run -n harmony python benchmarking/matlab_wt3/run.py
```

Checked-in artefacts are the **SVG overlay only**.

## Result (this machine, 2026-09-08)

200 log-spaced points, $1$–$1000\,\mathrm{Hz}$. Wall **~18 s**.

| Metric | Value |
|--------|--------|
| mean $\\|Y_H-Y_M\\|_F/\\|Y_M\\|_F$ | $1.7\times 10^{-6}$ |
| max | $1.1\times 10^{-5}$ @ $1.19\,\mathrm{Hz}$ |

Residual is Harmony CSV print precision. All four $Y_{dq}$ entries overlay.
