#include "harmony_banner_gui.h"

#include "harmony_banner.h"

#include <imgui.h>
#include <implot.h>

#include <filesystem>

namespace {

ImFont* addPrimaryFont(ImGuiIO& io, const float sizePixels) {
#ifdef HARMONY_IMGUI_FONT_PATH
	if (std::filesystem::exists(HARMONY_IMGUI_FONT_PATH)) {
		if (ImFont* font = io.Fonts->AddFontFromFileTTF(HARMONY_IMGUI_FONT_PATH, sizePixels)) {
			return font;
		}
	}
#endif

	ImFontConfig cfg;
	cfg.SizePixels = sizePixels;
	return io.Fonts->AddFontDefault(&cfg);
}

} // namespace

ImFont* harmonyInitUiFont(ImGuiIO& io, const float sizePixels) {
	io.Fonts->Clear();
	return addPrimaryFont(io, sizePixels);
}

void harmonyApplyUiStyleScale(const float scale) {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
	style.WindowPadding = ImVec2(18.f * scale, 14.f * scale);
	style.FramePadding = ImVec2(12.f * scale, 8.f * scale);
	style.ItemSpacing = ImVec2(12.f * scale, 10.f * scale);
	style.ItemInnerSpacing = ImVec2(10.f * scale, 6.f * scale);
	style.ScrollbarSize = 20.f * scale;
	style.GrabMinSize = 14.f * scale;
}

ImFont* harmonyConfigureLauncherUi(ImGuiIO& io, ImFont** bannerFontOut) {
	harmonyInitUiFont(io, kHarmonyUiFontSizePx);
	harmonyApplyUiStyleScale(kHarmonyUiStyleScale);

	ImFont* bannerFont = harmonyInitBannerFont(io, kHarmonyLauncherBannerFontSizePx);
	if (bannerFontOut != nullptr) {
		*bannerFontOut = bannerFont;
	}
	return io.Fonts->Fonts[0];
}

ImFont* harmonyConfigurePlotUi(ImGuiIO& io) {
	harmonyInitUiFont(io, kHarmonyPlotUiFontSizePx);
	harmonyApplyUiStyleScale(kHarmonyPlotUiStyleScale);
	harmonyApplyPlotStyle();
	return io.Fonts->Fonts[0];
}

void harmonyApplyPlotStyle() {
	ImPlotStyle& style = ImPlot::GetStyle();
	style.PlotBorderSize = 1.5f;
	style.MajorTickLen = ImVec2(14.0f, 14.0f);
	style.MinorTickLen = ImVec2(8.0f, 8.0f);
	style.MajorTickSize = ImVec2(2.0f, 2.0f);
	style.MinorTickSize = ImVec2(1.5f, 1.5f);
	style.MajorGridSize = ImVec2(1.5f, 1.5f);
	style.LabelPadding = ImVec2(12.0f, 10.0f);
	style.LegendPadding = ImVec2(20.0f, 20.0f);
	style.LegendInnerPadding = ImVec2(10.0f, 8.0f);
	style.LegendSpacing = ImVec2(10.0f, 4.0f);
	style.PlotPadding = ImVec2(18.0f, 18.0f);
	style.PlotDefaultSize = ImVec2(520.0f, 380.0f);
	style.PlotMinSize = ImVec2(280.0f, 200.0f);
	style.AnnotationPadding = ImVec2(8.0f, 8.0f);
	style.MousePosPadding = ImVec2(12.0f, 12.0f);
	style.DigitalPadding = 28.0f;
	style.DigitalSpacing = 6.0f;
}

ImFont* harmonyInitBannerFont(ImGuiIO& io, const float sizePixels) {
#ifdef HARMONY_IMGUI_FONT_PATH
	if (std::filesystem::exists(HARMONY_IMGUI_FONT_PATH)) {
		if (ImFont* font = io.Fonts->AddFontFromFileTTF(HARMONY_IMGUI_FONT_PATH, sizePixels)) {
			return font;
		}
	}
#endif

	ImFontConfig cfg;
	cfg.SizePixels = sizePixels;
	return io.Fonts->AddFontDefault(&cfg);
}

void harmonyDrawBannerImGui(ImFont* bannerFont, const bool compact) {
	if (bannerFont != nullptr) {
		ImGui::PushFont(bannerFont);
	}

	const float lineHeight = ImGui::GetTextLineHeight();
	const float visibleLines = compact ? 5.5f : 11.0f;
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
