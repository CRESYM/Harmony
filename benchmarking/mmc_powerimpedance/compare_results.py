"""Compare Harmony vs PowerImpedanceACDC MMC Y(f)."""
from __future__ import annotations

import csv
import json
import math
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "results"


def parse_harmony(path: Path):
    rows = []
    pat = re.compile(
        r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
    )
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip() != ""]
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
        reader = csv.DictReader(fh)
        for r in reader:
            f = float(r["freq_Hz"])
            vals = []
            for i in range(1, 4):
                for j in range(1, 4):
                    vals.append(complex(float(r[f"Re_Y{i}{j}"]), float(r[f"Im_Y{i}{j}"])))
            rows.append((f, vals))
    return rows


def nearest(rows, f_target):
    return min(rows, key=lambda r: abs(r[0] - f_target))


def rel_err(a: complex, b: complex) -> float:
    denom = max(abs(a), abs(b), 1e-18)
    return abs(a - b) / denom


def frob_rel(a, b):
    num = math.sqrt(sum(abs(a[k] - b[k]) ** 2 for k in range(9)))
    den = math.sqrt(sum(abs(b[k]) ** 2 for k in range(9)))
    return num / max(den, 1e-18)


def main():
    harm_path = RESULTS / "MMC1.csv"
    pi_path = RESULTS / "powerimpedance_Y.csv"
    h_rows = parse_harmony(harm_path)
    p_rows = parse_pi(pi_path)

    # Align on PowerImpedance frequency grid
    spectrum = []
    for pf, pv in p_rows:
        _, hv = nearest(h_rows, pf)
        spectrum.append({
            "f": pf,
            "frob_rel": frob_rel(hv, pv),
            "Yddc_rel": rel_err(hv[0], pv[0]),
            "Ydd_rel": rel_err(hv[4], pv[4]),
            "Yqq_rel": rel_err(hv[8], pv[8]),
            "harmony_Yddc": [hv[0].real, hv[0].imag],
            "pi_Yddc": [pv[0].real, pv[0].imag],
            "harmony_Ydd": [hv[4].real, hv[4].imag],
            "pi_Ydd": [pv[4].real, pv[4].imag],
            "harmony_Yqq": [hv[8].real, hv[8].imag],
            "pi_Yqq": [pv[8].real, pv[8].imag],
        })

    freqs = [1, 5, 10, 50, 100, 500, 1000]
    labels = ["Yddc", "Ydcd", "Ydcq", "Yadc", "Ydd", "Ydq", "Yqdc", "Yqd", "Yqq"]
    table = []
    for f in freqs:
        hf, hv = nearest(h_rows, f)
        pf, pv = nearest(p_rows, f)
        entry = {"f_Hz": f, "harmony_f": hf, "pi_f": pf, "frob_rel": frob_rel(hv, pv), "entries": []}
        for k, name in enumerate(labels):
            entry["entries"].append({
                "name": name,
                "harmony": [hv[k].real, hv[k].imag],
                "powerimpedance": [pv[k].real, pv[k].imag],
                "rel_err": rel_err(hv[k], pv[k]),
                "abs_err": abs(hv[k] - pv[k]),
            })
        table.append(entry)

    summary = {
        "notes": [
            "Same plant: Larm=50mH, Rarm=1.07Ω, Carm=10mF, N=400, Lr=60mH, Rr=0.535Ω",
            "Same OP: Vm=100kV peak, Vdc=200kV, P=100MW, Q=0",
            "Controllers: PowerImpedance default GFL (pll,p,q,occ,ccc) with SI gain conversion in Harmony",
            "Both use generator current convention (current exiting converter)",
        ],
        "mean_frob_rel": sum(s["frob_rel"] for s in spectrum) / len(spectrum),
        "median_frob_rel": sorted(s["frob_rel"] for s in spectrum)[len(spectrum) // 2],
        "max_frob_rel": max(s["frob_rel"] for s in spectrum),
        "mean_Yddc_rel": sum(s["Yddc_rel"] for s in spectrum) / len(spectrum),
        "spot_checks": table,
        "spectrum": spectrum,
    }
    out = RESULTS / "comparison_summary.json"
    out.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(f"Wrote {out}")
    print(f"mean/median/max Frobenius rel = "
          f"{summary['mean_frob_rel']:.4f} / {summary['median_frob_rel']:.4f} / {summary['max_frob_rel']:.4f}")
    print(f"mean |Yddc| rel = {summary['mean_Yddc_rel']:.4f}")
    for t in table:
        print(f"@{t['f_Hz']:4g} Hz  Frob={t['frob_rel']:.3f}  "
              + " ".join(f"{e['name']}={e['rel_err']:.2f}" for e in t['entries'] if e['name'] in ('Yddc','Ydd','Yqq')))


if __name__ == "__main__":
    main()
