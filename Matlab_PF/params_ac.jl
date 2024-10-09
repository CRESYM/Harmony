"""
add ac grid parameters

"""

function params_ac(acgrid_name::String, dcgrid_name::String)
    

    global network_ac
    global baseMVA_ac
    global bus_ac_entire
    global branch_ac_entire
    global gencost_ac_entire
    global ngrids
    global bus_ac
    global branch_ac
    global generator_ac
    global gencost_ac
    global recBuses
    global recRef
    global pd_ac
    global qd_ac
    global convids
    global genids
    global nbuses_ac
    global nbranch_ac
    global ngens_ac
    global GG_ac
    global BB_ac
    global refbuscount


    params_dc(dcgrid_name)
    network_ac = create_ac(acgrid_name)
    
    baseMVA_ac = network_ac["baseMVA"]
    bus_ac_entire = network_ac["bus"][network_ac["bus"][:, 2] .< 4, :]
    branch_ac_entire = network_ac["branch"][network_ac["branch"][:, 11] .== 1, :]
    generator_ac_entrie = network_ac["generator"]
    gencost_ac_entire = network_ac["gencost"]
    ngrids = length(unique(bus_ac_entire[:, 14]))

   

    bus_ac = Vector{Any}(undef, ngrids)
    branch_ac = Vector{Any}(undef, ngrids)
    generator_ac = Vector{Any}(undef, ngrids)
    gencost_ac = Vector{Any}(undef, ngrids)
    pd_ac = Vector{Any}(undef, ngrids)
    qd_ac = Vector{Any}(undef, ngrids)

    nbuses_ac = Vector{Any}(undef, ngrids)
    nbranches_ac = Vector{Any}(undef, ngrids)
    IDtoCountmap = Vector{Vector{Int}}(undef, ngrids)
    refbuscount = Vector{Any}(undef, ngrids)

    YY_ac = Vector{Any}(undef, ngrids)
    GG_ac = Vector{Any}(undef, ngrids)
    BB_ac = Vector{Any}(undef, ngrids)

    convids = Vector{Any}(undef, ngrids)
    genids = Vector{Any}(undef, ngrids)

    anglelim_rad = Vector{Any}(undef, ngrids)
    minangle = Vector{Any}(undef, ngrids)
    maxangle = Vector{Any}(undef, ngrids)

    recBuses = Vector{Vector{Int}}(undef, ngrids)
    recRef = Vector{Vector{Int}}(undef, ngrids)
    ngens_ac = Vector{Any}(undef, ngrids)

    for ng in 1:ngrids

        recBuses[ng] = Int[]
        recRef[ng] = Int[]

        bus_ac[ng] = bus_ac_entire[bus_ac_entire[:, 14] .== ng, :]
        branch_ac[ng] = branch_ac_entire[branch_ac_entire[:, 14] .== ng, :]
        generator_ac[ng] = generator_ac_entrie[generator_ac_entrie[:, 22] .== ng, :]
        gencost_ac[ng] = gencost_ac_entire[gencost_ac_entire[:, 8] .== ng, :]

        nbuses_ac[ng] = size(bus_ac[ng], 1)
        nbranches_ac[ng] = size(branch_ac[ng], 1)
        push!(recBuses[ng], nbuses_ac[ng])

        IDtoCountmap[ng] = zeros(Int, nbuses_ac[ng])
        refbuscount[ng] = 0
        for i in 1:nbuses_ac[ng]
            IDtoCountmap[ng][Int(bus_ac[ng][i, 1])] = i  
            if bus_ac[ng][i, 2] == 3  
               refbuscount[ng] = i  
            end
        end
        push!(recRef[ng], refbuscount[ng])

        ngens_ac[ng] = size(generator_ac[ng], 1)

        genids[ng] = Vector{Any}(nothing, nbuses_ac[ng])
    
        for i in 1:ngens_ac[ng]
            if generator_ac[ng][i, 8] == 0
                continue
            end
            index = IDtoCountmap[ng][Int(generator_ac[ng][i, 1])]
            genids[ng][index] = i
        end

        convids[ng] = Vector{Any}(nothing, nbuses_ac[ng])
           
        for i in 1:nconvs
              if conv[i, 3] == ng
                 convids[ng][Int(conv[i, 2])] = [i, Int(conv[i, 3])]
              end
        end
        
        anglelim_rad[ng] = zeros(nbranches_ac[ng], 2)
        for i in 1:nbranches_ac[ng]
            local minangle = branch_ac[ng][i, 12]
            local maxangle = branch_ac[ng][i, 13]
        
            if (minangle == 0 && maxangle == 0) || (minangle == -360 && maxangle == 360)
                minangle = -180
                maxangle = 180
            end
        
            anglelim_rad[ng][i, 1] = π * minangle / 180
            anglelim_rad[ng][i, 2] = π * maxangle / 180
        end
        
        YY_ac[ng] = makeYbus(baseMVA_ac, bus_ac[ng], branch_ac[ng])
        GG_ac[ng] = real(YY_ac[ng])
        BB_ac[ng] = imag(YY_ac[ng])

        pd_ac[ng] = bus_ac[ng][:, 3] / baseMVA_ac
        qd_ac[ng] = bus_ac[ng][:, 4] / baseMVA_ac
    end

    return
end
