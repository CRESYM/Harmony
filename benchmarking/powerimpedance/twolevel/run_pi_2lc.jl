# Device admittances PowerImpedance can build with the loops Harmony's 2LC has.
# Measurement filters and voltage/frequency support lags are left out on this side.

using PowerImpedance
using LinearAlgebra
using DelimitedFiles

const OUT = joinpath(@__DIR__, "results")
mkpath(OUT)

function logf(a, b, n)
    return [a * (b / a)^((k - 1) / (n - 1)) for k in 1:n]
end

function build(case)
    Zb = case.vll^2 / case.S
    Lf = case.L_H
    Rf = case.R_ohm
    elec = ElectricalTLC(Lᵣ = Lf, Rᵣ = Rf)
    sync = PLLSynchronization(pi_ctrl = PIControl(Kp = case.pll[1], Ki = case.pll[2]))
    inner = InnerCurrentPIControl(pi_ctrl = PIControl(Kp = case.current[1], Ki = case.current[2]))
    mod = case.delay == 0.0 ? NoModulation() :
        PadeModulation(timeDelay = case.delay, padeOrderNum = 3, padeOrderDen = 3)
    outerQ = OuterReactiveQControl(pi_ctrl = PIControl(Kp = case.qgain[1], Ki = case.qgain[2]))
    if case.vdc
        outerA = OuterActiveVdcControl(pi_ctrl = PIControl(Kp = case.pgain[1], Ki = case.pgain[2]))
        active_inputs = (v_dc_ref = 1.0,)
    else
        outerA = OuterActivePowerControl(pi_ctrl = PIControl(Kp = case.pgain[1], Ki = case.pgain[2]))
        active_inputs = (p_ref = case.p_into / case.S,)
    end
    dut = tlc(; Sbase = case.S, vACbase_LL_RMS = case.vll, vDCbase = case.vdc_kv,
        elec, sync, outerActive = outerA, outerReactive = outerQ,
        innerCurrent = inner, mod)
    dut = component(dut;
        outerActive = active_inputs,
        outerReactive = (q_ref = case.q_into / case.S,))
    vac = case.vll * sqrt(2 / 3)
    values = (
        ac = ACPortValues(vac, 0.0, case.p_into, case.q_into),
        dc = DCPortValues(case.vdc_kv, case.p_dc),
    )
    return dut, values
end

function save_case(case)
    dut, values = build(case)
    lin = linearize(dut, values)
    system = lin.system
    scale = PowerImpedance.SI_scale(dut)
    freqs = logf(1.0, 1e3, 31)
    rows = Matrix{Float64}(undef, length(freqs), 1 + 18)
    for (k, f) in enumerate(freqs)
        s = 2π * im * f
        n = size(system.A, 1)
        Y = (system.C * ((s * I(n) - system.A) \ system.B) + system.D) .* scale
        rows[k, 1] = f
        flat = vec(Y)
        rows[k, 2:2:end] = real.(flat)
        rows[k, 3:2:end] = imag.(flat)
    end
    header = ["freq_Hz"]
    for c in 1:3, r in 1:3
        push!(header, "Re_Y$(r)$(c)", "Im_Y$(r)$(c)")
    end
    open(joinpath(OUT, "pi_$(case.name).csv"), "w") do io
        println(io, join(header, ","))
        writedlm(io, rows, ',')
    end
    println(case.name, " states ", size(system.A, 1))
end

const PLL = (0.397887357729738, 7.957747154594767)
const CURRENT = (0.254647908947033, 0.8)
const Z220 = 220.0^2 / 100.0

cases = [
    (name = "p_q", S = 100.0, vll = 220.0, vdc_kv = 640.0,
        L_H = 0.08 * Z220 / (2π * 50), R_ohm = 0.01 * 0.08 * Z220,
        p_into = 70.0, q_into = 20.0, p_dc = -70.0, vdc = false,
        pll = PLL, pgain = (0.04, 40.0), qgain = (0.04, 40.0), current = CURRENT,
        delay = 200e-6),
    (name = "vdc_q", S = 100.0, vll = 220.0, vdc_kv = 640.0,
        L_H = 0.08 * Z220 / (2π * 50), R_ohm = 0.01 * 0.08 * Z220,
        p_into = 70.0, q_into = 20.0, p_dc = -70.0, vdc = true,
        pll = PLL, pgain = (5.0, 5.0), qgain = (0.04, 40.0), current = CURRENT,
        delay = 200e-6),
    (name = "statcom", S = 269.0, vll = 345.0, vdc_kv = 110.0,
        L_H = 0.08 * (345.0^2 / 269.0) / (2π * 50),
        R_ohm = 0.01 * 0.08 * (345.0^2 / 269.0),
        p_into = 0.0, q_into = -0.5 * 269.0, p_dc = 0.0, vdc = true,
        pll = PLL, pgain = (5.0, 5.0), qgain = (0.04, 40.0), current = CURRENT,
        delay = 200e-6),
    (name = "owf", S = 1000.0, vll = 220.0, vdc_kv = 640.0,
        L_H = 0.01232, R_ohm = 0.03872,
        p_into = -700.0, q_into = -150.0, p_dc = 700.0, vdc = false,
        pll = (1.0, 31.830988618379067), pgain = (0.01, 10.0), qgain = (0.01, 10.0),
        current = CURRENT, delay = 0.0),
    (name = "wind", S = 600.0, vll = 380.0, vdc_kv = 640.0,
        L_H = 0.08 * (380.0^2 / 600.0) / (2π * 50),
        R_ohm = 0.0008 * (380.0^2 / 600.0),
        p_into = -600.0, q_into = 0.0, p_dc = 600.0, vdc = false,
        pll = PLL, pgain = (0.01, 10.0), qgain = (0.01, 10.0), current = CURRENT,
        delay = 0.0),
]

for case in cases
    println("2LC ", case.name)
    save_case(case)
end
println("PI 2LC done")
