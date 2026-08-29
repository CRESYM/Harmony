#!/usr/bin/env julia
# Full P2P HVDC (@network + modular MMC) — export Z_dd at B7 with g1 eliminated.
using Pkg
Pkg.activate(@__DIR__)

using DelimitedFiles
using LinearAlgebra
using PowerImpedance
import PowerImpedance: @network

const OUTDIR = joinpath(@__DIR__, "results")
mkpath(OUTDIR)

transmissionVoltage = 380 / sqrt(3)
pHVDC1 = 100.0
qC1 = 100.0
qC2 = 100.0
Vdc_kV = 640.0

net = @network begin
    voltageBase = transmissionVoltage

    g1 = ac_source(
        setpoint = PowerImpedance.Setpoint(Vac = transmissionVoltage, Pac = pHVDC1),
        limits = PowerImpedance.Limits(P_min = -2000, P_max = 2000, Q_max = 1000, Q_min = -1000),
        pins = 3, transformation = true,
    )

    g4 = ac_source(
        setpoint = Setpoint(Vac = transmissionVoltage, Pac = pHVDC1),
        limits = Limits(P_min = -2000, P_max = 2000, Q_max = 1000, Q_min = -1000),
        pins = 3, transformation = true,
    )

    c1 = mmc(
        elec = PowerImpedance.ElectricalMMC(vDC_base = Vdc_kV),
        sync = PowerImpedance.PLLSynchronization(pi_ctrl = PowerImpedance.PIControl(Kp = 0.28, Ki = 12.5664)),
        delta_control = PowerImpedance.ΔdqControlGFL(
            outer_active = PowerImpedance.OuterActiveVdcControl(pi_ctrl = PowerImpedance.PIControl(Kp = 6, Ki = 15)),
            outer_reactive = PowerImpedance.OuterReactiveQControl(pi_ctrl = PowerImpedance.PIControl(Kp = 0.1, Ki = 31.4159)),
            occ = PowerImpedance.InnerCurrentPIControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 0.7691, Ki = 522.7654),
                activate_ω_c_multiplication = false,
            ),
        ),
        sigma_control = PowerImpedance.ΣdqzControlTEC(
            ccsc = PowerImpedance.CirculatingCurrentSuppressionControl(PowerImpedance.PIControl(Kp = 0.1048, Ki = 48.1914)),
        ),
        setpoint = PowerImpedance.Setpoint(
            Pac = -pHVDC1, Qac = qC1, θac = 0.0,
            Vac = transmissionVoltage * sqrt(2),
            Pdc = -pHVDC1, Vdc = Vdc_kV,
        ),
        limits = PowerImpedance.Limits(P_min = -1500, P_max = 1500, Q_min = -500, Q_max = 500),
    )

    c2 = mmc(
        elec = PowerImpedance.ElectricalMMC(
            vDC_base = Vdc_kV,
            vACbase_LL_RMS = 333,
            turnsRatio = 333 / 380,
            Lᵣ = 0.0461,
            Rᵣ = 0.4103,
            Lₐᵣₘ = 30e-3,
        ),
        sync = PowerImpedance.PLLSynchronization(pi_ctrl = PowerImpedance.PIControl(Kp = 0.28, Ki = 12.5664)),
        delta_control = PowerImpedance.ΔdqControlGFL(
            outer_active = PowerImpedance.OuterActivePowerControl(pi_ctrl = PowerImpedance.PIControl(Kp = 0.1, Ki = 31.4159)),
            outer_reactive = PowerImpedance.OuterReactiveQControl(pi_ctrl = PowerImpedance.PIControl(Kp = 0.1, Ki = 31.4159)),
            occ = PowerImpedance.InnerCurrentPIControl(
                pi_ctrl = PowerImpedance.PIControl(Kp = 0.7691, Ki = 522.7654),
                activate_ω_c_multiplication = false,
            ),
        ),
        sigma_control = PowerImpedance.ΣdqzControlTEC(
            ccsc = PowerImpedance.CirculatingCurrentSuppressionControl(PowerImpedance.PIControl(Kp = 0.1048, Ki = 48.1914)),
        ),
        setpoint = PowerImpedance.Setpoint(
            Pac = pHVDC1, Qac = qC2, θac = 0.0,
            Vac = transmissionVoltage * sqrt(2),
            Pdc = pHVDC1, Vdc = Vdc_kV,
        ),
        limits = PowerImpedance.Limits(P_min = -1000, P_max = 1000, Q_min = -1000, Q_max = 1000),
    )

    dc_line = cable(
        length = 100e3, positions = [(-0.5, 1), (0.5, 1)],
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
        conductors = Conductors(organization = :flat, nᵇ = 3, nˢᵇ = 1, Rᵈᶜ = 0.063, rᶜ = 0.015,
            yᵇᶜ = 30, Δyᵇᶜ = 0, Δxᵇᶜ = 10, Δ̃xᵇᶜ = 0, dˢᵇ = 0, dˢᵃᵍ = 10),
        groundwires = Groundwires(nᵍ = 2, Rᵍᵈᶜ = 0.92, rᵍ = 0.0062, Δxᵍ = 6.5, Δyᵍ = 7.5, dᵍˢᵃᵍ = 10),
        earth_parameters = (1, 1, 100), transformation = true,
    )

    tl78 = overhead_line(
        length = 90e3,
        conductors = Conductors(organization = :flat, nᵇ = 3, nˢᵇ = 1, Rᵈᶜ = 0.063, rᶜ = 0.015,
            yᵇᶜ = 30, Δyᵇᶜ = 0, Δxᵇᶜ = 10, Δ̃xᵇᶜ = 0, dˢᵇ = 0, dˢᵃᵍ = 10),
        groundwires = Groundwires(nᵍ = 2, Rᵍᵈᶜ = 0.92, rᵍ = 0.0062, Δxᵍ = 6.5, Δyᵍ = 7.5, dᵍˢᵃᵍ = 10),
        earth_parameters = (1, 1, 100), transformation = true,
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

imp_ac, omega_ac = determine_impedance(
    net;
    elim_elements = [:g1],
    input_pins = [:B7d, :B7q],
    output_pins = [:gndd, :gndq],
    freq_range = (10.0, 1000.0, 100),
)

Z_dd = getindex.(imp_ac, 1, 1)
freqs = omega_ac ./ (2π)
rows = hcat(freqs, real.(Z_dd), imag.(Z_dd))

open(joinpath(OUTDIR, "pi_p2p_Zdd.csv"), "w") do io
    println(io, "freq_Hz,Re_Zdd,Im_Zdd")
    writedlm(io, rows, ',')
end
println("Wrote ", joinpath(OUTDIR, "pi_p2p_Zdd.csv"))
