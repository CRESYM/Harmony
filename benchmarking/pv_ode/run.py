"""Sweep Ydq of PVplant and PVgeneric and write the overlay.

From the repository root, with a Release Harmony.exe:

    python benchmarking/pv_ode/run.py
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

import compare as pv_cmp  # noqa: E402


def find_harmony() -> Path:
    candidates = [
        REPO / "build" / "Release" / "Harmony.exe",
        REPO / "build" / "Harmony.exe",
        REPO / "build" / "Harmony",
    ]
    for path in candidates:
        if path.exists():
            return path
    raise FileNotFoundError("Harmony executable not found. Build Release, then rerun.")


def harmony_env() -> dict:
    env = os.environ.copy()
    conda_prefix = env.get("CONDA_PREFIX") or str(
        Path.home() / "AppData" / "Local" / "miniconda3" / "envs" / "harmony"
    )
    extra = [
        str(Path(conda_prefix) / "Library" / "bin"),
        str(Path(conda_prefix) / "bin"),
        str(Path(env.get("GUROBI_HOME", r"C:\gurobi1202\win64")) / "bin"),
    ]
    env["PATH"] = os.pathsep.join(extra + [env.get("PATH", "")])
    env["PYTHONIOENCODING"] = "utf-8"
    return env


def main() -> None:
    RESULTS.mkdir(parents=True, exist_ok=True)
    FILES.mkdir(parents=True, exist_ok=True)
    t0 = time.perf_counter()
    harmony = find_harmony()
    cmd = [str(harmony), "--cpp", "pv_ydq_compare", "--no-plot"]
    print(">", " ".join(cmd), flush=True)
    proc = subprocess.run(cmd, cwd=str(REPO), env=harmony_env(), text=True, encoding="utf-8", errors="replace")
    if proc.returncode != 0:
        if proc.stdout:
            print(proc.stdout)
        if proc.stderr:
            print(proc.stderr)
        raise subprocess.CalledProcessError(proc.returncode, cmd)
    if proc.stdout:
        print(proc.stdout)

    old_src = FILES / "PV_old.csv"
    ode_src = FILES / "PV_ode.csv"
    if not old_src.exists() or not ode_src.exists():
        raise FileNotFoundError("Harmony did not write files/PV_old.csv and files/PV_ode.csv")
    shutil.copy2(old_src, RESULTS / "pv_old.csv")
    shutil.copy2(ode_src, RESULTS / "pv_ode.csv")

    old_rows = pv_cmp.parse_harmony(RESULTS / "pv_old.csv")
    ode_rows = pv_cmp.parse_harmony(RESULTS / "pv_ode.csv")
    if not old_rows or not ode_rows:
        raise RuntimeError("Could not parse the Ydq sweeps.")
    ode_rows, flipped = pv_cmp.align_sign(old_rows, ode_rows)
    svg = RESULTS / "overlay_Y.svg"
    summary = pv_cmp.plot_overlay(old_rows, ode_rows, svg, flipped)
    elapsed = time.perf_counter() - t0
    sign = "ODE plotted as -Y (generator current)" if flipped else "same sign"
    print(
        f"Frobenius ||Y_old-Y_ode||/||Y_old||  n={summary['n']}  "
        f"mean={summary['mean_frob']:.4e}  max={summary['max_frob']:.4e} "
        f"@ {summary['f_max_hz']:.4g} Hz  {sign}  wall={elapsed:.1f} s"
    )
    print(f"wrote {svg}")


if __name__ == "__main__":
    main()
