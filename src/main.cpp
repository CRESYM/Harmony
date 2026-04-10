#include "network.h"
#include "Bus.h"
#include "Include_components.h"
#include "examples/Examples.h"

// Implot testing
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
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
	// example_PV_plant();
	//example_MMC();
	// example_OHL();
	// example_cable();
	//example_transformer();
	// example_generator();
	//example_point2point_case();

	// Solver examples
	//example_OPF();
	//example_OPF_1();
	//example_OPF_csv();
	//example_OPF_csv_1();
	


	//example_DQsym_math_operations();
	//example_DQsym_DSSS2();
	//example_DQsym_RLC();
	//example_DQsym_Simple_MMC();
	 
	// example_visuals();
	// example_stability_check();
	 //example_stability_check();
	// example_admittance_parameters();

	// Try1. Call
	// Manually initialize the contexts
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImPlot::CreateContext();

    // std::cout << "ImGui/ImPlot Contexts Created Successfully!" << std::endl;

    // // Check versions to be sure
    // std::cout << "ImGui Version: " << IMGUI_VERSION << std::endl;

    // // Clean up
    // ImPlot::DestroyContext();
    // ImGui::DestroyContext();	

	// -----------------------------------------
	// // Try2. Plot

	// 1. Initialize Contexts
	// IMGUI_CHECKVERSION();
	// ImGui::CreateContext();
	// ImPlot::CreateContext();

	// std::cout << "ImGui/ImPlot Contexts Created Successfully!" << std::endl;
	// // 2. Define your data (Example: a simple diagonal line)
	// float x_data[] = { 0, 1, 2, 3, 4, 5 };
	// float y_data[] = { 0, 1, 4, 9, 16, 25 };

	// // 3. Create the Plot Canvas
	// // This usually happens inside your main render loop
	// if (ImPlot::BeginPlot("My First Plot")) {
	// 	std::cout << "ImGui/ImPlot Contexts Created Successfully!" << std::endl;
	// 	// Set up axes (optional labels)
	// 	ImPlot::SetupAxes("X-Axis", "Y-Axis");

	// 	// 4. Draw the actual line
	// 	// "Line Label" is what shows in the legend
	// 	ImPlot::PlotLine("Quadratic Growth", x_data, y_data, 6);

	// 	// End the plot
	// 	ImPlot::EndPlot();
	// }

	// // 5. Clean up
	// ImPlot::DestroyContext();
	// ImGui::DestroyContext();

	// ---------------------------
	// Try3

	// 1. Initialize GLFW
    if (!glfwInit()) return 1;

    // macOS Compatibility hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Harmony - Grid Simulation Plot", NULL, NULL);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync (60 FPS)

    // 2. Setup ImGui/ImPlot Contexts
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // 3. Initialize Backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    // Sample data for your electrical grid simulation
    std::vector<float> x_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<float> y_data = {0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100};

    // 4. Main Render Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 5. Create the UI and Plot
        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Simulation Results");

        if (ImPlot::BeginPlot("My First ImPlot Line")) {
            ImPlot::SetupAxes("Time (s)", "Voltage (V)");
            ImPlot::PlotLine("Phase A", x_data.data(), y_data.data(), x_data.size());
            ImPlot::EndPlot();
        }

        ImGui::End();

        // 6. Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.15f, 0.15f, 0.15f, 1.00f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 7. Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();


	return 0;

}
