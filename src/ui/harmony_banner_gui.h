#pragma once

struct ImFont;
struct ImGuiIO;

/** @brief Default UI font size (pixels) for HarmonyUI. */
constexpr float kHarmonyUiFontSizePx = 20.0f;

/** @brief ASCII banner font size (pixels) in the launcher. */
constexpr float kHarmonyLauncherBannerFontSizePx = 10.0f;

/** @brief ImGui widget scale factor for HarmonyUI. */
constexpr float kHarmonyUiStyleScale = 1.35f;

/** @brief UI font size (pixels) in the plot / visualization window. */
constexpr float kHarmonyPlotUiFontSizePx = 24.0f;

/** @brief ImGui widget scale in the plot window. */
constexpr float kHarmonyPlotUiStyleScale = 1.45f;

/** @brief Default height for standard ImPlot panels. */
constexpr float kHarmonyPlotPanelHeightPx = 380.0f;

/** @brief Height for eigenvalue s-plane plots. */
constexpr float kHarmonyPlotEigenHeightPx = 640.0f;

/** @brief Height for abc / grouped waveform plots. */
constexpr float kHarmonyPlotWaveformHeightPx = 300.0f;

/** @brief Load the primary UI font and replace ImGui's default. */
ImFont* harmonyInitUiFont(ImGuiIO& io, float sizePixels = kHarmonyUiFontSizePx);

/** @brief Enlarge paddings, buttons, and scroll bars for readability. */
void harmonyApplyUiStyleScale(float scale = kHarmonyUiStyleScale);

/** @brief Enlarge ImPlot tick labels, legends, markers, and line weight. */
void harmonyApplyPlotStyle();

/** @brief Configure fonts and style for the HarmonyUI launcher window. */
ImFont* harmonyConfigureLauncherUi(ImGuiIO& io, ImFont** bannerFontOut);

/** @brief Configure fonts and style for the Harmony Visualization window. */
ImFont* harmonyConfigurePlotUi(ImGuiIO& io);

/** @brief Load a monospace font for the ASCII banner in the current ImGui context. */
ImFont* harmonyInitBannerFont(ImGuiIO& io, float sizePixels = 10.0f);

/** @brief Draw the embedded banner inside the active ImGui window. */
void harmonyDrawBannerImGui(ImFont* bannerFont, bool compact = false);
