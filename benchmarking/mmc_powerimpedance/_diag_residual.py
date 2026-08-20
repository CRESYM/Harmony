"""Diagnose residual Harmony vs PowerImpedance Y mismatch."""
from __future__ import annotations

import csv
import json
import math
import re
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent
SUMMARY = ROOT / "results" / "comparison_summary.json"
H_CSV = ROOT / "results" / "MMC1.csv"
P_CSV = ROOT / "results" / "powerimpedance_Y.csv"

NAMES = ["Yddc", "Ydcd", "Ydcq", "Yadc", "Ydd", "Ydq", "Yqdc", "Yqd", "Yqq"]


def parse_harmony(path: Path):
    rows = []
    pat = re.compile(
        r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)"
        r"\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
    )
    for line in path.read_text().splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip()]
        if not parts:
            continue
        f = float(parts[0])
        vals = []
        for p in parts[1:]:
            m = pat.match(p.replace(" ", ""))
            if not m:
                break
            vals.append(complex(float(m.group(1)), float(m.group(2))))
        if len(vals) == 9:
            rows.append((f, vals))
    return rows


def parse_pi(path: Path):
    rows = []
    with path.open(newline="") as fh:
        for r in csv.DictReader(fh):
            f = float(r["freq_Hz"])
            vals = [
                complex(float(r[f"Re_Y{i}{j}"]), float(r[f"Im_Y{i}{j}"]))
                for i in range(1, 4)
                for j in range(1, 4)
            ]
            rows.append((f, vals))
    return rows


def nearest(rows, f):
    return min(rows, key=lambda r: abs(r[0] - f))


def frob(a, b):
    da = np.asarray(a, dtype=complex)
    db = np.asarray(b, dtype=complex)
    return float(np.linalg.norm(da - db) / max(np.linalg.norm(db), 1e-18))


def log_interp(f_q, f_s, y_s):
    out = np.zeros((len(f_q), 9), dtype=complex)
    lx = np.log10(f_s)
    lq = np.log10(f_q)
    for k in range(9):
        out[:, k] = np.interp(lq, lx, y_s[:, k].real) + 1j * np.interp(
            lq, lx, y_s[:, k].imag
        )
    return out


def main():
    s = json.loads(SUMMARY.read_text())
    print(
        f"stored mean/median/max Frob = "
        f"{s['mean_frob_rel']:.4f} / {s['median_frob_rel']:.4f} / {s['max_frob_rel']:.4f}"
    )

    h = parse_harmony(H_CSV)
    p = parse_pi(P_CSV)
    hf = np.array([r[0] for r in h])
    hv = np.array([r[1] for r in h], dtype=complex)
    pf = np.array([r[0] for r in p])
    pv = np.array([r[1] for r in p], dtype=complex)

    print(f"H freqs: n={len(hf)} first={hf[0]:.4f} last={hf[-1]:.4f}")
    print(f"PI freqs: n={len(pf)} first={pf[0]:.4f} last={pf[-1]:.4f}")
    print(f"H geometric ratio (f1/f0) = {hf[1]/hf[0]:.6f}  expect 10^(3/39)={10**(3/39):.6f} or 10^(3/40)={10**(3/40):.6f}")
    print(f"PI geometric ratio = {pf[1]/pf[0]:.6f}")

    ranked = sorted(s["spectrum"], key=lambda t: -t["frob_rel"])
    print("\nWorst 8 (nearest-neighbour compare as stored):")
    for t in ranked[:8]:
        print(
            f"  f~{t['f']:8.3f} Frob={t['frob_rel']:.3f} "
            f"Yddc={t['Yddc_rel']:.3f} Ydd={t['Ydd_rel']:.3f} Yqq={t['Yqq_rel']:.3f}"
        )

    print("\nEntry abs-error leaders at PI resonance freqs:")
    for ft in [34.551, 83.768, 100.0, 492.388]:
        hf0, hv0 = nearest(h, ft)
        pf0, pv0 = nearest(p, ft)
        print(f"\n  target={ft:.3f}  H={hf0:.3f}  PI={pf0:.3f}  df={hf0-pf0:+.3f}  Frob={frob(hv0,pv0):.3f}")
        errs = []
        for k, n in enumerate(NAMES):
            ae = abs(hv0[k] - pv0[k])
            re = ae / max(abs(hv0[k]), abs(pv0[k]), 1e-18)
            errs.append((ae, re, n, hv0[k], pv0[k]))
        for ae, re, n, a, b in sorted(errs, reverse=True)[:4]:
            print(
                f"    {n:6s} abs={ae:.3e} rel={re:.3f}  "
                f"H={a.real:+.4e}{a.imag:+.4e}j  PI={b.real:+.4e}{b.imag:+.4e}j"
            )

    # Interp both ways on shared log grid
    mask = (hf >= pf.min()) & (hf <= pf.max())
    pi_on_h = log_interp(hf[mask], pf, pv)
    frobs = np.array([frob(hh, pp) for hh, pp in zip(hv[mask], pi_on_h)])
    print("\nLog-interp PI onto Harmony freqs:")
    print(
        f"  mean={frobs.mean():.4f} median={np.median(frobs):.4f} "
        f"max={frobs.max():.4f} at f={hf[mask][int(frobs.argmax())]:.2f}"
    )

    # Ideal shared grid like PI
    f_shared = pf
    # need Harmony on PI freqs — but we only have H at its grid; interp H→PI
    h_on_p = log_interp(pf, hf, hv)
    frobs2 = np.array([frob(hh, pp) for hh, pp in zip(h_on_p, pv)])
    print("Log-interp Harmony onto PI freqs:")
    print(
        f"  mean={frobs2.mean():.4f} median={np.median(frobs2):.4f} "
        f"max={frobs2.max():.4f} at f={pf[int(frobs2.argmax())]:.2f}"
    )

    # Exclude bands where |Yddc| peaks (resonance) — mid-band sensitivity
    yddc_pi = np.abs(pv[:, 0])
    peak_idx = int(np.argmax(yddc_pi))
    print(f"\nPI |Yddc| peak at f={pf[peak_idx]:.2f} Hz, |Y|={yddc_pi[peak_idx]:.4f}")
    # Frob away from peak (±1 octave)
    keep = (pf < pf[peak_idx] / 2) | (pf > pf[peak_idx] * 2)
    if keep.any():
        frobs3 = frobs2[keep]
        print(
            f"Frob excluding ±1 octave around peak: "
            f"mean={frobs3.mean():.4f} median={np.median(frobs3):.4f} max={frobs3.max():.4f}"
        )


if __name__ == "__main__":
    main()
