#pragma once
#include "imgui.h"

// DPI value
extern float dpi;

namespace font
{
	extern ImFont* icomoon;
	extern ImFont* lexend_bold;
	extern ImFont* lexend_regular;
	extern ImFont* lexend_general_bold;
	extern ImFont* icomoon_widget;
	extern ImFont* icomoon_widget2;
}

namespace c
{
	extern ImVec4 accent;

	namespace background
	{
		extern ImVec4 filling;
		extern ImVec4 stroke;
		extern ImVec2 size;
		extern float rounding;
	}

	namespace elements
	{
		extern ImVec4 mark;
		extern ImVec4 stroke;
		extern ImVec4 background;
		extern ImVec4 background_widget;
		extern ImVec4 text_active;
		extern ImVec4 text_hov;
		extern ImVec4 text;
		extern ImVec4 text_light;
		extern float rounding;
	}

	namespace child
	{
		// Future child properties can go here
	}

	namespace tab
	{
		extern ImVec4 tab_active;
		extern ImVec4 bg;
		extern ImVec4 bg2;
		extern ImVec4 border;
	}
}
