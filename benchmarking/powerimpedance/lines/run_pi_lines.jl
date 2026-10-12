# Standalone Y(f) for every OHL organization and coaxial cable layout.
using PowerImpedance
using Printf

const ROOT = @__DIR__
const OUTDIR = joinpath(ROOT, "results")
mkpath(OUTDIR)

include(joinpath(ROOT, "generated_line_cases.jl"))

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

function print_spot(label, Ys, freqs, f_target)
    i = argmin(abs.(freqs .- f_target))
    Y = Ys[i]
    println("  $label @ $(freqs[i]) Hz  size=$(size(Y))")
    show(stdout, "text/plain", Y)
    println()
end

function _ohl_from_gen(c)
    gw = if c.ng > 0
        Groundwires(
            nᵍ = c.ng,
            Rᵍᵈᶜ = c.Rgdc,
            rᵍ = c.rg,
            Δxᵍ = c.dxg,
            Δyᵍ = c.dyg,
            dᵍˢᵃᵍ = c.dgsag,
        )
    else
        Groundwires()
    end
    return overhead_line(
        length = c.length,
        conductors = Conductors(
            organization = c.organization,
            nᵇ = c.nb,
            nˢᵇ = c.nsb,
            Rᵈᶜ = c.Rdc,
            rᶜ = c.rc,
            yᵇᶜ = c.ybc,
            Δyᵇᶜ = c.dy,
            Δxᵇᶜ = c.dx,
            Δ̃xᵇᶜ = c.dxt,
            dˢᵇ = c.dsb,
            dˢᵃᵍ = c.dsag,
        ),
        groundwires = gw,
        earth_parameters = c.earth,
        transformation = false,
    )
end

function _cable_from_gen(c)
    return cable(
        length = c.length,
        positions = c.positions,
        type = c.cable_type,
        earth_parameters = c.earth,
        C1 = Conductor(rᵢ = 0.0, rₒ = 24.25e-3, ρ = 1.72e-8),
        C2 = Conductor(rᵢ = 41.75e-3, rₒ = 46.25e-3, ρ = 22e-8),
        C3 = Conductor(rᵢ = 49.75e-3, rₒ = 60.55e-3, ρ = 18e-8, μᵣ = 10),
        I1 = Insulator(rᵢ = 24.25e-3, rₒ = 41.75e-3, ϵᵣ = 2.3),
        I2 = Insulator(rᵢ = 46.25e-3, rₒ = 49.75e-3, ϵᵣ = 2.3),
        I3 = Insulator(rᵢ = 60.55e-3, rₒ = 65.75e-3, ϵᵣ = 2.3),
        transformation = false,
    )
end

function run_line_topologies()
    println("PowerImpedance.jl ", pkgversion(PowerImpedance))
    println("\n=== OHL and cable topologies ===")
    freqs = logspace(GEN_FREQ_START, GEN_FREQ_STOP, GEN_FREQ_N)
    failed = String[]
    for c in GEN_OHL
        try
            t = @elapsed begin
                println("  OHL ", c.id)
                elem = _ohl_from_gen(c)
                Ys = sweep_y(s -> PowerImpedance.get_y(elem, s), freqs)
                write_y_csv(joinpath(OUTDIR, "pi_$(c.id).csv"), freqs, Ys)
                print_spot(c.id, Ys, freqs, 50.0)
            end
            @printf("TIMING  %s  %.4f\n", c.id, t)
        catch err
            push!(failed, c.id)
            @error "OHL case failed" id = c.id exception = (err, catch_backtrace())
        end
    end
    for c in GEN_CABLE
        try
            t = @elapsed begin
                println("  cable ", c.id)
                elem = _cable_from_gen(c)
                Ys = sweep_y(s -> PowerImpedance.get_y(elem, s), freqs)
                write_y_csv(joinpath(OUTDIR, "pi_$(c.id).csv"), freqs, Ys)
                print_spot(c.id, Ys, freqs, 50.0)
            end
            @printf("TIMING  %s  %.4f\n", c.id, t)
        catch err
            push!(failed, c.id)
            @error "Cable case failed" id = c.id exception = (err, catch_backtrace())
        end
    end
    if !isempty(failed)
        error("Failed line cases: $(join(failed, ", "))")
    end
end

if abspath(PROGRAM_FILE) == @__FILE__
    run_line_topologies()
end
