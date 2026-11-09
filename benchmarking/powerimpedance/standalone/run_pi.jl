# PowerImpedance.jl side of the Harmony vs PowerImpedance admittance benchmarks.
# Evaluates standalone Y(f) for cases that map onto Harmony examples.
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
    return path
end

function sweep_y(eval_y, freqs)
    Ys = Vector{Matrix{ComplexF64}}(undef, length(freqs))
    for (i, f) in enumerate(freqs)
        Ys[i] = Matrix{ComplexF64}(eval_y(1im * 2π * f))
    end
    return Ys
end

function print_spot(label, Ys, freqs, f_target)
    i = argmin(abs.(freqs .- f_target))
    Y = Ys[i]
    println("  $label @ $(freqs[i]) Hz  size=$(size(Y))")
    show(stdout, "text/plain", Y)
    println()
end

function run_resistor()
    println("\n=== resistor (10 Ω series) ===")
    freqs = logspace(10.0, 10000.0, 11)
    elem = impedance(z = 10.0, conductors = 1)
    Ys = sweep_y(s -> PowerImpedance.eval_y(elem, s), freqs)
    write_y_csv(joinpath(OUTDIR, "pi_resistor.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
end

function run_transformer_yy()
    println("\n=== transformer YY (example_transformer windings) ===")
    freqs = logspace(10.0, 10000.0, 41)
    elem = transformer(
        :explicit;
        conductors = 3,
        organization = :YY,
        n = 2.0,
        Rₚ = 4.3218,
        Lₚ = 0.0,
        Rₛ = 0.7938,
        Lₛ = 0.084225,
    )
    Ys = sweep_y(s -> PowerImpedance.eval_y(elem, s), freqs)
    write_y_csv(joinpath(OUTDIR, "pi_transformer_yy.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
end

function run_cable()
    println("\n=== cable (example_cable / P2P layers, aerial, 100 km) ===")
    freqs = logspace(1.0, 1000.0, 41)
    elem = cable(
        length = 100e3,
        positions = [(0.0, 1.0)],
        type = :aerial,
        earth_parameters = (1.0, 1.0, 1.0),
        C1 = Conductor(rᵢ = 0.0, rₒ = 24.25e-3, ρ = 1.72e-8),
        C2 = Conductor(rᵢ = 41.75e-3, rₒ = 46.25e-3, ρ = 22e-8),
        C3 = Conductor(rᵢ = 49.75e-3, rₒ = 60.55e-3, ρ = 18e-8, μᵣ = 10),
        I1 = Insulator(rᵢ = 24.25e-3, rₒ = 41.75e-3, ϵᵣ = 2.3),
        I2 = Insulator(rᵢ = 46.25e-3, rₒ = 49.75e-3, ϵᵣ = 2.3),
        I3 = Insulator(rᵢ = 60.55e-3, rₒ = 65.75e-3, ϵᵣ = 2.3),
    )
    Ys = sweep_y(s -> PowerImpedance.eval_y(elem, s), freqs)
    write_y_csv(joinpath(OUTDIR, "pi_cable.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
end

function run_ohl()
    println("\n=== OHL (example_OHL two-bundle flat, 227 km) ===")
    freqs = logspace(1.0, 1000.0, 41)
    elem = overhead_line(
        length = 227e3,
        conductors = Conductors(
            organization = :flat,
            nᵇ = 2,
            nˢᵇ = 2,
            Rᵈᶜ = 0.06266,
            rᶜ = 0.01436,
            yᵇᶜ = 27.5,
            Δyᵇᶜ = 0.0,
            Δxᵇᶜ = 11.8,
            Δ̃xᵇᶜ = 0.0,
            dˢᵇ = 0.4572,
            dˢᵃᵍ = 10.0,
        ),
        groundwires = Groundwires(
            nᵍ = 2,
            Rᵍᵈᶜ = 0.9196,
            rᵍ = 0.0062,
            Δxᵍ = 6.5,
            Δyᵍ = 7.5,
            dᵍˢᵃᵍ = 10.0,
        ),
        earth_parameters = (1.0, 1.0, 1.0),
    )
    Ys = sweep_y(s -> PowerImpedance.eval_y(elem, s), freqs)
    write_y_csv(joinpath(OUTDIR, "pi_ohl.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
end

function y_from_linearization(model, values, freqs)
    lin = linearize(model, values)
    system = lin.system
    scale = PowerImpedance.SI_scale(model)
    n = size(system.A, 1)
    println("  states ", n)
    Ys = Vector{Matrix{ComplexF64}}(undef, length(freqs))
    for (i, f) in enumerate(freqs)
        s = 2π * im * f
        Ys[i] = (system.C * ((s * I(n) - system.A) \ system.B) + system.D) .* scale
    end
    return Ys
end

# Pac, Qac, Pdc are the old Setpoint numbers. ACPortValues power is into the device,
# and pftoinputs stores p_ac = -P_into/S, q_ac = -Q_into/S.
function gfl_mmc(; vdc_outer, S, vll, vdc, pac, qac, pdc, vac_peak, lr, rr, turns)
    PI = PowerImpedance
    outer = vdc_outer ?
        PI.OuterActiveVdcControl(pi_ctrl = PI.PIControl(Kp = 5.0, Ki = 15.0)) :
        PI.OuterActivePowerControl(pi_ctrl = PI.PIControl(Kp = 0.1, Ki = 31.4159))
    model = mmc(;
        Sbase = S, vACbase_LL_RMS = vll, vDC_base = vdc,
        elec = PI.ElectricalMMC(Lᵣ = lr, Rᵣ = rr, turnsRatio = turns),
        meas = PI.Measurement(),
        sync = PI.PLLSynchronization(pi_ctrl = PI.PIControl(Kp = 0.28, Ki = 12.5664)),
        delta_control = PI.ΔdqControlGFL(
            outer_active = outer,
            outer_reactive = PI.OuterReactiveQControl(
                pi_ctrl = PI.PIControl(Kp = 0.1, Ki = 31.4159),
            ),
            occ = PI.InnerCurrentPIControl(
                pi_ctrl = PI.PIControl(Kp = 0.7691, Ki = 522.7654),
                activate_ω_c_multiplication = true,
            ),
        ),
        sigma_control = PI.ΣdqzControlTEC(
            ccsc = PI.CirculatingCurrentSuppressionControl(
                PI.PIControl(Kp = 0.1048, Ki = 48.1914),
            ),
        ),
        modulation = PI.UncompensatedModulation(timeDelay = 0.0),
        limits = PI.Limits(P_min = -2.0, P_max = 2.0, Q_min = -1.0, Q_max = 1.0),
    )
    active = vdc_outer ? (v_dc_ref = 1.0,) : (p_ref = pac / S,)
    model = component(model;
        delta_control = (outer_active = active, outer_reactive = (q_ref = -qac / S,)))
    values = (
        ac = ACPortValues(vac_peak, 0.0, -pac, qac),
        dc = DCPortValues(vdc, pdc),
    )
    return model, values
end

function run_mmc_gfl()
    println("\n=== MMC GFL (example_MMC plant + PowerImpedance default GFL pu gains) ===")
    freqs = logspace(1.0, 1000.0, 81)
    Vm_kV = 100.0
    model, values = gfl_mmc(;
        vdc_outer = false, S = 100.0, vll = Vm_kV / sqrt(2 / 3), vdc = 200.0,
        pac = 100.0, qac = 0.0, pdc = 100.0, vac_peak = Vm_kV,
        lr = 60e-3, rr = 0.535, turns = 1.0,
    )
    Ys = y_from_linearization(model, values, freqs)
    write_y_csv(joinpath(OUTDIR, "pi_mmc_gfl.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
    print_spot("Y", Ys, freqs, 1.0)
    print_spot("Y", Ys, freqs, 1000.0)
end

function run_mmc_gfl_vdc()
    println("\n=== MMC GFL Vdc+Q (example_MMC plant, c1 pu gains) ===")
    freqs = logspace(1.0, 1000.0, 81)
    Vm_kV = 100.0
    model, values = gfl_mmc(;
        vdc_outer = true, S = 100.0, vll = Vm_kV / sqrt(2 / 3), vdc = 200.0,
        pac = 100.0, qac = 0.0, pdc = 100.0, vac_peak = Vm_kV,
        lr = 60e-3, rr = 0.535, turns = 1.0,
    )
    Ys = y_from_linearization(model, values, freqs)
    write_y_csv(joinpath(OUTDIR, "pi_mmc_gfl_vdc.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
end

function run_mmc_c1()
    println("\n=== P2P c1 standalone (Vdc+Q) ===")
    freqs = logspace(1.0, 1000.0, 81)
    vac = (380 / sqrt(3)) * sqrt(2)
    model, values = gfl_mmc(;
        vdc_outer = true, S = 1000.0, vll = 380.0, vdc = 800.0,
        pac = -100.0, qac = 100.0, pdc = -100.0, vac_peak = vac,
        lr = 60e-3, rr = 0.535, turns = 1.0,
    )
    Ys = y_from_linearization(model, values, freqs)
    write_y_csv(joinpath(OUTDIR, "pi_mmc_c1.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
end

function run_mmc_c2()
    println("\n=== P2P c2 standalone (P+Q, Lr=0.0461, Rr=0.4103) ===")
    freqs = logspace(1.0, 1000.0, 81)
    vac = (380 / sqrt(3)) * sqrt(2)
    model, values = gfl_mmc(;
        vdc_outer = false, S = 1000.0, vll = 380.0, vdc = 800.0,
        pac = 100.0, qac = 100.0, pdc = 100.0, vac_peak = vac,
        lr = 0.0461, rr = 0.4103, turns = 1.0,
    )
    Ys = y_from_linearization(model, values, freqs)
    write_y_csv(joinpath(OUTDIR, "pi_mmc_c2.csv"), freqs, Ys)
    print_spot("Y", Ys, freqs, 50.0)
end

function main()
    println("PowerImpedance.jl ", pkgversion(PowerImpedance))
    println("Writing CSVs to ", OUTDIR)
    cases = [
        "resistor" => run_resistor,
        "transformer_yy" => run_transformer_yy,
        "cable" => run_cable,
        "ohl" => run_ohl,
        "mmc_gfl" => run_mmc_gfl,
        "mmc_gfl_vdc" => run_mmc_gfl_vdc,
        "mmc_c1" => run_mmc_c1,
        "mmc_c2" => run_mmc_c2,
    ]
    failed = String[]
    for (name, fn) in cases
        try
            t = @elapsed fn()
            @printf("TIMING  %s  %.4f\n", name, t)
            println("OK  ", name)
        catch err
            push!(failed, name)
            @error "Case failed" name exception = (err, catch_backtrace())
        end
    end
    if !isempty(failed)
        error("Failed cases: $(join(failed, ", "))")
    end
    println("\nAll PowerImpedance cases finished.")
end

if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
