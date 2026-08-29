"""Compare Harmony vs PowerImpedance P2P driving-point / loop metrics."""
from __future__ import annotations

import csv
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "results"

HARMONY_CSV = RESULTS / "harmony_p2p_Zdd.csv"
PI_CSV = RESULTS / "pi_p2p_Zdd.csv"


def load_pi_z(path: Path):
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh)
        for r in reader:
            rows.append((float(r["freq_Hz"]), complex(float(r["Re_Zdd"]), float(r["Im_Zdd"]))))
    return rows


def load_harmony_h00(path: Path):
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh)
        for r in reader:
            key_re = "Re_H00" if "Re_H00" in r else "Re_Zdd"
            key_im = "Im_H00" if "Im_H00" in r else "Im_Zdd"
            try:
                val = complex(float(r[key_re]), float(r[key_im]))
            except ValueError:
                continue
            if not (math.isfinite(val.real) and math.isfinite(val.imag)):
                continue
            rows.append((float(r["freq_Hz"]), val))
    return rows


def rel_errors(ref, test):
    errs = []
    for f_ref, z_ref in ref:
        _, z_test = min(test, key=lambda r: abs(r[0] - f_ref))
        denom = max(abs(z_ref), 1e-12)
        errs.append(abs(z_test - z_ref) / denom)
    return errs


def main():
    summary = {
        "note": (
            "PI exports determine_impedance Z_dd at B7 (g1 eliminated). "
            "Harmony exports StabilityEstimate H[0,0] at c2 AC — related but not identical."
        ),
    }
    if not PI_CSV.is_file():
        summary["error"] = f"missing {PI_CSV.name} — run export_pi_p2p_full.jl"
    elif not HARMONY_CSV.is_file():
        summary["error"] = f"missing {HARMONY_CSV.name} — run Harmony --cpp pi_p2p_hvdc"
    else:
        pi = load_pi_z(PI_CSV)
        harm = load_harmony_h00(HARMONY_CSV)
        mag_pi = [20 * math.log10(abs(z)) for _, z in pi]
        mag_h = [20 * math.log10(abs(z)) for _, z in harm]
        summary.update({
            "n_freq": len(pi),
            "pi_peak_db": max(mag_pi),
            "harmony_peak_db": max(mag_h),
            "magnitude_rel_errors_vs_pi": {
                "max": max(rel_errors(pi, harm)),
                "mean": sum(rel_errors(pi, harm)) / len(pi),
            },
        })

    out = RESULTS / "p2p_summary.json"
    out.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
