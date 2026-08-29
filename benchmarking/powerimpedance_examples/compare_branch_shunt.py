"""Compare Harmony vs reference branch||shunt driving-point Z."""
from __future__ import annotations

import csv
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "results"


def load_csv(path: Path):
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh)
        for r in reader:
            rows.append(
                (
                    float(r["freq_Hz"]),
                    complex(float(r["Re_Z"]), float(r["Im_Z"])),
                    complex(float(r.get("Re_Z_analytic", r["Re_Z"])), float(r.get("Im_Z_analytic", r["Im_Z"]))),
                )
            )
    return rows


def main():
    summary = {}
    for tag in ("z8", "z10", "smallsignal"):
        harm = RESULTS / f"harmony_branch_shunt_{tag}.csv"
        if tag == "smallsignal":
            harm = RESULTS / "harmony_branch_shunt_smallsignal.csv"
        pi = RESULTS / f"pi_branch_shunt_{tag}.csv"
        if not harm.exists():
            summary[tag] = {"error": f"missing {harm.name} — run Harmony --cpp pi_branch_shunt"}
            continue
        h_rows = load_csv(harm)
        # Prefer Julia export; else use analytic column already in Harmony CSV.
        if pi.exists():
            p_rows = load_csv(pi)
            errs = []
            for pf, pz, _ in p_rows:
                _, hz, _ = min(h_rows, key=lambda r: abs(r[0] - pf))
                denom = max(abs(pz), 1e-18)
                errs.append(abs(hz - pz) / denom)
            summary[tag] = {
                "n": len(errs),
                "max_rel_Z": max(errs),
                "mean_rel_Z": sum(errs) / len(errs),
                "reference": "pi_csv",
            }
        else:
            errs = [abs(hz - za) / max(abs(za), 1e-18) for _, hz, za in h_rows]
            summary[tag] = {
                "n": len(errs),
                "max_rel_Z": max(errs),
                "mean_rel_Z": sum(errs) / len(errs),
                "reference": "analytic_in_harmony_csv",
            }

    RESULTS.mkdir(parents=True, exist_ok=True)
    out = RESULTS / "branch_shunt_summary.json"
    out.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps(summary, indent=2))
    print(f"Wrote {out}")


if __name__ == "__main__":
    main()
