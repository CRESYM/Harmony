from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

def overlay(frequencies, measured, curves, target, title):
    fig, axes = plt.subplots(4, 3, figsize=(14, 14), constrained_layout=True)
    for k, name in enumerate(["Ydd", "Ydq", "Yqd", "Yqq"]):
        axes[k, 0].semilogx(frequencies, 20 * np.log10(np.maximum(abs(measured[:, k]), 1e-30)),
                           "k--", lw=1.7, label="PSCAD archived measurement")
        axes[k, 1].semilogx(frequencies, np.rad2deg(np.unwrap(np.angle(measured[:, k]))), "k--")
        for label, predicted in curves.items():
            axes[k, 0].semilogx(frequencies, 20 * np.log10(np.maximum(abs(predicted[:, k]), 1e-30)), label=label)
            axes[k, 1].semilogx(frequencies, np.rad2deg(np.unwrap(np.angle(predicted[:, k]))))
            error = abs(predicted[:, k] - measured[:, k]) / np.maximum(abs(measured[:, k]), 1e-30)
            axes[k, 2].loglog(frequencies, np.maximum(100 * error, 1e-10))
        for j, ylabel in enumerate(["Magnitude (dB S)", "Phase (degrees)", "Relative error (%)"]):
            axes[k, j].set(xlabel="Frequency (Hz)", ylabel=f"{name}: {ylabel}")
            axes[k, j].grid(True, which="both", alpha=0.25)
    axes[0, 0].legend(fontsize=8)
    fig.suptitle(title)
    fig.savefig(target, dpi=160)
    fig.savefig(target.with_suffix(".svg"))
    plt.close(fig)
