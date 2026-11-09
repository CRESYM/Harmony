"""Harmony vs PowerImpedance for the examples both models can run.

Phase and dq sending-end impedances use the same log grid as run_pi_examples.jl.
Harmony blue, PowerImpedance orange.
"""
from __future__ import annotations

import json
import math
import os
import subprocess
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[2]
PI = ROOT / "results"
HARM = ROOT / "harmony"
FILES = REPO / "files"
EXE = REPO / "build" / "Release" / "Harmony.exe"

H_COLOR = "#1d4ed8"
PI_COLOR = "#c2410c"
F0 = 50.0


def logf(a: float, b: float, n: int) -> np.ndarray:
    return np.array([a * (b / a) ** (k / (n - 1)) for k in range(n)], dtype=float)


def park_kernels():
    ang = np.exp(-1j * 2 * np.pi / 3)
    j = 1j
    a = np.array([
        [1, ang, ang * ang],
        [-j, -j * ang, -j * ang * ang],
        [0, 0, 0],
    ], dtype=complex)
    a_conj = np.array([
        [1, np.conj(ang), np.conj(ang * ang)],
        [j, j * np.conj(ang), j * np.conj(ang * ang)],
        [0, 0, 0],
    ], dtype=complex)
    return a, a_conj


A, ACONJ = park_kernels()


def park_a0(y_minus: np.ndarray, y_plus: np.ndarray) -> np.ndarray:
    """Per-port Park A0. Phase blocks of 3 become dq blocks of 2."""
    nabc = y_minus.shape[0]
    nports = nabc // 3
    out = np.zeros((2 * nports, 2 * nports), dtype=complex)
    for pi in range(nports):
        for pj in range(nports):
            sl_i = slice(3 * pi, 3 * pi + 3)
            sl_j = slice(3 * pj, 3 * pj + 3)
            ya = y_minus[sl_i, sl_j]
            yb = y_plus[sl_i, sl_j]
            blk = (A @ ya @ ACONJ.T + ACONJ @ yb @ A.T) / 6.0
            out[2 * pi:2 * pi + 2, 2 * pj:2 * pj + 2] = blk[:2, :2]
    return out


def parse_harmony(path: Path) -> dict[float, np.ndarray]:
    rows = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip()]
        if not parts:
            continue
        freq = float(parts[0])
        vals = []
        for token in parts[1:]:
            real, imag = token.replace(" ", "").split("+1i*(")
            vals.append(complex(float(real), float(imag[:-1])))
        n = int(round(math.sqrt(len(vals))))
        rows[freq] = np.array(vals, dtype=complex).reshape(n, n)
    return rows


def nearest(table: dict[float, np.ndarray], freq: float) -> np.ndarray:
    key = min(table, key=lambda f: abs(f - freq))
    if abs(key - freq) > 1e-4 * max(1.0, abs(freq)):
        raise KeyError(f"no sample at {freq} Hz (nearest {key})")
    return table[key]


def y_at(table: dict[float, np.ndarray], freq: float) -> np.ndarray:
    if freq < 0:
        return np.conj(y_at(table, -freq))
    return nearest(table, freq)


def load_pi(path: Path):
    text = path.read_text(encoding="utf-8").splitlines()
    header = text[0].split(",")
    n = int(round(math.sqrt((len(header) - 1) / 2)))
    freqs, mats = [], []
    for line in text[1:]:
        if not line.strip():
            continue
        nums = [float(x) for x in line.split(",")]
        freqs.append(nums[0])
        z = np.zeros((n, n), dtype=complex)
        k = 1
        for i in range(n):
            for j in range(n):
                z[i, j] = complex(nums[k], nums[k + 1])
                k += 2
        mats.append(z)
    return np.array(freqs), mats


def frob(a: np.ndarray, b: np.ndarray) -> float:
    num = np.linalg.norm(a - b)
    den = np.linalg.norm(b)
    return float(num / max(den, 1e-18))


def sending_z(y: np.ndarray, phases: int, g_far: float = 0.0) -> np.ndarray:
    """Far-end grounded (g_far=0) or terminated by g_far siemens per phase."""
    y11 = y[:phases, :phases]
    if g_far == 0.0:
        return np.linalg.inv(y11)
    y12 = y[:phases, phases:]
    y21 = y[phases:, :phases]
    y22 = y[phases:, phases:]
    g = g_far * np.eye(phases, dtype=complex)
    return np.linalg.inv(y11 - y12 @ np.linalg.inv(y22 + g) @ y21)


def ohl_component(name: str, length_m: float, phases: int, twin: bool, earth_rho: float) -> dict:
    if twin:
        conductor = {
            "organization": "flat",
            "number_bundles": [2, 2],
            "geometry": [11.8, 27.5],
            "rc": 0.01436,
            "Rdc": 0.06266,
            "dsag": 10.0,
            "dsb": 0.4572,
        }
        ground = {"count": 2, "geometry": [0.9196, 0.0062, 10.0, 7.5, 6.5], "mu_g": 1.0}
    else:
        conductor = {
            "organization": "flat",
            "number_bundles": [phases, 1],
            "geometry": [10.0, 30.0],
            "rc": 0.015,
            "Rdc": 0.063,
            "dsag": 10.0,
            "dsb": 0.0,
        }
        ground = {"count": 2, "geometry": [0.92, 0.0062, 10.0, 7.5, 6.5], "mu_g": 1.0}
    return {
        "id": name,
        "location": "AC1",
        "type": "overhead_line",
        "length": length_m,
        "earth": [1.0, 1.0, earth_rho],
        "conductor": conductor,
        "groundwire": ground,
        "connected_buses": [
            {"bus_id": name + "_a", "terminal": 1},
            {"bus_id": name + "_b", "terminal": 2},
        ],
        "enabled": True,
    }


def buses_for(name: str, phases: int) -> list:
    return [
        {"id": name + "_a", "location": "AC1", "pins": phases, "enabled": True},
        {"id": name + "_b", "location": "AC1", "pins": phases, "enabled": True},
    ]


def cable_component() -> dict:
    return {
        "id": "cable",
        "location": "DC1",
        "type": "cable",
        "cable_type": "underground",
        "length": 60000.0,
        "earth": [1.0, 1.0, 1.0],
        "conductors": [
            {"id": "C1", "ri": 0.0, "ro": 25.45e-3, "resistivity": 2.63e-8},
            {"id": "C2", "ri": 50.65e-3, "ro": 52.35e-3, "resistivity": 22e-8},
            {"id": "C3", "ri": 55.75e-3, "ro": 66.55e-3, "resistivity": 18e-8, "permeability": 10.0},
        ],
        "insulators": [
            {
                "id": "I1", "ri": 25.45e-3, "ro": 50.65e-3, "permittivity": 2.3,
                "inner_semiconductor_outer_radius": 25.45e-3 + 2e-3,
                "outer_semiconductor_inner_radius": 50.65e-3 - 1.5e-3,
            },
            {"id": "I2", "ri": 52.35e-3, "ro": 55.75e-3, "permittivity": 2.3},
            {"id": "I3", "ri": 66.55e-3, "ro": 71.75e-3, "permittivity": 2.3},
        ],
        "positions": [[0.0, 1.317150236], [-0.07175, 1.441424882], [0.07175, 1.441424882]],
        "connected_buses": [
            {"bus_id": "cable_a", "terminal": 1},
            {"bus_id": "cable_b", "terminal": 2},
        ],
        "enabled": True,
    }


def write_case(name: str, components: list, buses: list, freqs: np.ndarray) -> Path:
    HARM.mkdir(parents=True, exist_ok=True)
    doc = {
        "simulation": {
            "title": name,
            "description": name,
            "output_directory": "./files",
            "frequency_range": {"frequencies": [float(f) for f in freqs]},
        },
        "buses": buses,
        "components": components,
        "computations": [{
            "type": "y_matrix",
            "frequency_range": {"frequencies": [float(f) for f in freqs]},
            "plot": False,
        }],
    }
    path = HARM / f"{name}.json"
    path.write_text(json.dumps(doc, indent=2), encoding="utf-8")
    return path


def run_harmony(json_path: Path) -> None:
    env = os.environ.copy()
    env["PATH"] = ";".join([
        r"C:\Users\alekic\AppData\Local\miniconda3\envs\harmony\Library\bin",
        str(REPO / "build" / "tests" / "Release"),
        str(EXE.parent),
        r"C:\Windows\System32",
        r"C:\Windows",
    ])
    proc = subprocess.run(
        [str(EXE), "--json", str(json_path), "--no-plot"],
        cwd=str(REPO),
        env=env,
        text=True,
        encoding="utf-8",
        errors="replace",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if proc.returncode != 0:
        sys.stdout.write(proc.stdout or "")
        raise RuntimeError(f"Harmony failed on {json_path.name} ({proc.returncode})")


def sideband_freqs(freqs: np.ndarray) -> np.ndarray:
    extra = [abs(f - F0) for f in freqs if abs(f - F0) > 1e-6]
    extra += [f + F0 for f in freqs]
    return np.unique(np.concatenate([freqs, np.array(extra, dtype=float)]))


def stats(harm, pi) -> dict:
    errs = [frob(a, b) for a, b in zip(harm, pi)]
    return {"mean": float(np.mean(errs)), "max": float(np.max(errs)), "n": len(errs)}


def plot_case(path: Path, freqs, harm, pi_mats, entries, title: str) -> None:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    n = pi_mats[0].shape[0]
    labels = [f"Z{i + 1}{j + 1}" for i, j in entries]
    fig, axes = plt.subplots(3, len(entries), figsize=(4.2 * len(entries), 8.2), squeeze=False)
    for col, (i, j) in enumerate(entries):
        mag_h = [20 * np.log10(max(abs(z[i, j]), 1e-18)) for z in harm]
        mag_p = [20 * np.log10(max(abs(z[i, j]), 1e-18)) for z in pi_mats]
        ph_h = [np.angle(z[i, j], deg=True) for z in harm]
        ph_p = [np.angle(z[i, j], deg=True) for z in pi_mats]
        rel = [abs(a[i, j] - b[i, j]) / max(abs(b[i, j]), 1e-18) for a, b in zip(harm, pi_mats)]
        axes[0, col].semilogx(freqs, mag_h, color=H_COLOR, lw=2.4, label="Harmony")
        axes[0, col].semilogx(freqs, mag_p, color=PI_COLOR, lw=1.6, ls="--", label="PowerImpedance")
        axes[1, col].semilogx(freqs, ph_h, color=H_COLOR, lw=2.4)
        axes[1, col].semilogx(freqs, ph_p, color=PI_COLOR, lw=1.6, ls="--")
        axes[2, col].semilogx(freqs, rel, color="#334155", lw=1.6)
        axes[0, col].set_title(labels[col])
        axes[0, col].set_ylabel("Magnitude [dB]" if col == 0 else None)
        axes[1, col].set_ylabel("Phase [deg]" if col == 0 else None)
        axes[2, col].set_ylabel("Relative error" if col == 0 else None)
        axes[2, col].set_xlabel("Frequency [Hz]")
        if col == 0:
            axes[0, col].legend(frameon=False)
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(path, dpi=140, bbox_inches="tight", facecolor="white")
    fig.savefig(path.with_suffix(".svg"), bbox_inches="tight", facecolor="white")
    plt.close(fig)


def compare_saved(name, freqs, harm, pi_path, entries, title) -> dict:
    pi_freqs, pi_mats = load_pi(pi_path)
    if len(pi_freqs) != len(freqs) or np.max(np.abs(pi_freqs - freqs)) > 1e-4:
        raise RuntimeError(f"frequency grid mismatch for {name}")
    summary = stats(harm, pi_mats)
    summary["id"] = name
    PI.mkdir(parents=True, exist_ok=True)
    plot_case(PI / f"overlay_{name}.svg", freqs, harm, pi_mats, entries, title)
    print(f"{name}: mean {summary['mean']:.3e}  max {summary['max']:.3e}")
    return summary


def line_cases() -> list[dict]:
    freqs = logf(1.0, 1e4, 61)
    listed = sideband_freqs(freqs)
    out = []

    ac = ohl_component("acohl", 200e3, 3, False, 100.0)
    write_case("acohl", [ac], buses_for("acohl", 3), listed)
    run_harmony(HARM / "acohl.json")
    table = parse_harmony(FILES / "acohl.csv")
    phase = [sending_z(y_at(table, f), 3) for f in freqs]
    out.append(compare_saved(
        "ac_ohl_phase", freqs, phase, PI / "pi_ac_ohl_phase.csv",
        [(0, 0), (0, 1)], "200 km overhead line, phase sending impedance"))
    dq = []
    for f in freqs:
        ydq = park_a0(y_at(table, f - F0), y_at(table, f + F0))
        dq.append(sending_z(ydq, 2))
    out.append(compare_saved(
        "ac_ohl_dq", freqs, dq, PI / "pi_ac_ohl_dq.csv",
        [(0, 0), (0, 1)], "200 km overhead line, dq sending impedance"))

    dc = ohl_component("dcohl", 227e3, 2, True, 100.0)
    write_case("dcohl", [dc], buses_for("dcohl", 2), freqs)
    run_harmony(HARM / "dcohl.json")
    table = parse_harmony(FILES / "dcohl.csv")
    pi_freqs, pi_mats = load_pi(PI / "pi_dc_ohl.csv")
    if pi_mats[0].shape == (1, 1):
        reduced = []
        for f in freqs:
            z = sending_z(y_at(table, f), 2)
            reduced.append(np.array([[z[0, 0] - z[0, 1] - z[1, 0] + z[1, 1]]]))
    else:
        reduced = [sending_z(y_at(table, f), pi_mats[0].shape[0]) for f in freqs]
    out.append(compare_saved(
        "dc_ohl", freqs, reduced, PI / "pi_dc_ohl.csv",
        [(0, 0)], "227 km DC overhead line, sending impedance"))

    write_case("cable60", [cable_component()], buses_for("cable", 3), freqs)
    run_harmony(HARM / "cable60.json")
    table = parse_harmony(FILES / "cable.csv")
    cable_z = [sending_z(y_at(table, f), 3, g_far=1e3) for f in freqs]
    out.append(compare_saved(
        "cable", freqs, cable_z, PI / "pi_cable.csv",
        [(0, 0), (0, 1)], "60 km cable, 1 mΩ far-end termination"))
    return out


IEEE9_LINES = (
    ("tl78", "Bus7", "Bus8", 76e3),
    ("tl89", "Bus8", "Bus9", 106e3),
    ("tl75", "Bus7", "Bus5", 170e3),
    ("tl96", "Bus9", "Bus6", 179e3),
    ("tl54", "Bus5", "Bus4", 89e3),
    ("tl64", "Bus6", "Bus4", 97e3),
)
IEEE9_TRANSFORMERS = (
    ("t1", "Gnd1", "Bus4", 16.5 / 230, 4.9916e-4 / 2, 0.097 / 2),
    ("t2", "Bus2", "Bus7", 18 / 230, 6.4458e-4 / 2, 0.1052 / 2),
    ("t3", "Gnd3", "Bus9", 13.8 / 230, 3.5523e-4 / 2, 0.0987 / 2),
)
IEEE9_LOADS = (
    ("l5", "Bus5", 1269.6, 10.1),
    ("l6", "Bus6", 1763.3, 16.84),
    ("l8", "Bus8", 1587.16, 14.43),
)
IEEE9_FREE = ("Bus2", "Bus4", "Bus5", "Bus6", "Bus7", "Bus8", "Bus9")


def series_z(name: str, expression: str, bus_a: str, bus_b: str) -> dict:
    return {
        "id": name,
        "location": "AC1",
        "type": "impedance",
        "pins": 3,
        "z_expr": expression,
        "connected_buses": [
            {"bus_id": bus_a, "terminal": 1},
            {"bus_id": bus_b, "terminal": 2},
        ],
        "enabled": True,
    }


def transformer(name, bus_a, bus_b, ratio, lp, ls) -> dict:
    return {
        "id": name,
        "location": "AC1",
        "type": "transformer_yy_real",
        "pins": 3,
        "values": {
            "R_primary": 0.0,
            "L_primary": lp,
            "R_secondary": 0.0,
            "L_secondary": ls,
            "turns_ratio": ratio,
            "phase_shift": 0.0,
        },
        "connected_buses": [
            {"bus_id": bus_a, "terminal": 1},
            {"bus_id": bus_b, "terminal": 2},
        ],
        "enabled": True,
    }


def ieee9_cases() -> list[dict]:
    freqs = logf(1.0, 1e4, 41)
    components = []
    buses = {}
    for name, a, b, length in IEEE9_LINES:
        components.append(ohl_component(name, length, 3, False, 100.0))
        components[-1]["connected_buses"] = [
            {"bus_id": a, "terminal": 1},
            {"bus_id": b, "terminal": 2},
        ]
        buses[a] = 3
        buses[b] = 3
    for name, a, b, ratio, lp, ls in IEEE9_TRANSFORMERS:
        components.append(transformer(name, a, b, ratio, lp, ls))
        buses[a] = 3
        buses[b] = 3
    for name, bus, resistance, inductance in IEEE9_LOADS:
        components.append(series_z(name, f"{resistance} + {inductance}*s", bus, "gnd"))
        buses[bus] = 3
        buses["gnd"] = 3
    write_case(
        "ieee9",
        components,
        [{"id": bus, "location": "AC1", "pins": pins, "enabled": True} for bus, pins in buses.items()],
        sideband_freqs(freqs),
    )
    run_harmony(HARM / "ieee9.json")
    tables = {name: parse_harmony(FILES / f"{name}.csv") for name, *_ in (
        *[(n, None) for n, *_ in IEEE9_LINES],
        *[(n, None) for n, *_ in IEEE9_TRANSFORMERS],
        *[(n, None) for n, *_ in IEEE9_LOADS],
    )}
    # The comprehension above is awkward if a name collided. Rebuild explicitly.
    tables = {}
    for name, *_rest in IEEE9_LINES:
        tables[name] = parse_harmony(FILES / f"{name}.csv")
    for name, *_rest in IEEE9_TRANSFORMERS:
        tables[name] = parse_harmony(FILES / f"{name}.csv")
    for name, *_rest in IEEE9_LOADS:
        tables[name] = parse_harmony(FILES / f"{name}.csv")

    index = {bus: 2 * i for i, bus in enumerate(IEEE9_FREE)}
    n = 2 * len(IEEE9_FREE)

    def stamp(y: np.ndarray, ynode: np.ndarray, bus_i, bus_j, grounded_i=False, grounded_j=False):
        blocks = [(0, bus_i, grounded_i), (2, bus_j, grounded_j)]
        for off_a, bus_a, gnd_a in blocks:
            if gnd_a:
                continue
            ia = index[bus_a]
            for off_b, bus_b, gnd_b in blocks:
                if gnd_b:
                    continue
                ib = index[bus_b]
                ynode[ia:ia + 2, ib:ib + 2] += y[off_a:off_a + 2, off_b:off_b + 2]

    harm = []
    for f in freqs:
        ynode = np.zeros((n, n), dtype=complex)
        for name, a, b, _length in IEEE9_LINES:
            ydq = park_a0(y_at(tables[name], f - F0), y_at(tables[name], f + F0))
            stamp(ydq, ynode, a, b)
        for name, a, b, *_ in IEEE9_TRANSFORMERS:
            ydq = park_a0(y_at(tables[name], f - F0), y_at(tables[name], f + F0))
            stamp(ydq, ynode, a, b, grounded_i=a.startswith("Gnd"), grounded_j=b.startswith("Gnd"))
        for name, bus, *_ in IEEE9_LOADS:
            ydq = park_a0(y_at(tables[name], f - F0), y_at(tables[name], f + F0))
            stamp(ydq, ynode, bus, "gnd", grounded_j=True)
        z = np.linalg.inv(ynode)
        i2 = index["Bus2"]
        harm.append(z[i2:i2 + 2, i2:i2 + 2])
    return [
        compare_saved(
            "ieee9_bus2", freqs, harm, PI / "pi_ieee9_bus2_nocap.csv",
            [(0, 0), (0, 1)],
            "IEEE 9 Bus 2 dq impedance, transformer capacitance omitted"),
        compare_saved(
            "ieee9_bus2_with_capacitance", freqs, harm, PI / "pi_ieee9_bus2.csv",
            [(0, 0), (0, 1)],
            "IEEE 9 Bus 2 dq impedance, PowerImpedance keeps 7 nF and 12 nF"),
    ]


def ieee39_cases() -> list[dict]:
    freqs = logf(1.0, 5e3, 31)
    rb = (345e3) ** 2 / 1000e6
    lb = rb / (2 * math.pi * 50)
    lines = [
        ("T1_2", 1, 2, 72.4730e3), ("T1_39", 1, 39, 44.0833e3), ("T2_3", 2, 3, 26.6263e3),
        ("T2_25", 2, 25, 15.1647e3), ("T3_4", 3, 4, 37.5590e3), ("T3_18", 3, 18, 23.4523e3),
        ("T4_5", 4, 5, 22.5707e3), ("T4_14", 4, 14, 22.7470e3), ("T5_6", 5, 6, 4.5847e3),
        ("T5_8", 5, 8, 19.7493e3), ("T6_7", 6, 7, 16.2227e3), ("T6_11", 6, 11, 14.4593e3),
        ("T7_8", 7, 8, 8.1113e3), ("T8_9", 8, 9, 64.009e3), ("T9_39", 9, 39, 44.0833e3),
        ("T10_11", 10, 11, 7.5823e3), ("T10_13", 10, 13, 7.5823e3), ("T13_14", 13, 14, 17.8097e3),
        ("T14_15", 14, 15, 38.2643e3), ("T15_16", 15, 16, 16.5753e3), ("T16_17", 16, 17, 15.6937e3),
        ("T16_19", 16, 19, 34.3850e3), ("T16_21", 16, 21, 23.8050e3), ("T16_24", 16, 24, 10.4037e3),
        ("T17_18", 17, 18, 14.4593e3), ("T17_27", 17, 27, 30.5057e3), ("T21_22", 21, 22, 24.6867e3),
        ("T22_23", 22, 23, 16.9280e3), ("T23_24", 23, 24, 61.7167e3), ("T25_26", 25, 26, 56.95573e3),
        ("T26_27", 26, 27, 25.9210e3), ("T26_28", 26, 28, 83.5820e3), ("T26_29", 26, 29, 110.2083e3),
        ("T28_29", 28, 29, 26.6263e3),
    ]
    transformers = [
        ("TR2_30", 2, 30, 0, 0.0181), ("TR6_31", 6, 31, 0, 0.025), ("TR10_32", 10, 32, 0, 0.02),
        ("TR11_12", 11, 12, 0.0016, 0.0435), ("TR12_13", 12, 13, 0.0016, 0.0435),
        ("TR19_20", 19, 20, 0.0007, 0.0138), ("TR19_33", 19, 33, 0.0007, 0.0142),
        ("TR20_34", 20, 34, 0.0009, 0.0180), ("TR22_35", 22, 35, 0, 0.0143),
        ("TR23_36", 23, 36, 0.0005, 0.0272), ("TR25_37", 25, 37, 0.0006, 0.0232),
        ("TR29_38", 29, 38, 0.0008, 0.0156),
    ]
    loads = [
        ("LoadB3", 3, 107.3333, 0.8, "L"), ("LoadB4a", 4, 166.6667, 61.3333, "L"),
        ("LoadB4b", 4, 0, 100, "C"), ("LoadB5", 5, 0, 200, "C"),
        ("LoadB7", 7, 77.93333, 28.0, "L"), ("LoadB8", 8, 174.0, 58.6667, "L"),
        ("LoadB12", 12, 2.5, 29.3333, "L"), ("LoadB15", 15, 106.6667, 51.0, "L"),
        ("LoadB16", 16, 109.8, 10.7667, "L"), ("LoadB18", 18, 52.6667, 10.0, "L"),
        ("LoadB20", 20, 226.6667, 34.3333, "L"), ("LoadB21", 21, 91.3333, 38.3333, "L"),
        ("LoadB23", 23, 82.5, 28.2, "L"), ("LoadB24", 24, 102.8667, 30.73333, "C"),
        ("LoadB25", 25, 74.6667, 15.7333, "L"), ("LoadB26", 26, 46.3333, 5.6667, "L"),
        ("LoadB27", 27, 93.6667, 25.1667, "L"), ("LoadB28", 28, 68.6667, 9.2, "L"),
        ("LoadB29", 29, 94.5, 8.9667, "L"), ("LoadB32", 32, 3.0667, 1.5333, "L"),
        ("LoadB39", 39, 368, 83.3333, "L"),
    ]
    components = []
    for name, a, b, length in lines:
        comp = ohl_component(name, length, 3, False, 100.0)
        comp["connected_buses"] = [
            {"bus_id": f"Bus{a}", "terminal": 1},
            {"bus_id": f"Bus{b}", "terminal": 2},
        ]
        components.append(comp)
    for name, a, b, r, x in transformers:
        components.append(transformer(
            name, f"Bus{a}", f"Bus{b}", 1.0, (x / 2) * lb, (x / 2) * lb))
        components[-1]["values"]["R_primary"] = (r / 2) * rb
        components[-1]["values"]["R_secondary"] = (r / 2) * rb
    for bus in range(30, 40):
        components.append(series_z(f"Zg{bus}", "0.191736 + 0.0122063*s", "gnd", f"Bus{bus}"))
    for name, bus, p_mw, q_mvar, kind in loads:
        p = p_mw * 1e6
        q = q_mvar * 1e6
        current = 3 * math.sqrt(p * p + q * q) / (math.sqrt(3) * 345e3)
        resistance = p / current ** 2
        react = q / (current ** 2 * 2 * math.pi * 50) if kind == "L" else current ** 2 / (q * 2 * math.pi * 50)
        expr = f"{resistance} + {react}*s" if kind == "L" else f"{resistance} + 1/({react}*s)"
        components.append(series_z(name, expr, f"Bus{bus}", "gnd"))
    buses = [{"id": f"Bus{i}", "location": "AC1", "pins": 3, "enabled": True} for i in range(1, 40)]
    buses.append({"id": "gnd", "location": "AC1", "pins": 3, "enabled": True})
    write_case("ieee39", components, buses, sideband_freqs(freqs))
    run_harmony(HARM / "ieee39.json")
    tables = {}
    for comp in components:
        tables[comp["id"]] = parse_harmony(FILES / f"{comp['id']}.csv")
    free = [f"Bus{i}" for i in range(1, 40)]
    index = {bus: 2 * i for i, bus in enumerate(free)}
    n = 2 * len(free)
    harm = []
    for f in freqs:
        ynode = np.zeros((n, n), dtype=complex)
        for comp in components:
            ydq = park_a0(y_at(tables[comp["id"]], f - F0), y_at(tables[comp["id"]], f + F0))
            ends = comp["connected_buses"]
            a, b = ends[0]["bus_id"], ends[1]["bus_id"]
            # local stamp: side1 block rows 0:2, side2 rows 2:4
            for off_a, bus_a in ((0, a), (2, b)):
                if bus_a == "gnd":
                    continue
                ia = index[bus_a]
                for off_b, bus_b in ((0, a), (2, b)):
                    if bus_b == "gnd":
                        continue
                    ib = index[bus_b]
                    ynode[ia:ia + 2, ib:ib + 2] += ydq[off_a:off_a + 2, off_b:off_b + 2]
        z = np.linalg.inv(ynode)
        i9 = index["Bus9"]
        harm.append(z[i9:i9 + 2, i9:i9 + 2])
    return [compare_saved(
        "ieee39_bus9", freqs, harm, PI / "pi_ieee39_bus9.csv",
        [(0, 0), (0, 1)],
        "IEEE 39 Bus 9 dq impedance, STATCOM removed")]


def main() -> None:
    if not EXE.exists():
        raise SystemExit(f"missing {EXE}")
    summaries = line_cases() + ieee9_cases() + ieee39_cases()
    (PI / "summary.json").write_text(json.dumps(summaries, indent=2), encoding="utf-8")
    print(json.dumps(summaries, indent=2))


if __name__ == "__main__":
    main()
