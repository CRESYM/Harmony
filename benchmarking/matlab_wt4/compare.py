"""Compare Harmony vs MATLAB WT4 Ydq(f) CSVs."""
from __future__ import annotations

import csv
import math
import re
from pathlib import Path

NAMES = ["Ydd", "Ydq", "Yqd", "Yqq"]
HARM_COLOR = "#1d4ed8"
ML_COLOR = "#ea580c"

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


def parse_matlab(path: Path):
    rows = []
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh)
        for r in reader:
            f = float(r["freq_Hz"])
            vals = [
                complex(float(r["Re_Ydd"]), float(r["Im_Ydd"])),
                complex(float(r["Re_Ydq"]), float(r["Im_Ydq"])),
                complex(float(r["Re_Yqd"]), float(r["Im_Yqd"])),
                complex(float(r["Re_Yqq"]), float(r["Im_Yqq"])),
            ]
            rows.append((f, vals))
    return rows


def mag_db(z: complex) -> float:
    return 20.0 * math.log10(max(abs(z), 1e-30))


def phase_deg(z: complex) -> float:
    return math.degrees(math.atan2(z.imag, z.real))


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


def rel_err(a: complex, b: complex) -> float:
    return abs(a - b) / max(abs(a), abs(b), 1e-18)


def frob_rel(a, b) -> float:
    n = min(len(a), len(b))
    num = math.sqrt(sum(abs(a[k] - b[k]) ** 2 for k in range(n)))
    den = math.sqrt(sum(abs(b[k]) ** 2 for k in range(n)))
    return num / max(den, 1e-18)


def nearest(rows, f: float):
    return min(rows, key=lambda r: abs(r[0] - f))


def stats(h_rows, m_rows) -> dict:
    n = min(len(h_rows), len(m_rows))
    freqs = [h_rows[i][0] for i in range(n)]
    frobs = [frob_rel(h_rows[i][1], m_rows[i][1]) for i in range(n)]
    imax = frobs.index(max(frobs))
    return {
        "n": n,
        "mean_frob": sum(frobs) / n,
        "max_frob": max(frobs),
        "min_frob": min(frobs),
        "f_max_hz": freqs[imax],
        "frobs": frobs,
        "freqs": freqs,
    }


def _style(ax, ylabel=None, xlabel=None, title=None):
    ax.set_xscale("log")
    ax.grid(True, which="major", color="#d0d5dd", lw=0.8)
    ax.grid(True, which="minor", color="#eef0f3", lw=0.5)
    ax.set_facecolor("white")
    if ylabel:
        ax.set_ylabel(ylabel)
    if xlabel:
        ax.set_xlabel(xlabel)
    if title:
        ax.set_title(title, fontsize=12)


def plot_overlay(h_rows, m_rows, out_path: Path, title: str | None = None) -> dict:
    import matplotlib.pyplot as plt

    summary = stats(h_rows, m_rows)
    n = summary["n"]
    freqs = summary["freqs"]

    fig, axes = plt.subplots(
        4, 3, sharex=True, figsize=(11.2, 13.6), constrained_layout=True
    )
    fig.patch.set_facecolor("white")

    for row, name in enumerate(NAMES):
        mag_h, mag_m, ph_h, ph_m, rel = [], [], [], [], []
        for i in range(n):
            zh = h_rows[i][1][row]
            zm = m_rows[i][1][row]
            mag_h.append(mag_db(zh))
            mag_m.append(mag_db(zm))
            ph_h.append(phase_deg(zh))
            ph_m.append(phase_deg(zm))
            rel.append(max(rel_err(zh, zm), 1e-16))
        ph_h = unwrap_deg(ph_h)
        ph_m = unwrap_deg(ph_m)

        axm, axp, axr = axes[row]
        axm.plot(freqs, mag_h, color=HARM_COLOR, lw=2.4, label="Harmony")
        axm.plot(freqs, mag_m, color=ML_COLOR, lw=1.6, ls="--", label="MATLAB")
        axp.plot(freqs, ph_h, color=HARM_COLOR, lw=2.4)
        axp.plot(freqs, ph_m, color=ML_COLOR, lw=1.6, ls="--")
        axr.plot(freqs, rel, color="#111827", lw=1.5)
        axr.set_yscale("log")

        _style(axm, ylabel=rf"$|{name}|$ (dB)", title=name if row == 0 else None)
        _style(axp, ylabel="phase (deg)")
        _style(axr, ylabel=r"$|H-M|/|M|$")
        if row == 0:
            axm.legend(frameon=False, loc="best")
        if row == 3:
            axm.set_xlabel("f (Hz)")
            axp.set_xlabel("f (Hz)")
            axr.set_xlabel("f (Hz)")

    fig.suptitle(
        title
        or (
            r"WT4 $Y_{dq}$: Harmony vs MATLAB RTDS reduced-$L$  "
            rf"(mean Frob {summary['mean_frob']:.2e}, "
            rf"max {summary['max_frob']:.2e} @ {summary['f_max_hz']:.4g} Hz)"
        ),
        fontsize=13,
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=140)
    plt.close(fig)
    return summary


def print_spots(h_rows, m_rows, spots=(1.0, 10.0, 49.0, 100.0, 200.0, 500.0, 1000.0)) -> None:
    print("Spot |Y_H - Y_M| / |Y_M|")
    for f in spots:
        fh, hv = nearest(h_rows, f)
        fm, mv = nearest(m_rows, f)
        print(f"  ~{f:g} Hz  (H {fh:.6g}, M {fm:.6g})")
        for k, name in enumerate(NAMES):
            ratio = abs(hv[k]) / max(abs(mv[k]), 1e-30)
            print(
                f"    {name:4s}  H={hv[k]:.6e}  M={mv[k]:.6e}  "
                f"rel={rel_err(hv[k], mv[k]):.3e}  |H|/|M|={ratio:.6f}"
            )
