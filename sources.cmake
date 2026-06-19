# -----------------------------------------------------------------------------
# sources.cmake
#
# This file keeps track of all Harmony source files (.cpp) and header
# files (.h).
#
# If you add, remove, or rename files in the project, this is the place
# to update them.
#
# The file also organizes the project structure shown in Visual Studio
# (folders/groups in the Solution Explorer) to keep the workspace tidy.
#
# CMakeLists.txt includes this file and uses the variables defined here
# (HARMONY_SOURCE_FILES and HARMONY_HEADER_FILES) to build Harmony.
#
# The file is organized into five sections:
#   1. Group header files (.h)
#   2. Group source files (.cpp)
#   3. Collect all header files into HARMONY_HEADER_FILES
#   4. Collect all source files into HARMONY_SOURCE_FILES
#   5. Define Visual Studio project groups (filters)
#
# In short:
# - Add new source/header files here
# - Keep project file organization here
# - No compiler settings or external library configuration are handled here
#
# Build settings are configured in CMakeLists.txt
# External libraries are configured in cmake/dependencies.cmake
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# 1. Group header files (.h)
# -----------------------------------------------------------------------------
set(Element_Header_Files src/Elements/Element.h)
set(Element_Converter_Header_Files
    src/Elements/Converter/Converter.h
    src/Elements/Converter/MMC.h
)
set(Element_Impedance_Header_Files
    src/Elements/Impedance/Impedance_base.h
    src/Elements/Impedance/Impedance.h
    src/Elements/Impedance/Resistor.h
    src/Elements/Impedance/Admittance.h
    src/Elements/Impedance/Capacitor.h
    src/Elements/Impedance/Inductor.h
)
set(Element_Load_Header_Files
    src/Elements/Load/Load_base.h
    src/Elements/Load/Load.h
    src/Elements/Load/Load_PQ.h
)
set(Element_Switch_Header_Files
    src/Elements/Switch/Switch.h
)
set(Element_Source_Header_Files 
    src/Elements/Source/Source_base.h
    src/Elements/Source/AC_source.h
    src/Elements/Source/Generator.h
    src/Elements/Source/DC_source.h
)
set(Element_Transformer_Header_Files
    src/Elements/Transformer/Transformer_base.h
    src/Elements/Transformer/Transformer_classic.h
    src/Elements/Transformer/Transformer_Delta_Delta.h
    src/Elements/Transformer/Transformer_Delta_Y.h
    src/Elements/Transformer/Transformer_Delta_Y_real.h
    src/Elements/Transformer/Transformer_real.h
    src/Elements/Transformer/Transformer_Y_Delta.h
    src/Elements/Transformer/Transformer_Y_Y.h
    src/Elements/Transformer/Transformer_Y_Y_real.h
)
set(Element_Transmission_Line_Header_Files
    src/Elements/Transmission_Line/Cable.h
    src/Elements/Transmission_Line/overhead_line.h
    src/Elements/Transmission_Line/Transmissionline.h
)
set(Element_RES_Header_Files
    src/Elements/RES/RES_base.h
    src/Elements/RES/WT_type_3.h
    src/Elements/RES/WT_type_4.h
    src/Elements/RES/WP_plant.h
    src/Elements/RES/PV_plant.h
)
set(Solver_State_Space_Model_Header_Files
    src/Solver/State_Space_Model/State_Space_Model.h
)
set(Solver_OPF_Header_Files
    src/Solver/OPF/Powerflow.h
)
set(Solver_DQsym_Header_Files
    src/Solver/DQsym/DQsym.h
)
set(Solver_Stability_Estimate_Header_Files
    src/Solver/Stability_Estimate/Stability_estimate.h
)
set(Solver_Helper_Functions_Header_Files
    src/Solver/Helper_Functions/Helper_Functions.h
    src/Solver/Helper_Functions/Standard_functions.h
    src/Solver/Helper_Functions/Differential_equations.h
    src/Solver/Helper_Functions/Symbolic_functions.h
    src/Solver/Helper_Functions/Visualization.h
    src/Solver/Helper_Functions/Writer.h
    src/Solver/Helper_Functions/stb_image_write.h
    src/Solver/Helper_Functions/DQsym_Conversion_Functions.h
)
set(Control_Header_Files
    src/Control/Control_block.h
    src/Control/Filter.h
    src/Control/Integrator.h
)
set(Controller_Header_Files
    src/Control/Controller/Controller.h
    src/Control/Controller/ProportionalIntegral_controller.h
    src/Control/Controller/Proportional_controller.h
)
set(Header_Files
    src/Bus.h
    src/Constants.h
    src/Include_components.h
    src/Include_control_blocks.h
    src/network.h
    src/SubNetwork.h
)
set(Examples_Header_files
    src/examples/Examples.h
)

# -----------------------------------------------------------------------------
# 2. Group source files (.cpp)
# -----------------------------------------------------------------------------
set(Element_Source_Files src/Elements/Element.cpp)
set(Element_Converter_Source_Files
    src/Elements/Converter/Converter.cpp
    src/Elements/Converter/MMC.cpp
)
set(Element_Impedance_Source_Files
    src/Elements/Impedance/Impedance_base.cpp
    src/Elements/Impedance/Impedance.cpp
    src/Elements/Impedance/Resistor.cpp
    src/Elements/Impedance/Admittance.cpp
    src/Elements/Impedance/Capacitor.cpp
    src/Elements/Impedance/Inductor.cpp
)
set(Element_Load_Source_Files
    src/Elements/Load/Load_base.cpp
    src/Elements/Load/Load.cpp
    src/Elements/Load/Load_PQ.cpp
)
set(Element_Switch_Source_Files
    src/Elements/Switch/Switch.cpp
)
set(Element_Source_Source_Files 
    src/Elements/Source/Source_base.cpp
    src/Elements/Source/AC_source.cpp
    src/Elements/Source/Generator.cpp
    src/Elements/Source/DC_source.cpp
)
set(Element_Transformer_Source_Files
    src/Elements/Transformer/Transformer_base.cpp
    src/Elements/Transformer/Transformer_classic.cpp
    src/Elements/Transformer/Transformer_Delta_Delta.cpp
    src/Elements/Transformer/Transformer_Delta_Y.cpp
    src/Elements/Transformer/Transformer_Delta_Y_real.cpp
    src/Elements/Transformer/Transformer_real.cpp
    src/Elements/Transformer/Transformer_Y_Delta.cpp
    src/Elements/Transformer/Transformer_Y_Y.cpp
    src/Elements/Transformer/Transformer_Y_Y_real.cpp
)
set(Element_Transmission_Line_Source_Files
    src/Elements/Transmission_Line/Cable.cpp
    src/Elements/Transmission_Line/overhead_line.cpp
    src/Elements/Transmission_Line/Transmissionline.cpp
)
set(Element_RES_Source_Files
    src/Elements/RES/RES_base.cpp
    src/Elements/RES/WT_type_3.cpp
    src/Elements/RES/WT_type_4.cpp
    src/Elements/RES/WP_plant.cpp
    src/Elements/RES/PV_plant.cpp
)
set(Solver_State_Space_Model_Source_Files
    src/Solver/State_Space_Model/State_Space_Model.cpp
)
set(Solver_Stability_Estimate_Source_Files
    src/Solver/Stability_Estimate/Stability_estimate.cpp
)
set(Solver_OPF_Source_Files
    src/Solver/OPF/Powerflow_data.cpp
    src/Solver/OPF/Powerflow_network_params.cpp
    src/Solver/OPF/Powerflow_solver.cpp
)
set(Solver_DQsym_Source_Files
    src/Solver/DQsym/DQsym.cpp
)
set(Solver_Helper_Functions_Source_Files
    src/Solver/Helper_Functions/Standard_functions.cpp
    src/Solver/Helper_Functions/Differential_equations.cpp
    src/Solver/Helper_Functions/Symbolic_functions.cpp
    src/Solver/Helper_Functions/Visualization.cpp
    src/Solver/Helper_Functions/Writer.cpp
    src/Solver/Helper_Functions/DQsym_Conversion_Functions.cpp
)
set(Control_Source_Files
    src/Control/Control_block.cpp
    src/Control/Filter.cpp
    src/Control/Integrator.cpp
)
set(Controller_Source_Files
    src/Control/Controller/Controller.cpp
    src/Control/Controller/ProportionalIntegral_controller.cpp
    src/Control/Controller/Proportional_controller.cpp
)
set(Source_Files
    src/Bus.cpp
    src/Constants.cpp
    src/cli.cpp
    src/main.cpp
    src/network.cpp
    src/SubNetwork.cpp
)
set(Json_Source_Files
    src/json/component_builder.cpp
    src/json/computation_runner.cpp
    src/json/json_validator.cpp
    src/json/simulation_builder.cpp
)
set(Json_Header_Files
    src/json/component_builder.h
    src/json/computation_runner.h
    src/json/json_validator.h
    src/json/simulation_builder.h
)
set(Examples_Source_files
    src/examples/example_OPF.cpp
    src/examples/example_OPF_csv.cpp
    src/examples/example_OPF_1.cpp
    src/examples/example_OPF_csv_1.cpp
    src/examples/example_OPF_PV.cpp
    src/examples/example_OPF_WT.cpp
    src/examples/example_DQsym_math_operations.cpp
    src/examples/example_generator.cpp
    src/examples/example_MMC.cpp
    src/examples/example_WT_type_3.cpp
    src/examples/example_WT_type_4.cpp
    src/examples/example_PV_plant.cpp
    src/examples/example_OHL.cpp
    src/examples/example_cable.cpp
    src/examples/example_transformer.cpp
    src/examples/example_constructors.cpp
    src/examples/example_visuals.cpp
    src/examples/example_stability_check.cpp
    src/examples/example_admittance_parameters.cpp
    src/examples/example_point2point_case.cpp
    src/examples/example_DQsym_DSSS2.cpp
    src/examples/example_DQsym_RLC.cpp
    src/examples/example_DQsym_Simple_MMC.cpp
    src/examples/example_state_space.cpp   
)

# -----------------------------------------------------------------------------
# 3. Define HARMONY_HEADER_FILES variable
# -----------------------------------------------------------------------------
set(HARMONY_HEADER_FILES
    ${Header_Files}
    ${Element_Header_Files}
    ${Element_Converter_Header_Files}
    ${Element_Generator_Header_Files}
    ${Element_Impedance_Header_Files}
    ${Element_Load_Header_Files}
    ${Element_Source_Header_Files}
    ${Element_Transformer_Header_Files}
    ${Element_Transmission_Line_Header_Files}
    ${Element_Switch_Header_Files}
    ${Element_RES_Header_Files}
    ${Control_Header_Files}
    ${Controller_Header_Files}
    ${Solver_OPF_Header_Files}
    ${Solver_DQsym_Header_Files}
    ${Solver_Stability_Estimate_Header_Files}
    ${Solver_Helper_Functions_Header_Files}
    ${Solver_State_Space_Model_Header_Files}
    ${Examples_Header_files}
    ${Json_Header_Files}
)

# -----------------------------------------------------------------------------
# 4. Define HARMONY_SOURCE_FILES variable
# -----------------------------------------------------------------------------
set(HARMONY_SOURCE_FILES
    ${Source_Files}
    ${Element_Source_Files}
    ${Element_Converter_Source_Files}
    ${Element_Generator_Source_Files}
    ${Element_Impedance_Source_Files}
    ${Element_Load_Source_Files}
    ${Element_Source_Source_Files}
    ${Element_Transformer_Source_Files}
    ${Element_Transmission_Line_Source_Files}
    ${Element_Switch_Source_Files}
    ${Element_RES_Source_Files}
    ${Control_Source_Files}
    ${Controller_Source_Files}
    ${Solver_OPF_Source_Files}
    ${Solver_DQsym_Source_Files}
    ${Solver_Stability_Estimate_Source_Files}
    ${Solver_Helper_Functions_Source_Files}
    ${Solver_State_Space_Model_Source_Files}
    ${Examples_Source_files}
    ${Json_Source_Files}
)

# -----------------------------------------------------------------------------
# 5. Define Visual Studio project filters
# -----------------------------------------------------------------------------
if(MSVC)
    source_group("Header Files" FILES ${Header_Files})
    source_group("Header Files\\Elements" FILES ${Element_Header_Files})
    source_group("Header Files\\Elements\\Converter" FILES ${Element_Converter_Header_Files})
    source_group("Header Files\\Elements\\Generator" FILES ${Element_Generator_Header_Files})
    source_group("Header Files\\Elements\\Impedance" FILES ${Element_Impedance_Header_Files})
    source_group("Header Files\\Elements\\Load" FILES ${Element_Load_Header_Files})
    source_group("Header Files\\Elements\\Source" FILES ${Element_Source_Header_Files})
    source_group("Header Files\\Elements\\Transformer" FILES ${Element_Transformer_Header_Files})
    source_group("Header Files\\Elements\\Transmission Line" FILES ${Element_Transmission_Line_Header_Files})
    source_group("Header Files\\Elements\\Switch" FILES ${Element_Switch_Header_Files})
    source_group("Header Files\\Elements\\RES" FILES ${Element_RES_Header_Files})
    source_group("Header Files\\Control" FILES ${Control_Header_Files})
    source_group("Header Files\\Control\\Controller" FILES ${Controller_Header_Files})
    source_group("Header Files\\Solver" FILES)
    source_group("Header Files\\Solver\\OPF" FILES ${Solver_OPF_Header_Files})
    source_group("Header Files\\Solver\\DQsym" FILES ${Solver_DQsym_Header_Files})
    source_group("Header Files\\Solver\\Stability Estimate" FILES ${Solver_Stability_Estimate_Header_Files})
    source_group("Header Files\\Solver\\State Space Model" FILES ${Solver_State_Space_Model_Header_Files})
    source_group("Header Files\\Solver\\Helper Functions" FILES ${Solver_Helper_Functions_Header_Files})
    source_group("Header Files\\Examples" FILES ${Examples_Header_files})
    source_group("Header Files\\JSON" FILES ${Json_Header_Files})


    source_group("Source Files" FILES ${Source_Files})
    source_group("Source Files\\Elements" FILES ${Element_Source_Files})
    source_group("Source Files\\Elements\\Converter" FILES ${Element_Converter_Source_Files})
    source_group("Source Files\\Elements\\Generator" FILES ${Element_Generator_Source_Files})
    source_group("Source Files\\Elements\\Impedance" FILES ${Element_Impedance_Source_Files})
    source_group("Source Files\\Elements\\Load" FILES ${Element_Load_Source_Files})
    source_group("Source Files\\Elements\\Source" FILES ${Element_Source_Source_Files})
    source_group("Source Files\\Elements\\Transformer" FILES ${Element_Transformer_Source_Files})
    source_group("Source Files\\Elements\\Transmission_Line" FILES ${Element_Transmission_Line_Source_Files})
    source_group("Source Files\\Elements\\Switch" FILES ${Element_Switch_Source_Files})
    source_group("Source Files\\Elements\\RES" FILES ${Element_RES_Source_Files})
    source_group("Source Files\\Control" FILES ${Control_Source_Files})
    source_group("Source Files\\Control\\Controller" FILES ${Controller_Source_Files})
    source_group("Source Files\\Solver" FILES)
    source_group("Source Files\\Solver\\OPF" FILES ${Solver_OPF_Source_Files})
    source_group("Source Files\\Solver\\DQsym" FILES ${Solver_DQsym_Source_Files})
    source_group("Source Files\\Solver\\Stability Estimate" FILES ${Solver_Stability_Estimate_Source_Files})
    source_group("Source Files\\Solver\\State Space Model" FILES ${Solver_State_Space_Model_Source_Files})
    source_group("Source Files\\Solver\\Helper Functions" FILES ${Solver_Helper_Functions_Source_Files})
    source_group("Source Files\\Examples" FILES ${Examples_Source_files})
    source_group("Source Files\\JSON" FILES ${Json_Source_Files})
endif()
