"""Run PowerImpedance.jl and Harmony JSON cases, then compare Y(f)."""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
RESULTS = ROOT / "results"
HARMONY_JSON = ROOT / "harmony"
FILES = REPO / "files"

CASES = [
    {"id": "resistor", "json": "resistor.json", "harmony_csv": "R1.csv"},
    {"id": "transformer_yy", "json": "transformer_yy.json", "harmony_csv": "T_YY.csv"},
    {"id": "cable", "json": "cable.json", "harmony_csv": "cable.csv"},
    {"id": "ohl", "json": "ohl.json", "harmony_csv": "ohl.csv"},
    {"id": "mmc_gfl", "json": "mmc_gfl.json", "harmony_csv": "MMC1.csv"},
    {"id": "mmc_c1", "json": "mmc_c1.json", "harmony_csv": "c1.csv"},
    {"id": "mmc_c2", "json": "mmc_c2.json", "harmony_csv": "c2.csv"},
    {"id": "p2p", "json": "p2p.json", "harmony_csvs": ["dc_line.csv", "c2_AC.csv", "c2_AC_Zin.csv", "harmony_linearization_op.csv"]},
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


def run(cmd, cwd: Path, env=None) -> tuple[float, str]:
    print(">", " ".join(str(c) for c in cmd))
    t0 = time.perf_counter()
    proc = subprocess.run(
        cmd,
        cwd=str(cwd),
        env=env,
        text=True,
        encoding="utf-8",
        errors="replace",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    elapsed = time.perf_counter() - t0
    out = proc.stdout or ""
    sys.stdout.write(out)
    if not out.endswith("\n"):
        sys.stdout.write("\n")
    print(f"  wall {elapsed:.3f} s")
    if proc.returncode != 0:
        raise subprocess.CalledProcessError(proc.returncode, cmd)
    return elapsed, out


def parse_julia_timings(output: str) -> dict[str, float]:
    times = {}
    for line in output.splitlines():
        if not line.startswith("TIMING"):
            continue
        parts = line.split()
        if len(parts) >= 3:
            times[parts[1]] = float(parts[2])
    return times


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--skip-julia", action="store_true")
    parser.add_argument("--skip-harmony", action="store_true")
    parser.add_argument("--skip-networks", action="store_true")
    parser.add_argument("--case", action="append")
    args = parser.parse_args()
    wanted = set(args.case) if args.case else None
    network_ids = {
        "p2p",
        "p2p_cable",
        "p2p_zin",
        "p2p_tf",
    }

    timings = {"harmony": {}, "powerimpedance": {}, "wall": {}}
    t_all = time.perf_counter()

    RESULTS.mkdir(parents=True, exist_ok=True)
    FILES.mkdir(parents=True, exist_ok=True)

    subprocess.run([sys.executable, str(ROOT / "build_network_cases.py")], cwd=str(ROOT), check=True)

    if not args.skip_julia:
        julia = shutil.which("julia")
        if not julia:
            raise FileNotFoundError("julia is not on PATH")
        standalone = [c for c in CASES if c["id"] != "p2p"]
        run_standalone = wanted is None or any(c["id"] in wanted for c in standalone)
        run_networks = (not args.skip_networks) and (wanted is None or bool(wanted & network_ids))
        if run_standalone:
            elapsed, out = run([julia, f"--project={ROOT}", str(ROOT / "run_pi.jl")], cwd=ROOT)
            timings["wall"]["pi_standalone"] = elapsed
            timings["powerimpedance"].update(parse_julia_timings(out))
        if run_networks:
            elapsed, out = run([julia, f"--project={ROOT}", str(ROOT / "run_pi_networks.jl")], cwd=ROOT)
            timings["wall"]["pi_networks"] = elapsed
            timings["powerimpedance"].update(parse_julia_timings(out))

    if not args.skip_harmony:
        harmony = find_harmony()
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
        for case in CASES:
            if wanted and case["id"] not in wanted:
                continue
            json_path = HARMONY_JSON / case["json"]
            print(f"\n--- Harmony {case['id']} ---")
            elapsed, _ = run(
                [str(harmony), "--json", str(json_path), "--no-plot"],
                cwd=REPO,
                env=env,
            )
            timings["harmony"][case["id"]] = elapsed
            csvs = case.get("harmony_csvs") or [case["harmony_csv"]]
            for name in csvs:
                src = FILES / name
                dst = RESULTS / name
                if not src.exists():
                    print(f"warning: Harmony did not write {src}")
                    continue
                shutil.copy2(src, dst)
                print(f"copied {src} -> {dst}")

    compare = [sys.executable, str(ROOT / "compare.py")]
    compare_ids = set()
    if wanted:
        mapping = {
            "p2p": ["p2p_cable", "p2p_zin", "p2p_tf"],
            "mmc_c1": ["mmc_c1"],
            "mmc_c2": ["mmc_c2"],
        }
        for case_id in wanted:
            compare_ids.update(mapping.get(case_id, [case_id]))
        for case_id in sorted(compare_ids):
            compare.extend(["--case", case_id])
    elapsed, _ = run(compare, cwd=ROOT)
    timings["wall"]["compare"] = elapsed
    timings["wall"]["total"] = time.perf_counter() - t_all
    print("Execution times (s):")
    for label, sec in timings["powerimpedance"].items():
        print(f"  PI     {label:20s} {sec:8.3f}")
    for label, sec in timings["harmony"].items():
        print(f"  Harmony {label:20s} {sec:8.3f}")
    for label, sec in timings["wall"].items():
        print(f"  wall   {label:20s} {sec:8.3f}")

    for path in list(RESULTS.glob("*.csv")) + [RESULTS / "timings.json"]:
        if path.exists():
            path.unlink()
            print(f"removed {path.name}")


if __name__ == "__main__":
    main()
