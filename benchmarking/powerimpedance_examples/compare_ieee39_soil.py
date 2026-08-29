"""Compare IEEE39 soil-resistivity overlays: PI NetworkBuilder vs Harmony OHL sanity."""
from __future__ import annotations

import csv
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "results"

BUSES = (9, 16, 29)
RHOS = (10, 100, 1000)


def load_pi_bus_rho(bus: int, rho: int):
    path = RESULTS / f"pi_ieee39_bus{bus}_rho{rho}.csv"
    if not path.is_file():
        return None
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        for r in csv.DictReader(fh):
            z = complex(float(r["Re_Zdd"]), float(r["Im_Zdd"]))
            rows.append((float(r["freq_Hz"]), 20 * math.log10(abs(z))))
    return rows


def max_spread_db(curves: list[list[float]]) -> float:
    if not curves:
        return 0.0
    n = min(len(c) for c in curves)
    spreads = [max(c[i] for c in curves) - min(c[i] for c in curves) for i in range(n)]
    return max(spreads)


def load_harmony_line_rho(rho: int):
    path = RESULTS / f"harmony_ieee39_t89_rho{rho}.csv"
    if not path.is_file():
        return None
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        for r in csv.DictReader(fh):
            try:
                y = complex(float(r["Re_Y11"]), float(r["Im_Y11"]))
            except ValueError:
                continue
            if not (math.isfinite(y.real) and math.isfinite(y.imag)):
                continue
            z = 1 / y if abs(y) > 1e-18 else complex(0)
            rows.append((float(r["freq_Hz"]), 20 * math.log10(abs(z))))
    return rows


def main():
    summary = {"pi_network_spread_db": {}, "harmony_line_t89_spread_db": None}

    for bus in BUSES:
        curves = [load_pi_bus_rho(bus, rho) for rho in RHOS]
        if all(c is not None for c in curves):
            mags = [[m for _, m in c] for c in curves]
            summary["pi_network_spread_db"][f"bus_{bus}"] = max_spread_db(mags)

    h_curves = [load_harmony_line_rho(rho) for rho in RHOS]
    if all(c is not None for c in h_curves):
        mags = [[m for _, m in c] for c in h_curves]
        summary["harmony_line_t89_spread_db"] = max_spread_db(mags)

    summary["note"] = (
        "PI uses full IEEE39 NetworkBuilder fixture (IEEE39bus_Gridspace.jl). "
        "Harmony line CSVs are driving-point Z of isolated T8_9 OHL (earth-model sanity)."
    )

    out = RESULTS / "ieee39_soil_summary.json"
    out.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
