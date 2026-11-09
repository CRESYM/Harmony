"""Run the point-to-point HVDC comparison, then compare Y(f)."""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
FILES = REPO / "files"
P2P = ROOT / "p2p"
RESULTS = P2P / "results"
JSON = P2P / "harmony" / "p2p_pi_op.json"
HARMONY_CSVS = ["dc_line.csv", "c2_AC.csv", "c2_AC_Zin.csv", "harmony_linearization_op.csv"]
COMPARE_IDS = ["p2p_cable", "p2p_zin", "p2p_tf", "p2p_yn", "p2p_zeq"]


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


def run(cmd, cwd: Path, env=None) -> None:
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


def harmony_env() -> dict[str, str]:
    env = os.environ.copy()
    conda_prefix = env.get("CONDA_PREFIX") or str(
        Path.home() / "AppData" / "Local" / "miniconda3" / "envs" / "harmony"
    )
    extra_bins = [
        str(Path(conda_prefix) / "Library" / "bin"),
        str(Path(conda_prefix) / "bin"),
        str(Path(env.get("GUROBI_HOME", r"C:\gurobi1202\win64")) / "bin"),
    ]
    env["PATH"] = os.pathsep.join(extra_bins + [env.get("PATH", "")])
    return env


def main() -> None:
    FILES.mkdir(parents=True, exist_ok=True)
    RESULTS.mkdir(parents=True, exist_ok=True)
    subprocess.run([sys.executable, str(P2P / "build_network_cases.py")], cwd=str(P2P), check=True)

    julia = shutil.which("julia")
    if not julia:
        raise FileNotFoundError("julia is not on PATH")
    run([julia, f"--project={ROOT}", str(P2P / "run_pi_networks.jl")], cwd=P2P)

    run([str(find_harmony()), "--json", str(JSON), "--no-plot"], cwd=REPO, env=harmony_env())
    for name in HARMONY_CSVS:
        src = FILES / name
        if not src.exists():
            print(f"warning: Harmony did not write {src}")
            continue
        shutil.copy2(src, RESULTS / name)
        print(f"copied {src} -> {RESULTS / name}")

    compare = [sys.executable, str(ROOT / "compare.py")]
    for case_id in COMPARE_IDS:
        compare.extend(["--case", case_id])
    run(compare, cwd=ROOT)

    for path in list(RESULTS.glob("*.csv")):
        path.unlink()
        print(f"removed {path.name}")


if __name__ == "__main__":
    main()
