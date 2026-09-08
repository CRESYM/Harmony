"""Run Harmony WT4 Y(f) against MATLAB WT4_RTDS_Implementation_v2 reduced-L.

From the repository root, with the ``harmony`` conda env and a Release
``Harmony.exe`` already built:

    conda run -n harmony python benchmarking/matlab_wt4/run.py
"""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

os.environ.setdefault("PYTHONIOENCODING", "utf-8")

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
FILES = REPO / "files"
RESULTS = ROOT / "results"

if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

import compare as wt4_cmp  # noqa: E402


def find_harmony() -> Path:
    candidates = [
        REPO / "build" / "Release" / "Harmony.exe",
        REPO / "build" / "Harmony.exe",
        REPO / "build" / "Harmony",
    ]
    for path in candidates:
        if path.exists():
            return path
    raise FileNotFoundError(
        "Harmony executable not found. Build Release once, then rerun this script."
    )


def find_matlab() -> Path:
    env = os.environ.get("MATLAB_EXE")
    if env:
        p = Path(env)
        if p.exists():
            return p
    candidates = [
        Path(r"C:\Program Files\MATLAB\R2026a\bin\matlab.exe"),
        Path(r"C:\Program Files\MATLAB\R2024\bin\matlab.exe"),
    ]
    for path in candidates:
        if path.exists():
            return path
    which = shutil.which("matlab")
    if which:
        return Path(which)
    raise FileNotFoundError("MATLAB executable not found. Set MATLAB_EXE or install MATLAB.")


def harmony_env() -> dict:
    env = os.environ.copy()
    extra_bins = []
    conda_prefix = env.get("CONDA_PREFIX") or str(
        Path.home() / "AppData" / "Local" / "miniconda3" / "envs" / "harmony"
    )
    extra_bins.append(str(Path(conda_prefix) / "Library" / "bin"))
    extra_bins.append(str(Path(conda_prefix) / "bin"))
    gurobi_home = Path(env.get("GUROBI_HOME", r"C:\gurobi1202\win64"))
    extra_bins.append(str(gurobi_home / "bin"))
    env["PATH"] = os.pathsep.join(extra_bins + [env.get("PATH", "")])
    env["PYTHONIOENCODING"] = "utf-8"
    return env


def run_harmony(harmony: Path, json_path: Path) -> None:
    cmd = [str(harmony), "--json", str(json_path), "--no-plot"]
    print(">", " ".join(cmd), flush=True)
    proc = subprocess.run(
        cmd,
        cwd=str(REPO),
        env=harmony_env(),
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if proc.returncode != 0:
        raise subprocess.CalledProcessError(proc.returncode, cmd)


def write_freq_csv(h_rows, path: Path) -> None:
    path.write_text("\n".join(f"{f:.16g}" for f, _ in h_rows) + "\n", encoding="utf-8")


def run_matlab(matlab: Path, freq_csv: Path, out_csv: Path) -> None:
    stmt = (
        f"cd('{ROOT.as_posix()}'); "
        f"export_wt4_y('{freq_csv.as_posix()}','{out_csv.as_posix()}');"
    )
    cmd = [str(matlab), "-batch", stmt]
    print(">", "matlab -batch export_wt4_y(...)", flush=True)
    proc = subprocess.run(cmd, cwd=str(ROOT), text=True, encoding="utf-8", errors="replace")
    if proc.returncode != 0:
        raise subprocess.CalledProcessError(proc.returncode, cmd)


def main() -> None:
    RESULTS.mkdir(parents=True, exist_ok=True)
    FILES.mkdir(parents=True, exist_ok=True)
    t0 = time.perf_counter()

    harmony = find_harmony()
    matlab = find_matlab()
    run_harmony(harmony, ROOT / "harmony" / "wt4_2p5mw.json")

    src = FILES / "PMSG.csv"
    if not src.exists():
        raise FileNotFoundError(f"Harmony did not write {src}")
    dst_h = RESULTS / "harmony_wt4.csv"
    shutil.copy2(src, dst_h)
    h_rows = wt4_cmp.parse_harmony(dst_h)
    if not h_rows:
        raise RuntimeError(f"Could not parse {dst_h}")

    freq_csv = RESULTS / "freq.csv"
    write_freq_csv(h_rows, freq_csv)
    dst_m = RESULTS / "matlab_wt4.csv"
    run_matlab(matlab, freq_csv, dst_m)
    m_rows = wt4_cmp.parse_matlab(dst_m)
    if len(m_rows) != len(h_rows):
        raise RuntimeError(f"Length mismatch: Harmony {len(h_rows)} vs MATLAB {len(m_rows)}")

    svg = RESULTS / "overlay_Y.svg"
    summary = wt4_cmp.plot_overlay(h_rows, m_rows, svg)
    elapsed = time.perf_counter() - t0
    wt4_cmp.print_spots(h_rows, m_rows)
    print(
        f"\nFrobenius ||Y_H-Y_M||/||Y_M||  n={summary['n']}  "
        f"mean={summary['mean_frob']:.4e}  max={summary['max_frob']:.4e} "
        f"@ {summary['f_max_hz']:.4g} Hz  wall={elapsed:.1f} s"
    )
    print(f"wrote {svg}")


if __name__ == "__main__":
    main()
