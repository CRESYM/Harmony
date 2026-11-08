from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

root=Path(__file__).resolve().parent
data=np.load(root/"results/benchmark_response.npz")
fig,axes=plt.subplots(2,2,figsize=(6.3,4),constrained_layout=True)
for k,ax in enumerate(axes.flat):
    for key,style,label in [("measured_y","--","PSCAD"),("predicted_y","-","Harmony")]:
        values=data[key].reshape(-1,4)[:,k]
        ax.semilogx(data["frequencies"],20*np.log10(np.maximum(abs(values),1e-30)),style,color="black",lw=1.15,label=label)
    ax.set(title=["$Y_{dd}$","$Y_{dq}$","$Y_{qd}$","$Y_{qq}$"][k],xlabel="Frequency (Hz)",ylabel="Magnitude (dB S)")
    ax.tick_params(labelsize=8)
    ax.grid(True,alpha=.15)
axes[0,0].legend(fontsize=7)
fig.savefig(root/"results/admittance_magnitudes.png",dpi=320)
fig.savefig(root/"results/admittance_magnitudes.svg")
plt.close(fig)
