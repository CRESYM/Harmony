using JuMP, Ipopt, Printf
 include("create_ac.jl") # define interconnceted ac grids
 include("create_dc.jl") # define multi-terminal dc grid
 include("makeYbus.jl") # calculate Bus Admittance Matrix
 include("params_dc.jl") # obtain ac grid parameters 
 include("params_ac.jl") # obtain dc grid parameters 
 include("acmtdcpf.jl") # run ac/dc opf 
 acmtdcpf("ac5ac9","mtdc3slack_a")