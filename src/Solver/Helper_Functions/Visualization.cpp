/**
 * @file Visualization.cpp
 * @brief Implementation of Interactive ImGui/ImPlot visualization for solver results.
 */
﻿#include "Visualization.h"

// stb_image_write — single-header PNG/BMP writer (no external lib required).
// Drop stb_image_write.h into your source tree from https://github.com/nothings/stb
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


// ============================================================
// INTERNAL STATE
// ============================================================

namespace {

    struct PlotTab
    {
        std::string           title;
        std::function<void()> draw;
    };

    std::vector<PlotTab>  g_tabs;
    std::mutex            g_mutex;

    std::thread           g_guiThread;
    std::atomic<bool>     g_running{ false };
    std::atomic<bool>     g_initialized{ false };
    std::atomic<bool>     g_stop{ false };

    GLFWwindow* g_window = nullptr;
    const char* glsl_version = "#version 130";

    struct GuiThreadGuard
    {
        ~GuiThreadGuard()
        {
            g_stop = true;
            if (g_guiThread.joinable())
                g_guiThread.join();
        }
    } g_guard;

    std::once_flag g_start_flag;  // guarantees ensure_running fires exactly once

    // ----------------------------------------------------------
    // SAVE STATE
    // Set g_pending_save to a filename to trigger a PNG capture
    // on the next rendered frame (before buffer swap).
    // ----------------------------------------------------------
    std::string g_pending_save;   // guarded by g_mutex when set externally

    // Sanitise a string for use as a filename (replace problematic chars).
    static std::string sanitise_filename(const std::string& s)
    {
        std::string out = s;
        for (char& c : out)
            if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*')
                c = '_';
        return out;
    }

    // Capture the current OpenGL back-buffer and write to a PNG file.
    // Must be called AFTER ImGui render draw data but BEFORE glfwSwapBuffers.
    static void save_framebuffer(const std::string& path)
    {
        int w = 0, h = 0;
        glfwGetFramebufferSize(g_window, &w, &h);
        if (w == 0 || h == 0) return;

        std::vector<uint8_t> buf(static_cast<size_t>(w * h * 3));
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf.data());

        // OpenGL origin is bottom-left; PNG expects top-left — flip rows.
        const size_t stride = static_cast<size_t>(w * 3);
        std::vector<uint8_t> tmp(stride);
        for (int y = 0; y < h / 2; ++y)
        {
            uint8_t* top = buf.data() + y * stride;
            uint8_t* bot = buf.data() + (h - 1 - y) * stride;
            std::copy(top, top + stride, tmp.data());
            std::copy(bot, bot + stride, top);
            std::copy(tmp.begin(), tmp.end(), bot);
        }

        const std::string safe = sanitise_filename(path);
        if (stbi_write_png(safe.c_str(), w, h, 3, buf.data(),
            static_cast<int>(stride)))
            std::cout << "[viz] Saved: " << safe << '\n';
        else
            std::cerr << "[viz] PNG write failed: " << safe << '\n';
    }

    // ----------------------------------------------------------
    // ORANGES COLORMAP  (shared by viz_opf)
    // ----------------------------------------------------------

    static const std::vector<float> oranges_stops = {
        0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f
    };
    static const std::vector<std::array<float, 3>> oranges_colors = {
        {1.00f,0.96f,0.92f}, {1.00f,0.90f,0.81f},
        {0.99f,0.82f,0.64f}, {0.99f,0.68f,0.42f},
        {0.99f,0.55f,0.23f}, {0.95f,0.41f,0.07f},
        {0.85f,0.28f,0.00f}, {0.65f,0.21f,0.01f},
        {0.50f,0.15f,0.02f}
    };

    static std::array<float, 3> oranges_colormap(float t)
    {
        constexpr float t_min = 0.05f;
        t = t_min + t * (1.0f - t_min);
        if (t <= oranges_stops.front()) return oranges_colors.front();
        if (t >= oranges_stops.back())  return oranges_colors.back();

        auto it = std::upper_bound(oranges_stops.begin(), oranges_stops.end(), t);
        int  idx = static_cast<int>(std::distance(oranges_stops.begin(), it));

        float t0 = oranges_stops[idx - 1], t1 = oranges_stops[idx];
        const auto& c0 = oranges_colors[idx - 1];
        const auto& c1 = oranges_colors[idx];
        float a = (t - t0) / (t1 - t0);

        return { c0[0] + a * (c1[0] - c0[0]),
                 c0[1] + a * (c1[1] - c0[1]),
                 c0[2] + a * (c1[2] - c0[2]) };
    }

    // ----------------------------------------------------------
    // FRUCHTERMAN-REINGOLD spring layout  (shared by viz_opf)
    // Replaces matplot's kawai layout algorithm.
    // ----------------------------------------------------------

    static void spring_layout(
        int                                          N,
        const std::vector<std::pair<size_t, size_t>>& edges,
        std::vector<double>& xs,
        std::vector<double>& ys,
        int                                          iterations = 250)
    {
        xs.assign(N, 0.0);
        ys.assign(N, 0.0);

        for (int i = 0; i < N; ++i)
        {
            xs[i] = std::cos(2.0 * M_PI * i / N);
            ys[i] = std::sin(2.0 * M_PI * i / N);
        }

        const double k = std::sqrt(4.0 / std::max(N, 1));
        double t = 1.0;

        std::vector<double> dx(N), dy(N);

        for (int iter = 0; iter < iterations; ++iter)
        {
            std::fill(dx.begin(), dx.end(), 0.0);
            std::fill(dy.begin(), dy.end(), 0.0);

            for (int u = 0; u < N; ++u)
                for (int v = u + 1; v < N; ++v)
                {
                    double ddx = xs[u] - xs[v];
                    double ddy = ys[u] - ys[v];
                    double dist = std::max(std::hypot(ddx, ddy), 1e-6);
                    double f = k * k / dist;
                    double fx = ddx / dist * f;
                    double fy = ddy / dist * f;
                    dx[u] += fx;  dy[u] += fy;
                    dx[v] -= fx;  dy[v] -= fy;
                }

            for (auto& [u, v] : edges)
            {
                double ddx = xs[u] - xs[v];
                double ddy = ys[u] - ys[v];
                double dist = std::max(std::hypot(ddx, ddy), 1e-6);
                double f = dist * dist / k;
                double fx = ddx / dist * f;
                double fy = ddy / dist * f;
                dx[u] -= fx;  dy[u] -= fy;
                dx[v] += fx;  dy[v] += fy;
            }

            for (int u = 0; u < N; ++u)
            {
                double disp = std::max(std::hypot(dx[u], dy[u]), 1e-6);
                xs[u] += dx[u] / disp * std::min(disp, t);
                ys[u] += dy[u] / disp * std::min(disp, t);
            }
            t *= 0.95;
        }
    }

} // namespace

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
// GUI LOOP  (runs on background thread)
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

        // Full-screen host window
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Harmony Visualization",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        if (ImGui::BeginTabBar("Plots"))
        {
            // Snapshot under lock — draw() runs outside the lock so that
            // callbacks can safely call add_tab() without deadlocking.
            std::vector<PlotTab> snapshot;
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                snapshot = g_tabs;
            }

            for (auto& tab : snapshot)
            {
                if (ImGui::BeginTabItem(tab.title.c_str()))
                {
                    // ---- Save button — right-aligned in the tab's toolbar ----
                    {
                        constexpr float BTN_W = 95.0f;
                        constexpr float BTN_H = 18.0f;
                        const float     avail = ImGui::GetContentRegionAvail().x;
                        const float     cursor = ImGui::GetCursorPosX();

                        ImGui::SameLine(cursor + avail - BTN_W);
                        ImGui::PushStyleColor(ImGuiCol_Button,
                            ImVec4(0.18f, 0.42f, 0.18f, 1.f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.25f, 0.58f, 0.25f, 1.f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                            ImVec4(0.12f, 0.30f, 0.12f, 1.f));

                        if (ImGui::Button((const char*)u8"💾 Save PNG", ImVec2(BTN_W, BTN_H)))
                        {
                            // Schedule capture — executed after this frame's
                            // render call but before SwapBuffers.
                            std::lock_guard<std::mutex> lk(g_mutex);
                            g_pending_save = tab.title + ".png";
                        }
                        ImGui::PopStyleColor(3);
                    }

                    ImGui::Separator();
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

        // Capture the back-buffer BEFORE the swap so glReadPixels
        // sees the fully-rendered frame.
        {
            std::string pending;
            {
                std::lock_guard<std::mutex> lk(g_mutex);
                std::swap(pending, g_pending_save);
            }
            if (!pending.empty())
                save_framebuffer(pending);
        }

        glfwSwapBuffers(g_window);
    }

    // ---- cleanup ----
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(g_window);
    glfwTerminate();

    g_running = false;
}

// ============================================================
// ENSURE RUNNING  (called automatically by every plot function)
// ============================================================

static void ensure_running()
{
    std::call_once(g_start_flag, []()
        {
            g_guiThread = std::thread(gui_loop);
            while (!g_initialized)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
}

// ============================================================
// INTERNAL TAB REGISTRATION
// ============================================================

static void add_tab(const std::string& title, std::function<void()> fn)
{
    ensure_running();
    std::lock_guard<std::mutex> lock(g_mutex);
    g_tabs.push_back({ title, fn });
}

// ============================================================
// PUBLIC API
// ============================================================

void visualization_stop()
{
    g_stop = true;
}

bool visualization_is_running()
{
    return g_running;
}

/// Block until the window is closed by the user.
void visualization_wait()
{
    if (g_guiThread.joinable())
        g_guiThread.join();
}

/// Schedule a PNG capture of the named tab on the next rendered frame.
/// The file is written as "<tab_title>.png" in the working directory.
/// Thread-safe: can be called from any thread.
void visualization_save_tab(const std::string& tab_title)
{
    std::lock_guard<std::mutex> lk(g_mutex);
    g_pending_save = tab_title + ".png";
}

/// Public interface used by external modules (e.g. viz_opf).
void add_plot_tab(const std::string& title, std::function<void()> fn)
{
    add_tab(title, std::move(fn));
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

            // ---- Magnitude ----
            if (ImPlot::BeginPlot("Magnitude (dB)", ImVec2(-1, 260)))
            {
                ImPlot::SetupAxes("Frequency (Hz)", "20 log10 |H(jw)|");
                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
                ImPlot::SetupLegend(ImPlotLocation_SouthWest);

                for (int j = 0; j < nSignals; ++j)
                {
                    std::vector<double> y(N);
                    for (int i = 0; i < N; ++i)
                        y[i] = mag_dB[i][j];

                    ImPlot::PlotLine(labels[j].c_str(), freq.data(), y.data(), N);
                }

                ImPlot::EndPlot();
            }

            ImGui::Dummy(ImVec2(0, 10));

            // ---- Phase ----
            if (ImPlot::BeginPlot("Phase (deg)", ImVec2(-1, 260)))
            {
                ImPlot::SetupAxes("Frequency (Hz)", "Angle H(jw)");
                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
                ImPlot::SetupLegend(ImPlotLocation_SouthWest);

                for (int j = 0; j < nSignals; ++j)
                {
                    std::vector<double> y(N);
                    for (int i = 0; i < N; ++i)
                        y[i] = phase_deg[i][j];

                    ImPlot::PlotLine(labels[j].c_str(), freq.data(), y.data(), N);
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
                ImPlot::SetupAxes("Re{H(jw)}", "Im{H(jw)}");

                // Auto-fit limits with equal scaling
                double min_re = 1e100, max_re = -1e100;
                double min_im = 1e100, max_im = -1e100;

                for (const auto& row : H_data)
                    for (const auto& v : row)
                    {
                        min_re = std::min(min_re, std::real(v));
                        max_re = std::max(max_re, std::real(v));
                        min_im = std::min(min_im, std::imag(v));
                        max_im = std::max(max_im, std::imag(v));
                    }

                double pad_re = 0.05 * (max_re - min_re);
                double pad_im = 0.05 * (max_im - min_im);
                min_re -= pad_re; max_re += pad_re;
                min_im -= pad_im; max_im += pad_im;

                double range = std::max(max_re - min_re, max_im - min_im);
                double cx = 0.5 * (min_re + max_re);
                double cy = 0.5 * (min_im + max_im);
                double half = 0.5 * range;

                ImPlot::SetupAxesLimits(cx - half, cx + half,
                    cy - half, cy + half,
                    ImPlotCond_Once);

                // Curves
                for (int j = 0; j < channels; ++j)
                {
                    std::vector<double> re(N), im(N);
                    for (int i = 0; i < N; ++i)
                    {
                        re[i] = std::real(H_data[i][j]);
                        im[i] = std::imag(H_data[i][j]);
                    }

                    const std::string lbl = (j < (int)labels.size())
                        ? labels[j]
                        : ("TF_" + std::to_string(j + 1));

                    ImPlot::PlotLine(lbl.c_str(), re.data(), im.data(), N);
                }

                // Unit circle
                std::vector<double> theta(500), xc(500), yc(500);
                for (int i = 0; i < 500; ++i)
                {
                    theta[i] = 2.0 * M_PI * (double)i / 499.0;
                    xc[i] = std::cos(theta[i]);
                    yc[i] = std::sin(theta[i]);
                }

                ImPlotSpec spec;
                spec.LineColor = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
                spec.LineWeight = 1.5f;
                ImPlot::PlotLine("Unit Circle", xc.data(), yc.data(), 500, spec);

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

            std::vector<double> re, im;
            re.reserve(eigvals.size());
            im.reserve(eigvals.size());

            for (const auto& l : eigvals)
            {
                re.push_back(std::real(l));
                im.push_back(std::imag(l));
            }

            ImGui::BeginChild("EigLayout");

            if (ImPlot::BeginPlot("Eigenvalues (s-plane)",
                ImVec2(-1, 520),
                ImPlotFlags_Equal))
            {
                ImPlot::SetupAxes("Re(lambda)", "Im(lambda)");
                ImPlot::SetupLegend(ImPlotLocation_SouthWest);

                ImPlotSpec scatterSpec; // style for scatter plot
                scatterSpec.Marker = ImPlotMarker_Circle;
                scatterSpec.MarkerSize = 8.0f;
                scatterSpec.MarkerFillColor = ImVec4(0.1f, 0.4f, 0.8f, 1.0f);
                scatterSpec.MarkerLineColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
      
                ImPlot::PlotScatter("Eigenvalues",
                    re.data(), im.data(),
                    (int)re.size(), scatterSpec);

                ImPlotSpec lineSpec; // style for reference axes
                lineSpec.LineColor = ImVec4(0.5f, 0.5f, 0.5f, 0.6f);
                lineSpec.LineWeight = 1.0f;
                
                // Reference axes — "##" prefix hides them from the legend
                double zero = 0.0;
                ImPlot::PlotInfLines("##re0", &zero, 1, lineSpec);
                lineSpec.Flags = ImPlotInfLinesFlags_Horizontal;
                ImPlot::PlotInfLines("##im0", &zero, 1, lineSpec);

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

            const int n_states = static_cast<int>(P.size());
            const int n_modes = static_cast<int>(P[0].size());

            // ----------------------------------------------------------
            // Column-wise normalisation (each mode sums to 1)
            // ----------------------------------------------------------
            std::vector<std::vector<double>> P_norm = P;
            for (int j = 0; j < n_modes; ++j)
            {
                double col_sum = 0.0;
                for (int i = 0; i < n_states; ++i)
                    col_sum += std::abs(P[i][j]);
                if (col_sum > 1e-12)
                    for (int i = 0; i < n_states; ++i)
                        P_norm[i][j] /= col_sum;
            }

            // ----------------------------------------------------------
            // Distinct 10-color palette (tab10-inspired)
            // ----------------------------------------------------------
            static const ImVec4 palette[10] = {
                {0.122f, 0.467f, 0.706f, 1.f},  // blue
                {1.000f, 0.498f, 0.055f, 1.f},  // orange
                {0.173f, 0.627f, 0.173f, 1.f},  // green
                {0.839f, 0.153f, 0.157f, 1.f},  // red
                {0.580f, 0.404f, 0.741f, 1.f},  // purple
                {0.549f, 0.337f, 0.294f, 1.f},  // brown
                {0.890f, 0.467f, 0.761f, 1.f},  // pink
                {0.498f, 0.498f, 0.498f, 1.f},  // grey
                {0.737f, 0.741f, 0.133f, 1.f},  // olive
                {0.090f, 0.745f, 0.812f, 1.f},  // cyan
            };

            constexpr double LABEL_THRESHOLD = 0.05;  // annotate bars >= 5 %

            // ----------------------------------------------------------
            // Plot
            // ----------------------------------------------------------
            ImGui::BeginChild("##pf_child", ImVec2(-1, -1), false);

            const double bar_width = 0.8 / n_modes;

            if (ImPlot::BeginPlot("##pf",
                ImVec2(-1, -1),
                ImPlotFlags_NoMouseText))
            {
                ImPlot::SetupAxes("State Variables",
                    "Normalised Participation Factor",
                    ImPlotAxisFlags_None,
                    ImPlotAxisFlags_None);

                // Y-axis: fixed 0–1.05, grid on
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.05, ImPlotCond_Always);

                // Custom tick labels on X
                std::vector<double>      x_ticks(n_states);
                std::vector<const char*> x_lbls(n_states);
                for (int i = 0; i < n_states; ++i)
                {
                    x_ticks[i] = static_cast<double>(i);
                    x_lbls[i] = state_labels[i].c_str();
                }
                ImPlot::SetupAxisTicks(ImAxis_X1,
                    x_ticks.data(), n_states,
                    x_lbls.data());

                // X limits: half bar-group margin on each side
                ImPlot::SetupAxisLimits(ImAxis_X1,
                    -0.6, n_states - 1 + 0.6,
                    ImPlotCond_Once);

                ImPlot::SetupLegend(ImPlotLocation_NorthEast,
                    ImPlotLegendFlags_Outside);

                // ---- draw bars ----
                for (int j = 0; j < n_modes; ++j)
                {
                    // Centered grouping: shift so bars are symmetric around tick
                    // offset = (j + 0.5) * bar_width - 0.4
                    std::vector<double> x_pos(n_states), y_val(n_states);
                    for (int i = 0; i < n_states; ++i)
                    {
                        x_pos[i] = i + (j + 0.5) * bar_width - 0.4;
                        y_val[i] = P_norm[i][j];
                    }

                    const ImVec4& col = palette[j % 10];
                    ImPlotSpec barSpec;
                    barSpec.FillColor = ImVec4(col.x, col.y, col.z, 0.85f);
                    barSpec.LineColor = ImVec4(col.x, col.y, col.z, 0.85f);
                    
                    ImPlot::PlotBars(mode_labels[j].c_str(),
                        x_pos.data(), y_val.data(),
                        n_states, bar_width * 0.92, barSpec); // slight gap between bars
                }

                // ---- value annotations above significant bars ----
                for (int j = 0; j < n_modes; ++j)
                {
                    for (int i = 0; i < n_states; ++i)
                    {
                        double v = P_norm[i][j];
                        if (v < LABEL_THRESHOLD) continue;

                        double xp = i + (j + 0.5) * bar_width - 0.4;
                        char buf[8];
                        snprintf(buf, sizeof(buf), "%.2f", v);
                        ImPlot::PlotText(buf, xp, v + 0.015, ImVec2(0, 0));
                    }
                }

                // ---- hover tooltip ----
                if (ImPlot::IsPlotHovered())
                {
                    ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                    int nearest_state = static_cast<int>(std::round(mouse.x));
                    if (nearest_state >= 0 && nearest_state < n_states)
                    {
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(state_labels[nearest_state].c_str());
                        ImGui::Separator();
                        for (int j = 0; j < n_modes; ++j)
                        {
                            const ImVec4& c = palette[j % 10];
                            ImGui::ColorButton("##c", c,
                                ImGuiColorEditFlags_NoTooltip |
                                ImGuiColorEditFlags_NoBorder,
                                ImVec2(10, 10));
                            ImGui::SameLine();
                            ImGui::Text("%s: %.3f", mode_labels[j].c_str(),
                                P_norm[nearest_state][j]);
                        }
                        ImGui::EndTooltip();
                    }
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

            if (ImPlot::BeginPlot(("ABC")))
            {
                ImPlot::SetupAxes("t (s)", "Amplitude");
                ImPlot::PlotLine("xa", t.data(), xa.data(), (int)t.size());
                ImPlot::PlotLine("xb", t.data(), xb.data(), (int)t.size());
                ImPlot::PlotLine("xc", t.data(), xc.data(), (int)t.size());
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
            // Each group gets a fixed slice of height; wrap in a scrollable child
            // so the tab remains usable regardless of group count.
            constexpr float PLOT_H = 220.0f;

            ImGui::BeginChild("##abc_groups_scroll", ImVec2(-1, -1), false,
                ImGuiWindowFlags_HorizontalScrollbar);

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

                ImGui::PushID(static_cast<int>(g));
                if (ImPlot::BeginPlot(("Group " + std::to_string(g + 1)).c_str(),
                    ImVec2(-1, PLOT_H)))
                {
                    ImPlot::SetupAxes("t (s)", "Amplitude");
                    ImPlot::PlotLine("xa", t.data(), xa.data(), static_cast<int>(t.size()));
                    ImPlot::PlotLine("xb", t.data(), xb.data(), static_cast<int>(t.size()));
                    ImPlot::PlotLine("xc", t.data(), xc.data(), static_cast<int>(t.size()));
                    ImPlot::EndPlot();
                }
                ImGui::PopID();
            }

            ImGui::EndChild();
        });
}

// ============================================================
// VIZ OPF — AC/DC optimal power flow network visualisation
// ============================================================

void viz_opf(const OPFVisualData& d)
{
    // ----------------------------------------------------------
    // 1.  REINDEX AC DATA
    // ----------------------------------------------------------

    const int numBuses_ac = d.bus_entire_ac.rows();
    const int numColsBuses = d.bus_entire_ac.cols();
    const int numColsBranches_ac = d.branch_entire_ac.cols();
    const int numBranches_ac = d.branch_entire_ac.rows();

    Eigen::MatrixXd bus_ac_new = d.bus_entire_ac;
    for (int i = 0; i < numBuses_ac; ++i) {
        bus_ac_new(i, 0) = i + 1;
    }

    std::map<std::pair<int, int>, int> mapping_ac;
    for (int i = 0; i < numBuses_ac; ++i)
    {
        int oldId = static_cast<int>(d.bus_entire_ac(i, 0));
        int area = static_cast<int>(d.bus_entire_ac(i, numColsBuses - 1));
        mapping_ac[{oldId, area}] = i + 1;
    }

    Eigen::MatrixXd branch_ac_new = d.branch_entire_ac;
    for (int i = 0; i < numBranches_ac; ++i)
    {
        int area = static_cast<int>(d.branch_entire_ac(i, numColsBranches_ac - 1));
        auto itF = mapping_ac.find({ (int)d.branch_entire_ac(i, 0), area });
        auto itT = mapping_ac.find({ (int)d.branch_entire_ac(i, 1), area });
        if (itF != mapping_ac.end()) branch_ac_new(i, 0) = itF->second;
        if (itT != mapping_ac.end()) branch_ac_new(i, 1) = itT->second;
    }

    // ----------------------------------------------------------
    // 2.  REINDEX DC DATA
    // ----------------------------------------------------------

    const int numBuses_dc = d.bus_dc.rows();
    const int numBranches_dc = d.branch_dc.rows();
    const int numConvs = d.conv_dc.rows();

    Eigen::MatrixXd bus_dc_new = d.bus_dc;
    bus_dc_new.col(0).array() += numBuses_ac;

    Eigen::MatrixXd branch_dc_new = d.branch_dc;
    branch_dc_new.leftCols<2>().array() += numBuses_ac;

    Eigen::MatrixXd conv_dc_new = d.conv_dc;
    conv_dc_new.col(0).array() += numBuses_ac;

    std::map<std::pair<int, int>, int> mapping_conv;
    for (int i = 0; i < numBuses_ac; ++i)
    {
        int old_ac = static_cast<int>(d.bus_entire_ac(i, 0));
        int new_ac = static_cast<int>(bus_ac_new(i, 0));
        int area = static_cast<int>(d.bus_entire_ac(i, numColsBuses - 1));
        mapping_conv[{old_ac, area}] = new_ac;
    }
    for (int i = 0; i < numConvs; ++i)
    {
        int old_ac = static_cast<int>(d.conv_dc(i, 1));
        int area = static_cast<int>(d.conv_dc(i, 2));
        auto it = mapping_conv.find({ old_ac, area });
        if (it != mapping_conv.end())
            conv_dc_new(i, 1) = it->second;
    }

    // ----------------------------------------------------------
    // 3.  REINDEX GENERATOR DATA + INJECT SOLUTION VALUES
    // ----------------------------------------------------------

    Eigen::MatrixXd gen_ac_new = d.gen_entire_ac;
    const int numGens = d.gen_entire_ac.rows();
    const int numGenCols = d.gen_entire_ac.cols();

    for (int i = 0; i < numGens; ++i)
    {
        int old_bus = static_cast<int>(d.gen_entire_ac(i, 0));
        int area = static_cast<int>(d.gen_entire_ac(i, numGenCols - 1));
        auto it = mapping_conv.find({ old_bus, area });
        if (it != mapping_conv.end())
            gen_ac_new(i, 0) = it->second;
    }

    int idxGen = 0;
    for (int ng = 0; ng < d.ngrids; ++ng)
        for (int j = 0; j < d.ngens_ac[ng]; ++j)
        {
            gen_ac_new(idxGen, 1) = d.pgen_ac_k[ng](j) * d.baseMVA_ac;
            gen_ac_new(idxGen, 2) = d.qgen_ac_k[ng](j) * d.baseMVA_ac;
            ++idxGen;
        }

    // ----------------------------------------------------------
    // 4.  BUILD EDGE LIST
    // ----------------------------------------------------------

    const int nFrom = numBranches_ac + numBranches_dc + numConvs;

    Eigen::VectorXi fromNode(nFrom), toNode(nFrom);
    fromNode.head(numBranches_ac) = branch_ac_new.col(0).cast<int>();
    fromNode.segment(numBranches_ac, numBranches_dc) = branch_dc_new.col(0).cast<int>();
    fromNode.tail(numConvs) = conv_dc_new.col(0).cast<int>();

    toNode.head(numBranches_ac) = branch_ac_new.col(1).cast<int>();
    toNode.segment(numBranches_ac, numBranches_dc) = branch_dc_new.col(1).cast<int>();
    toNode.tail(numConvs) = conv_dc_new.col(1).cast<int>();

    std::vector<int> allNodes;
    allNodes.reserve(2 * nFrom);
    for (int k = 0; k < nFrom; ++k)
    {
        allNodes.push_back(fromNode[k]);
        allNodes.push_back(toNode[k]);
    }
    std::sort(allNodes.begin(), allNodes.end());
    allNodes.erase(std::unique(allNodes.begin(), allNodes.end()), allNodes.end());

    std::vector<std::pair<size_t, size_t>> edges;
    edges.reserve(nFrom);
    for (int k = 0; k < nFrom; ++k)
    {
        size_t u = static_cast<size_t>(fromNode[k] - 1);
        size_t v = static_cast<size_t>(toNode[k] - 1);
        if (u > v) std::swap(u, v);
        edges.emplace_back(u, v);
    }
    std::sort(edges.begin(), edges.end(),
        [](auto& a, auto& b) {
            return a.first != b.first ? a.first < b.first
                : a.second < b.second;
        });

    // ----------------------------------------------------------
    // 5.  SPRING LAYOUT
    // ----------------------------------------------------------

    const int N = static_cast<int>(allNodes.size());
    std::vector<double> xs, ys;
    spring_layout(N, edges, xs, ys);

    // ----------------------------------------------------------
    // 6.  NODE CLASSIFICATION
    // ----------------------------------------------------------

    Eigen::VectorXi acNodesVec = bus_ac_new.col(0).cast<int>();
    Eigen::VectorXi dcNodesVec = bus_dc_new.col(0).cast<int>();

    std::vector<size_t> idx_dc, idx_ac;
    idx_dc.reserve(numBuses_dc);
    for (int i = 0; i < dcNodesVec.size(); ++i) {
        idx_dc.push_back(static_cast<size_t>(dcNodesVec(i) - 1));
    }

    for (size_t i = 0; i < static_cast<size_t>(N); ++i) {
        if (std::find(idx_dc.begin(), idx_dc.end(), i) == idx_dc.end()) {
            idx_ac.push_back(i);
        }
    }

    // ----------------------------------------------------------
    // 7.  PER-NODE POWER MAGNITUDES
    // ----------------------------------------------------------

    Eigen::VectorXd loadPower = Eigen::VectorXd::Zero(N);
    {
        Eigen::ArrayXd lmag = (bus_ac_new.col(2).array().square()
            + bus_ac_new.col(3).array().square()).sqrt();
        for (size_t k = 0; k < idx_ac.size(); ++k)
            loadPower(idx_ac[k]) = lmag(idx_ac[k]);
    }

    Eigen::VectorXd genPower = Eigen::VectorXd::Zero(N);
    {
        Eigen::ArrayXd gmag = (gen_ac_new.col(1).array().square()
            + gen_ac_new.col(2).array().square()).sqrt();
        for (int i = 0; i < numGens; ++i)
        {
            size_t idx = static_cast<size_t>(gen_ac_new(i, 0) - 1);
            genPower(idx) = gmag(i);
        }
    }

    // ----------------------------------------------------------
    // 8.  VOLTAGE LABELS
    // ----------------------------------------------------------

    std::vector<int> orig_idx_ac(numBuses_ac), orig_idx_dc(numBuses_dc);
    for (int i = 0; i < numBuses_ac; ++i)
        orig_idx_ac[i] = static_cast<int>(d.bus_entire_ac(i, 0));
    for (int i = 0; i < numBuses_dc; ++i)
        orig_idx_dc[i] = static_cast<int>(d.bus_dc(i, 0));

    std::unordered_map<int, int> acNodeToRow, dcNodeToRow;
    for (int i = 0; i < acNodesVec.size(); ++i) acNodeToRow[acNodesVec(i)] = i;
    for (int i = 0; i < dcNodesVec.size(); ++i) dcNodeToRow[dcNodesVec(i)] = i;

    Eigen::VectorXd voltMag_ac = Eigen::VectorXd::Zero(N);
    {
        int row = 0;
        for (int ng = 0; ng < d.ngrids; ++ng)
            for (int i = 0; i < d.nbuses_ac[ng]; ++i)
                voltMag_ac(row++) = std::sqrt(d.vn2_ac_k[ng](i));
    }

    Eigen::VectorXd voltMag_dc = Eigen::VectorXd::Zero(N);
    for (int i = 0; i < numBuses_dc; ++i)
    {
        size_t gi = static_cast<size_t>(dcNodesVec(i) - 1);
        if (gi < static_cast<size_t>(N))
            voltMag_dc(gi) = std::sqrt(d.vn2_dc_k(i));
    }

    std::vector<double> voltLabel(N, 0.0);
    for (int i = 0; i < N; ++i)
    {
        int node = allNodes[i];
        if (acNodeToRow.count(node))
            voltLabel[i] = voltMag_ac(acNodeToRow[node]);
        else if (dcNodeToRow.count(node))
            voltLabel[i] = voltMag_dc(dcNodeToRow[node]);
    }

    // ----------------------------------------------------------
    // 9.  PER-EDGE POWER
    // ----------------------------------------------------------

    Eigen::Array<bool, Eigen::Dynamic, 1> isAcNode(N), isDcNode(N);
    isAcNode.setConstant(false);
    isDcNode.setConstant(false);
    for (int i = 0; i < acNodesVec.size(); ++i) isAcNode(acNodesVec(i) - 1) = true;
    for (int i = 0; i < dcNodesVec.size(); ++i) isDcNode(dcNodesVec(i) - 1) = true;

    std::vector<double> edgePower(edges.size(), 0.0);

    for (size_t k = 0; k < edges.size(); ++k)
    {
        auto [u, v] = edges[k];
        int bu = (int)u + 1, bv = (int)v + 1;

        if (isAcNode(u) && isAcNode(v))
        {
            for (int row = 0; row < branch_ac_new.rows(); ++row)
            {
                int fi = (int)branch_ac_new(row, 0);
                int ti = (int)branch_ac_new(row, 1);
                if (!((fi == bu && ti == bv) || (fi == bv && ti == bu))) continue;

                int i = (int)d.branch_entire_ac(row, 0);
                int j = (int)d.branch_entire_ac(row, 1);
                int ng = (int)d.branch_entire_ac(row, d.branch_entire_ac.cols() - 1);
                double P = d.pij_ac_k[ng - 1](i - 1, j - 1);
                double Q = d.qij_ac_k[ng - 1](i - 1, j - 1);
                edgePower[k] = std::hypot(P, Q) * d.baseMVA_ac;
                break;
            }
        }
        else if (isDcNode(u) && isDcNode(v))
        {
            for (int row = 0; row < branch_dc_new.rows(); ++row)
            {
                int fi = (int)branch_dc_new(row, 0);
                int ti = (int)branch_dc_new(row, 1);
                if (!((fi == bu && ti == bv) || (fi == bv && ti == bu))) continue;

                int f = (int)d.branch_dc(row, 0);
                int h = (int)d.branch_dc(row, 1);
                edgePower[k] = std::abs(d.pij_dc_k(f - 1, h - 1)
                    * d.baseMW_dc * d.pol_dc);
                break;
            }
        }
        else
        {
            for (int i = 0; i < d.nconvs_dc; ++i)
            {
                int f = (int)conv_dc_new(i, 0);
                int t = (int)conv_dc_new(i, 1);
                if (!((f == bu && t == bv) || (f == bv && t == bu))) continue;

                edgePower[k] = std::hypot(d.ps_dc_k[i] * d.baseMW_dc,
                    d.qs_dc_k[i] * d.baseMW_dc);
                break;
            }
        }
    }

    const double minPower = *std::min_element(edgePower.begin(), edgePower.end());
    const double maxPower = *std::max_element(edgePower.begin(), edgePower.end());

    // Convert Eigen vectors to std::vector for clean lambda capture
    std::vector<double> loadPow_v(loadPower.data(), loadPower.data() + N);
    std::vector<double> genPow_v(genPower.data(), genPower.data() + N);

    // ----------------------------------------------------------
    // 10.  REGISTER DRAW TAB  (lambda runs every frame)
    // ----------------------------------------------------------

    add_tab("AC/DC OPF", [=]()
        {
            constexpr float CB_W = 65.0f;
            constexpr float LG_W = 170.0f;
            const     float height = ImGui::GetContentRegionAvail().y - 4.0f;
            const     float netW = ImGui::GetContentRegionAvail().x
                - CB_W - LG_W - 18.0f;

            // ---- A) COLORBAR ----
            ImGui::BeginChild("##opf_cb", ImVec2(CB_W, height), false);
            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2      pos = ImGui::GetCursorScreenPos();

                const float STRIP_W = 22.0f;
                const float STRIP_H = height - 55.0f;
                constexpr int NSEG = 128;

                for (int i = 0; i < NSEG; ++i)
                {
                    float t = 1.0f - static_cast<float>(i) / (NSEG - 1);
                    auto  rgb = oranges_colormap(t);
                    ImU32 col = IM_COL32((int)(rgb[0] * 255),
                        (int)(rgb[1] * 255),
                        (int)(rgb[2] * 255), 255);
                    float y0 = pos.y + i * (STRIP_H / NSEG);
                    float y1 = pos.y + (i + 1) * (STRIP_H / NSEG);
                    dl->AddRectFilled({ pos.x + 8, y0 },
                        { pos.x + 8 + STRIP_W, y1 }, col);
                }
                dl->AddRect({ pos.x + 8,          pos.y },
                    { pos.x + 8 + STRIP_W, pos.y + STRIP_H },
                    IM_COL32(100, 100, 100, 255));

                char lbl[32];
                snprintf(lbl, 32, "%.1f", maxPower);
                ImGui::SetCursorScreenPos({ pos.x + 32, pos.y });
                ImGui::TextUnformatted(lbl);

                snprintf(lbl, 32, "%.1f", (maxPower + minPower) * 0.5);
                ImGui::SetCursorScreenPos({ pos.x + 32, pos.y + STRIP_H * 0.5f - 7 });
                ImGui::TextUnformatted(lbl);

                snprintf(lbl, 32, "%.1f", minPower);
                ImGui::SetCursorScreenPos({ pos.x + 32, pos.y + STRIP_H - 15 });
                ImGui::TextUnformatted(lbl);

                ImGui::SetCursorScreenPos({ pos.x + 4, pos.y + STRIP_H + 6 });
                ImGui::TextUnformatted("Branch");
                ImGui::SetCursorScreenPos({ pos.x + 4, pos.y + STRIP_H + 22 });
                ImGui::TextUnformatted("(MW)");
            }
            ImGui::EndChild();

            ImGui::SameLine(0, 4);

            // ---- B) NETWORK PLOT ----
            ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(6, 6));
            if (ImPlot::BeginPlot("##opf_net",
                ImVec2(netW, height),
                ImPlotFlags_Equal |
                ImPlotFlags_NoLegend |
                ImPlotFlags_NoMenus |
                ImPlotFlags_NoTitle))
            {
                ImPlot::SetupAxes(nullptr, nullptr,
                    ImPlotAxisFlags_NoDecorations,
                    ImPlotAxisFlags_NoDecorations);

                // Edges
                for (size_t k = 0; k < edges.size(); ++k)
                {
                    auto [u, v] = edges[k];
                    float t_n = static_cast<float>(
                        (edgePower[k] - minPower) / (maxPower - minPower + 1e-6));
                    auto rgb = oranges_colormap(t_n);
                    ImPlotSpec edgeSpec;
                    edgeSpec.LineColor = ImVec4(rgb[0], rgb[1], rgb[2], 1.0f);
                    edgeSpec.LineWeight = 2.0f;
                    double ex[2] = { xs[u], xs[v] };
                    double ey[2] = { ys[u], ys[v] };
                    ImPlot::PlotLine("##e", ex, ey, 2, edgeSpec);
                }

                // AC nodes
                for (size_t idx : idx_ac)
                {
                    double px = xs[idx], py = ys[idx];

                    auto sz = [](double mw) -> float {
                        return static_cast<float>(
                            std::min(std::max(4.0 + mw * 0.012, 4.0), 18.0));
                        };

                    float lsz = sz(loadPow_v[idx]);
                    float gsz = sz(genPow_v[idx]);
                    bool  hasGen = genPow_v[idx] > 0.0;

                    if (hasGen && gsz >= lsz)
                    {
                        ImPlotSpec genSpec;
                        genSpec.Marker = ImPlotMarker_Circle;
                        genSpec.MarkerSize = gsz;
                        genSpec.MarkerFillColor = ImVec4(0.01f, 0.5f, 0.5f, 1.f);
                        genSpec.MarkerLineColor = ImVec4(0.01f, 0.5f, 0.5f, 1.f);
                        ImPlot::PlotScatter("##gen", &px, &py, 1, genSpec);

                        ImPlotSpec loadSpec;
                        loadSpec.Marker = ImPlotMarker_Circle;
                        loadSpec.MarkerSize = lsz;
                        loadSpec.MarkerFillColor = ImVec4(0.9f, 0.01f, 0.01f, 1.f);
                        loadSpec.MarkerLineColor = ImVec4(0.9f, 0.01f, 0.01f, 1.f);
                        ImPlot::PlotScatter("##load", &px, &py, 1, loadSpec);
                    }
                    else
                    {
                        ImPlotSpec loadSpec;
                        loadSpec.Marker = ImPlotMarker_Circle;
                        loadSpec.MarkerSize = lsz;
                        loadSpec.MarkerFillColor = ImVec4(0.9f, 0.01f, 0.01f, 1.f);
                        loadSpec.MarkerLineColor = ImVec4(0.9f, 0.01f, 0.01f, 1.f);
                        ImPlot::PlotScatter("##load", &px, &py, 1, loadSpec);

                        if (hasGen)
                        {
                            ImPlotSpec genSpec;
                            genSpec.Marker = ImPlotMarker_Circle;
                            genSpec.MarkerSize = gsz;
                            genSpec.MarkerFillColor = ImVec4(0.01f, 0.5f, 0.5f, 1.f);
                            genSpec.MarkerLineColor = ImVec4(0.01f, 0.5f, 0.5f, 1.f);
                            ImPlot::PlotScatter("##gen", &px, &py, 1, genSpec);
                        }
                    }
                }

                // DC nodes
                for (size_t idx : idx_dc)
                {
                    double px = xs[idx], py = ys[idx];
                    ImPlotSpec dcSpec;
                    dcSpec.Marker = ImPlotMarker_Up;
                    dcSpec.MarkerSize = 10.0f;
                    dcSpec.MarkerFillColor = ImVec4(0.1f, 0.1f, 0.8f, 1.f);
                    dcSpec.MarkerLineColor = ImVec4(0.1f, 0.1f, 0.8f, 1.f);
                    ImPlot::PlotScatter("##dc", &px, &py, 1, dcSpec);
                }

                // Labels
                for (int i = 0; i < N; ++i)
                {
                    int node = allNodes[i];

                    std::string numLbl;
                    if (acNodeToRow.count(node))
                        numLbl = "#" + std::to_string(orig_idx_ac[acNodeToRow.at(node)]);
                    else if (dcNodeToRow.count(node))
                        numLbl = "#" + std::to_string(orig_idx_dc[dcNodeToRow.at(node)]);

                    ImPlot::PlotText(numLbl.c_str(), xs[i], ys[i], ImVec2(0, -14));

                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(3) << voltLabel[i] << "pu";
                    ImPlot::PlotText(oss.str().c_str(), xs[i], ys[i], ImVec2(0, 10));
                }

                ImPlot::EndPlot();
            }
            ImPlot::PopStyleVar();

            ImGui::SameLine(0, 4);

            // ---- C) LEGEND ----
            ImGui::BeginChild("##opf_lg", ImVec2(LG_W, height), true);
            {
                ImGui::Spacing();
                ImGui::TextUnformatted("  Legend");
                ImGui::Separator();
                ImGui::Spacing();

                ImDrawList* dl = ImGui::GetWindowDrawList();

                auto circle_item = [&](ImVec4 col, const char* label)
                    {
                        ImVec2 p = ImGui::GetCursorScreenPos();
                        dl->AddCircleFilled({ p.x + 10, p.y + 9 }, 8,
                            IM_COL32((int)(col.x * 255), (int)(col.y * 255),
                                (int)(col.z * 255), 255));
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 24);
                        ImGui::TextUnformatted(label);
                    };

                {
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    auto mid = oranges_colormap(0.6f);
                    dl->AddLine({ p.x + 2,p.y + 9 }, { p.x + 22,p.y + 9 },
                        IM_COL32((int)(mid[0] * 255), (int)(mid[1] * 255),
                            (int)(mid[2] * 255), 255), 3.0f);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 26);
                    ImGui::TextUnformatted("Branch Lines");
                }
                ImGui::Spacing();
                circle_item({ 0.9f,0.01f,0.01f,1.f }, "AC Loads");
                ImGui::Spacing();
                circle_item({ 0.01f,0.5f,0.5f,1.f }, "AC Generators");
                ImGui::Spacing();
                {
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    dl->AddTriangleFilled({ p.x + 10,p.y + 1 },
                        { p.x + 2, p.y + 17 },
                        { p.x + 18,p.y + 17 },
                        IM_COL32(25, 25, 200, 255));
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 24);
                    ImGui::TextUnformatted("VSC Converters");
                }
            }
            ImGui::EndChild();
        });
}
