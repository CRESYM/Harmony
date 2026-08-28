# Matched standalone MMC: Harmony example_MMC plant + PowerImpedance default GFL controls.
# Compares SI 3×3 Y(f) against Harmony's example_MMC export.
using PowerImpedanceACDC
using LinearAlgebra
using DelimitedFiles

const OUTDIR = joinpath(@__DIR__, "results")
mkpath(OUTDIR)

const Sbase_MW = 100.0
const Vdc_kV = 200.0
const Vm_kV = 100.0
const Vac_LL_RMS_kV = Vm_kV / sqrt(2 / 3)

# Use PowerImpedance's documented default GFL controller set (pu), with the
# same arm/reactor plant as Harmony example_MMC.cpp.
elem = mmc(
    ω₀ = 100π,
    Vᵈᶜ = Vdc_kV,
    vDCbase = Vdc_kV,
    Vₘ = Vm_kV,
    vACbase_LL_RMS = Vac_LL_RMS_kV,
    Sbase = Sbase_MW,
    turnsRatio = 1.0,
    P = Sbase_MW,
    Q = 0.0,
    P_dc = Sbase_MW,
    P_max = 200.0,
    P_min = -200.0,
    Q_max = 100.0,
    Q_min = -100.0,
    Lₐᵣₘ = 50e-3,
    Rₐᵣₘ = 1.07,
    Cₐᵣₘ = 10e-3,
    N = 400,
    Lᵣ = 60e-3,
    Rᵣ = 0.535,
    timeDelay = 0.0,
    occ = PI_control(Kₚ = 0.7691, Kᵢ = 522.7654),
    ccc = PI_control(Kₚ = 0.1048, Kᵢ = 48.1914),
    pll = PI_control(Kₚ = 0.28, Kᵢ = 12.5664),
    p = PI_control(Kₚ = 0.1, Kᵢ = 31.4159, ref = [Sbase_MW]),
    q = PI_control(Kₚ = 0.1, Kᵢ = 31.4159, ref = [0.0]),
)

conv = elem.element_value
println("Controls: $(collect(keys(conv.controls)))")

PowerImpedanceACDC.update!(conv, Vm_kV, 0.0, Sbase_MW, 0.0, Vdc_kV, Sbase_MW)
println("Equilibrium length=$(length(conv.equilibrium)) A=$(size(conv.A)) successful=$(size(conv.A,1) > 1)")

freqs = exp10.(range(log10(1.0), log10(1000.0); length=200))
rows = Matrix{Float64}(undef, length(freqs), 1 + 18)
for (i, f) in enumerate(freqs)
    Y = PowerImpedanceACDC.eval_parameters(conv, 1im * 2π * f)
    rows[i, 1] = f
    k = 2
    for r in 1:3, c in 1:3
        rows[i, k] = real(Y[r, c]); k += 1
        rows[i, k] = imag(Y[r, c]); k += 1
    end
end

header = ["freq_Hz",
    "Re_Y11","Im_Y11","Re_Y12","Im_Y12","Re_Y13","Im_Y13",
    "Re_Y21","Im_Y21","Re_Y22","Im_Y22","Re_Y23","Im_Y23",
    "Re_Y31","Im_Y31","Re_Y32","Im_Y32","Re_Y33","Im_Y33"]
open(joinpath(OUTDIR, "powerimpedance_Y.csv"), "w") do io
    println(io, join(header, ","))
    writedlm(io, rows, ',')
end

Y50 = PowerImpedanceACDC.eval_parameters(conv, 1im * 2π * 50)
println("Y @ 50 Hz:")
show(stdout, "text/plain", Y50); println()
open(joinpath(OUTDIR, "powerimpedance_Y50.txt"), "w") do io
    println(io, Y50)
end
