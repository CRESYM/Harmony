/**
 * @file harmony_launcher.cpp
 * @brief HarmonyUI launcher: pick examples/JSON, run solvers, optional plots and PNG export.
 */
#include "harmony_launcher.h"

#include "Constants.h"
#include "cli.h"
#include "ui/harmony_banner_gui.h"
#include "ui/harmony_glfw_setup.h"
#include "log_capture.h"
#include "Solver/Helper_Functions/Visualization.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <future>
#include <mutex>
#include <optional>
#include <thread>

namespace {

enum class SourceKind {
	Cpp,
	Json
};

enum class MainTab {
	Launcher = 0,
	Plots = 1,
	Log = 2
};

struct LauncherState {
	SourceKind sourceKind = SourceKind::Json;
	int cppIndex = 0;
	int jsonIndex = 0;
	bool plot = false;
	bool verbose = true;
	std::atomic<bool> running{ false };
	std::atomic<int> lastExitCode{ 0 };
	std::atomic<bool> switchToPlotsTab{ false };
	MainTab selectedMainTab = MainTab::Launcher;
	char jsonPathInput[512]{ "opf_csv.json" };
	char outputDir[512]{ "." };

	std::vector<std::string> cppNames;
	std::vector<std::string> cppOpfNames;
	std::vector<std::string> cppOtherNames;
	std::vector<std::filesystem::path> jsonFiles;
	std::vector<std::filesystem::path> jsonOpfFiles;
	std::vector<std::filesystem::path> jsonOtherFiles;
	std::vector<std::filesystem::path> searchPaths;

	std::mutex logMutex;
	std::deque<std::string> logLines;
	std::string logText;
	bool logScrollToBottom = true;

	std::future<void> worker;

	ImFont* bannerFont = nullptr;
};

void drawEmbeddedBanner(LauncherState& state) {
	harmonyDrawBannerImGui(state.bannerFont, /*compact*/ false);
	ImGui::Separator();
}

bool isOpfCppExample(const std::string& name) {
	static const char* kOpf[] = {
		"opf", "opf_csv", "opf_1", "opf_csv_1", "opf_pv", "opf_wt"
	};
	for (const char* item : kOpf) {
		if (name == item) {
			return true;
		}
	}
	return false;
}

bool isOpfJsonPath(const std::filesystem::path& path) {
	const std::string stem = path.stem().string();
	std::string lower = stem;
	std::transform(lower.begin(), lower.end(), lower.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return lower.find("opf") != std::string::npos;
}

void appendLogLine(LauncherState& state, const std::string& line) {
	std::lock_guard<std::mutex> lock(state.logMutex);
	state.logLines.push_back(line);
	while (state.logLines.size() > 4000) {
		state.logLines.pop_front();
	}
	state.logText.clear();
	for (const auto& entry : state.logLines) {
		state.logText += entry;
		state.logText += '\n';
	}
	state.logScrollToBottom = true;
}

void clearLog(LauncherState& state) {
	std::lock_guard<std::mutex> lock(state.logMutex);
	state.logLines.clear();
	state.logText.clear();
}

void refreshCatalog(LauncherState& state) {
	state.searchPaths = buildJsonSearchPaths({}, {});

	state.cppNames = cppExampleNames();
	state.cppOpfNames.clear();
	state.cppOtherNames.clear();
	for (const auto& name : state.cppNames) {
		if (isOpfCppExample(name)) {
			state.cppOpfNames.push_back(name);
		}
		else {
			state.cppOtherNames.push_back(name);
		}
	}

	state.jsonFiles = discoverJsonFiles(state.searchPaths);
	state.jsonOpfFiles.clear();
	state.jsonOtherFiles.clear();
	for (const auto& path : state.jsonFiles) {
		if (isOpfJsonPath(path)) {
			state.jsonOpfFiles.push_back(path);
		}
		else {
			state.jsonOtherFiles.push_back(path);
		}
	}

	if (state.cppIndex >= static_cast<int>(state.cppNames.size())) {
		state.cppIndex = 0;
	}
	if (state.jsonIndex >= static_cast<int>(state.jsonFiles.size())) {
		state.jsonIndex = 0;
	}
}

std::filesystem::path selectedJsonPath(const LauncherState& state) {
	if (state.jsonIndex >= 0 && state.jsonIndex < static_cast<int>(state.jsonFiles.size())) {
		return state.jsonFiles[static_cast<size_t>(state.jsonIndex)];
	}
	return resolveJsonPath(state.jsonPathInput, state.searchPaths);
}

std::string selectedCppName(const LauncherState& state) {
	if (state.cppIndex >= 0 && state.cppIndex < static_cast<int>(state.cppNames.size())) {
		return state.cppNames[static_cast<size_t>(state.cppIndex)];
	}
	return {};
}

void drawCppCombo(LauncherState& state) {
	if (ImGui::BeginCombo("C++ example", selectedCppName(state).c_str())) {
		if (!state.cppOpfNames.empty()) {
			ImGui::SeparatorText("OPF");
			for (const auto& name : state.cppOpfNames) {
				const bool selected = (selectedCppName(state) == name);
				if (ImGui::Selectable(name.c_str(), selected)) {
					const auto it = std::find(state.cppNames.begin(), state.cppNames.end(), name);
					if (it != state.cppNames.end()) {
						state.cppIndex = static_cast<int>(std::distance(state.cppNames.begin(), it));
					}
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		if (!state.cppOtherNames.empty()) {
			ImGui::SeparatorText("Other");
			for (const auto& name : state.cppOtherNames) {
				const bool selected = (selectedCppName(state) == name);
				if (ImGui::Selectable(name.c_str(), selected)) {
					const auto it = std::find(state.cppNames.begin(), state.cppNames.end(), name);
					if (it != state.cppNames.end()) {
						state.cppIndex = static_cast<int>(std::distance(state.cppNames.begin(), it));
					}
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}
}

void drawJsonCombo(LauncherState& state) {
	const auto current = selectedJsonPath(state);
	const std::string currentLabel = current.empty() ? "(custom path)" : current.filename().string();
	if (ImGui::BeginCombo("JSON file", currentLabel.c_str())) {
		if (!state.jsonOpfFiles.empty()) {
			ImGui::SeparatorText("OPF");
			for (const auto& path : state.jsonOpfFiles) {
				const std::string label = path.filename().string();
				const bool selected = (current == path);
				if (ImGui::Selectable(label.c_str(), selected)) {
					const auto it = std::find(state.jsonFiles.begin(), state.jsonFiles.end(), path);
					if (it != state.jsonFiles.end()) {
						state.jsonIndex = static_cast<int>(std::distance(state.jsonFiles.begin(), it));
					}
					std::strncpy(state.jsonPathInput, path.filename().string().c_str(), sizeof(state.jsonPathInput) - 1);
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		if (!state.jsonOtherFiles.empty()) {
			ImGui::SeparatorText("Other");
			for (const auto& path : state.jsonOtherFiles) {
				const std::string label = path.filename().string();
				const bool selected = (current == path);
				if (ImGui::Selectable(label.c_str(), selected)) {
					const auto it = std::find(state.jsonFiles.begin(), state.jsonFiles.end(), path);
					if (it != state.jsonFiles.end()) {
						state.jsonIndex = static_cast<int>(std::distance(state.jsonFiles.begin(), it));
					}
					std::strncpy(state.jsonPathInput, path.filename().string().c_str(), sizeof(state.jsonPathInput) - 1);
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}

	ImGui::InputText("Or path / filename", state.jsonPathInput, IM_ARRAYSIZE(state.jsonPathInput));
	if (ImGui::Button("Refresh list")) {
		refreshCatalog(state);
	}
}

void startRun(LauncherState& state) {
	if (state.running.load()) {
		return;
	}

	clearLog(state);
	if (state.plot) {
		visualization_clear_tabs();
	}
	state.running = true;
	state.lastExitCode = 0;

	const SourceKind kind = state.sourceKind;
	const bool plot = state.plot;
	const bool verbose = state.verbose;
	const std::string cppName = selectedCppName(state);
	const std::filesystem::path jsonPath = selectedJsonPath(state);
	const std::vector<std::filesystem::path> searchPaths = state.searchPaths;

	state.worker = std::async(std::launch::async, [&, kind, plot, verbose, cppName, jsonPath, searchPaths]() {
		StreamCapture capture([&state](const std::string& line) {
			appendLogLine(state, line);
		});

		int exitCode = EXIT_FAILURE;
		try {
			if (kind == SourceKind::Cpp) {
				appendLogLine(state, "Running C++ example: " + cppName
					+ " (plot=" + std::string(plot ? "on" : "off") + ")");
				exitCode = runCppExample(cppName, plot, verbose);
			}
			else {
				std::filesystem::path resolved = jsonPath;
				if (!std::filesystem::exists(resolved)) {
					resolved = resolveJsonPath(jsonPath.generic_string(), searchPaths);
				}
				if (!std::filesystem::exists(resolved)) {
					appendLogLine(state, "JSON file not found: " + jsonPath.generic_string());
					exitCode = EXIT_FAILURE;
				}
				else {
					appendLogLine(state, "Running JSON: " + resolved.generic_string()
						+ " (plot=" + std::string(plot ? "on" : "off") + ")");
					exitCode = runJsonSimulation(resolved, verbose, plot, /*waitForPlotClose*/ false);
				}
			}
		}
		catch (const std::exception& ex) {
			appendLogLine(state, std::string("Run failed: ") + ex.what());
			exitCode = EXIT_FAILURE;
		}

		if (plot && visualization_has_tabs()) {
			appendLogLine(state, "Plots ready — open the Plots tab.");
			state.switchToPlotsTab = true;
		}
		else if (plot) {
			appendLogLine(state, "Plot was checked but no visualization tabs were created.");
		}

		appendLogLine(state, exitCode == EXIT_SUCCESS ? "Finished successfully." : "Finished with errors.");
		state.lastExitCode = exitCode;
		state.running = false;
	});
}

void drawLauncherContent(LauncherState& state) {
	drawEmbeddedBanner(state);

	if (const auto root = harmonyRepoRoot()) {
		ImGui::TextDisabled("Repo: %s", root->generic_string().c_str());
	}
	else {
		ImGui::TextDisabled("Repo: (not detected — use full paths or run from repo root)");
	}

	ImGui::Separator();

	const char* sourceLabels[] = { "C++ example", "JSON file" };
	int sourceIndex = static_cast<int>(state.sourceKind);
	if (ImGui::Combo("Source", &sourceIndex, sourceLabels, IM_ARRAYSIZE(sourceLabels))) {
		state.sourceKind = static_cast<SourceKind>(sourceIndex);
	}

	if (state.sourceKind == SourceKind::Cpp) {
		drawCppCombo(state);
		ImGui::TextDisabled("%zu OPF examples listed above.", state.cppOpfNames.size());
	}
	else {
		drawJsonCombo(state);
		ImGui::TextDisabled("%zu OPF JSON files listed above.", state.jsonOpfFiles.size());
	}

	ImGui::Separator();

	ImGui::Checkbox("Plot", &state.plot);
	ImGui::SameLine();
	ImGui::TextDisabled("(visualization only when checked)");
	ImGui::Checkbox("Verbose log", &state.verbose);

	ImGui::InputText("PNG output directory", state.outputDir, IM_ARRAYSIZE(state.outputDir));

	ImGui::Separator();

	if (state.running.load()) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Run")) {
		startRun(state);
	}
	ImGui::SameLine();
	if (ImGui::Button("Validate JSON") && state.sourceKind == SourceKind::Json) {
		clearLog(state);
		std::filesystem::path resolved = selectedJsonPath(state);
		if (!std::filesystem::exists(resolved)) {
			resolved = resolveJsonPath(state.jsonPathInput, state.searchPaths);
		}
		std::string error;
		if (validateJsonFile(resolved, error)) {
			appendLogLine(state, "Validation OK: " + resolved.generic_string());
		}
		else {
			appendLogLine(state, "Validation failed: " + error);
		}
	}
	if (state.running.load()) {
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::Text("Running…");
	}
	else if (state.lastExitCode.load() != 0) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.45f, 1.f), "Last run failed (%d)", state.lastExitCode.load());
	}
	else if (!state.logText.empty()) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.45f, 1.f, 0.55f, 1.f), "Last run OK");
	}
}

void drawPlotsContent(LauncherState& state) {
	if (!visualization_has_tabs()) {
		ImGui::TextWrapped(
			"Enable Plot on the Launcher tab and run a case that produces charts.");
		return;
	}

	const std::filesystem::path outDir(state.outputDir);
	if (!outDir.empty() && !std::filesystem::exists(outDir)) {
		ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f), "PNG output directory does not exist yet.");
		if (ImGui::Button("Create output directory")) {
			std::error_code ec;
			std::filesystem::create_directories(outDir, ec);
		}
		ImGui::Separator();
	}

	if (ImGui::Button("Save all tabs as PNG")) {
		for (const auto& title : visualization_tab_titles()) {
			visualization_save_tab(title, outDir);
		}
		appendLogLine(state, "Scheduled PNG save for all plot tabs.");
	}

	ImGui::Separator();
	visualization_draw_embedded("HarmonyPlotTabs");
}

void drawLogContent(LauncherState& state) {
	if (ImGui::Button("Clear")) {
		clearLog(state);
	}
	ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &state.logScrollToBottom);

	std::string textCopy;
	{
		std::lock_guard<std::mutex> lock(state.logMutex);
		textCopy = state.logText;
	}

	const ImVec2 size = ImVec2(0, 0);
	ImGui::BeginChild("LogScroll", size, false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::TextUnformatted(textCopy.c_str());
	if (state.logScrollToBottom) {
		ImGui::SetScrollHereY(1.0f);
	}
	ImGui::EndChild();
}

void drawMainWindow(LauncherState& state) {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(io.DisplaySize);
	ImGui::Begin(
		"HarmonyUI",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus);

	if (state.switchToPlotsTab.exchange(false)) {
		state.selectedMainTab = MainTab::Plots;
	}

	const auto tabButton = [&](const char* label, MainTab tab) {
		const bool selected = (state.selectedMainTab == tab);
		if (selected) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.52f, 0.78f, 1.f));
		}
		if (ImGui::Button(label)) {
			state.selectedMainTab = tab;
		}
		if (selected) {
			ImGui::PopStyleColor();
		}
		ImGui::SameLine();
	};

	tabButton("Launcher", MainTab::Launcher);
	tabButton("Plots", MainTab::Plots);
	tabButton("Log", MainTab::Log);
	ImGui::NewLine();
	ImGui::Separator();

	switch (state.selectedMainTab) {
	case MainTab::Launcher:
		drawLauncherContent(state);
		break;
	case MainTab::Plots:
		drawPlotsContent(state);
		break;
	case MainTab::Log:
		drawLogContent(state);
		break;
	default:
		break;
	}

	ImGui::End();
}

} // namespace

int runHarmonyLauncher(int argc, char* argv[]) {
	if (argc > 0) {
		initCliPaths(argv[0]);
	}

	LauncherState state;
	refreshCatalog(state);
	visualization_set_embedded_mode(true);

	glfwSetErrorCallback([](int, const char* desc) {
		std::cerr << "GLFW Error: " << desc << '\n';
	});
	if (!glfwInit()) {
		std::cerr << "HarmonyUI: GLFW init failed\n";
		return EXIT_FAILURE;
	}

#ifdef __APPLE__
	const char* glslVersion = "#version 150";
#else
	const char* glslVersion = "#version 130";
#endif
	harmonyConfigureGlfwOpenGLHints();

	GLFWwindow* window = nullptr;
	int initialWidth = 1500;
	int initialHeight = 980;
	if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) {
		if (const GLFWvidmode* mode = glfwGetVideoMode(monitor)) {
			initialWidth = static_cast<int>(mode->width * 0.88);
			initialHeight = static_cast<int>(mode->height * 0.88);
		}
	}

	window = glfwCreateWindow(initialWidth, initialHeight, "HarmonyUI", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "HarmonyUI: failed to create window\n";
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	if (!harmonyInitOpenGLLoader()) {
		std::cerr << "HarmonyUI: OpenGL loader init failed\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);

	harmonyConfigurePlotUi(ImGui::GetIO());
	state.bannerFont = harmonyInitBannerFont(ImGui::GetIO(), kHarmonyLauncherBannerFontSizePx);
	ImGui::GetIO().Fonts->Build();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	ImGui_ImplOpenGL3_CreateDeviceObjects();

	appendLogLine(state,
		"HarmonyUI ready. Select a case, check Plot only if you want charts, then Run.\n"
		"Plots appear in the Plots tab (same window).\n"
		"Developer CLI unchanged: use Harmony --cpp / --json from a terminal.");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		if (state.worker.valid()) {
			using namespace std::chrono_literals;
			if (state.worker.wait_for(0ms) == std::future_status::ready) {
				state.worker.get();
			}
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		drawMainWindow(state);

		ImGui::Render();
		int fbW = 0;
		int fbH = 0;
		glfwGetFramebufferSize(window, &fbW, &fbH);
		glViewport(0, 0, fbW, fbH);
		glClearColor(0.12f, 0.12f, 0.14f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		visualization_process_pending_save(window);
		glfwSwapBuffers(window);
	}

	visualization_stop();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();

	if (state.worker.valid()) {
		state.worker.wait();
	}

	return EXIT_SUCCESS;
}
