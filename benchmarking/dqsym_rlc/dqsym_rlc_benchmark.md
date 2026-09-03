# DQsym RLC benchmark 

The Python and Julia implementations use the same matrices, inputs, time step
($2\times10^{-5}$ s), frequency (50 Hz), and 25,000 simulation steps as
Harmony's `example_DQsym_RLC`.

## Performance trend

| Implementation | Runtime (s) | Peak memory (MB) |
|---|---:|---:|
| Harmony C++/Eigen | 0.084 | 18.8 |
| Python/NumPy | 4.714 | 32.1 |
| Julia | 0.078 | 375.9 |