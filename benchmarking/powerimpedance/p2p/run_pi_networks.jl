# PowerImpedance P2P HVDC cases (reduced frequency grids).
using PowerImpedance
import PowerImpedance: @network
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
        transformation = false,
    )
end

function run_p2p_components()
    println("\n=== P2P bipolar DC cable Y (phase domain) ===")
    freqs = logspace(10.0, 1000.0, 41)
    write_y_csv(joinpath(OUTDIR, "pi_p2p_cable.csv"), freqs, sweep_y(s -> PowerImpedance.get_y(p2p_dc_cable(), s), freqs))
end

function p2p_mmc_c1()
    transmissionVoltage = 380 / sqrt(3)
    pHVDC1 = 100.0
    qC1 = 100.0
    return PowerImpedance.mmc(
        elec = PowerImpedance.ElectricalMMC(vDC_base = 800),
        sync = PowerImpedance.PLLSynchronization(
            pi_ctrl = PowerImpedance.PIControl(Kp = 0.28, Ki = 12.5664),
        ),
        delta_control = PowerImpedance.ΔdqControlGFL(
            outer_active = PowerImpedance.OuterActiveVdcControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 5, Ki = 15),
            ),
            outer_reactive = PowerImpedance.OuterReactiveQControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 0.1, Ki = 31.4159),
            ),
            occ = PowerImpedance.InnerCurrentPIControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 0.7691, Ki = 522.7654),
                # Harmony OCC decoupling uses ω_C = Δω + ω_0 (PLL). Keep PI's
                # default ω_c multiplier on; false froze the vq column of Yn.
                activate_ω_c_multiplication = true,
            ),
        ),
        sigma_control = PowerImpedance.ΣdqzControlTEC(
            ccsc = PowerImpedance.CirculatingCurrentSuppressionControl(
                PowerImpedance.PIControl(Kp = 0.1048, Ki = 48.1914),
            ),
        ),
        setpoint = PowerImpedance.Setpoint(
            Pac = -pHVDC1,
            # Network PF writes Qac back with a sign flip. Seed −100 so the
            # solved setpoint is +100, pftoinputs q_ac=−0.1, report Iq<0.
            Qac = -qC1,
            θac = 0.0,
            Vac = transmissionVoltage * sqrt(2),
            Pdc = -pHVDC1,
            Vdc = 800,
        ),
        limits = PowerImpedance.Limits(P_min = -1500, P_max = 1500, Q_min = -500, Q_max = 500),
    )
end

function p2p_mmc_c2()
    transmissionVoltage = 380 / sqrt(3)
    pHVDC1 = 100.0
    qC2 = 100.0
    return PowerImpedance.mmc(
        elec = PowerImpedance.ElectricalMMC(
            vDC_base = 800,
            vACbase_LL_RMS = 380,
            turnsRatio = 1.0,
            Lᵣ = 0.0461,
            Rᵣ = 0.4103,
        ),
        sync = PowerImpedance.PLLSynchronization(
            pi_ctrl = PowerImpedance.PIControl(Kp = 0.28, Ki = 12.5664),
        ),
        delta_control = PowerImpedance.ΔdqControlGFL(
            outer_active = PowerImpedance.OuterActivePowerControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 0.1, Ki = 31.4159),
            ),
            outer_reactive = PowerImpedance.OuterReactiveQControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 0.1, Ki = 31.4159),
            ),
            occ = PowerImpedance.InnerCurrentPIControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 0.7691, Ki = 522.7654),
                # Harmony OCC decoupling uses ω_C = Δω + ω_0 (PLL). Keep PI's
                # default ω_c multiplier on; false froze the vq column of Yn.
                activate_ω_c_multiplication = true,
            ),
        ),
        sigma_control = PowerImpedance.ΣdqzControlTEC(
            ccsc = PowerImpedance.CirculatingCurrentSuppressionControl(
                PowerImpedance.PIControl(Kp = 0.1048, Ki = 48.1914),
            ),
        ),
        setpoint = PowerImpedance.Setpoint(
            Pac = pHVDC1,
            Qac = -qC2,
            θac = 0.0,
            Vac = transmissionVoltage * sqrt(2),
            Pdc = pHVDC1,
            Vdc = 800,
        ),
        limits = PowerImpedance.Limits(P_min = -1000, P_max = 1000, Q_min = -1000, Q_max = 1000),
    )
end

function build_p2p_network()
    transmissionVoltage = 380 / sqrt(3)
    pHVDC1 = 100.0
    return @network begin
        voltageBase = transmissionVoltage

        g1 = ac_source(
            setpoint = Setpoint(Vac = transmissionVoltage, Pac = pHVDC1),
            limits = PowerImpedance.Limits(P_min = -2000, P_max = 2000, Q_max = 1000, Q_min = -1000),
            pins = 3,
            transformation = true,
        )
        g4 = ac_source(
            setpoint = Setpoint(Vac = transmissionVoltage, Pac = pHVDC1),
            limits = PowerImpedance.Limits(P_min = -2000, P_max = 2000, Q_max = 1000, Q_min = -1000),
            pins = 3,
            transformation = true,
        )

        c1 = p2p_mmc_c1()
        c2 = p2p_mmc_c2()

        dc_line = cable(
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

        tl1 = overhead_line(
            length = 25e3,
            conductors = Conductors(
                organization = :flat, nᵇ = 3, nˢᵇ = 1, Rᵈᶜ = 0.063, rᶜ = 0.015,
                yᵇᶜ = 30, Δyᵇᶜ = 0, Δxᵇᶜ = 10, Δ̃xᵇᶜ = 0, dˢᵇ = 0, dˢᵃᵍ = 10,
            ),
            groundwires = Groundwires(nᵍ = 2, Rᵍᵈᶜ = 0.92, rᵍ = 0.0062, Δxᵍ = 6.5, Δyᵍ = 7.5, dᵍˢᵃᵍ = 10),
            earth_parameters = (1, 1, 100),
            transformation = true,
        )
        tl78 = overhead_line(
            length = 90e3,
            conductors = Conductors(
                organization = :flat, nᵇ = 3, nˢᵇ = 1, Rᵈᶜ = 0.063, rᶜ = 0.015,
                yᵇᶜ = 30, Δyᵇᶜ = 0, Δxᵇᶜ = 10, Δ̃xᵇᶜ = 0, dˢᵇ = 0, dˢᵃᵍ = 10,
            ),
            groundwires = Groundwires(nᵍ = 2, Rᵍᵈᶜ = 0.92, rᵍ = 0.0062, Δxᵍ = 6.5, Δyᵍ = 7.5, dᵍˢᵃᵍ = 10),
            earth_parameters = (1, 1, 100),
            transformation = true,
        )

        c1[2.1] ⟷ tl1[2.1] ⟷ B3d
        c1[2.2] ⟷ tl1[2.2] ⟷ B3q
        g4[1.1] ⟷ tl1[1.1] ⟷ B2d
        g4[1.2] ⟷ tl1[1.2] ⟷ B2q
        g4[2.1] ⟷ gndd
        g4[2.2] ⟷ gndq
        c1[1.1] ⟷ dc_line[1.1] ⟷ B4
        c2[1.1] ⟷ dc_line[2.1] ⟷ B5
        c2[2.1] == tl78[1.1] == B6d
        c2[2.2] == tl78[1.2] == B6q
        g1[1.1] == tl78[2.1] == B7d
        g1[1.2] == tl78[2.2] == B7q
        g1[2.1] == gndd
        g1[2.2] == gndq
    end
end

function dump_pi_linearization_op(net)
    path = joinpath(OUTDIR, "pi_linearization_op.csv")
    open(path, "w") do io
        println(io, "id,Pac_MW,Qac_MVAR,Pdc_MW,Vac_pu,theta_deg,Vdc_kV,Vgd_kV,Vgq_kV,Id_A,Iq_A,iSz_A,p_ac_pu,q_ac_pu,p_dc_pu")
        for name in (:c1, :c2)
            el = net.elements[name]
            sp = el.setpoint
            m = el.element_model
            elec = m.elec
            inputs, spu = PowerImpedance.pftoinputs(m, sp)
            vac_pu = sp.Vac / elec.vAC_base
            vgd_kV = inputs.vG_d * elec.vAC_base
            vgq_kV = inputs.vG_q * elec.vAC_base
            i_ac_base = 2 * elec.Sbase / (3 * elec.vAC_base) * 1e3
            i_dc_base = elec.Sbase / elec.vDC_base * 1e3
            mag2 = inputs.vG_d^2 + inputs.vG_q^2
            iΔ_d_pu = (inputs.vG_d * spu.p_ac - inputs.vG_q * spu.q_ac) / mag2
            iΔ_q_pu = (inputs.vG_q * spu.p_ac + inputs.vG_d * spu.q_ac) / mag2
            iΣ_z_pu = spu.p_dc / 3 / inputs.v_dc
            @printf(
                io,
                "%s,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f\n",
                string(name),
                sp.Pac,
                sp.Qac,
                sp.Pdc,
                vac_pu,
                rad2deg(sp.θac),
                sp.Vdc,
                vgd_kV,
                vgq_kV,
                iΔ_d_pu * i_ac_base,
                iΔ_q_pu * i_ac_base,
                iΣ_z_pu * i_dc_base,
                spu.p_ac,
                spu.q_ac,
                spu.p_dc,
            )
        end
    end
    println("  wrote ", path)
end

# Park q-polarity: Harmony uses Z_dq ≈ −ωL, Z_qd ≈ +ωL. PowerImpedance's
# assembled network 2×2 has the opposite cross terms. S = diag(1,−1)
# reflects q voltage and q current (S⁻¹ = S).
const S_DQ = Diagonal(ComplexF64[1, -1])
qalign(Z::AbstractMatrix) = S_DQ * Z * S_DQ

function ac_yn_from_ss(Y3::AbstractMatrix, Ydc_ext::Number)
    # Generator-sign AC currents (PowerImpedance MMC Y is load-sign).
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
    t = @elapsed net = build_p2p_network()
    @printf("TIMING  p2p_build  %.4f\n", t)
    dump_pi_linearization_op(net)
    t = @elapsed Zin, omegas = determine_impedance(
        net;
        elim_elements = Symbol[],
        input_pins = [:B6d, :B6q],
        output_pins = [:gndd, :gndq],
        freq_range = (10.0, 1000.0, 41),
    )
    @printf("TIMING  p2p_zin  %.4f\n", t)
    t = @elapsed Zeq, _ = determine_impedance(
        net;
        elim_elements = [:c2],
        input_pins = [:B6d, :B6q],
        output_pins = [:gndd, :gndq],
        freq_range = (10.0, 1000.0, 41),
    )
    @printf("TIMING  p2p_zeq  %.4f\n", t)
    t = @elapsed Zdc, _ = determine_impedance(
        net;
        elim_elements = [:c2],
        input_pins = [:B5],
        output_pins = [:gndd],
        freq_range = (10.0, 1000.0, 41),
    )
    @printf("TIMING  p2p_zdc  %.4f\n", t)
    c2 = net.elements[:c2]
    Zin_al = [qalign(Z) for Z in Zin]
    Zeq_al = [qalign(Z) for Z in Zeq]
    Hs = Vector{Matrix{ComplexF64}}(undef, length(omegas))
    Yns = Vector{Matrix{ComplexF64}}(undef, length(omegas))
    freqs = omegas ./ (2π)
    t = @elapsed begin
        for i in eachindex(omegas)
            Y3 = PowerImpedance.eval_y(c2, 1im * omegas[i])
            zdc = Zdc[i]
            Ydc_ext = inv(isa(zdc, Number) ? zdc : zdc[1, 1])
            Yn = ac_yn_from_ss(Y3, Ydc_ext)
            Yns[i] = Yn
            Hs[i] = Yn * Zeq_al[i]
        end
        write_y_csv(joinpath(OUTDIR, "pi_p2p_Z.csv"), freqs, Zin_al)
        write_y_csv(joinpath(OUTDIR, "pi_p2p_Zeq.csv"), freqs, Zeq_al)
        write_y_csv(joinpath(OUTDIR, "pi_p2p_Yn.csv"), freqs, Yns)
        write_y_csv(joinpath(OUTDIR, "pi_p2p_H.csv"), freqs, Hs)
    end
    @printf("TIMING  p2p_H  %.4f\n", t)
    i50 = argmin(abs.(freqs .- 50))
    println("  Zin (q-aligned) @ $(freqs[i50]) Hz:\n", Zin_al[i50])
    println("  Zeq (q-aligned) @ $(freqs[i50]) Hz:\n", Zeq_al[i50])
    println("  H = Yn Zeq     @ $(freqs[i50]) Hz:\n", Hs[i50])
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
