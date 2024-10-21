    include("create_ac.jl") # define interconnected ac grids
    include("create_dc.jl") # define multi-terminal dc grid
    include("makeYbus.jl") # calculate Bus Admittance Matrix
    include("params_dc.jl") # obtain ac grid parameters 
    include("params_ac.jl") # obtain dc grid parameters 
    acmtdcpf("ac5ac9ac14", "mtdc4lsack") # run ac/dc opf


