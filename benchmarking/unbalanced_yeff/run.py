"""Run the unbalanced MMC cases and write stacked H overlays.

From the repository root, with the ``harmony`` conda env and a Release
``Harmony.exe`` already built:

    conda run -n harmony python benchmarking/unbalanced_yeff/run.py

That is the only command needed to regenerate every SVG under ``rlc/results/``
and ``ohl/results/``.
Optional: ``--only ohl_conc`` (or ``rlc``, ``strong``, ``2gw``, ``ohl``).
"""
from __future__ import annotations

import argparse
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

if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

import plot as yeff_plot  # noqa: E402

CASES = [
    {
        "key": "rlc",
        "folder": "rlc",
        "json": "mmc_unbalanced_rlc.json",
        "a0": "MMC1_AC.csv",
        "yeff": "MMC1_AC_yeff.csv",
        "out": "overlay_H.svg",
        "title": r"R-only unbalance ($R_c=5\,\Omega$): $H=Y_n Z_{\mathrm{eq}}$",
    },
    {
        "key": "strong",
        "folder": "rlc",
        "json": "mmc_unbalanced_rlc_strong.json",
        "a0": "MMC1_AC_strong.csv",
        "yeff": "MMC1_AC_yeff_strong.csv",
        "out": "overlay_H_strong.svg",
        "title": r"R, L, C unbalance on phase $c$: $H=Y_n Z_{\mathrm{eq}}$",
    },
    {
        "key": "2gw",
        "folder": "rlc",
        "json": "mmc_unbalanced_rlc_2gw.json",
        "a0": "MMC1_AC_2gw.csv",
        "yeff": "MMC1_AC_yeff_2gw.csv",
        "out": "overlay_H_2gw.svg",
        "title": r"$2\,\mathrm{GW}$ RLC unbalance: $H=Y_n Z_{\mathrm{eq}}$",
    },
    {
        "key": "ohl",
        "folder": "ohl",
        "json": "mmc_unbalanced_ohl_2gw.json",
        "a0": "MMC1_AC_ohl.csv",
        "yeff": "MMC1_AC_yeff_ohl.csv",
        "out": "overlay_H_ohl.svg",
        "title": r"$2\,\mathrm{GW}$ untransposed vertical OHL ($150\,\mathrm{km}$): $H=Y_n Z_{\mathrm{eq}}$",
    },
    {
        "key": "ohl_conc",
        "folder": "ohl",
        "json": "mmc_unbalanced_ohl_conc_2gw.json",
        "a0": "MMC1_AC_ohl_conc.csv",
        "yeff": "MMC1_AC_yeff_ohl_conc.csv",
        "out": "overlay_H_ohl_conc.svg",
        "title": r"$2\,\mathrm{GW}$ untransposed offset-vertical OHL ($250\,\mathrm{km}$): $H=Y_n Z_{\mathrm{eq}}$",
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
    raise FileNotFoundError(
        "Harmony executable not found. Build Release once, then rerun this script."
    )


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


def select_cases(keys: list[str] | None) -> list[dict]:
    if not keys:
        return CASES
    by_key = {c["key"]: c for c in CASES}
    unknown = [k for k in keys if k not in by_key]
    if unknown:
        raise SystemExit(
            "Unknown case(s): "
            + ", ".join(unknown)
            + ". Choose from: "
            + ", ".join(c["key"] for c in CASES)
        )
    return [by_key[k] for k in keys]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--only",
        nargs="+",
        metavar="CASE",
        help="Subset of cases: rlc strong 2gw ohl ohl_conc (default: all)",
    )
    args = parser.parse_args()

    FILES.mkdir(parents=True, exist_ok=True)
    harmony = find_harmony()
    cases = select_cases(args.only)
    summary = []
    t_all = time.perf_counter()

    for case in cases:
        outdir = ROOT / case["folder"] / "results"
        outdir.mkdir(parents=True, exist_ok=True)
        t0 = time.perf_counter()
        run_harmony(harmony, ROOT / case["folder"] / case["json"])
        src_a0 = FILES / "MMC1_AC.csv"
        src_yeff = FILES / "MMC1_AC_yeff.csv"
        if not src_a0.exists() or not src_yeff.exists():
            raise FileNotFoundError(f"Harmony did not write TF CSVs for {case['json']}")
        dst_a0 = outdir / case["a0"]
        dst_yeff = outdir / case["yeff"]
        shutil.copy2(src_a0, dst_a0)
        shutil.copy2(src_yeff, dst_yeff)
        print(f"copied TF CSVs -> {case['a0']}, {case['yeff']}", flush=True)
        a0 = yeff_plot.parse_harmony(dst_a0)
        yeff = yeff_plot.parse_harmony(dst_yeff)
        if not a0 or not yeff:
            raise FileNotFoundError(f"Could not parse TF CSVs for {case['json']}")
        stats = yeff_plot.plot_tf(a0, yeff, outdir / case["out"], title=case["title"])
        elapsed = time.perf_counter() - t0
        stats["wall_s"] = elapsed
        print(
            f"wrote {outdir / case['out']}  samples={stats['n']}  "
            f"mean Frob={stats['mean_frob']:.4e}  max={stats['max_frob']:.4e} "
            f"@ {stats['f_max_hz']:.4g} Hz  wall={elapsed:.1f} s",
            flush=True,
        )
        summary.append((case["key"], stats, outdir))

    for folder in ("rlc", "ohl"):
        for path in (ROOT / folder / "results").glob("*.csv"):
            path.unlink()
            print(f"removed {path.name}", flush=True)

    print("\nRelative Frobenius ||H_eff-A0||/||A0||")
    for key, stats, _ in summary:
        print(
            f"  {key:8s}  mean={stats['mean_frob']:.4e}  "
            f"max={stats['max_frob']:.4e}  @ {stats['f_max_hz']:.4g} Hz  "
            f"{stats['wall_s']:.1f} s"
        )
    print(f"  total wall {time.perf_counter() - t_all:.1f} s")


if __name__ == "__main__":
    main()
