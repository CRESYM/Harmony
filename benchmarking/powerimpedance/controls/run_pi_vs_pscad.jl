# PowerImpedance.jl versus the PSCAD/Z-tool files shipped with its tests.
# Harmony has no two-level converter, synchronous machine, induction machine,
# VSE grid-forming MMC, compensated modulation, or leg-energy control, so these
# cases are scored here against the validation data rather than against Harmony.
using PowerImpedance
import PowerImpedance: @network
using Test
using LinearAlgebra
using Printf
import PowerImpedance.NetworkBuilder
const NB = PowerImpedance.NetworkBuilder

const TESTDIR = normpath(joinpath(dirname(pathof(PowerImpedance)), "..", "test"))
const OUTDIR = joinpath(@__DIR__, "results")
mkpath(OUTDIR)

function frob_stats(pairs)
    vals = Float64[]
    for (A, B) in pairs
        den = norm(B)
        push!(vals, norm(A - B) / max(den, 1e-18))
    end
    return (mean = sum(vals) / length(vals), max = maximum(vals), min = minimum(vals), n = length(vals))
end

function report(name, pairs)
    s = frob_stats(pairs)
    @printf("FROB  %-18s  n=%4d  mean=%.4e  max=%.4e  min=%.4e\n", name, s.n, s.mean, s.max, s.min)
    return s
end

function write_y_csv(path, freqs, Ys)
    n = size(Ys[1], 1)
    open(path, "w") do io
        print(io, "freq_Hz")
        for i in 1:n, j in 1:n
            print(io, ",Re_Y$(i)$(j),Im_Y$(i)$(j)")
        end
        println(io)
        for (f, Y) in zip(freqs, Ys)
            print(io, f)
            for i in 1:n, j in 1:n
                print(io, ",", real(Y[i, j]), ",", imag(Y[i, j]))
            end
            println(io)
        end
    end
end

println("PowerImpedance.jl ", pkgversion(PowerImpedance))

println("\n======== GFM uncompensated, total energy (adm_MMC_test) ========")
include(joinpath(TESTDIR, "adm_MMC_test.jl"))
const gfm_tec_Y = copy(Y_MMC)
const gfm_tec_ref = copy(Y_validation)
const gfm_tec_w = copy(omegas)
report("gfm_tec", zip(gfm_tec_Y, gfm_tec_ref))
write_y_csv(joinpath(OUTDIR, "pi_gfm_tec.csv"), gfm_tec_w ./ (2π), gfm_tec_Y)
write_y_csv(joinpath(OUTDIR, "pscad_gfm_tec.csv"), gfm_tec_w ./ (2π), gfm_tec_ref)

println("\n======== GFM compensated modulation, leg energy ========")
include(joinpath(TESTDIR, "adm_MMC_compensated_test.jl"))
grid_cm = build_mmc_grid()
elem_cm = grid_cm.elements[:DUT]
w_cm, Yref_cm = read_validation_data(joinpath(TESTDIR, "data", "data_MMC_CM_validation.txt"))
Y_cm = Matrix{ComplexF64}[]
for ω in w_cm
    push!(Y_cm, mmc_plot_convention(element_y(elem_cm, 1im * ω)))
end
report("gfm_lec_cm", zip(Y_cm, Yref_cm))
write_y_csv(joinpath(OUTDIR, "pi_gfm_lec_cm.csv"), w_cm ./ (2π), Y_cm)
write_y_csv(joinpath(OUTDIR, "pscad_gfm_lec_cm.csv"), w_cm ./ (2π), Yref_cm)

println("\n======== Two-level converter ========")
include(joinpath(TESTDIR, "adm_TLC_test.jl"))

function tlc_pair(builder, file, flip_dc::Bool)
    grid = builder()
    elem = grid.elements[:DUT]
    w, Yref = read_validation_data(joinpath(TESTDIR, "data", file))
    Y = Matrix{ComplexF64}[]
    for ω in w
        push!(Y, tlc_plot_convention(element_y(elem, 1im * ω)))
    end
    if flip_dc
        for M in Yref
            M[1, :] .*= -1
        end
    end
    return w, Y, Yref
end

w1, Y1, R1 = tlc_pair(build_case1_grid, "data_TLC_validation_1.txt", false)
report("tlc_vdc_qv", zip(Y1, R1))
write_y_csv(joinpath(OUTDIR, "pi_tlc_vdc_qv.csv"), w1 ./ (2π), Y1)
w2, Y2, R2 = tlc_pair(build_case2_grid, "data_TLC_validation_2.txt", true)
report("tlc_pq", zip(Y2, R2))
write_y_csv(joinpath(OUTDIR, "pi_tlc_pq.csv"), w2 ./ (2π), Y2)

println("\n======== Synchronous machine ========")
include(joinpath(TESTDIR, "adm_SM_test.jl"))
ωs, Ys_pscad = SM_pscad_data()
net_sm = synchronousmachinecircuit()
Ybase = 1000 / 380^2
Ys_pi = [Ybase * PowerImpedance.eval_y(net_sm.elements[:sg1], s) for s in (ωs .* im)]
report("synchronous_machine", zip(Ys_pi, Ys_pscad))
write_y_csv(joinpath(OUTDIR, "pi_sm.csv"), ωs ./ (2π), Ys_pi)

println("\n======== Induction machine ========")
include(joinpath(TESTDIR, "adm_IM_test.jl"))
ωi, Yi_pscad = read_im_validation_data()
lnn = inductionmachinecircuit()
Yi_pi = NB.get_y(lnn, :im1, ωi .* im)
# get_y may return nω × 2 × 2 or a vector of matrices.
if ndims(Yi_pi) == 3
    pairs_im = [(Yi_pi[:, :, k], Yi_pscad[:, :, k]) for k in 1:size(Yi_pi, 3)]
    Yflat = [Yi_pi[:, :, k] for k in 1:size(Yi_pi, 3)]
else
    pairs_im = zip(Yi_pi, eachslice(Yi_pscad; dims=3))
    Yflat = Yi_pi
end
report("induction_machine", pairs_im)
write_y_csv(joinpath(OUTDIR, "pi_im.csv"), ωi ./ (2π), Yflat)

println("\n======== Bipolar MMC scan on disk (no 4-port model in PowerImpedance 0.4.0) ========")
bipolar_path = joinpath(TESTDIR, "data", "data_MMC_bipolar.txt")
blines = readlines(bipolar_path)
header = split(blines[1])
println("  file entries: ", length(header) - 1, "  frequencies: ", length(blines) - 1)
println("  ports: ", join(header[2:end], ", "))

println("\nPSCAD COMPARISONS DONE")
