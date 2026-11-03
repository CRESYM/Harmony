"""Bode of Hdd, Hdq, Hqd, Hqq, and Nyquist–Bode of the eigenvalues of H."""
from __future__ import annotations

import math
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent

HARM_PAT = re.compile(
    r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
)

NAMES = ["Hdd", "Hdq", "Hqd", "Hqq"]
A0_COLOR = "#1d4ed8"
YEFF_COLOR = "#dc2626"


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

        tag = name[1:]
        axm, axp, axr = axes[row]
        axm.plot(freqs, mag_a, color=A0_COLOR, lw=2.6, label=r"block $A_0$")
        axm.plot(freqs, mag_y, color=YEFF_COLOR, lw=1.9, ls="--", label=r"block $Y_{\mathrm{eff}}$")
        axp.plot(freqs, ph_a, color=A0_COLOR, lw=2.6, label=r"block $A_0$")
        axp.plot(freqs, ph_y, color=YEFF_COLOR, lw=1.9, ls="--", label=r"block $Y_{\mathrm{eff}}$")
        axr.plot(freqs, rel, color="#334155", lw=2.0)
        axr.set_yscale("log")
        axr.set_ylim(1e-6, 2.0)

        _style(
            axm,
            ylabel=rf"$|H_{{{tag}}}|$ [dB]",
            title="Magnitude" if row == 0 else None,
        )
        _style(
            axp,
            ylabel=rf"$\angle H_{{{tag}}}$ [deg]",
            title="Phase" if row == 0 else None,
        )
        _style(
            axr,
            ylabel=rf"$|H_{{{tag}}}^{{\mathrm{{eff}}}}-H_{{{tag}}}^{{A_0}}|/\max(|H_{{{tag}}}|)$",
            title="Relative error" if row == 0 else None,
        )
        if row == len(NAMES) - 1:
            axm.set_xlabel("Frequency [Hz]")
            axp.set_xlabel("Frequency [Hz]")
            axr.set_xlabel("Frequency [Hz]")

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


def eigenvalues_2x2(vals):
    """Eigenvalues of [[Hdd, Hdq], [Hqd, Hqq]]."""
    hdd, hdq, hqd, hqq = vals[:4]
    trace = hdd + hqq
    det = hdd * hqq - hdq * hqd
    root = (trace * trace - 4.0 * det) ** 0.5
    return [(trace + root) / 2.0, (trace - root) / 2.0]


def _order_pair(pair, previous):
    first, second = pair
    if previous is None:
        if abs(second) > abs(first):
            return [second, first]
        return [first, second]
    direct = abs(first - previous[0]) + abs(second - previous[1])
    swapped = abs(second - previous[0]) + abs(first - previous[1])
    if swapped < direct:
        return [second, first]
    return [first, second]


def tracked_eigenvalues(rows):
    """Two eigenvalue loci, each continuous in frequency."""
    loci = [[], []]
    previous = None
    for _, vals in rows:
        ordered = _order_pair(eigenvalues_2x2(vals), previous)
        loci[0].append(ordered[0])
        loci[1].append(ordered[1])
        previous = ordered
    return loci


def matched_eigenvalues(reference_rows, other_rows):
    """Track A0 along frequency, then pair Yeff to those loci at each frequency."""
    ref = tracked_eigenvalues(reference_rows)
    other = [[], []]
    n = min(len(reference_rows), len(other_rows))
    for i in range(n):
        paired = _order_pair(eigenvalues_2x2(other_rows[i][1]), [ref[0][i], ref[1][i]])
        other[0].append(paired[0])
        other[1].append(paired[1])
    return [ref[0][:n], ref[1][:n]], other


def closest_to_minus_one(freqs, locus):
    distances = [abs(z + 1.0) for z in locus]
    index = min(range(len(distances)), key=distances.__getitem__)
    return distances[index], freqs[index]


def plot_nyquist(a0_rows, yeff_rows, out_path: Path, title: str | None = None) -> dict:
    """Nyquist and Bode plots of the two eigenvalues of H. The point -1 is the boundary."""
    import matplotlib.pyplot as plt

    n = min(len(a0_rows), len(yeff_rows))
    freqs = [a0_rows[i][0] for i in range(n)]
    a0_loci, yeff_loci = matched_eigenvalues(a0_rows[:n], yeff_rows[:n])

    fig, axes = plt.subplots(
        2,
        3,
        figsize=(11.2, 7.4),
        constrained_layout=True,
    )
    fig.patch.set_facecolor("white")
    stats = {"n": n, "loci": []}

    for row in range(2):
        la = a0_loci[row]
        ly = yeff_loci[row]
        da, fa = closest_to_minus_one(freqs, la)
        dy, fy = closest_to_minus_one(freqs, ly)
        moves = [rel_err(ly[i], la[i]) for i in range(n)]
        imax = max(range(n), key=moves.__getitem__)
        stats["loci"].append(
            {
                "eig": row + 1,
                "dist_a0": da,
                "f_a0": fa,
                "dist_yeff": dy,
                "f_yeff": fy,
                "max_rel": moves[imax],
                "f_max_rel": freqs[imax],
            }
        )

        axn, axm, axp = axes[row]
        axn.plot([z.real for z in la], [z.imag for z in la], color=A0_COLOR, lw=2.2, label=r"$A_0$")
        axn.plot(
            [z.real for z in ly],
            [z.imag for z in ly],
            color=YEFF_COLOR,
            lw=1.8,
            ls="--",
            label=r"$Y_{\mathrm{eff}}$",
        )
        axn.plot(la[0].real, la[0].imag, "o", color=A0_COLOR, ms=5)
        axn.plot(ly[0].real, ly[0].imag, "o", color=YEFF_COLOR, ms=4)
        axn.plot(la[-1].real, la[-1].imag, "s", color=A0_COLOR, ms=5)
        axn.plot(ly[-1].real, ly[-1].imag, "s", color=YEFF_COLOR, ms=4)
        axn.plot(-1.0, 0.0, "+", color="#111827", ms=12, mew=1.6, label=r"$-1$")
        axn.set_aspect("equal", adjustable="datalim")
        axn.grid(True, color="#d0d5dd", lw=0.8)
        axn.set_facecolor("white")
        axn.set_xlabel(r"Real part of $\lambda$")
        axn.set_ylabel(r"Imaginary part of $\lambda$")
        axn.set_title(f"Nyquist, eigenvalue {row + 1}")
        axn.text(
            0.02,
            0.98,
            f"closest to $-1$\n$A_0$: {da:.3g} at {fa:.4g} Hz\n"
            f"$Y_{{\\mathrm{{eff}}}}$: {dy:.3g} at {fy:.4g} Hz",
            transform=axn.transAxes,
            va="top",
            ha="left",
            fontsize=8,
            color="#111827",
        )

        mag_a = [mag_db(z) for z in la]
        mag_y = [mag_db(z) for z in ly]
        ph_a = unwrap_deg([phase_deg(z) for z in la])
        ph_y = unwrap_deg([phase_deg(z) for z in ly])
        axm.plot(freqs, mag_a, color=A0_COLOR, lw=2.2, label=r"$A_0$")
        axm.plot(freqs, mag_y, color=YEFF_COLOR, lw=1.8, ls="--", label=r"$Y_{\mathrm{eff}}$")
        axm.axhline(0.0, color="#111827", lw=0.8)
        axp.plot(freqs, ph_a, color=A0_COLOR, lw=2.2)
        axp.plot(freqs, ph_y, color=YEFF_COLOR, lw=1.8, ls="--")
        axp.axhline(-180.0, color="#111827", lw=0.8)
        axp.axhline(180.0, color="#111827", lw=0.8)
        _style(axm, ylabel=rf"$|\lambda_{row + 1}|$ [dB]", title="Magnitude" if row == 0 else None)
        _style(axp, ylabel=rf"$\angle\lambda_{row + 1}$ [deg]", title="Phase" if row == 0 else None)
        if row == 1:
            axm.set_xlabel("Frequency [Hz]")
            axp.set_xlabel("Frequency [Hz]")

    axes[0][0].legend(loc="best", frameon=True, fontsize=8)
    heading = title or r"Eigenvalues of $H=Y_n Z_{\mathrm{eq}}$"
    fig.suptitle(
        heading + "\n" + r"Circle: 10 Hz. Square: 1 kHz. The point $-1$ is the boundary.",
        fontsize=12,
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=160, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    return stats


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--dir", default="rlc/results", help="Folder with CSVs and output SVG")
    parser.add_argument("--a0", default="MMC1_AC.csv")
    parser.add_argument("--yeff", default="MMC1_AC_yeff.csv")
    parser.add_argument("--out", default="overlay_H.svg")
    parser.add_argument("--title", default="")
    args = parser.parse_args()
    outdir = Path(args.dir)
    if not outdir.is_absolute():
        outdir = ROOT / outdir
    a0 = parse_harmony(outdir / args.a0)
    yeff = parse_harmony(outdir / args.yeff)
    if not a0 or not yeff:
        raise FileNotFoundError(f"Need {outdir / args.a0} and {outdir / args.yeff}")
    svg = outdir / args.out
    stats = plot_tf(a0, yeff, svg, title=args.title or None)
    print(
        f"wrote {svg}  samples={stats['n']}  "
        f"mean Frob={stats['mean_frob']:.4e}  max={stats['max_frob']:.4e} "
        f"@ {stats['f_max_hz']:.4g} Hz"
    )


if __name__ == "__main__":
    main()
