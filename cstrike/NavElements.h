#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

#include "..\dependencies\imgui\imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "..\dependencies\imgui\imgui_internal.h"

extern ImFont* tab_icons;

namespace elements
{
	bool Checkbox(const char* label, bool* v);
	bool tab(const char* icon, bool boolean);
	bool subtab(const char* name, bool boolean);
}
