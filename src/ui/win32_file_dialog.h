#pragma once

#include <filesystem>
#include <optional>

/** @brief Native JSON file picker on Windows; std::nullopt on cancel or other platforms. */
std::optional<std::filesystem::path> browseJsonFileNative();
