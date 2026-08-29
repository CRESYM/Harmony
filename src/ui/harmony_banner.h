#pragma once

#include <iosfwd>
#include <string_view>

struct ImFont;
struct ImGuiIO;

/** @brief HARMONY ASCII art banner (BiGER / project branding). */
std::string_view harmonyAsciiBanner();

/** @brief Print the banner to a stream (CLI, log capture). */
void printHarmonyBanner(std::ostream& out);

/**
 * @brief Load a monospace font for the ASCII banner in the current ImGui context.
 * @return Font pointer, or nullptr if loading failed (caller may still draw with default font).
 */
ImFont* harmonyInitBannerFont(ImGuiIO& io, float sizePixels = 11.0f);

/** @brief Draw the embedded banner inside the active ImGui window. */
void harmonyDrawBannerImGui(ImFont* bannerFont, bool compact = false);
