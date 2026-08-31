"""Stacked Bode of Hdd, Hdq, Hqd, Hqq: block A0 vs block Yeff."""
from __future__ import annotations

import math
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "results"

HARM_PAT = re.compile(
    r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
)

NAMES = ["Hdd", "Hdq", "Hqd", "Hqq"]
A0_COLOR = "#1d4ed8"
YEFF_COLOR = "#0f766e"


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


def plot_tf(a0_rows, yeff_rows, out_path: Path, title: str | None = None) -> dict:
    import matplotlib.pyplot as plt

    n = min(len(a0_rows), len(yeff_rows))
    freqs = [a0_rows[i][0] for i in range(n)]
    frobs = [frob_rel(yeff_rows[i][1], a0_rows[i][1]) for i in range(n)]
    stats = {
        "n": n,
        "mean_frob": sum(frobs) / n,
        "max_frob": max(frobs),
        "min_frob": min(frobs),
        "f_max_hz": freqs[frobs.index(max(frobs))],
    }

    fig, axes = plt.subplots(
        4,
        3,
        sharex=True,
        figsize=(11.2, 13.6),
        constrained_layout=True,
    )
    fig.patch.set_facecolor("white")

    for row, name in enumerate(NAMES):
        mag_a, mag_y, ph_a, ph_y, rel = [], [], [], [], []
        for i in range(n):
            za = a0_rows[i][1][row]
            zy = yeff_rows[i][1][row]
            mag_a.append(mag_db(za))
            mag_y.append(mag_db(zy))
            ph_a.append(phase_deg(za))
            ph_y.append(phase_deg(zy))
            rel.append(max(rel_err(zy, za), 1e-16))
        ph_a = unwrap_deg(ph_a)
        ph_y = unwrap_deg(ph_y)

        axm, axp, axr = axes[row]
        axm.plot(freqs, mag_a, color=A0_COLOR, lw=2.6, label=r"block $A_0$")
        axm.plot(freqs, mag_y, color=YEFF_COLOR, lw=1.9, ls="--", label=r"block $Y_{\mathrm{eff}}$")
        axp.plot(freqs, ph_a, color=A0_COLOR, lw=2.6, label=r"block $A_0$")
        axp.plot(freqs, ph_y, color=YEFF_COLOR, lw=1.9, ls="--", label=r"block $Y_{\mathrm{eff}}$")
        axr.plot(freqs, rel, color="#334155", lw=2.0)
        axr.set_yscale("log")
        axr.set_ylim(1e-6, 2.0)

        _style(axm, ylabel="Magnitude [dB]", title=rf"$H_{{{name[1:]}}}$")
        _style(axp, ylabel="Phase [deg]")
        _style(
            axr,
            ylabel=r"$|H_{\mathrm{eff}}-A_0|/\max(|H|)$",
            xlabel="Frequency [Hz]" if row == 3 else None,
        )

    axes[0][0].legend(loc="best", frameon=True, fontsize=9)
    fig.suptitle(
        title
        or r"MMC AC cut $H=Y_n Z_{\mathrm{eq}}$: block $A_0$ vs block $Y_{\mathrm{eff}}$",
        fontsize=13,
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=160, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    return stats


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--a0", default="MMC1_AC.csv")
    parser.add_argument("--yeff", default="MMC1_AC_yeff.csv")
    parser.add_argument("--out", default="overlay_H.svg")
    parser.add_argument("--title", default="")
    args = parser.parse_args()
    a0 = parse_harmony(RESULTS / args.a0)
    yeff = parse_harmony(RESULTS / args.yeff)
    if not a0 or not yeff:
        raise FileNotFoundError(f"Need {RESULTS / args.a0} and {RESULTS / args.yeff}")
    svg = RESULTS / args.out
    stats = plot_tf(a0, yeff, svg, title=args.title or None)
    print(
        f"wrote {svg}  samples={stats['n']}  "
        f"mean Frob={stats['mean_frob']:.4e}  max={stats['max_frob']:.4e} "
        f"@ {stats['f_max_hz']:.4g} Hz"
    )


if __name__ == "__main__":
    main()
