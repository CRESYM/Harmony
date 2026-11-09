"""Overlay symbolic PVplant and ODE PVgeneric Ydq(f)."""
from __future__ import annotations

import math
import re
from pathlib import Path

NAMES = ["Ydd", "Ydq", "Yqd", "Yqq"]
OLD_COLOR = "#1d4ed8"
ODE_COLOR = "#ea580c"
HARM_PAT = re.compile(
    r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
)


def parse_harmony(path: Path):
    rows = []
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip() != ""]
        if not parts:
            continue
        freq = float(parts[0])
        vals = []
        for part in parts[1:]:
            match = HARM_PAT.match(part.replace(" ", ""))
            if not match:
                break
            vals.append(complex(float(match.group(1)), float(match.group(2))))
        if len(vals) >= 4:
            rows.append((freq, vals[:4]))
    return rows


def mag_db(value: complex) -> float:
    return 20.0 * math.log10(max(abs(value), 1e-30))


def phase_deg(value: complex) -> float:
    return math.degrees(math.atan2(value.imag, value.real))


def unwrap_deg(phases):
    out = []
    prev = None
    for phase in phases:
        if prev is not None:
            while phase - prev > 180.0:
                phase -= 360.0
            while phase - prev < -180.0:
                phase += 360.0
        out.append(phase)
        prev = phase
    return out


def rel_err(left: complex, right: complex) -> float:
    return abs(left - right) / max(abs(left), abs(right), 1e-18)


def frob_rel(left, right) -> float:
    num = math.sqrt(sum(abs(left[k] - right[k]) ** 2 for k in range(4)))
    den = math.sqrt(sum(abs(right[k]) ** 2 for k in range(4)))
    return num / max(den, 1e-18)


def mean_frob(old_rows, ode_rows) -> float:
    n = min(len(old_rows), len(ode_rows))
    return sum(frob_rel(old_rows[i][1], ode_rows[i][1]) for i in range(n)) / n


def align_sign(old_rows, ode_rows):
    """PVplant uses the Zhao injection sign. The ODE port current leaves the plant."""
    flipped = [(freq, [-value for value in vals]) for freq, vals in ode_rows]
    if mean_frob(old_rows, flipped) < mean_frob(old_rows, ode_rows):
        return flipped, True
    return ode_rows, False


def plot_overlay(old_rows, ode_rows, out_path: Path, sign_flipped: bool) -> dict:
    import matplotlib.pyplot as plt

    n = min(len(old_rows), len(ode_rows))
    freqs = [old_rows[i][0] for i in range(n)]
    frobs = [frob_rel(old_rows[i][1], ode_rows[i][1]) for i in range(n)]
    imax = frobs.index(max(frobs))
    summary = {
        "n": n,
        "mean_frob": sum(frobs) / n,
        "max_frob": frobs[imax],
        "f_max_hz": freqs[imax],
        "sign_flipped": sign_flipped,
    }

    fig, axes = plt.subplots(4, 2, sharex=True, figsize=(10.4, 12.2), constrained_layout=True)
    fig.patch.set_facecolor("white")
    ode_label = r"PVgeneric ($-Y$)" if sign_flipped else "PVgeneric"

    for row, name in enumerate(NAMES):
        mag_old, mag_ode, ph_old, ph_ode = [], [], [], []
        for i in range(n):
            old = old_rows[i][1][row]
            ode = ode_rows[i][1][row]
            mag_old.append(mag_db(old))
            mag_ode.append(mag_db(ode))
            ph_old.append(phase_deg(old))
            ph_ode.append(phase_deg(ode))
        ph_old = unwrap_deg(ph_old)
        ph_ode = unwrap_deg(ph_ode)
        tag = name[1:]
        ax_mag, ax_phase = axes[row]
        ax_mag.plot(freqs, mag_old, color=OLD_COLOR, lw=2.2, label="PVplant")
        ax_mag.plot(freqs, mag_ode, color=ODE_COLOR, lw=1.6, ls="--", label=ode_label)
        ax_phase.plot(freqs, ph_old, color=OLD_COLOR, lw=2.2)
        ax_phase.plot(freqs, ph_ode, color=ODE_COLOR, lw=1.6, ls="--")
        for ax in (ax_mag, ax_phase):
            ax.set_xscale("log")
            ax.grid(True, which="major", color="#d0d5dd", lw=0.8)
            ax.grid(True, which="minor", color="#eef0f3", lw=0.5)
            ax.set_facecolor("white")
        ax_mag.set_ylabel(rf"$|Y_{{{tag}}}|$ [dB]")
        ax_phase.set_ylabel(rf"$\angle Y_{{{tag}}}$ [deg]")
        if row == 0:
            ax_mag.set_title("Magnitude")
            ax_phase.set_title("Phase")
            ax_mag.legend(frameon=False, loc="best")
        if row == len(NAMES) - 1:
            ax_mag.set_xlabel("Frequency [Hz]")
            ax_phase.set_xlabel("Frequency [Hz]")

    fig.suptitle(
        r"PV $Y_{dq}$: PVplant vs PVgeneric  "
        rf"(mean Frob {summary['mean_frob']:.2e}, "
        rf"max {summary['max_frob']:.2e} @ {summary['f_max_hz']:.4g} Hz)",
        fontsize=13,
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=140)
    plt.close(fig)
    return summary
