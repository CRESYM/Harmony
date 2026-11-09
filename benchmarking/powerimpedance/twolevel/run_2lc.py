"""Harmony 2LC against every PowerImpedance two-level case those loops can run.

Magnitude is 20*log10(|Y|) in dB. The matrix measurement is
20*log10 of the relative Frobenius norm, also in dB. Harmony AC current rows
are negated: PowerImpedance publishes the opposite of the reactor current.
"""
from __future__ import annotations

import json
import math
import os
import subprocess
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[2]
OUT = ROOT / "results"
HARM = ROOT / "harmony"
FILES = REPO / "files"
EXE = REPO / "build" / "Release" / "Harmony.exe"
OMEGA = 2.0 * math.pi * 50.0
PLL = (0.397887357729738, 7.957747154594767)
CURRENT = (0.254647908947033, 0.8)

CASES = [
    dict(name="p_q", title="2LC P+Q", S=100e6, vll=220e3, vdc=640e3,
         L=0.08 * (220.0 ** 2 / 100.0) / OMEGA, R=0.01 * 0.08 * (220.0 ** 2 / 100.0),
         p_into=70e6, q_into=20e6, p_dc=-70e6, vdc_control=False,
         pll=PLL, pgain=(0.04, 40.0), qgain=(0.04, 40.0), current=CURRENT, delay=200e-6),
    dict(name="vdc_q", title="2LC Vdc+Q", S=100e6, vll=220e3, vdc=640e3,
         L=0.08 * (220.0 ** 2 / 100.0) / OMEGA, R=0.01 * 0.08 * (220.0 ** 2 / 100.0),
         p_into=70e6, q_into=20e6, p_dc=-70e6, vdc_control=True,
         pll=PLL, pgain=(5.0, 5.0), qgain=(0.04, 40.0), current=CURRENT, delay=200e-6),
    dict(name="statcom", title="2LC STATCOM Vdc+Q", S=269e6, vll=345e3, vdc=110e3,
         L=0.08 * (345.0 ** 2 / 269.0) / OMEGA, R=0.01 * 0.08 * (345.0 ** 2 / 269.0),
         p_into=0.0, q_into=-0.5 * 269e6, p_dc=0.0, vdc_control=True,
         pll=PLL, pgain=(5.0, 5.0), qgain=(0.04, 40.0), current=CURRENT, delay=200e-6),
    dict(name="owf", title="2LC energy-hub wind P+Q", S=1000e6, vll=220e3, vdc=640e3,
         L=0.01232, R=0.03872, p_into=-700e6, q_into=-150e6, p_dc=700e6,
         vdc_control=False, pll=(1.0, 31.830988618379067),
         pgain=(0.01, 10.0), qgain=(0.01, 10.0), current=CURRENT, delay=0.0),
    dict(name="wind", title="2LC weak-grid wind P+Q", S=600e6, vll=380e3, vdc=640e3,
         L=0.08 * (380.0 ** 2 / 600.0) / OMEGA, R=0.0008 * (380.0 ** 2 / 600.0),
         p_into=-600e6, q_into=0.0, p_dc=600e6, vdc_control=False,
         pll=PLL, pgain=(0.01, 10.0), qgain=(0.01, 10.0), current=CURRENT, delay=0.0),
]


def logf(n=31):
    a, b = 1.0, 1e3
    return np.array([a * (b / a) ** (k / (n - 1)) for k in range(n)])


def db_abs(z):
    return 20.0 * np.log10(np.maximum(np.abs(z), 1e-30))


def pack(case: dict) -> dict:
    vm = case["vll"] * math.sqrt(2.0 / 3.0)
    zac = case["vll"] ** 2 / case["S"]
    iac = 2.0 * case["S"] / (3.0 * vm)
    p_h = -case["p_into"]
    q_h = -case["q_into"]
    kp_pll = case["pll"][0] * OMEGA / vm
    ki_pll = case["pll"][1] * OMEGA / vm
    kp_i = case["current"][0] * zac
    ki_i = case["current"][1] * zac
    kp_q = case["qgain"][0] * iac / case["S"]
    ki_q = case["qgain"][1] * iac / case["S"]
    if case["vdc_control"]:
        kp_p = case["pgain"][0] * iac / case["vdc"]
        ki_p = case["pgain"][1] * iac / case["vdc"]
    else:
        kp_p = case["pgain"][0] * iac / case["S"]
        ki_p = case["pgain"][1] * iac / case["S"]
    delay = case["delay"]
    conv = [OMEGA, p_h, q_h, 0.0, vm, case["p_dc"], case["vdc"], case["L"], case["R"], delay]
    if delay:
        conv.append(3.0)
    ctrl = [1, 0, kp_pll, ki_pll, 1, 0]
    if case["vdc_control"]:
        ctrl += [1, 0, kp_p, ki_p, 1, case["vdc"], 0]
    else:
        ctrl += [0, 1, 0, kp_p, ki_p, 1, p_h]
    ctrl += [0, 1, 0, kp_q, ki_q, 1, q_h, 0, 0]
    ctrl += [1, 0, kp_i, ki_i, 2, 0.0, 0.0, 0, 0, 0]
    return {
        "id": "c2lc",
        "location": "AC1_DC1",
        "type": "2lc",
        "converter_params": conv,
        "controller_params": ctrl,
        "connected_buses": [
            {"bus_id": "ac", "terminal": 1},
            {"bus_id": "dc", "terminal": 2},
        ],
        "enabled": True,
    }


def write_json(case: dict, freqs: np.ndarray) -> None:
    HARM.mkdir(parents=True, exist_ok=True)
    name = case["name"]
    doc = {
        "simulation": {
            "title": name,
            "description": case["title"],
            "output_directory": "./files",
            "frequency_range": {"frequencies": [float(f) for f in freqs]},
        },
        "buses": [
            {"id": "ac", "location": "AC1", "pins": 3, "enabled": True},
            {"id": "dc", "location": "DC1", "pins": 2, "enabled": True},
        ],
        "components": [pack(case)],
        "computations": [{
            "type": "y_matrix",
            "component_id": "c2lc",
            "frequency_range": {"frequencies": [float(f) for f in freqs]},
            "plot": False,
        }],
    }
    (HARM / f"{name}.json").write_text(json.dumps(doc), encoding="utf-8")


def run_harmony(name: str) -> None:
    env = os.environ.copy()
    env["PATH"] = ";".join([
        r"C:\Users\alekic\AppData\Local\miniconda3\envs\harmony\Library\bin",
        str(REPO / "build" / "tests" / "Release"),
        str(EXE.parent),
        r"C:\Windows\System32",
        r"C:\Windows",
    ])
    subprocess.run(
        [str(EXE), "--json", str(HARM / f"{name}.json"), "--no-plot"],
        cwd=REPO, env=env, check=True)


def load_pi(path: Path):
    text = path.read_text(encoding="utf-8").splitlines()[1:]
    freqs, mats = [], []
    for line in text:
        parts = line.split(",")
        freqs.append(float(parts[0]))
        vals = [complex(float(parts[i]), float(parts[i + 1])) for i in range(1, 19, 2)]
        mats.append(np.array(vals, dtype=complex).reshape(3, 3, order="F"))
    return np.array(freqs), mats


def load_harmony(path: Path, freqs: np.ndarray):
    rows = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip()]
        if not parts:
            continue
        f = float(parts[0])
        vals = []
        for token in parts[1:]:
            real, imag = token.replace(" ", "").split("+1i*(")
            vals.append(complex(float(real), float(imag[:-1])))
        n = int(round(math.sqrt(len(vals))))
        rows[f] = np.array(vals, dtype=complex).reshape(n, n)
    out = []
    for f in freqs:
        key = min(rows, key=lambda g: abs(g - f) / max(f, 1.0))
        if abs(key - f) / max(f, 1.0) > 1e-4:
            raise KeyError(f"no Harmony sample at {f}")
        out.append(rows[key])
    return out


def frob(a, b):
    return float(np.linalg.norm(a - b) / max(np.linalg.norm(b), 1e-18))


def readable_ylim(ax, values, floor):
    lo = float(np.min(values))
    hi = float(np.max(values))
    if hi - lo < floor:
        mid = 0.5 * (lo + hi)
        lo = mid - 0.5 * floor
        hi = mid + 0.5 * floor
    pad = 0.06 * (hi - lo)
    ax.set_ylim(lo - pad, hi + pad)
    ax.ticklabel_format(axis="y", useOffset=False, style="plain")


def plot(name, freqs, harm, pi, title):
    fig, axes = plt.subplots(3, 3, figsize=(11.4, 7.2), sharex=True)
    for col, (r, c) in enumerate(((0, 0), (1, 1), (2, 2))):
        hm = np.array([m[r, c] for m in harm])
        pm = np.array([m[r, c] for m in pi])
        mag_h, mag_p = db_abs(hm), db_abs(pm)
        phase_h, phase_p = np.angle(hm, deg=True), np.angle(pm, deg=True)
        err = mag_h - mag_p
        axes[0, col].semilogx(freqs, mag_h, color="#1d4ed8", label="Harmony")
        axes[0, col].semilogx(freqs, mag_p, color="#c2410c", label="PowerImpedance")
        axes[1, col].semilogx(freqs, phase_h, color="#1d4ed8")
        axes[1, col].semilogx(freqs, phase_p, color="#c2410c")
        axes[2, col].semilogx(freqs, err, color="#1d4ed8")
        axes[2, col].axhline(0.0, color="#94a3b8", linewidth=0.6)
        readable_ylim(axes[0, col], np.concatenate([mag_h, mag_p]), 2.0)
        readable_ylim(axes[1, col], np.concatenate([phase_h, phase_p]), 20.0)
        readable_ylim(axes[2, col], err, 0.2)
        axes[0, col].set_title(f"Y{r+1}{c+1}")
        axes[2, col].set_xlabel("Frequency [Hz]")
    axes[0, 0].set_ylabel("Magnitude [dB]")
    axes[1, 0].set_ylabel("Phase [deg]")
    axes[2, 0].set_ylabel("Magnitude error [dB]")
    axes[0, 0].legend(frameon=False)
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(OUT / f"overlay_{name}.svg", bbox_inches="tight", facecolor="white")
    fig.savefig(OUT / f"overlay_{name}.png", dpi=140, bbox_inches="tight", facecolor="white")
    plt.close(fig)


def main():
    freqs = logf()
    summary = []
    for case in CASES:
        name = case["name"]
        write_json(case, freqs)
        run_harmony(name)
        pi_f, pi = load_pi(OUT / f"pi_{name}.csv")
        harm = load_harmony(FILES / "c2lc.csv", freqs)
        series = [m.copy() for m in harm]
        for m in series:
            # Both AC current rows: load versus generator. Frames match.
            m[1, :] *= -1
            m[2, :] *= -1
        errs = np.array([frob(a, b) for a, b in zip(series, pi)])
        db = 20.0 * np.log10(np.maximum(errs, 1e-30))
        plot(name, freqs, series, pi, case["title"])
        src = FILES / "c2lc.csv"
        dest = OUT / f"harmony_{name}.csv"
        dest.write_bytes(src.read_bytes())
        row = {
            "id": name,
            "title": case["title"],
            "mean_dB": float(np.mean(db)),
            "max_dB": float(np.max(db)),
            "n": int(len(errs)),
        }
        summary.append(row)
        print(name, f"mean {row['mean_dB']:.2f} dB", f"max {row['max_dB']:.2f} dB")
    (OUT / "summary.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
