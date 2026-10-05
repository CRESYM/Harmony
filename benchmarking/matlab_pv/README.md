# Harmony PV $Y_{dq}$ vs MATLAB Zhao RTDS engine

Standalone plant admittance for `PV_RTDS_ZhaoValidatedEngine.m`
(1 MW, 315 V, 50 Hz, $C_{\mathrm{dc}}=32\,\mathrm{mF}$,
$Z_{\mathrm{pv}}=-A^{-1}B$). Harmony uses the SI pack in
`example_PV_plant.cpp`.

```
matlab_pv/
  harmony/pv_tablev.json
  export_pv_y.m
  run.py  compare.py
  results/overlay_Y.svg
```

Overlay: Harmony **blue solid**, MATLAB **orange dashed**.

```bash
conda run -n harmony python benchmarking/matlab_pv/run.py
```

The script finds `build/Release/Harmony.exe` and MATLAB R2026a (or
`MATLAB_EXE`), writes $Y(f)$ on Harmony's log grid $1$–$1000\,\mathrm{Hz}$
(200 points), and deletes nothing except regenerating the SVG.

Checked-in artefacts are the **SVG overlay only**.

## Result (this machine, 2026-09-08)

200 log-spaced points, $1$–$1000\,\mathrm{Hz}$. Wall **~16 s** (Harmony + MATLAB R2026a).

| Metric | Value |
|--------|--------|
| mean $\\|Y_H-Y_M\\|_F/\\|Y_M\\|_F$ | $2.0\times 10^{-6}$ |
| max | $5.1\times 10^{-6}$ @ $499.5\,\mathrm{Hz}$ |

Residual is Harmony CSV print precision. All four $Y_{dq}$ entries overlay.
