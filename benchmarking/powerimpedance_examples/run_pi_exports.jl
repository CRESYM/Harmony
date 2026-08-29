#!/usr/bin/env julia
# Run all PowerImpedance reference exports (isolated env in this folder).
using Pkg
Pkg.activate(@__DIR__)

include(joinpath(@__DIR__, "install_powerimpedance.jl"))

for script in ("export_pi_cable.jl", "export_pi_branch_shunt.jl", "export_pi_p2p_full.jl", "export_pi_ieee39_soil.jl")
    path = joinpath(@__DIR__, script)
    println("\n=== ", script, " ===")
    include(path)
end

println("\nAll PI exports complete. Run Harmony examples + compare_*.py next.")
