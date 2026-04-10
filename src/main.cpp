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


    // --------- IMPLOT

    // -------- This block prints the version numbers but does not plot -------------
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
    // -------------------------------------------------------------------------------



    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    // --- PLATFORM SPECIFIC SETUP ---
    const char* glsl_version;
#ifdef __APPLE__
    // macOS requires Core Profile and Forward Compatibility
    glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // Windows and Linux
    glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Harmony - Electrical Grid Simulator", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // --- INITIALIZE CONTEXTS ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Setup Style
    ImGui::StyleColorsDark();

    // --- INITIALIZE BACKENDS ---
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- DATA FOR PLOTTING ---
    // Example: A 60Hz Sine Wave for your grid simulation
    std::vector<float> x_data, y_data;
    for (int i = 0; i < 1000; ++i) {
        float t = i * 0.001f;
        x_data.push_back(t);
        y_data.push_back(sinf(2.0f * 3.14159f * 60.0f * t)); // 60Hz signal
    }

    // --- MAIN LOOP ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show a simple control panel
        ImGui::Begin("Simulation Controls");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        if (ImGui::Button("Reset Simulation")) {
            // Your logic here
        }
        ImGui::End();

        // 2. Show the Plot
        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Grid Visualization");

        if (ImPlot::BeginPlot("Voltage Phase A", ImVec2(-1, -1))) {
            ImPlot::SetupAxes("Time (s)", "Voltage (V)");
            ImPlot::PlotLine("Bus 1", x_data.data(), y_data.data(), (int)x_data.size());
            ImPlot::EndPlot();
        }
        ImGui::End();

        // --- RENDERING ---
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // --- CLEANUP ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();


	return 0;

}
