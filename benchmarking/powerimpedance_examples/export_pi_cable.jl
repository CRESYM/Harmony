#!/usr/bin/env julia
# Export P2P README cable Y(f) using PowerImpedance 0.3.x (isolated env).
using Pkg
Pkg.activate(@__DIR__)

using DelimitedFiles
using LinearAlgebra
using PowerImpedance

const OUTDIR = joinpath(@__DIR__, "results")
mkpath(OUTDIR)

function write_y_csv(path, freqs, Yrows)
    open(path, "w") do io
        println(io, "freq_Hz,Re_Y11,Im_Y11,Re_Y12,Im_Y12,Re_Y21,Im_Y21,Re_Y22,Im_Y22")
        for (i, f) in enumerate(freqs)
            y = Yrows[i]
            println(io, f, ",",
                real(y[1, 1]), ",", imag(y[1, 1]), ",",
                real(y[1, 2]), ",", imag(y[1, 2]), ",",
                real(y[2, 1]), ",", imag(y[2, 1]), ",",
                real(y[2, 2]), ",", imag(y[2, 2]))
        end
    end
end

freqs = exp10.(range(log10(10.0), log10(1000.0); length=100))
Yrows = Vector{Matrix{ComplexF64}}(undef, length(freqs))

dc_elem = cable(
    length = 100e3,
    positions = [(-0.5, 1), (0.5, 1)],
    C1 = Conductor(rₒ = 24.25e-3, ρ = 1.72e-8),
    C2 = Conductor(rᵢ = 41.75e-3, rₒ = 46.25e-3, ρ = 22e-8),
    C3 = Conductor(rᵢ = 49.75e-3, rₒ = 60.55e-3, ρ = 18e-8, μᵣ = 10),
    I1 = Insulator(rᵢ = 24.25e-3, rₒ = 41.75e-3, ϵᵣ = 2.3),
    I2 = Insulator(rᵢ = 46.25e-3, rₒ = 49.75e-3, ϵᵣ = 2.3),
    I3 = Insulator(rᵢ = 60.55e-3, rₒ = 65.75e-3, ϵᵣ = 2.3),
    transformation = true,
)

const c_model = dc_elem.element_model

for (i, f) in enumerate(freqs)
    _, Y = PowerImpedance.eval_parameters(c_model, 1im * 2π * f)
    Yrows[i] = Y
end

out = joinpath(OUTDIR, "pi_dc_line.csv")
write_y_csv(out, freqs, Yrows)
println("Wrote ", out)

open(joinpath(OUTDIR, "pi_cable_geometry.txt"), "w") do io
    println(io, "P2P README cable exported via PowerImpedance ", pkgversion(PowerImpedance))
    println(io, "length=100e3, positions=(-0.5,1),(0.5,1)")
    println(io, "Harmony counterpart: json/p2p_cable_readme.json")
end
