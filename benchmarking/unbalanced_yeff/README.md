# Unbalanced AC block: $A_0$ vs $Y_{\mathrm{eff}}$

Harmony comparison of **block** Park $A_0$ against the $N_h=1$ effective
admittance $Y_{\mathrm{eff}}$ on a one-MMC network. The cut is the MMC AC
port. Overlays: block $A_0$ **blue solid**, block $Y_{\mathrm{eff}}$ **red
dashed**.

```
unbalanced_yeff/
  run.py  plot.py
  rlc/
    mmc_unbalanced_rlc.json
    mmc_unbalanced_rlc_strong.json
    mmc_unbalanced_rlc_2gw.json
    results/overlay_H{,_strong,_2gw}.svg
  ohl/
    mmc_unbalanced_ohl_2gw.json
    mmc_unbalanced_ohl_conc_2gw.json
    results/overlay_H_ohl{,_conc}.svg
```

Checked-in artefacts are **SVG overlays only**.

## Redo the plots

```bash
conda run -n harmony python benchmarking/unbalanced_yeff/run.py
```

Optional: `--only rlc`, `--only strong`, `--only 2gw`, `--only ohl`,
`--only ohl_conc`. The script finds `build/Release/Harmony.exe`, runs both
$A_0$ and $Y_{\mathrm{eff}}$ scans, writes SVGs, and deletes intermediate
CSVs.

JSON flags (optional; omit them for per-component $A_0$):

```json
"park_per_component": false,
"yeff": true
```

OPF is skipped (`skip_opf: true`). Do not evaluate $Y_{\mathrm{eff}}$ at
exactly $50\,\mathrm{Hz}$ or $150\,\mathrm{Hz}$.

## Execution times

Wall times on this machine (2026-09-01, Harmony Release). Each case is two
stability scans (block $A_0$ and $Y_{\mathrm{eff}}$).

| Case | Typical wall |
|------|----------------|
| `rlc` / `strong` / `2gw` | ~10–15 s each |
| `ohl` (150 km vertical) | ~15 s |
| `ohl_conc` (250 km offset-vertical) | ~13 s |
| all five | ~1–2 min |

`run.py` prints per-case `wall=` and a total at the end.

## Cases

**RLC** (`rlc/`):

```
Vs_ac -- R -- L -- Bpcc -- MMC -- Vs_dc
                 |
                 C (shunt)
```

| Key | JSON | Unbalance |
|-----|------|-----------|
| `rlc` | `mmc_unbalanced_rlc.json` | $R_a=R_b=1\,\Omega$, $R_c=5\,\Omega$ |
| `strong` | `mmc_unbalanced_rlc_strong.json` | $R,L,C$ all changed on phase $c$ |
| `2gw` | `mmc_unbalanced_rlc_2gw.json` | $2\,\mathrm{GW}$, $\pm 525\,\mathrm{kV}$, same RLC pattern |

**OHL** (`ohl/`), same $2\,\mathrm{GW}$ MMC, stiff AC bus:

```
Vs_ac -- OHL -- Bpcc -- MMC -- Vs_dc
```

| Key | JSON | Line |
|-----|------|------|
| `ohl` | `mmc_unbalanced_ohl_2gw.json` | 150 km vertical, twin bundle |
| `ohl_conc` | `mmc_unbalanced_ohl_conc_2gw.json` | 250 km offset-vertical (concentric), $\Delta\tilde{x}=4\,\mathrm{m}$ |

The 150 km vertical corridor is close to balanced. The 250 km offset-vertical
tower enlarges $Y_{PN}$; $\lambda/4$ is near $340\,\mathrm{Hz}$ and a Schur
peak sits near $840\,\mathrm{Hz}$.

## Block Park maps

Let $Y_{\mathrm{abc}}(j\omega)$ be the assembled $3n\times 3n$ abc port
admittance of the AC area (converters excluded). With
$v^{\mathrm{T}}=[1,\,j]$, $e^{\mathrm{T}}=[1,\,e^{j\varphi},\,e^{2j\varphi}]$,
$\varphi=-2\pi/3$, and $\omega_o=2\pi\cdot 50$,

$$
\sigma_H(\omega)=e^{\mathrm{T}} Y_{\mathrm{abc}}(j\omega)\,e^{\ast},\qquad
\sigma_T(\omega)=e^{\mathrm{T}} Y_{\mathrm{abc}}(j\omega)\,e,
$$

$$
A_0(\omega)=\frac16\Bigl[\sigma_H(\omega-\omega_o)\,M_H
+\sigma_H^{\ast}(\omega+\omega_o)\,M_H^{\ast}\Bigr],
$$

$$
Y_{\mathrm{eff}}(\omega)=A_0(\omega)
-C_-(\omega)\,A_0(\omega-2\omega_o)^{-1} C_+(\omega-2\omega_o)
-C_+(\omega)\,A_0(\omega+2\omega_o)^{-1} C_-(\omega+2\omega_o).
$$

At the MMC AC cut, $H(s)=Y_n(s)\,Z_{\mathrm{eq}}(s)$.

## Results

Relative Frobenius $\|H_{\mathrm{eff}}-A_0\|/\|A_0\|$, 81 samples from
$10\,\mathrm{Hz}$ to $1\,\mathrm{kHz}$ (Harmony Release, 2026-09-01):

| Case | Mean | Max | $f$ at max |
|------|------|-----|------------|
| $R_c=5\,\Omega$ only | $4.30\times 10^{-2}$ | $0.247$ | $299\,\mathrm{Hz}$ |
| $R,L,C$ unbalance on $c$ | $0.216$ | $0.249$ | $750\,\mathrm{Hz}$ |
| $2\,\mathrm{GW}$ RLC unbalance | $0.204$ | $0.261$ | $750\,\mathrm{Hz}$ |
| $2\,\mathrm{GW}$ untransposed vertical OHL | $1.18\times 10^{-2}$ | $0.163$ | $447\,\mathrm{Hz}$ |
| $2\,\mathrm{GW}$ offset-vertical OHL $250\,\mathrm{km}$ | $9.51\times 10^{-2}$ | $1.77$ | $841\,\mathrm{Hz}$ |

### $H$ at the MMC AC cut ($R_c$ only)

![TF H stacked](rlc/results/overlay_H.svg)

### $H$ at the MMC AC cut ($R$, $L$, $C$ unbalance)

![TF H strong](rlc/results/overlay_H_strong.svg)

### $H$ at the MMC AC cut ($2\,\mathrm{GW}$, $\pm 525\,\mathrm{kV}$)

![TF H 2 GW](rlc/results/overlay_H_2gw.svg)

### $H$ at the MMC AC cut ($2\,\mathrm{GW}$ untransposed vertical OHL)

![TF H OHL](ohl/results/overlay_H_ohl.svg)

### $H$ at the MMC AC cut ($2\,\mathrm{GW}$ offset-vertical OHL, $250\,\mathrm{km}$)

![TF H OHL concentric](ohl/results/overlay_H_ohl_conc.svg)

## Nyquist–Bode of the eigenvalues

The Bode plots above show the four entries of $H$. The stability curve is the two eigenvalues of $H$. Nyquist draws each eigenvalue in the complex plane. The boundary is the point $-1$. Bode of the same eigenvalues is magnitude and phase versus frequency. Circle marks $10\,\mathrm{Hz}$, square marks $1\,\mathrm{kHz}$.

| Case | SVG |
|------|-----|
| $R_c=5\,\Omega$ only | [nyquist_H.svg](rlc/results/nyquist_H.svg) |
| $R,L,C$ unbalance on $c$ | [nyquist_H_strong.svg](rlc/results/nyquist_H_strong.svg) |
| $2\,\mathrm{GW}$ RLC unbalance | [nyquist_H_2gw.svg](rlc/results/nyquist_H_2gw.svg) |
| $150\,\mathrm{km}$ vertical OHL | [nyquist_H_ohl.svg](ohl/results/nyquist_H_ohl.svg) |
| $250\,\mathrm{km}$ offset-vertical OHL | [nyquist_H_ohl_conc.svg](ohl/results/nyquist_H_ohl_conc.svg) |

On the $10\,\mathrm{Hz}$–$1\,\mathrm{kHz}$ scan, $A_0$ and $Y_{\mathrm{eff}}$ do not give two different Nyquist results.

The RLC cases stay about $0.9$ away from $-1$ for both formulas. The $250\,\mathrm{km}$ line is where $H$ itself differs the most ($1.77$ at $841\,\mathrm{Hz}$). That eigenvalue stays about $0.75$ away from $-1$, so the large Bode difference does not move the stability boundary.

The $150\,\mathrm{km}$ vertical line is the curve that does go around $-1$. Both formulas do it, on eigenvalue 2, near $422\,\mathrm{Hz}$. A finer sweep from $300$ to $600\,\mathrm{Hz}$ ($301$ points) puts the closest approach at $0.013$ for $A_0$ ($420\,\mathrm{Hz}$) and $0.012$ for $Y_{\mathrm{eff}}$ ($419\,\mathrm{Hz}$). Both cross the real axis at about $-1.05$. The positive-frequency arc winds by $-0.97$ turns about $-1$ for both. Same loop, shifted by about $1\,\mathrm{Hz}$.
