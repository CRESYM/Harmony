using Pkg
Pkg.activate(@__DIR__)
using PowerImpedance
e = cable(
    length = 100e3,
    positions = [(-0.5, 1), (0.5, 1)],
    C1 = Conductor(rₒ = 24.25e-3, ρ = 1.72e-8),
)
println("typeof(e) = ", typeof(e))
println("fieldnames = ", fieldnames(typeof(e)))
for f in fieldnames(typeof(e))
    println(f, " => ", getfield(e, f))
end
