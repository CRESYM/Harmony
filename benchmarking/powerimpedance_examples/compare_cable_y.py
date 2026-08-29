"""Compare Harmony vs PowerImpedance P2P README cable Y(f)."""
from __future__ import annotations

import csv
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "results"


def parse_harmony_y(path: Path):
    rows = []
    pat = re.compile(
        r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
    )
    for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip()]
        if not parts:
            continue
        try:
            f = float(parts[0])
        except ValueError:
            continue
        m = pat.match(parts[1].replace(" ", ""))
        if not m:
            continue
        y11 = complex(float(m.group(1)), float(m.group(2)))
        rows.append((f, y11))
    return rows


def load_pi_csv(path: Path):
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh)
        for r in reader:
            rows.append((
                float(r["freq_Hz"]),
                complex(float(r["Re_Y11"]), float(r["Im_Y11"])),
            ))
    return rows


def main():
    harmony = RESULTS / "dc_line.csv"
    pi = RESULTS / "pi_dc_line.csv"

    summary = {
        "note": (
            "PI exports 2×2 Y after Kron elimination; Harmony JSON writes the full "
            "bipolar cable Y matrix (4×4). Direct Y11 comparison is not port-aligned."
        ),
    }

    if harmony.is_file() and pi.is_file():
        h_rows = parse_harmony_y(harmony)
        p_rows = load_pi_csv(pi)
        summary["harmony_file"] = str(harmony)
        summary["pi_file"] = str(pi)
        summary["n_freq_samples"] = min(len(h_rows), len(p_rows))
        if h_rows and p_rows:
            summary["harmony_Y11_abs_at_first"] = abs(h_rows[0][1])
            summary["pi_Y11_abs_at_first"] = abs(p_rows[0][1])
    elif harmony.is_file():
        summary["note"] = "Harmony Y only — run export_pi_cable.jl for PI reference."
        summary["n_freq_samples"] = len(parse_harmony_y(harmony))
    else:
        summary["note"] = "Run Harmony --json json/p2p_cable_readme.json and export_pi_cable.jl"

    out = RESULTS / "cable_summary.json"
    out.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
