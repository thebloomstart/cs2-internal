#include "imgui/imgui_settings.h"

// DPI value
float dpi = 1.f;

namespace font
{
	ImFont* icomoon = nullptr;
	ImFont* lexend_bold = nullptr;
	ImFont* lexend_regular = nullptr;
	ImFont* lexend_general_bold = nullptr;
	ImFont* icomoon_widget = nullptr;
	ImFont* icomoon_widget2 = nullptr;
}

namespace c
{
	ImVec4 accent = ImColor(112, 110, 215);

	namespace background
	{
		ImVec4 filling = ImColor(11, 11, 11, 155);
		ImVec4 stroke = ImColor(24, 26, 36);
		ImVec2 size = ImVec2(800, 520);
		float rounding = 6;
	}

	namespace elements
	{
		ImVec4 mark = ImColor(255, 255, 255);
		ImVec4 stroke = ImColor(28, 26, 37);
		ImVec4 background = ImColor(11, 13, 15);
		ImVec4 background_widget = ImColor(21, 23, 26);
		ImVec4 text_active = ImColor(255, 255, 255);
		ImVec4 text_hov = ImColor(81, 92, 109);
		ImVec4 text = ImColor(43, 51, 63);
		ImVec4 text_light = ImColor(125, 125, 125);
		float rounding = 4;
	}

	namespace tab
	{
		ImVec4 tab_active = ImColor(20, 20, 21);
		ImVec4 bg = ImColor(14, 14, 15);
		ImVec4 bg2 = ImColor(9, 9, 9);
		ImVec4 border = ImColor(14, 14, 15);
	}
}
