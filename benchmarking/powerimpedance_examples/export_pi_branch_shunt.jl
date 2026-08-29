# Export branch||shunt driving-point Z for PowerImpedance Gridspace_uncertainty cases.
using Pkg
Pkg.activate(@__DIR__)

using DelimitedFiles
using LinearAlgebra

const OUTDIR = joinpath(@__DIR__, "results")
mkpath(OUTDIR)

function write_case(tag, z_branch, z_shunt; f0=1.0, f1=1e3, n=40)
    y = 1/z_branch + 1/z_shunt
    z = 1/y
    freqs = exp10.(range(log10(f0), log10(f1); length=n))
    rows = Matrix{Float64}(undef, length(freqs), 5)
    for (i, f) in enumerate(freqs)
        rows[i, :] .= (f, real(y), imag(y), real(z), imag(z))
    end
    path = joinpath(OUTDIR, "pi_branch_shunt_$(tag).csv")
    open(path, "w") do io
        println(io, "freq_Hz,Re_Y,Im_Y,Re_Z,Im_Z")
        writedlm(io, rows, ',')
    end
    println("Wrote $path  Z=$(z)")
end

write_case("z8", 8.0, 20.0)
write_case("z10", 10.0, 20.0)
write_case("smallsignal", 3.5, 4.0; n=80)

# If full PowerImpedance is available, also exercise NetworkBuilder (optional).
try
    using PowerImpedance
    println("PowerImpedance loaded — NetworkBuilder path available.")
catch
    println("PowerImpedance not installed; analytical CSVs only (exact for R networks).")
end
