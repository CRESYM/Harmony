#include "network.h"
#include "Bus.h"
#include "Include_components.h"
#include "examples/Examples.h"



// [Helper] Error callback for GLFW
void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


/**
 * @brief Main entry point for the circuit simulation program.
 *
 * This function initializes and runs various examples to demonstrate the
 * capabilities of the circuit analysis library.
 *
 * @return 0 on successful execution.
 */
int main() {

	//example_WT_type_3();
	//example_WT_type_4();
	//example_PV_plant();
	//example_MMC();
	//example_OHL();
	//example_cable();
	//example_transformer();
	//example_generator();

	//Solver examples
	//example_OPF();
	//example_OPF_1();
	//example_OPF_csv();
	//example_OPF_csv_1();
	//example_point2point_case();

	//example_DQsym_math_operations();
	//example_DQsym_DSSS2();
    //example_DQsym_RLC();
	example_DQsym_Simple_MMC();
	//example_DQsym_validation();

	// State-space examples
	//example_state_space();
	 
	//example_visuals();
	//example_stability_check();
	//example_admittance_parameters();

	return 0;

}
