# PowerImpedance P2P HVDC cases (reduced frequency grids).
using PowerImpedance
using LinearAlgebra
using Printf

const ROOT = @__DIR__
const OUTDIR = joinpath(ROOT, "results")
mkpath(OUTDIR)

logspace(start, stop, n) = exp10.(range(log10(start), log10(stop); length=n))

function write_y_csv(path, freqs, Ys)
    Y0 = Ys[1]
    n = size(Y0, 1)
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
    println("  wrote ", path, "  size=", size(Y0), "  n=", length(freqs))
    return path
end

function sweep_y(eval_y, freqs)
    Ys = Vector{Matrix{ComplexF64}}(undef, length(freqs))
    for (i, f) in enumerate(freqs)
        Ys[i] = Matrix{ComplexF64}(eval_y(1im * 2π * f))
    end
    return Ys
end

function p2p_dc_cable()
    return cable(
        length = 100e3,
        positions = [(-0.5, 1.0), (0.5, 1.0)],
        C1 = Conductor(rₒ = 24.25e-3, ρ = 1.72e-8),
        C2 = Conductor(rᵢ = 41.75e-3, rₒ = 46.25e-3, ρ = 22e-8),
        C3 = Conductor(rᵢ = 49.75e-3, rₒ = 60.55e-3, ρ = 18e-8, μᵣ = 10),
        I1 = Insulator(rᵢ = 24.25e-3, rₒ = 41.75e-3, ϵᵣ = 2.3),
        I2 = Insulator(rᵢ = 46.25e-3, rₒ = 49.75e-3, ϵᵣ = 2.3),
        I3 = Insulator(rᵢ = 60.55e-3, rₒ = 65.75e-3, ϵᵣ = 2.3),
        type = :underground,
    )
end

function run_p2p_components()
    println("\n=== P2P bipolar DC cable Y (phase domain) ===")
    freqs = logspace(10.0, 1000.0, 41)
    write_y_csv(joinpath(OUTDIR, "pi_p2p_cable.csv"), freqs, sweep_y(s -> PowerImpedance.eval_y(p2p_dc_cable(), s), freqs))
end

function p2p_conductors()
    return Conductors(
        organization = :flat, nᵇ = 3, nˢᵇ = 1, Rᵈᶜ = 0.063, rᶜ = 0.015,
        yᵇᶜ = 30, Δyᵇᶜ = 0, Δxᵇᶜ = 10, Δ̃xᵇᶜ = 0, dˢᵇ = 0, dˢᵃᵍ = 10,
    ), Groundwires(nᵍ = 2, Rᵍᵈᶜ = 0.92, rᵍ = 0.0062, Δxᵍ = 6.5, Δyᵍ = 7.5, dᵍˢᵃᵍ = 10)
end

function p2p_line(length_m)
    conductors, groundwires = p2p_conductors()
    return overhead_line(;
        length = length_m, conductors, groundwires, earth_parameters = (1, 1, 100))
end

function p2p_mmc(vdc_outer, lr, rr)
    outer = vdc_outer ?
        OuterActiveVdcControl(pi_ctrl = PIControl(Kp = 5, Ki = 15)) :
        OuterActivePowerControl(pi_ctrl = PIControl(Kp = 0.1, Ki = 31.4159))
    active = vdc_outer ? (v_dc_ref = 1.0,) : (p_ref = 0.1,)
    model = mmc(;
        vDC_base = 800, vACbase_LL_RMS = 380, Sbase = 1000,
        elec = ElectricalMMC(Lᵣ = lr, Rᵣ = rr, turnsRatio = 1.0),
        sync = PLLSynchronization(pi_ctrl = PIControl(Kp = 0.28, Ki = 12.5664)),
        delta_control = ΔdqControlGFL(
            outer_active = outer,
            outer_reactive = OuterReactiveQControl(pi_ctrl = PIControl(Kp = 0.1, Ki = 31.4159)),
            occ = InnerCurrentPIControl(
                pi_ctrl = PIControl(Kp = 0.7691, Ki = 522.7654),
                activate_ω_c_multiplication = true,
            ),
        ),
        sigma_control = ΣdqzControlTEC(
            ccsc = CirculatingCurrentSuppressionControl(PIControl(Kp = 0.1048, Ki = 48.1914)),
        ),
        limits = PowerImpedance.Limits(P_min = -1.5, P_max = 1.5, Q_min = -0.5, Q_max = 0.5),
    )
    b = bases(model)
    return component(model;
        delta_control = (outer_active = active, outer_reactive = (q_ref = -0.1,)),
        powerflow = (; v_dc = 800 / b.v_dc, v_ac = 1.0,
            p_ac = vdc_outer ? -0.1 : 0.1))
end

function build_p2p_network()
    voltage = 380 / sqrt(3)
    limits = PowerImpedance.Limits(P_min = -2000, P_max = 2000, Q_min = -1000, Q_max = 1000)
    elements = (;
        g4 = component(ac_source(v = voltage * sqrt(2), p = 100.0); limits),
        g1 = component(ac_source(v = voltage * sqrt(2), p = 100.0); limits),
        c1 = p2p_mmc(true, 0.06, 0.535),
        c2 = p2p_mmc(false, 0.0461, 0.4103),
        dc_line = p2p_dc_cable(),
        tl1 = p2p_line(25e3),
        tl78 = p2p_line(90e3),
    )
    connections = (
        NetworkBuilder.Bus(:B2; ports = ((:g4, :ac), (:tl1, :side1)), domain = ACDQ),
        NetworkBuilder.Bus(:B3; ports = ((:c1, :ac), (:tl1, :side2)), domain = ACDQ),
        NetworkBuilder.Bus(:B4; ports = ((:c1, :dc), (:dc_line, :side1))),
        NetworkBuilder.Bus(:B5; ports = ((:c2, :dc), (:dc_line, :side2))),
        NetworkBuilder.Bus(:B6; ports = ((:c2, :ac), (:tl78, :side1)), domain = ACDQ),
        NetworkBuilder.Bus(:B7; ports = ((:g1, :ac), (:tl78, :side2)), domain = ACDQ),
    )
    return NetworkBuilder.define(elements, connections;
        options = (; voltageBase = voltage,
            power_flow = (; is_bounded = (; bus_voltage = true),
                optimizer = (; hessian_approximation = "limited-memory"))))
end

function slice_z(Z)
    return [Z[:, :, k] for k in axes(Z, 3)]
end

# Harmony uses the same frame. Both AC current rows are load versus generator.
function ac_yn_from_ss(Y3::AbstractMatrix, Ydc_ext::Number)
    Y = Matrix{ComplexF64}(Y3)
    Y[2:3, :] .*= -1
    Ydc = Y[1, 1]
    B = Y[1:1, 2:3]
    A = Y[2:3, 1:1]
    Ydq = Y[2:3, 2:3]
    return Ydq - A * inv(Ydc + Ydc_ext) * B
end

function run_p2p_network()
    println("\n=== P2P HVDC Zin, Zeq, and TF H = Yn Zeq at B6 ===")
    t = @elapsed network = build_p2p_network()
    @printf("TIMING  p2p_build  %.4f\n", t)
    t = @elapsed pf = compute(PowerFlowProblem(network), ACDCPowerFlow())
    @printf("TIMING  p2p_pf  %.4f\n", t)
    println(pf)
    t = @elapsed lin = compute(LinearizationProblem(network, pf), AdmittanceLinearization())
    @printf("TIMING  p2p_lin  %.4f\n", t)
    model = lin.network_model
    freq = (10.0, 1000.0, 41)
    t = @elapsed Zin, omegas = determine_impedance(model; nets = [:B6d, :B6q], freq_range = freq)
    @printf("TIMING  p2p_zin  %.4f\n", t)
    t = @elapsed Zeq, _ = determine_impedance(model;
        nets = [:B6d, :B6q], elim_elements = [:c2], freq_range = freq)
    @printf("TIMING  p2p_zeq  %.4f\n", t)
    t = @elapsed Zdc, _ = determine_impedance(model;
        nets = [:B5], elim_elements = [:c2], freq_range = freq)
    @printf("TIMING  p2p_zdc  %.4f\n", t)
    freqs = real.(omegas) ./ (2π)
    s = im .* omegas
    Y3 = PowerImpedance.NetworkBuilder.get_y(model, :c2, s)
    Zin_al = slice_z(Zin)
    Zeq_al = slice_z(Zeq)
    Hs = Vector{Matrix{ComplexF64}}(undef, length(omegas))
    Yns = Vector{Matrix{ComplexF64}}(undef, length(omegas))
    for i in eachindex(omegas)
        zdc = Zdc[1, 1, i]
        Yn = ac_yn_from_ss(Y3[:, :, i], inv(zdc))
        Yns[i] = Yn
        Hs[i] = Yn * Zeq_al[i]
    end
    write_y_csv(joinpath(OUTDIR, "pi_p2p_Z.csv"), freqs, Zin_al)
    write_y_csv(joinpath(OUTDIR, "pi_p2p_Zeq.csv"), freqs, Zeq_al)
    write_y_csv(joinpath(OUTDIR, "pi_p2p_Yn.csv"), freqs, Yns)
    write_y_csv(joinpath(OUTDIR, "pi_p2p_H.csv"), freqs, Hs)
    i50 = argmin(abs.(freqs .- 50))
    println("  Zin @ $(freqs[i50]) Hz:\n", Zin_al[i50])
    println("  Zeq @ $(freqs[i50]) Hz:\n", Zeq_al[i50])
    println("  H = Yn Zeq @ $(freqs[i50]) Hz:\n", Hs[i50])
end

function main()
    println("PowerImpedance.jl ", pkgversion(PowerImpedance))
    t = @elapsed run_p2p_components()
    @printf("TIMING  p2p_cable  %.4f\n", t)
    try
        run_p2p_network()
    catch err
        @error "P2P network impedance failed" exception = (err, catch_backtrace())
    end
    println("\nPowerImpedance network cases finished.")
end

if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
