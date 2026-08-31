"""Compare Harmony vs PowerImpedance.jl standalone Y(f) CSVs."""
from __future__ import annotations

import argparse
import csv
import math
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "results"
REPO = ROOT.parents[1]

CASES = [
    {
        "id": "resistor",
        "title": "10 Ohm series resistor",
        "harmony_csv": "R1.csv",
        "pi_csv": "pi_resistor.csv",
        "spot_hz": [10, 50, 1000, 10000],
        "plot_entries": ["Y11"],
    },
    {
        "id": "transformer_yy",
        "title": "Y-Y real transformer (example windings)",
        "harmony_csv": "T_YY.csv",
        "pi_csv": "pi_transformer_yy.csv",
        "spot_hz": [10, 50, 500, 5000, 10000],
        "plot_entries": ["Y11", "Y14", "Y44"],
    },
    {
        "id": "cable",
        "title": "Aerial coaxial cable (example_cable layers)",
        "harmony_csv": "cable.csv",
        "pi_csv": "pi_cable.csv",
        "spot_hz": [1, 10, 50, 100, 1000],
        "plot_entries": ["Y11", "Y12"],
    },
    {
        "id": "ohl",
        "title": "Two-bundle flat OHL (example_OHL)",
        "harmony_csv": "ohl.csv",
        "pi_csv": "pi_ohl.csv",
        "spot_hz": [1, 10, 50, 100, 1000],
        "plot_entries": ["Y11", "Y12", "Y13", "Y14"],
    },
    {
        "id": "mmc_gfl",
        "title": "MMC GFL (example_MMC plant + PI default GFL)",
        "harmony_csv": "MMC1.csv",
        "pi_csv": "pi_mmc_gfl.csv",
        "spot_hz": [1, 5, 10, 50, 100, 500, 1000],
        "named_entries": ["Yddc", "Ydcd", "Ydcq", "Yadc", "Ydd", "Ydq", "Yqdc", "Yqd", "Yqq"],
        # New PowerImpedance MMC uses load-sign AC currents; Harmony uses generator sign.
        "pi_flip_rows": [2, 3],
        "plot_entries": ["Yddc", "Ydd", "Yqq"],
    },
    {
        "id": "mmc_c1",
        "title": "P2P c1 MMC Vdc+Q",
        "harmony_csv": "c1.csv",
        "pi_csv": "pi_mmc_c1.csv",
        "spot_hz": [1, 5, 10, 50, 100, 500, 1000],
        "named_entries": ["Yddc", "Ydcd", "Ydcq", "Yadc", "Ydd", "Ydq", "Yqdc", "Yqd", "Yqq"],
        "pi_flip_rows": [2, 3],
        "plot_entries": ["Ydd", "Ydq", "Yqd", "Yqq"],
    },
    {
        "id": "mmc_c2",
        "title": "P2P c2 MMC P+Q",
        "harmony_csv": "c2.csv",
        "pi_csv": "pi_mmc_c2.csv",
        "spot_hz": [1, 5, 10, 50, 100, 500, 1000],
        "named_entries": ["Yddc", "Ydcd", "Ydcq", "Yadc", "Ydd", "Ydq", "Yqdc", "Yqd", "Yqq"],
        "pi_flip_rows": [2, 3],
        "plot_entries": ["Ydd", "Ydq", "Yqd", "Yqq"],
    },
    {
        "id": "p2p_cable",
        "title": "P2P bipolar DC cable (100 km, two cores)",
        "harmony_csv": "dc_line.csv",
        "pi_csv": "pi_p2p_cable.csv",
        "spot_hz": [10, 50, 100, 500, 1000],
        "plot_entries": ["Y11", "Y12", "Y13"],
    },
    {
        "id": "p2p_zin",
        "title": "P2P HVDC driving-point Z at c2 AC (B6, full network)",
        "harmony_csv": "c2_AC_Zin.csv",
        "harmony_block_csv": "c2_AC_Zin_block.csv",
        "pi_csv": "pi_p2p_Z.csv",
        "spot_hz": [10, 50, 100, 500, 1000],
        "named_entries": ["Zdd", "Zdq", "Zqd", "Zqq"],
        "plot_entries": ["Zdd", "Zdq", "Zqd", "Zqq"],
    },
    {
        "id": "p2p_tf",
        "title": "P2P HVDC MIMO TF H = Yn Zeq at c2 AC",
        "harmony_csv": "c2_AC.csv",
        "harmony_block_csv": "c2_AC_block.csv",
        "pi_csv": "pi_p2p_H.csv",
        "spot_hz": [10, 50, 100, 500, 1000],
        "named_entries": ["Hdd", "Hdq", "Hqd", "Hqq"],
        "plot_entries": ["Hdd", "Hdq", "Hqd", "Hqq"],
    },
]


HARM_PAT = re.compile(
    r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
)


def parse_harmony(path: Path):
    rows = []
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip() != ""]
        if not parts:
            continue
        f = float(parts[0])
        vals = []
        for p in parts[1:]:
            m = HARM_PAT.match(p.replace(" ", ""))
            if not m:
                break
            vals.append(complex(float(m.group(1)), float(m.group(2))))
        if vals:
            rows.append((f, vals))
    return rows


def parse_pi(path: Path):
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh)
        fields = reader.fieldnames or []
        i = 1
        while f"Re_Y{i}{i}" in fields:
            i += 1
        n = i - 1
        if n < 1:
            raise ValueError(f"No Re_Yij columns in {path}")
        for r in reader:
            f = float(r["freq_Hz"])
            vals = []
            for ii in range(1, n + 1):
                for jj in range(1, n + 1):
                    vals.append(complex(float(r[f"Re_Y{ii}{jj}"]), float(r[f"Im_Y{ii}{jj}"])))
            rows.append((f, vals))
    return rows


def apply_row_flips(vals, n: int, rows_1based):
    if not rows_1based:
        return vals
    out = list(vals)
    for r in rows_1based:
        for c in range(n):
            k = (r - 1) * n + c
            out[k] = -out[k]
    return out


def prepare_rows(case, h_rows, p_rows):
    n = int(round(math.sqrt(len(p_rows[0][1]))))
    flip_rows = case.get("pi_flip_rows") or []
    if flip_rows:
        p_rows = [(f, apply_row_flips(v, n, flip_rows)) for f, v in p_rows]
    return h_rows, p_rows, n, flip_rows


def nearest(rows, f_target):
    return min(rows, key=lambda r: abs(r[0] - f_target))


def rel_err(a: complex, b: complex) -> float:
    denom = max(abs(a), abs(b), 1e-18)
    return abs(a - b) / denom


def frob_rel(a, b):
    n = min(len(a), len(b))
    num = math.sqrt(sum(abs(a[k] - b[k]) ** 2 for k in range(n)))
    den = math.sqrt(sum(abs(b[k]) ** 2 for k in range(n)))
    return num / max(den, 1e-18)


def compare_case(case: dict) -> dict:
    harm_path = RESULTS / case["harmony_csv"]
    pi_path = RESULTS / case["pi_csv"]
    out = {
        "id": case["id"],
        "title": case["title"],
        "harmony_csv": str(harm_path),
        "pi_csv": str(pi_path),
        "ok": False,
    }
    if not harm_path.exists() or not pi_path.exists():
        out["error"] = f"missing CSV (harmony={harm_path.exists()}, pi={pi_path.exists()})"
        return out

    h_rows = parse_harmony(harm_path)
    p_rows = parse_pi(pi_path)
    if not h_rows or not p_rows:
        out["error"] = f"empty CSV (harmony={len(h_rows)}, pi={len(p_rows)})"
        return out

    n_h = len(h_rows[0][1])
    n_p = len(p_rows[0][1])
    out["harmony_entries"] = n_h
    out["pi_entries"] = n_p
    out["harmony_samples"] = len(h_rows)
    out["pi_samples"] = len(p_rows)
    if n_h != n_p:
        out["error"] = f"Y size mismatch: Harmony {n_h} entries vs PowerImpedance {n_p}"
        return out

    h_rows, p_rows, n, flip_rows = prepare_rows(case, h_rows, p_rows)
    if flip_rows:
        out["pi_flip_rows"] = flip_rows

    spectrum = []
    for pf, pv in p_rows:
        _, hv = nearest(h_rows, pf)
        spectrum.append({"f": pf, "frob_rel": frob_rel(hv, pv)})

    frobs = [s["frob_rel"] for s in spectrum]
    frobs_sorted = sorted(frobs)
    table = []
    n = int(round(math.sqrt(n_p)))
    labels = case.get("named_entries") or [f"Y{i}{j}" for i in range(1, n + 1) for j in range(1, n + 1)]
    for f in case["spot_hz"]:
        hf, hv = nearest(h_rows, f)
        pf, pv = nearest(p_rows, f)
        entry = {
            "f_Hz": f,
            "harmony_f": hf,
            "pi_f": pf,
            "frob_rel": frob_rel(hv, pv),
            "entries": [],
        }
        for k, name in enumerate(labels[:n_p]):
            entry["entries"].append(
                {
                    "name": name,
                    "harmony": [hv[k].real, hv[k].imag],
                    "powerimpedance": [pv[k].real, pv[k].imag],
                    "rel_err": rel_err(hv[k], pv[k]),
                    "abs_err": abs(hv[k] - pv[k]),
                }
            )
        table.append(entry)

    out.update(
        {
            "ok": True,
            "mean_frob_rel": sum(frobs) / len(frobs),
            "median_frob_rel": frobs_sorted[len(frobs_sorted) // 2],
            "max_frob_rel": max(frobs),
            "min_frob_rel": min(frobs),
            "spot_checks": table,
            "spectrum": spectrum,
        }
    )
    block_name = case.get("harmony_block_csv")
    if block_name and (RESULTS / block_name).exists():
        b_rows = parse_harmony(RESULTS / block_name)
        b_rows, p_rows2, _, _ = prepare_rows(case, b_rows, p_rows)
        bf = [frob_rel(nearest(b_rows, pf)[1], pv) for pf, pv in p_rows2]
        out["mean_frob_rel_block"] = sum(bf) / len(bf)
        out["max_frob_rel_block"] = max(bf)
        hb = [frob_rel(nearest(h_rows, pf)[1], nearest(b_rows, pf)[1]) for pf, _ in p_rows2]
        out["mean_frob_rel_block_vs_percomp"] = sum(hb) / len(hb)
        out["max_frob_rel_block_vs_percomp"] = max(hb)
    return out


def mag_db(z: complex) -> float:
    return 20.0 * math.log10(max(abs(z), 1e-30))


def phase_deg(z: complex) -> float:
    return math.degrees(math.atan2(z.imag, z.real))


def entry_index(labels, name: str) -> int:
    if name in labels:
        return labels.index(name)
    # Allow Y11 / Zdd style aliases when labels are Yij.
    raise KeyError(name)


def labels_for(case: dict, n_entries: int) -> list:
    n = int(round(math.sqrt(n_entries)))
    return case.get("named_entries") or [f"Y{i}{j}" for i in range(1, n + 1) for j in range(1, n + 1)]


def unwrap_deg(phases):
    out = []
    prev = None
    for p in phases:
        if prev is not None:
            while p - prev > 180.0:
                p -= 360.0
            while p - prev < -180.0:
                p += 360.0
        out.append(p)
        prev = p
    return out


H_COLOR = "#1d4ed8"  # blue, per-component A0
BLOCK_COLOR = "#0f766e"  # teal, block A0
PI_COLOR = "#c2410c"  # orange-red


def _style_axes(ax, ylabel=None, xlabel=None, title=None):
    ax.set_xscale("log")
    ax.grid(True, which="major", color="#d0d5dd", lw=0.8)
    ax.grid(True, which="minor", color="#eef0f3", lw=0.5)
    ax.set_facecolor("white")
    if ylabel:
        ax.set_ylabel(ylabel)
    if xlabel:
        ax.set_xlabel(xlabel)
    if title:
        ax.set_title(title, fontsize=11)


def _save_overlay(fig, out_path: Path) -> list:
    import matplotlib.pyplot as plt

    fig.patch.set_facecolor("white")
    svg = out_path.with_suffix(".svg")
    fig.savefig(svg, dpi=160, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    return [str(svg)]


def plot_overlay(case: dict, h_rows, p_rows, out_path: Path, h_block_rows=None) -> list:
    import matplotlib.pyplot as plt

    labels = labels_for(case, len(p_rows[0][1]))
    wanted = case.get("plot_entries") or labels[: min(3, len(labels))]
    freqs_p = [f for f, _ in p_rows]
    ncols = max(len(wanted), 1)
    fig, axes = plt.subplots(
        3,
        ncols,
        sharex=True,
        figsize=(4.6 * ncols, 9.4),
        squeeze=False,
        constrained_layout=True,
    )
    mark = max(1, len(freqs_p) // 10)
    harm_label = "Harmony block A0" if h_block_rows is None else "Harmony per-component"
    for col, name in enumerate(wanted):
        k = entry_index(labels, name)
        mag_h, ph_h, mag_p, ph_p = [], [], [], []
        mag_b, ph_b = [], []
        for pf, pv in p_rows:
            _, hv = nearest(h_rows, pf)
            mag_h.append(mag_db(hv[k]))
            ph_h.append(phase_deg(hv[k]))
            mag_p.append(mag_db(pv[k]))
            ph_p.append(phase_deg(pv[k]))
            if h_block_rows is not None:
                _, bv = nearest(h_block_rows, pf)
                mag_b.append(mag_db(bv[k]))
                ph_b.append(phase_deg(bv[k]))
        ph_h = unwrap_deg(ph_h)
        ph_p = unwrap_deg(ph_p)
        if mag_b:
            ph_b = unwrap_deg(ph_b)
        axm = axes[0][col]
        axp = axes[1][col]
        axr = axes[2][col]
        axm.plot(freqs_p, mag_h, color=H_COLOR, lw=3.2, ls="-", zorder=2, label=harm_label)
        if mag_b:
            axm.plot(
                freqs_p, mag_b, color=BLOCK_COLOR, lw=2.0, ls="-", zorder=3, label="Harmony block A0"
            )
        axm.plot(
            freqs_p,
            mag_p,
            color=PI_COLOR,
            lw=1.8,
            ls="--",
            marker="o",
            markersize=6,
            markevery=mark,
            markerfacecolor="white",
            markeredgecolor=PI_COLOR,
            markeredgewidth=1.6,
            zorder=4,
            label="PowerImpedance",
        )
        axp.plot(freqs_p, ph_h, color=H_COLOR, lw=3.2, ls="-", zorder=2, label=harm_label)
        if mag_b:
            axp.plot(
                freqs_p, ph_b, color=BLOCK_COLOR, lw=2.0, ls="-", zorder=3, label="Harmony block A0"
            )
        axp.plot(
            freqs_p,
            ph_p,
            color=PI_COLOR,
            lw=1.8,
            ls="--",
            marker="o",
            markersize=6,
            markevery=mark,
            markerfacecolor="white",
            markeredgecolor=PI_COLOR,
            markeredgewidth=1.6,
            zorder=4,
            label="PowerImpedance",
        )
        rel = []
        rel_b = []
        for pf, pv in p_rows:
            _, hv = nearest(h_rows, pf)
            rel.append(max(rel_err(hv[k], pv[k]), 1e-16))
            if h_block_rows is not None:
                _, bv = nearest(h_block_rows, pf)
                rel_b.append(max(rel_err(bv[k], pv[k]), 1e-16))
        axr.plot(freqs_p, rel, color="#334155", lw=2.0, label=harm_label + " vs PI")
        if rel_b:
            axr.plot(freqs_p, rel_b, color=BLOCK_COLOR, lw=1.8, label="block A0 vs PI")
        axr.set_yscale("log")
        _style_axes(axm, ylabel="Magnitude [dB]" if col == 0 else None, title=name)
        _style_axes(axp, ylabel="Phase [deg]" if col == 0 else None)
        err_ylabel = "|H-PI|/max(|H|,|PI|)" if col == 0 else None
        _style_axes(axr, ylabel=err_ylabel, xlabel="Frequency [Hz]")
        axr.set_ylim(1e-8, 2.0)
        if col == 0 and rel_b:
            axr.legend(loc="best", frameon=True, fontsize=8)
    axes[0][0].legend(loc="best", frameon=True, fontsize=9)
    title = case["title"]
    if h_block_rows is not None:
        title = title + " (per-component vs block A0 vs PI)"
    fig.suptitle(title, fontsize=13)
    return _save_overlay(fig, out_path)


def print_case(summary: dict) -> None:
    print(f"\n== {summary['id']}: {summary['title']} ==")
    if not summary.get("ok"):
        print("  SKIP/FAIL:", summary.get("error"))
        return
    print(
        f"  samples Harmony/PI = {summary['harmony_samples']}/{summary['pi_samples']}  "
        f"entries = {summary['harmony_entries']}"
    )
    print(
        f"  Frobenius rel  mean/median/max/min = "
        f"{summary['mean_frob_rel']:.4e} / {summary['median_frob_rel']:.4e} / "
        f"{summary['max_frob_rel']:.4e} / {summary['min_frob_rel']:.4e}"
    )
    if "mean_frob_rel_block" in summary:
        print(
            f"  block A0 vs PI     mean/max = "
            f"{summary['mean_frob_rel_block']:.4e} / {summary['max_frob_rel_block']:.4e}"
        )
        print(
            f"  block vs per-comp  mean/max = "
            f"{summary['mean_frob_rel_block_vs_percomp']:.4e} / "
            f"{summary['max_frob_rel_block_vs_percomp']:.4e}"
        )
    for t in summary["spot_checks"]:
        top = sorted(t["entries"], key=lambda e: e["rel_err"], reverse=True)[:3]
        print(
            f"  @{t['f_Hz']:8g} Hz  Frob={t['frob_rel']:.3e}  "
            + " ".join(f"{e['name']}={e['rel_err']:.2e}" for e in top)
        )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--case", action="append", help="Restrict to given case id(s)")
    parser.add_argument("--no-plot", action="store_true")
    args = parser.parse_args()
    wanted = set(args.case) if args.case else None

    RESULTS.mkdir(parents=True, exist_ok=True)
    summaries = []
    for case in CASES:
        if wanted and case["id"] not in wanted:
            continue
        summary = compare_case(case)
        print_case(summary)
        summaries.append(summary)
        if args.no_plot or not summary.get("ok"):
            continue
        try:
            h_rows = parse_harmony(RESULTS / case["harmony_csv"])
            p_rows = parse_pi(RESULTS / case["pi_csv"])
            h_rows, p_rows, _, _ = prepare_rows(case, h_rows, p_rows)
            h_block = None
            block_name = case.get("harmony_block_csv")
            if block_name and (RESULTS / block_name).exists():
                h_block = parse_harmony(RESULTS / block_name)
            out_base = RESULTS / f"overlay_{case['id']}"
            plot_overlay(case, h_rows, p_rows, out_base, h_block_rows=h_block)
            print(f"  plot {out_base.with_suffix('.svg')}")
        except Exception as exc:
            print(f"  plot skipped: {exc}")

    ok = [s for s in summaries if s.get("ok")]
    if ok:
        print("Summary:")
        for s in ok:
            print(
                f"  {s['id']:16s}  mean Frob={s['mean_frob_rel']:.4e}  "
                f"max={s['max_frob_rel']:.4e}"
            )


if __name__ == "__main__":
    main()
