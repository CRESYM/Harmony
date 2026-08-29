#!/usr/bin/env julia
# Bootstrap an isolated Julia env with PowerImpedance 0.3.x (not global ACDC 0.1).
using Pkg

const BENCH_DIR = @__DIR__
Pkg.activate(BENCH_DIR)

if !isfile(joinpath(BENCH_DIR, "Manifest.toml"))
    println("Instantiating PowerImpedance benchmark environment (first run may take several minutes)...")
    Pkg.instantiate()
    try
        Pkg.precompile()
    catch err
        @warn "Precompile warning (non-fatal)" err
    end
end

using PowerImpedance
println("PowerImpedance ", pkgversion(PowerImpedance), " active in ", BENCH_DIR)
