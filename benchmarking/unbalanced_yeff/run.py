"""Run the unbalanced MMC cases and write stacked H overlays."""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
RESULTS = ROOT / "results"
FILES = REPO / "files"

CASES = [
    {
        "json": "mmc_unbalanced_rlc.json",
        "a0": "MMC1_AC.csv",
        "yeff": "MMC1_AC_yeff.csv",
        "out": "overlay_H.svg",
        "title": r"R-only unbalance ($R_c=5\,\Omega$): $H=Y_n Z_{\mathrm{eq}}$",
    },
    {
        "json": "mmc_unbalanced_rlc_strong.json",
        "a0": "MMC1_AC_strong.csv",
        "yeff": "MMC1_AC_yeff_strong.csv",
        "out": "overlay_H_strong.svg",
        "title": r"R, L, C unbalance on phase $c$: $H=Y_n Z_{\mathrm{eq}}$",
    },
]


def find_harmony() -> Path:
    candidates = [
        REPO / "build" / "Release" / "Harmony.exe",
        REPO / "build" / "Harmony.exe",
        REPO / "build" / "Harmony",
    ]
    for path in candidates:
        if path.exists():
            return path
    raise FileNotFoundError("Harmony executable not found. Build Release first.")


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
    return env


def run_harmony(harmony: Path, json_path: Path) -> None:
    cmd = [str(harmony), "--json", str(json_path), "--no-plot"]
    print(">", " ".join(cmd))
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


def main() -> None:
    RESULTS.mkdir(parents=True, exist_ok=True)
    FILES.mkdir(parents=True, exist_ok=True)
    harmony = find_harmony()

    for case in CASES:
        run_harmony(harmony, ROOT / "harmony" / case["json"])
        src_a0 = FILES / "MMC1_AC.csv"
        src_yeff = FILES / "MMC1_AC_yeff.csv"
        if not src_a0.exists() or not src_yeff.exists():
            raise FileNotFoundError(f"Harmony did not write TF CSVs for {case['json']}")
        shutil.copy2(src_a0, RESULTS / case["a0"])
        shutil.copy2(src_yeff, RESULTS / case["yeff"])
        print(f"copied TF CSVs -> {case['a0']}, {case['yeff']}")
        subprocess.run(
            [
                sys.executable,
                str(ROOT / "plot.py"),
                "--a0",
                case["a0"],
                "--yeff",
                case["yeff"],
                "--out",
                case["out"],
                "--title",
                case["title"],
            ],
            cwd=str(ROOT),
            check=True,
        )

    for path in RESULTS.glob("*.csv"):
        path.unlink()
        print(f"removed {path.name}")


if __name__ == "__main__":
    main()
