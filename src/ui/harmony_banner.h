#ifndef HARMONY_UI_BANNER_H
#define HARMONY_UI_BANNER_H

#include <iosfwd>
#include <string_view>

/** @brief HARMONY ASCII art banner (BiGER / project branding). */
std::string_view harmonyAsciiBanner();

/** @brief Print the banner to a stream (CLI, log capture). */
void printHarmonyBanner(std::ostream& out);

#endif // HARMONY_UI_BANNER_H