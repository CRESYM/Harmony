#!/usr/bin/env julia
# IEEE39 NetworkBuilder soil-resistivity overlays (IEEE39bus_Gridspace.jl parity).
using Pkg
Pkg.activate(@__DIR__)

using DelimitedFiles
using LinearAlgebra
using PowerImpedance
using PowerImpedance.NetworkBuilder: Grid, Gridspace, NetworkState, define

const OUTDIR = joinpath(@__DIR__, "results")
mkpath(OUTDIR)

function include_ieee39_networkbuilder_fixture()
    isdefined(@__MODULE__, :ieee39bus_elements) && return nothing
    skip_testsets(expression) =
        if expression isa Expr &&
           expression.head === :macrocall &&
           expression.args[1] === Symbol("@testset")
            :(nothing)
        else
            expression
        end
    path = joinpath(pkgdir(PowerImpedance), "test", "NetworkBuilder_test.jl")
    Base.include(skip_testsets, @__MODULE__, path)
    return nothing
end

include_ieee39_networkbuilder_fixture()

const IEEE39_SOIL_RESISTIVITY = (10.0, 100.0, 1000.0)
const IEEE39_REPRESENTATIVE_BUSES = (9, 16, 29)
const IEEE39_BUILDER_OPTIONS = (;
    voltageBase = Vm1,
    power_flow = (; is_bounded = (; bus_voltage = true)),
)

function element_at_soil_resistivity(element, soil_resistivity)
    model = element.element_model
    if model isa PowerImpedance.Overhead_line
        return overhead_line(
            length = model.length,
            conductors = deepcopy(model.conductors),
            groundwires = deepcopy(model.groundwires),
            earth_parameters = (model.earth_parameters[1], model.earth_parameters[2], soil_resistivity),
            transformation = element.transformation,
            connection = element.connection,
        )
    elseif model isa PowerImpedance.Cable
        conductors = NamedTuple{Tuple(keys(model.conductors))}(
            Tuple(deepcopy(value) for value in values(model.conductors)),
        )
        insulators = NamedTuple{Tuple(keys(model.insulators))}(
            Tuple(deepcopy(value) for value in values(model.insulators)),
        )
        return cable(;
            length = model.length,
            positions = deepcopy(model.positions),
            earth_parameters = (model.earth_parameters[1], model.earth_parameters[2], soil_resistivity),
            configuration = model.configuration,
            type = model.type,
            eliminate = model.eliminate,
            transformation = element.transformation,
            connection = element.connection,
            conductors...,
            insulators...,
        )
    end
    return deepcopy(element)
end

function ieee39_elements_at_soil_resistivity(base_elements, soil_resistivity)
    names = keys(base_elements)
    elements = map(values(base_elements)) do element
        element_at_soil_resistivity(element, soil_resistivity)
    end
    return NamedTuple{names}(Tuple(elements))
end

function ieee39_soil_builder_space(soil_resistivities = IEEE39_SOIL_RESISTIVITY; base_elements = ieee39bus_elements())
    connections = ieee39bus_connections()
    materialize(soil_resistivity) = define(
        ieee39_elements_at_soil_resistivity(base_elements, soil_resistivity),
        connections;
        options = IEEE39_BUILDER_OPTIONS,
    )
    return Gridspace{NetworkState}(
        materialize,
        (Grid(collect(soil_resistivities)),),
        (:soil_resistivity,),
    )
end

function ieee39_bus_nets(buses)
    return reduce(vcat, ([Symbol("Bus$(bus)d"), Symbol("Bus$(bus)q")] for bus in buses))
end

buses = collect(IEEE39_REPRESENTATIVE_BUSES)
networks = ieee39_soil_builder_space(collect(IEEE39_SOIL_RESISTIVITY))
problems = PowerImpedanceProblem(
    networks;
    nodes = ieee39_bus_nets(buses),
    eliminated_elements = IEEE39_ELIM_ELEMENTS,
    frequency_range = (1.0, 5000.0, 160),
)
result = compute(ParametricProblem(problems), Combinatorial(NodalImpedance()))

for (case_idx, case) in enumerate(result.values)
    rho = IEEE39_SOIL_RESISTIVITY[case_idx]
    for (bus_idx, bus) in enumerate(buses)
        net_index = 2 * bus_idx - 1
        Z = vec(case.response[net_index, net_index, :])
        freqs = real.(case.frequencies) ./ (2π)
        rows = hcat(freqs, real.(Z), imag.(Z))
        tag = "pi_ieee39_bus$(bus)_rho$(Int(rho)).csv"
        open(joinpath(OUTDIR, tag), "w") do io
            println(io, "freq_Hz,Re_Zdd,Im_Zdd")
            writedlm(io, rows, ',')
        end
        println("Wrote ", tag)
    end
end

open(joinpath(OUTDIR, "pi_ieee39_soil_meta.json"), "w") do io
    println(io, "{\"buses\": [9, 16, 29], \"soil_resistivity_ohm_m\": [10, 100, 1000], \"freq_points\": 160}")
end
