#include "win32_file_dialog.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

#include <filesystem>

std::optional<std::filesystem::path> browseJsonFileNative() {
#ifdef _WIN32
	wchar_t file[MAX_PATH]{};
	OPENFILENAMEW ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	if (!GetOpenFileNameW(&ofn)) {
		return std::nullopt;
	}
	return std::filesystem::path(file);
#else
	return std::nullopt;
#endif
}
