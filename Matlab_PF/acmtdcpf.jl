# 
##############################################################################
## acMtdcPf can be used to do AC/MTDDC PF calculation by solving OPF model ##
##############################################################################

#  References:
#  [1] J. Beerten, S. Cole and R. Belmans, "Generalized Steady-State VSC MTDC Model 
#  for Sequential AC/DC Power Flow Algorithms," in IEEE Transactions on Power Systems, 
#  vol. 27, no. 2, pp. 821-829, May 2012, doi: 10.1109/TPWRS.2011.2177867.
#  [2]  Mauro Escobar, https://github.com/me2533/acopf
#  [3]  Matacdc1.0 user's manual, https://www.esat.kuleuven.be/electa/teaching/matacdc/MatACDCManual

# Contact: h.li199305ee@gmail.com

using JuMP, Ipopt, Printf


function acmtdcpf(ac_name::String, dc_name::String)

elapsed_time = @elapsed begin

    params_dc(dc_name)
    params_ac(ac_name, dc_name)


    # Defining dc grid and converter primal variables
    # [vn_dc, pn_dc, ps, qs, pc, qc, vs, vf, vc, as, af, ac, Ic];
    # 1. vn_dc: dc voltage 
    # 2. pn_dc: dc power injection 
    # 3. ps：active power injection at node s of converter ac side 
    # 4. qs：reactive power injection at node s of converter ac side 
    # 5. pc：active power injection at node c of converter ac side 
    # 6. qc：reactive power injection at node c of converter ac side 
    # 7. vs： nodal voltage amplitude at node s of converter ac side 
    # 8. vf： nodal voltage amplitude at node f of converter ac side 
    # 9. vc： nodal voltage amplitude at node c of converter ac side 
    # 10. as： nodal voltage angle at node s of converter ac side
    # 11. af： nodal voltage angle at node f of converter ac side 
    # 12. ac： nodal voltage angle at node c of converter ac side 
    # 13. Ic: converter current 

    model = Model(Ipopt.Optimizer)
    var_size_dc = 2*nbuses_dc + 11*nconvs

    lb_dc = fill(Inf, var_size_dc)
    ub_dc = fill(-Inf, var_size_dc)

    # 1. Voltage bounds
    lb_dc[1:nbuses_dc] = bus_dc[1:nbuses_dc, 13]
    ub_dc[1:nbuses_dc] = bus_dc[1:nbuses_dc, 12]


    # 2. Power bounds
    lb_dc[nbuses_dc+1:2*nbuses_dc] .= -Inf
    ub_dc[nbuses_dc+1:2*nbuses_dc] .= Inf


    # 3-6. Active and reactive power bounds for converters
    lb_dc[2*nbuses_dc+1:2*nbuses_dc+nconvs] .= -Inf
    ub_dc[2*nbuses_dc+1:2*nbuses_dc+nconvs] .= Inf

    lb_dc[2*nbuses_dc+nconvs+1:2*nbuses_dc+2*nconvs] .= -Inf
    ub_dc[2*nbuses_dc+nconvs+1:2*nbuses_dc+2*nconvs] .= Inf

    lb_dc[2*nbuses_dc+2*nconvs+1:2*nbuses_dc+3*nconvs] .= -Inf
    ub_dc[2*nbuses_dc+2*nconvs+1:2*nbuses_dc+3*nconvs] .= Inf

    lb_dc[2*nbuses_dc+3*nconvs+1:2*nbuses_dc+4*nconvs] .= -Inf
    ub_dc[2*nbuses_dc+3*nconvs+1:2*nbuses_dc+4*nconvs] .= Inf


    # 7-9. Voltage bounds for s, f, and c nodes
    lb_dc[2*nbuses_dc+4*nconvs+1:2*nbuses_dc+5*nconvs] = conv[1:nconvs, 16]
    ub_dc[2*nbuses_dc+4*nconvs+1:2*nbuses_dc+5*nconvs] = conv[1:nconvs, 15]

    lb_dc[2*nbuses_dc+5*nconvs+1:2*nbuses_dc+6*nconvs] = conv[1:nconvs, 16]
    ub_dc[2*nbuses_dc+5*nconvs+1:2*nbuses_dc+6*nconvs] = conv[1:nconvs, 15]

    lb_dc[2*nbuses_dc+6*nconvs+1:2*nbuses_dc+7*nconvs] = conv[1:nconvs, 16]
    ub_dc[2*nbuses_dc+6*nconvs+1:2*nbuses_dc+7*nconvs] = conv[1:nconvs, 15]


    # 10-12. Voltage angle bounds
    lb_dc[2*nbuses_dc+7*nconvs+1:2*nbuses_dc+8*nconvs] .= -π
    ub_dc[2*nbuses_dc+7*nconvs+1:2*nbuses_dc+8*nconvs] .= π

    lb_dc[2*nbuses_dc+8*nconvs+1:2*nbuses_dc+9*nconvs] .= -π
    ub_dc[2*nbuses_dc+8*nconvs+1:2*nbuses_dc+9*nconvs] .= π

    lb_dc[2*nbuses_dc+9*nconvs+1:2*nbuses_dc+10*nconvs] .= -π
    ub_dc[2*nbuses_dc+9*nconvs+1:2*nbuses_dc+10*nconvs] .= π

    # 13. Converter current bounds
    lb_dc[2*nbuses_dc+10*nconvs+1:2*nbuses_dc+11*nconvs] .= 0
    ub_dc[2*nbuses_dc+10*nconvs+1:2*nbuses_dc+11*nconvs] = conv[1:nconvs, 17]

    # Define DC decision variables
    @variable(model, var_dc[1:var_size_dc])
    vn_dc = var_dc[1:nbuses_dc]
    pn_dc = var_dc[nbuses_dc+1:2*nbuses_dc]
    ps = var_dc[2*nbuses_dc+1:2*nbuses_dc+nconvs]
    qs = var_dc[2*nbuses_dc+nconvs+1:2*nbuses_dc+2*nconvs]
    pc = var_dc[2*nbuses_dc+2*nconvs+1:2*nbuses_dc+3*nconvs]
    qc = var_dc[2*nbuses_dc+3*nconvs+1:2*nbuses_dc+4*nconvs]
    vs = var_dc[2*nbuses_dc+4*nconvs+1:2*nbuses_dc+5*nconvs]
    vf = var_dc[2*nbuses_dc+5*nconvs+1:2*nbuses_dc+6*nconvs]
    vc = var_dc[2*nbuses_dc+6*nconvs+1:2*nbuses_dc+7*nconvs]
    as = var_dc[2*nbuses_dc+7*nconvs+1:2*nbuses_dc+8*nconvs]
    af = var_dc[2*nbuses_dc+8*nconvs+1:2*nbuses_dc+9*nconvs]
    ac = var_dc[2*nbuses_dc+9*nconvs+1:2*nbuses_dc+10*nconvs]
    Ic = var_dc[2*nbuses_dc+10*nconvs+1:2*nbuses_dc+11*nconvs]
    @variable(model, convPloss[1:nconvs])


    # Constraints for decision variables
    for i in 1:var_size_dc
        @constraint(model, lb_dc[i] <= var_dc[i] <= ub_dc[i])
    end

    # DC power balance
    for i in 1:nbuses_dc
    @constraint(model, pn_dc[i] == pol * vn_dc[i] * sum(y_dc[i, j] * (vn_dc[i] - vn_dc[j]) for j in 1:nbuses_dc))
    end

    # AC power balance for converters
    for i in 1:nconvs
        @constraint(model, ps[i] == -vs[i] * vc[i] * (gtfc[i] * cos(as[i] - ac[i]) + btfc[i] * sin(as[i] - ac[i])) + vs[i]^2 * gtfc[i])
        @constraint(model, qs[i] == -vs[i] * vc[i] * (gtfc[i] * sin(as[i] - ac[i]) - btfc[i] * cos(as[i] - ac[i])) - vs[i]^2 * btfc[i])
        @constraint(model, pc[i] == -vc[i] * vs[i] * (gtfc[i] * cos(ac[i] - as[i]) + btfc[i] * sin(ac[i] - as[i])) + vc[i]^2 * gtfc[i])
        @constraint(model, qc[i] == -vc[i] * vs[i] * (gtfc[i] * sin(ac[i] - as[i]) - btfc[i] * cos(ac[i] - as[i])) - vc[i]^2 * btfc[i])
    end

    # Converter loss model
    for i in 1:nconvs
        @constraint(model, pc[i] + pn_dc[i] + convPloss[i] == 0)
        @constraint(model, 0 <= convPloss[i] <= 1)
        @constraint(model, convPloss[i] == aloss[i] + Ic[i]*bloss[i] + Ic[i]^2*closs[i])
        @constraint(model, (Ic[i] * vc[i])^2 == pc[i]^2 + qc[i]^2)
    end

    # Converter control logic
    for i in 1:nconvs
        # DC side
        if conv[i, 4] == 1  # P control
            @constraint(model, ps[i] == -conv[i, 6] / baseMVA_dc)
        elseif conv[i, 4] == 2  # V control
            @constraint(model, vn_dc[i] == conv[i, 8])
        else  # Droop control
            @constraint(model, pn_dc[i] == (conv[i, 24] - 1 / conv[i, 23] * (vn_dc[i] - conv[i, 25])) / baseMVA_dc * (-1))
        end

        # AC side
        if conv[i, 5] == 1  # Q control
        @constraint(model, qs[i] == -conv[i, 7] / 100)
        else  # V control
        @constraint(model, vs[i] == conv[i, 8])
        end

        # Inverter or converter mode
        if convState[i] == 0  # Rectifier
            @constraint(model, ps[i] >= 0)
            @constraint(model, pn_dc[i] >= 0)
            @constraint(model, pc[i] <= 0)
        else  # Inverter
            @constraint(model, ps[i] <= 0)
            @constraint(model, pn_dc[i] <= 0)
            @constraint(model, pc[i] >= 0)
        end
    end

    # Defining ac grid primal variables

    # [vm_ac, va_ac, pn_ac, qn_ac, pij_ac, qij_ac, pgen_ac, qgen_ac, theta_diff, vn_product_] 
    # 1 nodal voltage amplitude 
    # 2 nodal voltage phase 
    # 3 nodal active power injection 
    # 4 nodal reactive power injection 
    # 5 active power flow used for calculate active power injection
    # 6 rective power flow used for calculate active power injection
    # 7 generator active power 
    # 8 generator reactive power 
    # 9 node voltage difference 
    # 10 node voltage production

    # multiple ac grids

    var_ac = Vector{Any}(undef, ngrids)
    vm_ac = Vector{Any}(undef, ngrids)
    va_ac = Vector{Any}(undef, ngrids)
    pn_ac = Vector{Any}(undef, ngrids)
    qn_ac = Vector{Any}(undef, ngrids)
    pij_ac = Vector{Any}(undef, ngrids)
    qij_ac = Vector{Any}(undef, ngrids)
    pgen_ac = Vector{Any}(undef, ngrids)
    qgen_ac = Vector{Any}(undef, ngrids)
    va_ac_mat = Vector{Any}(undef, ngrids)
    va_ac_mat_t = Vector{Any}(undef, ngrids)
    theta_diff = Vector{Any}(undef, ngrids)
    vn_product = Vector{Any}(undef, ngrids)
    actgen_ac = Vector{Any}(undef, ngrids)
    lb_ac = Vector{Any}(undef, ngrids)
    ub_ac = Vector{Any}(undef, ngrids)

    obj = Vector{Any}(undef, ngrids)


    for ng = 1:ngrids

        local var_size_ac = 4*nbuses_ac[ng] + 2*ngens_ac[ng]

        lb_ac[ng] = fill(Inf, var_size_ac)
        ub_ac[ng] = fill(-Inf, var_size_ac)

        # 1. lb and ub for nodal voltage amplitude
        lb_ac[ng][1:nbuses_ac[ng]] = bus_ac[ng][:, 13]
        ub_ac[ng][1:nbuses_ac[ng]] = bus_ac[ng][:, 12]

        # Nodal voltage amplitude for refbus is determined
        lb_ac[ng][refbuscount[ng]] = generator_ac[ng][findall(x -> x == refbuscount[ng], generator_ac[ng][:, 1]), 6][1]
        ub_ac[ng][refbuscount[ng]] = generator_ac[ng][findall(x -> x == refbuscount[ng], generator_ac[ng][:, 1]), 6][1]

        # 2. lb and ub for nodal phase
        k = nbuses_ac[ng]
        lb_ac[ng][k+1:k+nbuses_ac[ng]] .= -π
        ub_ac[ng][k+1:k+nbuses_ac[ng]] .= π

        # Nodal voltage phase for refbus is determined
        lb_ac[ng][k + refbuscount[ng]] = π * bus_ac[ng][refbuscount[ng], 9] / 180
        ub_ac[ng][k + refbuscount[ng]] = π * bus_ac[ng][refbuscount[ng], 9] / 180

        # 3. lb and ub of nodal active power injection
        k = 2 * nbuses_ac[ng]
        lb_ac[ng][k+1:k+nbuses_ac[ng]] .= -Inf
        ub_ac[ng][k+1:k+nbuses_ac[ng]] .= Inf

        # 4. lb and ub of nodal reactive power injection
        k = 3 * nbuses_ac[ng]
        lb_ac[ng][k+1:k+nbuses_ac[ng]] .= -Inf
        ub_ac[ng][k+1:k+nbuses_ac[ng]] .= Inf

        # 5. lb and ub of generator active power output
        k = 4 * nbuses_ac[ng]
        lb_ac[ng][k+1:k+ngens_ac[ng]] = generator_ac[ng][:, 10] .* generator_ac[ng][:, 8] / baseMVA_ac
        ub_ac[ng][k+1:k+ngens_ac[ng]] = generator_ac[ng][:, 9] .* generator_ac[ng][:, 8] / baseMVA_ac

        # 6. lb and ub of generator reactive power output
        k = 4 * nbuses_ac[ng] + ngens_ac[ng]
        lb_ac[ng][k+1:k+ngens_ac[ng]] = generator_ac[ng][:, 5] .* generator_ac[ng][:, 8] / baseMVA_ac
        ub_ac[ng][k+1:k+ngens_ac[ng]] = generator_ac[ng][:, 4] .* generator_ac[ng][:, 8] / baseMVA_ac

    end

    for ng = 1:ngrids
        var_ac[ng] = @variable(model, [1:4*nbuses_ac[ng] + 2*ngens_ac[ng]])
        vm_ac[ng] = var_ac[ng][1:nbuses_ac[ng]]
        va_ac[ng] = var_ac[ng][nbuses_ac[ng]+1:2*nbuses_ac[ng]]
        pn_ac[ng] = var_ac[ng][2*nbuses_ac[ng]+1:3*nbuses_ac[ng]]
        qn_ac[ng] = var_ac[ng][3*nbuses_ac[ng]+1:4*nbuses_ac[ng]]
        pgen_ac[ng] = var_ac[ng][4*nbuses_ac[ng]+1:4*nbuses_ac[ng]+ngens_ac[ng]]
        qgen_ac[ng] = var_ac[ng][4*nbuses_ac[ng]+ngens_ac[ng]+1:4*nbuses_ac[ng]+2*ngens_ac[ng]]
    end

    for ng = 1:ngrids
        @constraint(model, lb_ac[ng] .<= var_ac[ng] .<= ub_ac[ng]) 
        va_ac_mat[ng] = repeat(va_ac[ng], 1, nbuses_ac[ng])  
        va_ac_mat_t[ng] = repeat(va_ac[ng]', nbuses_ac[ng], 1) 
        theta_diff[ng] = va_ac_mat[ng] .- va_ac_mat_t[ng]
        vn_product[ng] = vm_ac[ng] * vm_ac[ng]'
        pij_ac[ng] = vn_product[ng] .* (GG_ac[ng] .* cos.(theta_diff[ng]) .+ BB_ac[ng] .* sin.(theta_diff[ng]))
        qij_ac[ng] = vn_product[ng] .* (GG_ac[ng] .* sin.(theta_diff[ng]) .- BB_ac[ng] .* cos.(theta_diff[ng]))
        @constraint(model, pn_ac[ng] == sum(pij_ac[ng], dims=2))
        @constraint(model, qn_ac[ng] == sum(qij_ac[ng], dims=2))
    end
    

    for ng = 1:ngrids
    for i in 1:nbuses_ac[ng]
            if !isnothing(genids[ng][i]) && !isnothing(convids[ng][i])
            
            if convids[ng][i][2] == ng
                    @constraint(model, pn_ac[ng][i] == sum(pgen_ac[ng][genids[ng][i]]) - pd_ac[ng][i] - ps[convids[ng][i][1]])
                    @constraint(model, qn_ac[ng][i] == sum(qgen_ac[ng][genids[ng][i]]) - qd_ac[ng][i] - qs[convids[ng][i][1]])
            else
                    @constraint(model, pn_ac[ng][i] == sum(pgen_ac[ng][genids[ng][i]]) - pd_ac[ng][i])
                    @constraint(model, qn_ac[ng][i] == sum(qgen_ac[ng][genids[ng][i]]) - qd_ac[ng][i])
            end

        elseif !isnothing(genids[ng][i])
                @constraint(model, pn_ac[ng][i] == sum(pgen_ac[ng][genids[ng][i]]) - pd_ac[ng][i])
                @constraint(model, qn_ac[ng][i] == sum(qgen_ac[ng][genids[ng][i]]) - qd_ac[ng][i])

            elseif !isnothing(convids[ng][i])
                
                if convids[ng][i][2] == ng
                    @constraint(model, pn_ac[ng][i] == -pd_ac[ng][i] - ps[convids[ng][i][1]])
                    @constraint(model, qn_ac[ng][i] == -qd_ac[ng][i] - qs[convids[ng][i][1]])
                else
                    @constraint(model, pn_ac[ng][i] == -pd_ac[ng][i])
                    @constraint(model, qn_ac[ng][i] == -qd_ac[ng][i])
                end
                
        else
            @constraint(model, pn_ac[ng][i] == -pd_ac[ng][i])
            @constraint(model, qn_ac[ng][i] == -qd_ac[ng][i])
        end

        
        end
    end

    for ng = 1:ngrids
        for i in 1:nbuses_ac[ng]
            if !isnothing(convids[ng][i])  
                if convids[ng][i][2] == ng
                    @constraint(model, vm_ac[ng][i] == vs[convids[ng][i][1]])
                    @constraint(model, va_ac[ng][i] == as[convids[ng][i][1]])
                end
            end
        end
    end

    for ng = 1:ngrids
        actgen_ac[ng] = generator_ac[ng][:, 8]

        if gencost_ac[ng][1, 4] == 3
        obj[ng] = sum(actgen_ac[ng] .* 
                    (baseMVA_ac^2 .* gencost_ac[ng][:, 5] .* pgen_ac[ng].^2 + 
                    baseMVA_ac .* gencost_ac[ng][:, 6] .* pgen_ac[ng] + 
                    gencost_ac[ng][:, 7]))
    elseif gencost_ac[ng][1, 4] == 2
        obj[ng] = sum(actgen_ac[ng] .* 
                    (baseMVA_ac .* gencost_ac[ng][:, 5] .* pgen_ac[ng] + 
                    gencost_ac[ng][:, 6]))
    end
    end

    @objective(model, Min, sum(obj))
    set_optimizer_attribute(model, "mu_strategy", "adaptive")
    set_optimizer_attribute(model, "tol", 1e-5)
    set_optimizer_attribute(model, "dual_inf_tol", 1e-1)
    set_optimizer_attribute(model, "compl_inf_tol", 1e-5)
    set_optimizer_attribute(model, "acceptable_compl_inf_tol", 1e-3)
    optimize!(model)

end


# Power flow result outputs
println("================================================================================")
println("|   AC  Bus Data                                                                 |")
println("================================================================================")
println(" Bus      Area      Voltage          Generation             Load        ")
println(" #        #    Mag [pu] Ang [deg]   P [MW]   Q [MVAr]   P [MW]  Q [MVAr]")
println("-----   -----  --------  --------  --------  --------  -------- --------")

for ng in 1:ngrids
    genindex = generator_ac[ng][:, 1]

    for i in 1:recBuses[ng][]
        
        formatted_vm_ac = @sprintf("%.3f", value(vm_ac[ng][i]))
        formatted_va_ac = @sprintf("%.3f", value(va_ac[ng][i]) / π * 180)

        print(lpad(i, 3), " ", lpad(ng, 7, " "), " ", 
              lpad(formatted_vm_ac, 9, " "),  
              lpad(formatted_va_ac, 11, " "))
        
         if i == recRef[ng][]
             print("*")
         end

        
         if i in genindex
            m = generator_ac[ng][:, 1]
            formatted_pgen_ac = @sprintf("%.2f", value(pgen_ac[ng][findfirst(m .== i)[1], 1]) * baseMVA_ac)
            formatted_qgen_ac = @sprintf("%.2f", value(qgen_ac[ng][findfirst(m .== i)[1], 1]) * baseMVA_ac)
            
            if i == recRef[ng][]
                print( lpad(formatted_pgen_ac, 9, " "),
                       lpad(formatted_qgen_ac, 10, " "))
            else
                print(lpad(formatted_pgen_ac, 10, " "),
                      lpad(formatted_qgen_ac, 10, " "))
            end
         
            formatted_pd = @sprintf("%.2f", value(pd_ac[ng][i]) * baseMVA_ac)
            formatted_qd = @sprintf("%.2f", value(qd_ac[ng][i]) * baseMVA_ac)

            print(lpad(formatted_pd, 10, " "), lpad(formatted_qd, 8, " "))

         else
            formatted_pd = @sprintf("%.2f", value(pd_ac[ng][i]) * baseMVA_ac)
            formatted_qd = @sprintf("%.2f", value(qd_ac[ng][i]) * baseMVA_ac)
            print(" " * "       -         -")
            print(lpad(formatted_pd, 11, " "), lpad(formatted_qd, 8, " "))
        end
        print("\n") 
     end

end

GenCost = objective_value(model)
println("-----   ----- --------  --------  --------  --------  -------- --------")
println(@sprintf("The total generation costs is ＄%.2f/MWh (€%.2f/MWh)", GenCost, GenCost / 1.08))
print("\n") 


println("================================================================================")
println("|   DC bus data                                                                 |")
println("================================================================================")
println(" Bus   Bus    AC   DC Voltage   DC Power   PCC Bus Injection   Converter loss")
println(" DC #  AC #  Area   Vdc [pu]    Pdc [MW]   Ps [MW]  Qs [MVAr]  Conv_Ploss [MW]")
println("-----  ----  ----  ---------    --------   -------  --------    --------")

for i in 1:nbuses_dc
    formatted_vn_dc = @sprintf("%.3f", value(vn_dc[i]))
    formatted_pn_dc = @sprintf("%.3f", value(pn_dc[i]) * baseMVA_dc)
    formatted_ps = @sprintf("%.3f", value(ps[i]) * baseMVA_dc)
    formatted_qs = @sprintf("%.3f", value(qs[i]) * baseMVA_dc)
    formatted_convPloss = @sprintf("%.3f", value(convPloss[i]) * baseMVA_dc)

    print(
    lpad(i, 3), " ", 
    lpad(conv[i, 2], 5), " ", 
    lpad(conv[i, 3], 5), " ", 
    lpad(formatted_vn_dc, 9), " ", 
    lpad(formatted_pn_dc, 12), " ", 
    lpad(formatted_ps, 9), " ", 
    lpad(formatted_qs, 9), " ", 
    lpad(formatted_convPloss, 11),
    "\n")

end

    println("-----  ----  ----  ---------    --------   -------  --------    --------")
    formatted_total_loss = @sprintf("%.3f", value(sum(convPloss)) * baseMVA_dc)
    @printf("The total converter losses is %s MW", formatted_total_loss)
    print("\n") 

    println("\n Computation time is: ", elapsed_time, " s")

 return

end








