"""Replay every saved closed-loop multi-tone scan with the hybrid V3 model.

The 40 perturbation cases and the baseline are integrated as an array batch.
Only voltage waveforms determine the excitation; measured currents are used
solely for validation. No admittance or controller parameters are fitted.
"""
from pathlib import Path
import json
import sys
import time

import numpy as np
sys.path.insert(0, str(Path(__file__).resolve().parent / "harmony"))
from model_harmony_mmc import HarmonyMmcGfl

BASE = Path(__file__).resolve().parent
from compare import overlay

def parse_pscad_ztool(path):
    rows=[]
    seen=set()
    for line in path.read_text().splitlines()[1:]:
        parts=line.split()
        if len(parts)<5: continue
        f=float(complex(parts[0]).real)
        if f in seen: continue
        seen.add(f)
        rows.append((f,[complex(v) for v in parts[1:5]]))
    return rows

def statistics(frequencies, predicted, measured):
    errors=np.linalg.norm(predicted-measured,axis=1)/np.linalg.norm(measured,axis=1)
    return {"points":len(frequencies),"mean_frobenius_percent":float(100*errors.mean()),
        "max_frobenius_percent":float(100*errors.max()),
        "max_error_frequency_Hz":float(frequencies[errors.argmax()]),
        "acceptance_mean_percent":5.0,"target_met":bool(errors.mean()<.05),
        "bands_percent":{f"{lo}-{min(hi,1000)} Hz":float(100*errors[(frequencies>=lo)&(frequencies<hi)].mean()) for lo,hi in [(1,10),(10,50),(50,100),(100,500),(500,1001)]}},errors


def main():
    started = time.monotonic()
    out = BASE / "results"
    out.mkdir(exist_ok=True)
    excitation=np.load(BASE / "data/voltage_excitation.npz")
    groups=excitation["groups"]
    coefficients=excitation["coefficients"]
    case_frequencies=excitation["case_frequencies"]
    max_residual=float(excitation["maximum_reconstruction_error_V"])
    model = HarmonyMmcGfl(Pac=99.8456e6, Qac=-0.320945e6)
    model.solve_equilibrium()
    initial = np.repeat(model.x_eq[:, None], 41, axis=1)
    model.Pac_ref = 100e6
    model.Qac_ref = 0
    angular = 2 * np.pi * case_frequencies

    def inputs(t):
        angles = angular * t
        perturbation = np.einsum("bi,bic->bc", np.cos(angles), coefficients[:, :8])
        perturbation += np.einsum("bi,bic->bc", np.sin(angles), coefficients[:, 8:16])
        values = np.repeat(model.u0[:, None], 41, axis=1)
        values[1:, 1:] += perturbation.T
        return values

    # Periodic progress is emitted by the input callback, at most once per
    # quarter simulation second. This does not alter integration state.
    last_progress = [-1]
    def monitored_inputs(t):
        quarter = int(t * 4)
        if quarter > last_progress[0]:
            last_progress[0] = quarter
            print(f"Integrated time {t:.2f}/2.00 s; elapsed {time.monotonic()-started:.1f}s", flush=True)
        return inputs(t)

    times, states, freezes = model.simulate(2, monitored_inputs, timestep=20e-6,
                                           output_stride=13, initial_state=initial)
    select = times >= 1
    t = times[select]
    response = -(states[select, 12:14, 1:] - states[select, 12:14, :1])
    predicted = []
    measured_rows = parse_pscad_ztool(BASE / "data" / "pscad_measured.txt")
    measured_by_frequency = {f: np.array(y) for f, y in measured_rows}
    for batch, frequencies in enumerate(groups):
        basis = np.column_stack([np.cos(2*np.pi*t[:, None]*frequencies),
                                 np.sin(2*np.pi*t[:, None]*frequencies),
                                 np.ones(len(t))])
        phasors = []
        voltage_phasors = []
        for case in [batch, batch + 20]:
            fit = np.linalg.lstsq(basis, response[:, :, case], rcond=None)[0]
            phasors.append(fit[:8] - 1j * fit[8:16])
            voltage_phasors.append(coefficients[case, :8] - 1j * coefficients[case, 8:16])
        for j, frequency in enumerate(frequencies):
            y = np.column_stack([p[j] for p in phasors]) @ np.linalg.inv(np.column_stack([v[j] for v in voltage_phasors]))
            predicted.append((frequency, y.reshape(4)))
    predicted.sort(key=lambda r: r[0])
    frequencies = np.array([r[0] for r in predicted])
    y = np.array([r[1] for r in predicted])
    measured = np.array([measured_by_frequency[f] for f in frequencies])
    stats, errors = statistics(frequencies, y, measured)
    stats["maximum_voltage_reconstruction_error_V"] = max_residual
    stats["description"] = "Finite-amplitude hybrid averaged replay at measured voltage excitations; comparison directly against saved Z-tool FFT admittance. No current-response fitting."
    stats["timestep_us"] = 20
    stats["controller_timestep_us"] = 20
    stats["sampled_modulation"] = True
    (out / "benchmark_metrics.json").write_text(json.dumps(stats, indent=2) + "\n")
    np.savez(out / "benchmark_response.npz", frequencies=frequencies, predicted_y=y, measured_y=measured, errors=errors)
    with (out / "harmony_admittance.csv").open("w") as stream:
        stream.write("frequency_Hz,Ydd,Ydq,Yqd,Yqq\n")
        for f, row in zip(frequencies, y):
            stream.write(str(f) + "," + ",".join(str(z) for z in row) + "\n")
    overlay(frequencies, measured, {"Harmony": y}, out / "overlay_Y.png",
            "GFL MMC: Harmony vs archived PSCAD measurements")
    print(json.dumps(stats, indent=2), flush=True)
    if not stats["target_met"]: raise SystemExit("Mean error exceeds the 5% acceptance criterion")


if __name__ == "__main__":
    main()
