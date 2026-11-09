## Point-to-point HVDC

P2P HVDC at c2 AC (B6) against
[PowerImpedance.jl](https://github.com/Electa-Git/PowerImpedance.jl) 0.4.0:
[`powerimpedance/`](powerimpedance/).

```bash
python benchmarking/powerimpedance/run_all.py
```

See [`powerimpedance/README.md`](powerimpedance/README.md).

| Quantity | Mean Frobenius rel |
|----------|--------------------|
| DC cable $Y$ | $1.16\times 10^{-5}$ |
| $Y_n$ / $Z_{\mathrm{eq}}$ / $H$ | $6.83\times 10^{-5}$ / $4.38\times 10^{-3}$ / $4.35\times 10^{-3}$ |
| $Z_{\mathrm{in}}$ | $7.28\times 10^{-2}$ |
