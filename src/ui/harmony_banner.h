#pragma once

#include <iosfwd>
#include <string_view>

/** @brief HARMONY ASCII art banner (BiGER / project branding). */
std::string_view harmonyAsciiBanner();

/** @brief Print the banner to a stream (CLI, log capture). */
void printHarmonyBanner(std::ostream& out);
