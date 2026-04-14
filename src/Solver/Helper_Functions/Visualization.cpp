#include "Visualization.h"


// ============================================================
// INTERNAL STATE
// ============================================================

namespace {
    struct PlotTab
    {
        std::string title;
        std::function<void()> draw;
    };

    std::vector<PlotTab> g_tabs;
    std::mutex g_mutex;

    std::thread g_guiThread;
    std::atomic<bool> g_running{ false };
    std::atomic<bool> g_initialized{ false };
    std::atomic<bool> g_stop{ false };

    GLFWwindow* g_window = nullptr;
    const char* glsl_version = "#version 130";
}

// ============================================================
// GLFW ERROR
// ============================================================

static void glfw_error_callback(int error, const char* desc)
{
    std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
}

// ============================================================
// INIT GUI
// ============================================================

static void init_gui()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        throw std::runtime_error("GLFW init failed");

#ifdef __APPLE__
    glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    g_window = glfwCreateWindow(1280, 720, "Harmony Visualization", NULL, NULL);
    if (!g_window)
        throw std::runtime_error("Failed to create window");

    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    g_initialized = true;
    g_running = true;
}

// ============================================================
// GUI LOOP (BACKGROUND THREAD)
// ============================================================

static void gui_loop()
{
    init_gui();

    while (!glfwWindowShouldClose(g_window) && !g_stop)
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Harmony Visualization");

        if (ImGui::BeginTabBar("Plots"))
        {
            std::lock_guard<std::mutex> lock(g_mutex);

            for (auto& tab : g_tabs)
            {
                if (ImGui::BeginTabItem(tab.title.c_str()))
                {
                    tab.draw();
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();

        int w, h;
        glfwGetFramebufferSize(g_window, &w, &h);

        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(g_window);
    }

    g_running = false;
}

// ============================================================
// START THREAD (AUTO)
// ============================================================

static void ensure_running()
{
    if (g_initialized)
        return;

    g_guiThread = std::thread(gui_loop);
    g_guiThread.detach();

    while (!g_initialized)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// ============================================================
// REGISTER TAB
// ============================================================

static void add_tab(const std::string& title, std::function<void()> fn)
{
    ensure_running();

    std::lock_guard<std::mutex> lock(g_mutex);
    g_tabs.push_back({ title, fn });
}

// ============================================================
// PUBLIC CONTROL
// ============================================================

void visualization_stop()
{
    g_stop = true;
}

bool visualization_is_running()
{
    return g_running;
}

// ============================================================
// BODE
// ============================================================

void bode_plot_implot(
    const std::vector<double>& freq,
    const std::vector<std::vector<double>>& mag_dB,
    const std::vector<std::vector<double>>& phase_deg,
    const std::vector<std::string>& labels,
    const std::string& title)
{
    add_tab(title, [=]()
        {
            if (freq.empty() || mag_dB.empty() || phase_deg.empty())
                return;

            const int N = (int)freq.size();
            const int nSignals = (int)labels.size();

            ImGui::BeginChild("BodeLayout");

            // ========================================================
            // MAGNITUDE PLOT
            // ========================================================
            if (ImPlot::BeginPlot("Magnitude (dB)", ImVec2(-1, 260)))
            {
                ImPlot::SetupAxes("Frequency (Hz)", "20 log10 |H(jw)|");
                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);

                ImPlot::SetupLegend(ImPlotLocation_SouthWest);

                for (int j = 0; j < nSignals; ++j)
                {
                    std::vector<double> y(N);

                    for (int i = 0; i < N; ++i)
                    {
                        y[i] = mag_dB[i][j];   
                    }

                    ImPlot::PlotLine(
                        labels[j].c_str(),
                        freq.data(),
                        y.data(),
                        N);
                }

                ImPlot::EndPlot();
            }

            ImGui::Dummy(ImVec2(0, 10));

            // ========================================================
            // PHASE PLOT
            // ========================================================
            if (ImPlot::BeginPlot("Phase (deg)", ImVec2(-1, 260)))
            {
                ImPlot::SetupAxes("Frequency (Hz)", "Angle H(jw)");
                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);

                ImPlot::SetupLegend(ImPlotLocation_SouthWest);

                for (int j = 0; j < nSignals; ++j)
                {
                    std::vector<double> y(N);

                    for (int i = 0; i < N; ++i)
                    {
                        y[i] = phase_deg[i][j];  // ✔ CORRECT: same layout
                    }

                    ImPlot::PlotLine(
                        labels[j].c_str(),
                        freq.data(),
                        y.data(),
                        N);
                }

                ImPlot::EndPlot();
            }

            ImGui::EndChild();
        });
}

// ============================================================
// NYQUIST
// ============================================================

void nyquist_plot_implot(
    const std::vector<std::vector<std::complex<double>>>& H_data,
    const std::vector<std::string>& labels,
    const std::string& title)
{
    add_tab(title, [=]()
        {
            if (H_data.empty())
                return;

            const int N = (int)H_data.size();
            const int channels = (int)H_data[0].size();

            ImGui::BeginChild("NyquistLayout");

            if (ImPlot::BeginPlot("Nyquist"))
            {
                // ========================================================
                // AXES (MATLAB STYLE)
                // ========================================================
                ImPlot::SetupAxes("Re{H(jw)}", "Im{H(jw)}");

                // ========================================================
                // COMPUTE AUTO LIMITS (NO FIXED UNIT CIRCLE)
                // ========================================================
                double min_re = 1e100, max_re = -1e100;
                double min_im = 1e100, max_im = -1e100;

                for (const auto& row : H_data)
                {
                    for (const auto& v : row)
                    {
                        double re = std::real(v);
                        double im = std::imag(v);

                        min_re = std::min(min_re, re);
                        max_re = std::max(max_re, re);
                        min_im = std::min(min_im, im);
                        max_im = std::max(max_im, im);
                    }
                }

                // expand slightly (MATLAB-like padding)
                double pad_re = 0.05 * (max_re - min_re);
                double pad_im = 0.05 * (max_im - min_im);

                min_re -= pad_re; max_re += pad_re;
                min_im -= pad_im; max_im += pad_im;

                // equal scaling (critical for Nyquist correctness)
                double re_range = max_re - min_re;
                double im_range = max_im - min_im;
                double range = std::max(re_range, im_range);

                double cx = 0.5 * (min_re + max_re);
                double cy = 0.5 * (min_im + max_im);

                double half = 0.5 * range;

                ImPlot::SetupAxesLimits(
                    cx - half, cx + half,
                    cy - half, cy + half,
                    ImPlotCond_Always
                );

                // ========================================================
                // NYQUIST CURVES
                // ========================================================
                for (int j = 0; j < channels; ++j)
                {
                    std::vector<double> re(N), im(N);

                    for (int i = 0; i < N; ++i)
                    {
                        re[i] = std::real(H_data[i][j]);
                        im[i] = std::imag(H_data[i][j]);
                    }

                    const std::string label =
                        (j < (int)labels.size())
                        ? labels[j]
                        : ("TF_" + std::to_string(j + 1));

                    ImPlot::PlotLine(label.c_str(), re.data(), im.data(), N);
                }

                // ========================================================
                // UNIT CIRCLE (MATPLOTLIB EQUIVALENT)
                // ========================================================
                std::vector<double> theta(500), xc(500), yc(500);

                for (int i = 0; i < 500; ++i)
                {
                    theta[i] = 2.0 * M_PI * (double)i / 499.0;
                    xc[i] = std::cos(theta[i]);
                    yc[i] = std::sin(theta[i]);
                }

                ImPlot::SetNextLineStyle(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), 1.5f);
                ImPlot::PlotLine("Unit Circle", xc.data(), yc.data(), 500);

                ImPlot::EndPlot();
            }

            ImGui::EndChild();
        });
}

// ============================================================
// EIGENVALUES
// ============================================================
void plot_eigenvalues_implot(
    const std::vector<std::complex<double>>& eigvals,
    const std::string& title)
{
    add_tab(title, [=]()
        {
            if (eigvals.empty())
                return;

            std::vector<double> real_part;
            std::vector<double> imag_part;

            real_part.reserve(eigvals.size());
            imag_part.reserve(eigvals.size());

            for (const auto& λ : eigvals)
            {
                real_part.push_back(std::real(λ));
                imag_part.push_back(std::imag(λ));
            }

            ImGui::BeginChild("BodeLayout");

            if (ImPlot::BeginPlot("Eigenvalues (s-plane)",
                ImVec2(-1, 520),
                ImPlotFlags_Equal))   // ✅ correct "axis equal"
            {
                ImPlot::SetupAxes("Re(lambda)", "Im(lambda)");
                ImPlot::SetupLegend(ImPlotLocation_SouthWest);

                ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 8.0f);
                ImPlot::PushStyleColor(ImPlotCol_MarkerFill, ImVec4(0.1f, 0.4f, 0.8f, 1.0f));
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0, 0, 0, 1));

                ImPlot::PlotScatter(
                    "Eigenvalues",
                    real_part.data(),
                    imag_part.data(),
                    (int)real_part.size()
                );

                ImPlot::PopStyleColor(2);
                ImPlot::PopStyleVar();

                // -----------------------------
                // Origin reference lines (correct way)
                // -----------------------------
                double zero = 0.0;
                ImPlot::PlotInfLines("Re = 0", &zero, 1); // vertical line at x=0
                ImPlot::PlotInfLines("Im = 0", &zero, 1); // horizontal line at y=0

                ImPlot::EndPlot();
            }

            ImGui::EndChild();
        });
}

// ============================================================
// PARTICIPATION FACTORS
// ============================================================
void plot_participation_factors_implot(
    const std::vector<std::vector<double>>& P,
    const std::vector<std::string>& state_labels,
    const std::vector<std::string>& mode_labels,
    const std::string& title)
{
    add_tab(title, [=]()
        {
            if (P.empty() || state_labels.empty() || mode_labels.empty())
                return;

            ImGui::BeginChild("BodeLayout");

            const size_t n_states = P.size();
            const size_t n_modes = P[0].size();

            // =========================================================
            // NORMALIZATION (column-wise like your Matplot version)
            // =========================================================
            std::vector<std::vector<double>> P_norm = P;

            for (size_t j = 0; j < n_modes; ++j)
            {
                double col_sum = 0.0;

                for (size_t i = 0; i < n_states; ++i)
                    col_sum += std::abs(P[i][j]);

                if (col_sum > 0)
                {
                    for (size_t i = 0; i < n_states; ++i)
                        P_norm[i][j] /= col_sum;
                }
            }

            // X positions (state index)
            std::vector<double> x_positions(n_states);
            for (size_t i = 0; i < n_states; ++i)
                x_positions[i] = (double)i;

            // =========================================================
            // PLOT
            // =========================================================
            if (ImPlot::BeginPlot("Participation Factors",
                ImVec2(-1, 600)))
            {
                ImPlot::SetupAxes("State Variables", "Normalized Participation Factor");

                // ---- Custom tick labels (state names) ----
                std::vector<const char*> x_labels;
                x_labels.reserve(state_labels.size());
                for (auto& s : state_labels)
                    x_labels.push_back(s.c_str());

                ImPlot::SetupAxisTicks(ImAxis_X1,
                    0,
                    (double)(n_states - 1),
                    (int)n_states,
                    x_labels.data());

                ImPlot::SetupLegend(ImPlotLocation_NorthEast);

                // =====================================================
                // BAR GROUPING STRATEGY
                // We shift each mode slightly in X direction
                // =====================================================
                const double bar_width = 0.8 / (double)n_modes;

                for (size_t j = 0; j < n_modes; ++j)
                {
                    std::vector<double> y(n_states);
                    std::vector<double> x_shifted(n_states);

                    for (size_t i = 0; i < n_states; ++i)
                    {
                        y[i] = P_norm[i][j];

                        // offset each mode inside the group
                        x_shifted[i] = x_positions[i]
                            + (j * bar_width)
                            - (0.4);
                    }

                    ImPlot::PlotBars(
                        mode_labels[j].c_str(),
                        x_shifted.data(),
                        y.data(),
                        (int)n_states,
                        bar_width
                    );
                }

                ImPlot::EndPlot();
            }

            ImGui::EndChild();
        });
}

// ============================================================
// ABC WAVEFORMS
// ============================================================

void plot_abc_waveforms_implot(
    const std::vector<double>& t,
    const Eigen::MatrixXd& Xabc,
    const std::string& title)
{
    add_tab(title, [=]()
        {
            std::vector<double> xa(t.size()), xb(t.size()), xc(t.size());

            for (size_t i = 0; i < t.size(); ++i)
            {
                xa[i] = Xabc(i, 0);
                xb[i] = Xabc(i, 1);
                xc[i] = Xabc(i, 2);
            }

            if (ImPlot::BeginPlot("ABC"))
            {
                ImPlot::PlotLine("xa", t.data(), xa.data(), t.size());
                ImPlot::PlotLine("xb", t.data(), xb.data(), t.size());
                ImPlot::PlotLine("xc", t.data(), xc.data(), t.size());
                ImPlot::EndPlot();
            }
        });
}

// ============================================================
// ABC GROUPS
// ============================================================

void plot_abc_groups_implot(
    const std::vector<double>& t,
    const std::vector<Eigen::MatrixXd>& Xabc_groups,
    const std::string& title)
{
    add_tab(title, [=]()
        {
            for (size_t g = 0; g < Xabc_groups.size(); ++g)
            {
                const auto& X = Xabc_groups[g];

                std::vector<double> xa(t.size()), xb(t.size()), xc(t.size());

                for (size_t i = 0; i < t.size(); ++i)
                {
                    xa[i] = X(i, 0);
                    xb[i] = X(i, 1);
                    xc[i] = X(i, 2);
                }

                ImGui::PushID((int)g);

                if (ImPlot::BeginPlot(("Group " + std::to_string(g + 1)).c_str()))
                {
                    ImPlot::PlotLine("xa", t.data(), xa.data(), t.size());
                    ImPlot::PlotLine("xb", t.data(), xb.data(), t.size());
                    ImPlot::PlotLine("xc", t.data(), xc.data(), t.size());
                    ImPlot::EndPlot();
                }

                ImGui::PopID();
            }
        });
}