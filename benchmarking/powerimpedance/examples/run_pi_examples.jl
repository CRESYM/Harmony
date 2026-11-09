# Nodal impedances for the PowerImpedance examples Harmony can rebuild.
# Frequencies match examples/run_examples.py (log grid, last point exact).

using PowerImpedance

const OUT = joinpath(@__DIR__, "results")
mkpath(OUT)

logf(a, b, n) = [a * (b / a) ^ ((k - 1) / (n - 1)) for k in 1:n]

function save_z(path, response)
    freqs = real.(response.frequencies) ./ (2π)
    Z = response.response
    n = size(Z, 1)
    open(path, "w") do io
        labels = String["freq_Hz"]
        for i in 1:n, j in 1:n
            append!(labels, ["Re_Z$(i)$(j)", "Im_Z$(i)$(j)"])
        end
        println(io, join(labels, ","))
        for k in eachindex(freqs)
            row = Any[freqs[k]]
            for i in 1:n, j in 1:n
                z = Z[i, j, k]
                push!(row, real(z), imag(z))
            end
            println(io, join(row, ","))
        end
    end
    println("wrote $(path)  $(n)x$(n)  N=$(length(freqs))")
end

function ac_line_elements()
    conductors = Conductors(
        organization = :flat, nᵇ = 3, Rᵈᶜ = 0.063,
        rᶜ = 0.015, yᵇᶜ = 30, Δxᵇᶜ = 10, dˢᵃᵍ = 10)
    groundwires = Groundwires(
        nᵍ = 2, Rᵍᵈᶜ = 0.92, rᵍ = 0.0062,
        Δxᵍ = 6.5, Δyᵍ = 7.5, dᵍˢᵃᵍ = 10)
    return overhead_line(
        length = 200e3, conductors = conductors,
        groundwires = groundwires, earth_parameters = (1, 1, 100))
end

function run_ac_ohl()
    line = ac_line_elements()
    freqs = logf(1.0, 1e4, 61)
    phase = NetworkBuilder.define(
        (; line,),
        (
            NetworkBuilder.Net(:a; terminals = ((:line, :side1, :a),)),
            NetworkBuilder.Net(:b; terminals = ((:line, :side1, :b),)),
            NetworkBuilder.Net(:c; terminals = ((:line, :side1, :c),)),
            NetworkBuilder.Bus(:gnd; ports = ((:line, :side2),)),
        ))
    save_z(joinpath(OUT, "pi_ac_ohl_phase.csv"), compute(
        PowerImpedanceProblem(phase; nodes = [:a, :b, :c], frequency_range = (1.0, 1e4, 61)),
        NodalImpedance()))
    dq = NetworkBuilder.define(
        (; line = ac_line_elements(),),
        (
            NetworkBuilder.Bus(Symbol(""); ports = ((:line, :side1),), domain = ACDQ),
            NetworkBuilder.Bus(:gnd; ports = ((:line, :side2),), domain = ACDQ),
        ))
    save_z(joinpath(OUT, "pi_ac_ohl_dq.csv"), compute(
        PowerImpedanceProblem(dq; nodes = [:d, :q], frequency_range = (1.0, 1e4, 61)),
        NodalImpedance()))
end

function run_dc_ohl()
    line = overhead_line(
        length = 227e3,
        conductors = Conductors(
            nᵇ = 2, nˢᵇ = 2, organization = :flat,
            Rᵈᶜ = 0.06266, rᶜ = 0.01436, yᵇᶜ = 27.5,
            Δxᵇᶜ = 11.8, dˢᵇ = 0.4572, dˢᵃᵍ = 10),
        groundwires = Groundwires(
            nᵍ = 2, Δxᵍ = 6.5, Δyᵍ = 7.5,
            Rᵍᵈᶜ = 0.9196, rᵍ = 0.0062, dᵍˢᵃᵍ = 10),
        earth_parameters = (1, 1, 100))
    network = NetworkBuilder.define(
        (; line,),
        (
            NetworkBuilder.Bus(:positive; ports = ((:line, :side1),)),
            NetworkBuilder.Bus(:gnd; ports = ((:line, :side2),)),
        ))
    save_z(joinpath(OUT, "pi_dc_ohl.csv"), compute(
        PowerImpedanceProblem(network; nodes = [:positive], frequency_range = (1.0, 1e4, 61)),
        NodalImpedance()))
end

function run_cable()
    line = cable(
        length = 60e3,
        positions = [(0, 1.317150236), (-0.07175, 1.441424882), (0.07175, 1.441424882)],
        C1 = Conductor(rₒ = 25.45e-3, ρ = 2.63e-8, μᵣ = 1),
        I1 = Insulator(rᵢ = 25.45e-3, rₒ = 50.65e-3, a = 2e-3, b = 1.5e-3, ϵᵣ = 2.3),
        C2 = Conductor(rᵢ = 50.65e-3, rₒ = 52.35e-3, ρ = 22e-8, μᵣ = 1),
        I2 = Insulator(rᵢ = 52.35e-3, rₒ = 55.75e-3, ϵᵣ = 2.3),
        C3 = Conductor(rᵢ = 55.75e-3, rₒ = 66.55e-3, ρ = 18e-8, μᵣ = 10),
        I3 = Insulator(rᵢ = 66.55e-3, rₒ = 71.75e-3, ϵᵣ = 2.3),
        earth_parameters = (1, 1, 1))
    termination = impedance(z = 1e-3, conductors = 3)
    network = NetworkBuilder.define(
        (; line, termination),
        (
            NetworkBuilder.Net(:a; terminals = ((:line, :side1, :a),)),
            NetworkBuilder.Net(:b; terminals = ((:line, :side1, :b),)),
            NetworkBuilder.Net(:c; terminals = ((:line, :side1, :c),)),
            NetworkBuilder.Net(:end_a; terminals = ((:line, :side2, :a), (:termination, :side1, :a))),
            NetworkBuilder.Net(:end_b; terminals = ((:line, :side2, :b), (:termination, :side1, :b))),
            NetworkBuilder.Net(:end_c; terminals = ((:line, :side2, :c), (:termination, :side1, :c))),
            NetworkBuilder.Bus(:gnd; ports = ((:termination, :side2),)),
        ))
    save_z(joinpath(OUT, "pi_cable.csv"), compute(
        PowerImpedanceProblem(network; nodes = [:a, :b, :c], frequency_range = (1.0, 1e4, 61)),
        NodalImpedance()))
end

const OHL3 = (
    conductors = Conductors(
        organization = :flat, nᵇ = 3, nˢᵇ = 1, Rᵈᶜ = 0.063, rᶜ = 0.015,
        yᵇᶜ = 30, Δyᵇᶜ = 0, Δxᵇᶜ = 10, Δ̃xᵇᶜ = 0, dˢᵇ = 0, dˢᵃᵍ = 10),
    groundwires = Groundwires(
        nᵍ = 2, Rᵍᵈᶜ = 0.92, rᵍ = 0.0062, Δxᵍ = 6.5, Δyᵍ = 7.5, dᵍˢᵃᵍ = 10),
)

function ieee9_network(; capacitance::Bool)
    c = OHL3
    ct = capacitance ? 7e-9 : 0.0
    cs = capacitance ? 12e-9 : 0.0
    elements = (
        g1 = ac_source(v = 16.5 * sqrt(2 / 3)),
        g3 = ac_source(v = 13.8 * sqrt(2 / 3)),
        tl78 = overhead_line(length = 76e3, conductors = c.conductors, groundwires = c.groundwires, earth_parameters = (1, 1, 100)),
        tl89 = overhead_line(length = 106e3, conductors = c.conductors, groundwires = c.groundwires, earth_parameters = (1, 1, 100)),
        tl75 = overhead_line(length = 170e3, conductors = c.conductors, groundwires = c.groundwires, earth_parameters = (1, 1, 100)),
        tl96 = overhead_line(length = 179e3, conductors = c.conductors, groundwires = c.groundwires, earth_parameters = (1, 1, 100)),
        tl54 = overhead_line(length = 89e3, conductors = c.conductors, groundwires = c.groundwires, earth_parameters = (1, 1, 100)),
        tl64 = overhead_line(length = 97e3, conductors = c.conductors, groundwires = c.groundwires, earth_parameters = (1, 1, 100)),
        t1 = transformer(n = 16.5 / 230, Lₚ = 4.9916e-4 / 2, Lₛ = 0.097 / 2, Rₚ = 0, Rₛ = 0,
            V₁ᵒ = 16.5e3 / sqrt(3), V₂ᵒ = 230e3 / sqrt(3), Cₜ = ct, Cₛ = cs, conductors = 3),
        t2 = transformer(n = 18 / 230, Lₚ = 6.4458e-4 / 2, Lₛ = 0.1052 / 2, Rₚ = 0, Rₛ = 0,
            V₁ᵒ = 18e3 / sqrt(3), V₂ᵒ = 230e3 / sqrt(3), Cₜ = ct, Cₛ = cs, conductors = 3),
        t3 = transformer(n = 13.8 / 230, Lₚ = 3.5523e-4 / 2, Lₛ = 0.0987 / 2, Rₚ = 0, Rₛ = 0,
            V₁ᵒ = 13.8e3 / sqrt(3), V₂ᵒ = 230e3 / sqrt(3), Cₜ = ct, Cₛ = cs, conductors = 3),
        l5 = impedance(z = s -> 1269.6 + 10.1s, conductors = 3),
        l6 = impedance(z = s -> 1763.3 + 16.84s, conductors = 3),
        l8 = impedance(z = s -> 1587.16 + 14.43s, conductors = 3),
    )
    connections = (
        NetworkBuilder.Bus(:Bus1; ports = ((:g1, :ac), (:t1, :side1)), domain = ACDQ),
        NetworkBuilder.Bus(:Bus4; ports = ((:t1, :side2), (:tl64, :side1), (:tl54, :side1)), domain = ACDQ),
        NetworkBuilder.Bus(:Bus6; ports = ((:tl64, :side2), (:l6, :side1), (:tl96, :side1)), domain = ACDQ),
        NetworkBuilder.Bus(:gnd; ports = ((:l6, :side2), (:l5, :side2), (:l8, :side2)), domain = ACDQ),
        NetworkBuilder.Bus(:Bus5; ports = ((:tl54, :side2), (:l5, :side1), (:tl75, :side1)), domain = ACDQ),
        NetworkBuilder.Bus(:Bus9; ports = ((:tl96, :side2), (:tl89, :side1), (:t3, :side2)), domain = ACDQ),
        NetworkBuilder.Bus(:Bus7; ports = ((:tl75, :side2), (:tl78, :side1), (:t2, :side2)), domain = ACDQ),
        NetworkBuilder.Bus(:Bus8; ports = ((:tl78, :side2), (:tl89, :side2), (:l8, :side1)), domain = ACDQ),
        NetworkBuilder.Bus(:Bus2; ports = ((:t2, :side1),), domain = ACDQ),
        NetworkBuilder.Bus(:Bus3; ports = ((:t3, :side1), (:g3, :ac)), domain = ACDQ),
    )
    return NetworkBuilder.define(elements, connections)
end

function run_ieee9()
    for (name, cap) in (("pi_ieee9_bus2.csv", true), ("pi_ieee9_bus2_nocap.csv", false))
        save_z(joinpath(OUT, name), compute(
            PowerImpedanceProblem(ieee9_network(; capacitance = cap);
                nodes = [:Bus2d, :Bus2q], frequency_range = (1.0, 1e4, 41)),
            NodalImpedance()))
    end
end

println("AC overhead line")
run_ac_ohl()
println("DC overhead line")
run_dc_ohl()
println("Cable")
run_cable()
println("IEEE 9")
run_ieee9()

function run_ieee39()
    Vm1 = 345 / sqrt(3)
    Rb = (345e3)^2 / 1000e6
    Lb = Rb / (2π * 50)
    c = OHL3
    line_data = (
        (name = :T1_2, from = 1, to = 2, length = 72.4730e3),
        (name = :T1_39, from = 1, to = 39, length = 44.0833e3),
        (name = :T2_3, from = 2, to = 3, length = 26.6263e3),
        (name = :T2_25, from = 2, to = 25, length = 15.1647e3),
        (name = :T3_4, from = 3, to = 4, length = 37.5590e3),
        (name = :T3_18, from = 3, to = 18, length = 23.4523e3),
        (name = :T4_5, from = 4, to = 5, length = 22.5707e3),
        (name = :T4_14, from = 4, to = 14, length = 22.7470e3),
        (name = :T5_6, from = 5, to = 6, length = 4.5847e3),
        (name = :T5_8, from = 5, to = 8, length = 19.7493e3),
        (name = :T6_7, from = 6, to = 7, length = 16.2227e3),
        (name = :T6_11, from = 6, to = 11, length = 14.4593e3),
        (name = :T7_8, from = 7, to = 8, length = 8.1113e3),
        (name = :T8_9, from = 8, to = 9, length = 64.009e3),
        (name = :T9_39, from = 9, to = 39, length = 44.0833e3),
        (name = :T10_11, from = 10, to = 11, length = 7.5823e3),
        (name = :T10_13, from = 10, to = 13, length = 7.5823e3),
        (name = :T13_14, from = 13, to = 14, length = 17.8097e3),
        (name = :T14_15, from = 14, to = 15, length = 38.2643e3),
        (name = :T15_16, from = 15, to = 16, length = 16.5753e3),
        (name = :T16_17, from = 16, to = 17, length = 15.6937e3),
        (name = :T16_19, from = 16, to = 19, length = 34.3850e3),
        (name = :T16_21, from = 16, to = 21, length = 23.8050e3),
        (name = :T16_24, from = 16, to = 24, length = 10.4037e3),
        (name = :T17_18, from = 17, to = 18, length = 14.4593e3),
        (name = :T17_27, from = 17, to = 27, length = 30.5057e3),
        (name = :T21_22, from = 21, to = 22, length = 24.6867e3),
        (name = :T22_23, from = 22, to = 23, length = 16.9280e3),
        (name = :T23_24, from = 23, to = 24, length = 61.7167e3),
        (name = :T25_26, from = 25, to = 26, length = 56.95573e3),
        (name = :T26_27, from = 26, to = 27, length = 25.9210e3),
        (name = :T26_28, from = 26, to = 28, length = 83.5820e3),
        (name = :T26_29, from = 26, to = 29, length = 110.2083e3),
        (name = :T28_29, from = 28, to = 29, length = 26.6263e3),
    )
    transformer_data = (
        (name = :TR2_30, from = 2, to = 30, r = 0, x = 0.0181),
        (name = :TR6_31, from = 6, to = 31, r = 0, x = 0.025),
        (name = :TR10_32, from = 10, to = 32, r = 0, x = 0.02),
        (name = :TR11_12, from = 11, to = 12, r = 0.0016, x = 0.0435),
        (name = :TR12_13, from = 12, to = 13, r = 0.0016, x = 0.0435),
        (name = :TR19_20, from = 19, to = 20, r = 0.0007, x = 0.0138),
        (name = :TR19_33, from = 19, to = 33, r = 0.0007, x = 0.0142),
        (name = :TR20_34, from = 20, to = 34, r = 0.0009, x = 0.0180),
        (name = :TR22_35, from = 22, to = 35, r = 0, x = 0.0143),
        (name = :TR23_36, from = 23, to = 36, r = 0.0005, x = 0.0272),
        (name = :TR25_37, from = 25, to = 37, r = 0.0006, x = 0.0232),
        (name = :TR29_38, from = 29, to = 38, r = 0.0008, x = 0.0156),
    )
    function series_load(P, Q, kind)
        P = P * 1e6
        Q = Q * 1e6
        V = 345e3
        S = 3 * sqrt(P^2 + Q^2)
        I = S / (sqrt(3) * V)
        R = P / I^2
        X = kind == :L ? Q / (I^2 * 2π * 50) : I^2 / (Q * 2π * 50)
        return (; R, X)
    end
    load_data = (
        (name = :LoadB3, bus = 3, P = 107.3333, Q = 0.8, kind = :L),
        (name = :LoadB4a, bus = 4, P = 166.6667, Q = 61.3333, kind = :L),
        (name = :LoadB4b, bus = 4, P = 0, Q = 100, kind = :C),
        (name = :LoadB5, bus = 5, P = 0, Q = 200, kind = :C),
        (name = :LoadB7, bus = 7, P = 77.93333, Q = 28.0, kind = :L),
        (name = :LoadB8, bus = 8, P = 174.0, Q = 58.6667, kind = :L),
        (name = :LoadB12, bus = 12, P = 2.5, Q = 29.3333, kind = :L),
        (name = :LoadB15, bus = 15, P = 106.6667, Q = 51.0, kind = :L),
        (name = :LoadB16, bus = 16, P = 109.8, Q = 10.7667, kind = :L),
        (name = :LoadB18, bus = 18, P = 52.6667, Q = 10.0, kind = :L),
        (name = :LoadB20, bus = 20, P = 226.6667, Q = 34.3333, kind = :L),
        (name = :LoadB21, bus = 21, P = 91.3333, Q = 38.3333, kind = :L),
        (name = :LoadB23, bus = 23, P = 82.5, Q = 28.2, kind = :L),
        (name = :LoadB24, bus = 24, P = 102.8667, Q = 30.73333, kind = :C),
        (name = :LoadB25, bus = 25, P = 74.6667, Q = 15.7333, kind = :L),
        (name = :LoadB26, bus = 26, P = 46.3333, Q = 5.6667, kind = :L),
        (name = :LoadB27, bus = 27, P = 93.6667, Q = 25.1667, kind = :L),
        (name = :LoadB28, bus = 28, P = 68.6667, Q = 9.2, kind = :L),
        (name = :LoadB29, bus = 29, P = 94.5, Q = 8.9667, kind = :L),
        (name = :LoadB32, bus = 32, P = 3.0667, Q = 1.5333, kind = :L),
        (name = :LoadB39, bus = 39, P = 368, Q = 83.3333, kind = :L),
    )
    elements = (;
        (Symbol("Zg", bus) => impedance(z = s -> 0.191736 + s * 0.0122063, conductors = 3)
            for bus in 30:39)...,
        (row.name => transformer(
            n = 1.0, Lₚ = (row.x / 2) * Lb, Lₛ = (row.x / 2) * Lb,
            Rₚ = (row.r / 2) * Rb, Rₛ = (row.r / 2) * Rb, conductors = 3)
            for row in transformer_data)...,
        (row.name => overhead_line(
            length = row.length, conductors = c.conductors, groundwires = c.groundwires,
            earth_parameters = (1, 1, 100)) for row in line_data)...,
        (begin
            sl = series_load(row.P, row.Q, row.kind)
            row.name => impedance(
                z = s -> sl.R + (row.kind == :L ? s * sl.X : 1 / (s * sl.X)),
                conductors = 3)
        end for row in load_data)...,
    )
    connections = (
        (NetworkBuilder.Bus(Symbol("Bus", bus);
            ports = ((Symbol("Zg", bus), :side2),), domain = ACDQ) for bus in 30:39)...,
        (NetworkBuilder.Bus(:gnd; ports = ((Symbol("Zg", bus), :side1),), domain = ACDQ)
            for bus in 30:39)...,
        (NetworkBuilder.Bus(Symbol("Bus", bus); ports = ((row.name, port),), domain = ACDQ)
            for row in (line_data..., transformer_data...)
            for (port, bus) in ((:side1, row.from), (:side2, row.to)))...,
        (NetworkBuilder.Bus(Symbol("Bus", row.bus); ports = ((row.name, :side1),), domain = ACDQ)
            for row in load_data)...,
        (NetworkBuilder.Bus(:gnd; ports = ((row.name, :side2),), domain = ACDQ)
            for row in load_data)...,
    )
    network = NetworkBuilder.define(elements, connections)
    save_z(joinpath(OUT, "pi_ieee39_bus9.csv"), compute(
        PowerImpedanceProblem(network; nodes = [:Bus9d, :Bus9q], frequency_range = (1.0, 5e3, 31)),
        NodalImpedance()))
end

println("IEEE 39 grid at Bus 9, STATCOM removed")
run_ieee39()
println("PI examples done")
