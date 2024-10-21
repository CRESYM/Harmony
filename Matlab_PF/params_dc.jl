
"""
add dc grid parameters

"""

function params_dc(case_name::String)

    global network_dc
    global baseMW_dc
    global bus_dc
    global branch_dc
    global conv
    global pol
    global nbuses_dc
    global nbranches_dc
    global nconvs
    global y_dc
    global rtf
    global xtf
    global bf
    global rec
    global xc
    global ztfc
    global gtfc
    global btfc
    global aloss
    global bloss
    global closs
    global convState
    global pol
    global baseMW_dc
    global basekV_dc

  
    network_dc = create_dc(case_name)
    baseMW_dc = network_dc["baseMW"]
    bus_dc = network_dc["bus"]
    branch_dc = network_dc["branch"]
    conv = network_dc["converter"]
    pol = network_dc["pol"]
    basekV_dc = conv[:, 14]

    nbuses_dc = size(bus_dc, 1)
    nbranches_dc = size(branch_dc, 1)
    nconvs = size(conv, 1)

    y_dc = makeYbus(baseMW_dc, bus_dc, branch_dc);
    y_dc = abs.(y_dc)

    rtf = conv[:, 9]   # transformer resistance
    xtf = conv[:, 10]  # transformer reactance
    bf = conv[:, 11]   # filter susceptance
    rec = conv[:, 12]  # phase reactor resistance
    xc = conv[:, 13]   # phase reactor reactance

    ztfc = Complex.(rtf .+ rec, xtf .+ xc)  # element-wise creation of complex numbers
    gtfc = real.(1 ./ ztfc)                 # element-wise real part of admittance
    btfc = imag.(1 ./ ztfc)                 # element-wise imaginary part of admittance

    # Initialize loss arrays and converter state
    aloss = zeros(nconvs)  # quadratic converter loss 'a'
    bloss = zeros(nconvs)  # quadratic converter loss 'b'
    closs = zeros(nconvs)  # quadratic converter loss 'c'
    convState = zeros(Int, nconvs)  # state (inverter/rectifier) of converter


    # Loop through converters
    for i in 1:nconvs
        # p-control
        if conv[i, 4] == 1
            if conv[i, 6] >= 0  # inverter state
               closs[i] = conv[i, 22]
               convState[i] = 1
           else  # rectifier state
               closs[i] = conv[i, 21]
               convState[i] = 0
           end
       end

        # v-control
       if conv[i, 4] == 2
              if sum(conv[:, 6]) <= 0  # inverter state
              closs[i] = conv[i, 22]
              convState[i] = 1
           else  # rectifier state
              closs[i] = conv[i, 21]
              convState[i] = 0
           end
       end

       # droop control
       if conv[i, 4] == 3
          if conv[i, 6] >= 0  # inverter state
             closs[i] = conv[i, 22]
             convState[i] = 1
          else  # rectifier state
             closs[i] = conv[i, 21]
             convState[i] = 0
           end
       end
   end

# Calculate losses
closs .= closs ./ (basekV_dc.^2 / baseMW_dc)
aloss .= conv[:, 19] ./ baseMW_dc
bloss .= conv[:, 20] ./ basekV_dc
   return
end