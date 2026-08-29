#include "harmony_banner.h"

#include <imgui.h>

#include <filesystem>
#include <ostream>

namespace {

constexpr const char kBanner[] = R"( ,ggg,        gg            ,ggg, ,ggggggggggg,   ,ggg, ,ggg,_,ggg,    _,gggggg,_      ,ggg, ,ggggggg,  ,ggg,         gg 
dP""Y8b       88           dP""8IdP"""88""""""Y8,dP""Y8dP""Y8dP""Y8b ,d8P""d8P"Y8b,   dP""Y8,8P"""""Y8bdP""Y8a        88 
Yb, `88       88          dP   88Yb,  88      `8bYb, `88'  `88'  `88,d8'   Y8   "8b,dPYb, `8dP'     `88Yb, `88        88 
 `"  88       88         dP    88 `"  88      ,8P `"  88    88    88d8'    `Ybaaad88P' `"  88'       88 `"  88        88 
     88aaaaaaa88        ,8'    88     88aaaad8P"      88    88    888P       `""""Y8       88        88     88        88 
     88"""""""88        d88888888     88""""Yb,       88    88    888b            d8       88        88     88        88 
     88       88  __   ,8"     88     88     "8b      88    88    88Y8,          ,8P       88        88     88       ,88 
     88       88 dP"  ,8P      Y8     88      `8i     88    88    88`Y8,        ,8P'       88        88     Y8b,___,d888 
     88       Y8,Yb,_,dP       `8b,   88       Yb,    88    88    Y8,`Y8b,,__,,d8P'        88        Y8,     "Y88888P"88,
     88       `Y8 "Y8P"         `Y8   88        Y8    88    88    `Y8  `"Y8888P"'          88        `Y8          ,ad8888
                                                                                                                 d8P" 88 
                                                                                                               ,d8'   88 
                                                                                                               d8'    88 
                                                                                                               88     88 
                                                                                                               Y8,_ _,88 
                                                                                                                "Y888P"  
)";

} // namespace

std::string_view harmonyAsciiBanner() {
	return kBanner;
}

void printHarmonyBanner(std::ostream& out) {
	out << kBanner << '\n';
}

ImFont* harmonyInitBannerFont(ImGuiIO& io, const float sizePixels) {
	thread_local ImFont* cached = nullptr;
	if (cached != nullptr) {
		return cached;
	}

#ifdef HARMONY_IMGUI_FONT_PATH
	if (std::filesystem::exists(HARMONY_IMGUI_FONT_PATH)) {
		cached = io.Fonts->AddFontFromFileTTF(HARMONY_IMGUI_FONT_PATH, sizePixels);
		if (cached != nullptr) {
			return cached;
		}
	}
#endif

	ImFontConfig cfg;
	cfg.SizePixels = sizePixels;
	cached = io.Fonts->AddFontDefault(&cfg);
	return cached;
}

void harmonyDrawBannerImGui(ImFont* bannerFont, const bool compact) {
	if (bannerFont != nullptr) {
		ImGui::PushFont(bannerFont);
	}

	const float lineHeight = ImGui::GetTextLineHeight();
	const float visibleLines = compact ? 9.0f : 16.5f;
	const ImVec2 bannerSize(0.f, lineHeight * visibleLines);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.f));
	ImGui::BeginChild(
		"##HarmonyAsciiBanner",
		bannerSize,
		compact ? ImGuiChildFlags_Borders : ImGuiChildFlags_None,
		ImGuiWindowFlags_HorizontalScrollbar);
	const std::string_view banner = harmonyAsciiBanner();
	ImGui::TextUnformatted(banner.data(), banner.data() + banner.size());
	ImGui::EndChild();
	ImGui::PopStyleColor();

	if (bannerFont != nullptr) {
		ImGui::PopFont();
	}

	if (!compact) {
		ImGui::TextDisabled("Hybrid AC/DC power-system framework — CRESYM / BiGER");
	}

	ImGui::Spacing();
}
