using SparseArrays
using LinearAlgebra

function makeYbus(baseMVA, bus, branch)

    # constants
    nb = size(bus, 1)  
    nl = size(branch, 1)  

    # define named indices into bus, branch matrices
    BUS_I, GS, BS = 1, 5, 6  
    F_BUS, T_BUS, BR_R, BR_X, BR_B, TAP, SHIFT, BR_STATUS = 1, 2, 3, 4, 5, 9, 10, 11

    if any(bus[:, BUS_I] .!= collect(1:nb))
        error("makeYbus: buses must be numbered consecutively in bus matrix")
    end

    # for each branch, compute the elements of the branch admittance matrix where

    #   | If |   | Yff  Yft |   | Vf |
    #   |    | = |          | * |    |
    #   | It |   | Ytf  Ytt |   | Vt |
    stat = branch[:, BR_STATUS]  # ones at in-service branches
    Ys = stat ./ (branch[:, BR_R] .+ 1im * branch[:, BR_X])  # series admittance
    Bc = stat .* branch[:, BR_B]  # line charging susceptance

    # calucate ratio
    tap = ones(nl)  # default tap ratio = 1
    idx = findall(branch[:, TAP] .!= 0)  # indices of non-zero tap ratios
    tap[idx] .= branch[idx, TAP]  # assign non-zero tap ratios
   
   # ensure branch[:, SHIFT] is converted to Float64
   shifts = Float64.(branch[:, SHIFT])  # convert to Float64 array
   phase_shifts = 1im * π / 180 * shifts  # calculate phase shifts
   tap .= tap .* exp.(phase_shifts)

    # calculate addimtance matrix
    Ytt = Ys .+ 1im * Bc / 2
    Yff = Ytt ./ (tap .* conj(tap))
    Yft = -Ys ./ conj(tap)
    Ytf = -Ys ./ tap

    # compute shunt admittance
    Ysh = (bus[:, GS] .+ 1im * bus[:, BS]) / baseMVA

    # bus indices
    f = branch[:, F_BUS]  # list of "from" buses
    t = branch[:, T_BUS]  # list of "to" buses

    # build Yf and Yt 
    i = vcat(1:nl, 1:nl)  # double set of row indices
    Yf = sparse(i, vcat(f, t), vcat(Yff, Yft), nl, nb)
    Yt = sparse(i, vcat(f, t), vcat(Ytf, Ytt), nl, nb)

    Ybus = sparse(vcat(f, f, t, t), vcat(f, t, f, t), vcat(Yff, Yft, Ytf, Ytt), nb, nb) +
           sparse(1:nb, 1:nb, Ysh, nb, nb)

    return Ybus
end
