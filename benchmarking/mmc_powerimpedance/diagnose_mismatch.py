import json
from pathlib import Path

s = json.loads(Path("benchmarking/mmc_powerimpedance/results/comparison_summary.json").read_text())
ranked = sorted(s["spectrum"], key=lambda p: -p["frob_rel"])
print("TOP 8 Frob:")
for p in ranked[:8]:
    print(
        f"  f={p['f']:.3f} Frob={p['frob_rel']:.3f} "
        f"Yddc={p['Yddc_rel']:.3f} Ydd={p['Ydd_rel']:.3f} Yqq={p['Yqq_rel']:.3f}"
    )

for target in [1, 34.55, 49.24, 83.77, 100, 492, 838]:
    sp = min(s["spectrum"], key=lambda t: abs(t["f"] - target))
    print(f"--- f~{sp['f']:.2f} Frob={sp['frob_rel']:.3f} ---")
    print(f"  H Yddc={sp['harmony_Yddc']} PI={sp['pi_Yddc']}")
    print(f"  H Ydd ={sp['harmony_Ydd']} PI={sp['pi_Ydd']}")
    print(f"  H Yqq ={sp['harmony_Yqq']} PI={sp['pi_Yqq']}")

# Also dump full entry table at spike and at 50 Hz from spot_checks
for target in [1.0, 49.2388, 83.7678, 492.388]:
    # rebuild from spot if available else skip
    pass

# Parse raw CSVs at spike frequency for all 9 entries
import re, csv, math

def parse_harmony(path):
    rows = []
    pat = re.compile(
        r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\+1i\*\(([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\)"
    )
    for line in Path(path).read_text().splitlines():
        parts = [p.strip() for p in line.strip().rstrip(",").split(",") if p.strip()]
        if not parts:
            continue
        f = float(parts[0])
        vals = []
        for p in parts[1:]:
            m = pat.match(p.replace(" ", ""))
            if not m:
                break
            vals.append(complex(float(m.group(1)), float(m.group(2))))
        if len(vals) == 9:
            rows.append((f, vals))
    return rows

def parse_pi(path):
    rows = []
    with open(path, newline="") as fh:
        for r in csv.DictReader(fh):
            f = float(r["freq_Hz"])
            vals = [complex(float(r[f"Re_Y{i}{j}"]), float(r[f"Im_Y{i}{j}"]))
                    for i in range(1,4) for j in range(1,4)]
            rows.append((f, vals))
    return rows

h = parse_harmony("benchmarking/mmc_powerimpedance/results/MMC1.csv")
p = parse_pi("benchmarking/mmc_powerimpedance/results/powerimpedance_Y.csv")
names = ["Yddc","Ydcd","Ydcq","Yadc","Ydd","Ydq","Yqdc","Yqd","Yqq"]

def nearest(rows, f):
    return min(rows, key=lambda r: abs(r[0]-f))

print("\n=== FULL 3x3 at key frequencies ===")
for ftarget in [1.0, 49.24, 83.77, 492.4]:
    hf, hv = nearest(h, ftarget)
    pf, pv = nearest(p, ftarget)
    print(f"\nf_H={hf:.3f} f_PI={pf:.3f}")
    for k,n in enumerate(names):
        e = abs(hv[k]-pv[k]) / max(abs(hv[k]), abs(pv[k]), 1e-18)
        # also phase difference
        import cmath
        ph = abs(cmath.phase(hv[k]) - cmath.phase(pv[k])) if abs(hv[k])>1e-12 and abs(pv[k])>1e-12 else float('nan')
        if ph > math.pi:
            ph = 2*math.pi - ph
        print(f"  {n:6s} H={hv[k].real:+.4e}{hv[k].imag:+.4e}j  "
              f"PI={pv[k].real:+.4e}{pv[k].imag:+.4e}j  "
              f"rel={e:.3f}  dphase={ph*180/math.pi:.1f}deg")
