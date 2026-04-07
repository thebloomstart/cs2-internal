#include "draw.h"

// used: cheat variables
#include "../core/variables.h"
// used: viewmatrix
#include "../core/sdk.h"

// used: m_deg2rad
#include "math.h"
// used: memoryset
#include "crt.h"
// used: easing
#include "../Fonts.h"
#include "easing.h"
// used: ipt
#include "inputsystem.h"

// used: [ext] imgui
#include "../../dependencies/imgui/imgui_freetype.h"
#include "../../dependencies/imgui/imgui_impl_dx11.h"
#include "../../dependencies/imgui/imgui_impl_win32.h"
#include "../../dependencies/imgui/imgui_settings.h"
#include "../core/font.h"

// used: [resouces] font awesome
#include "../../resources/fa_solid_900.h"
#include "../../resources/font_awesome_5.h"
#include "../WeaponFont.h"
// used: iinputsystem
#include "../core/interfaces.h"
#include "../sdk/interfaces/iinputsystem.h"
// used: bMainWindowOpened
#include "../core/menu.h"
#include "../MuseoSansCyrl-900.h"
#include "../FontRandom.h"
#include "../sdk\EntityList\onetap.h"
#include "../cstrike/colors.h"

// used: hkIsRelativeMouseMode.GetOriginal();
#include "../core/hooks.h"
#include <map>

#pragma region imgui_extended
static constexpr const char* arrKeyNames[] = {
	"",
	"mouse 1", "mouse 2", "cancel", "mouse 3", "mouse 4", "mouse 5", "",
	"backspace", "tab", "", "", "clear", "enter", "", "",
	"shift", "control", "alt", "pause", "caps", "", "", "", "", "", "",
	"escape", "", "", "", "", "space", "page up", "page down",
	"end", "home", "left", "up", "right", "down", "", "", "",
	"print", "insert", "delete", "",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"", "", "", "", "", "", "",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
	"l", "m", "n", "o", "p", "q", "r", "s", "t", "u",
	"v", "w", "x", "y", "z", "lwin", "rwin", "", "", "",
	"num0", "num1", "num2", "num3", "num4", "num5",
	"num6", "num7", "num8", "num9",
	"*", "+", "", "-", ".", "/",
	"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8",
	"f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16",
	"f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24",
	"", "", "", "", "", "", "", "",
	"num lock", "scroll lock",
	"", "", "", "", "", "", "",
	"", "", "", "", "", "", "",
	"lshift", "rshift", "lctrl",
	"rctrl", "lmenu", "rmenu"
};
const char* keys[] = {
	"None",
	"Mouse 1",
	"Mouse 2",
	"CN",
	"Mouse 3",
	"Mouse 4",
	"Mouse 5",
	"-",
	"Back",
	"Tab",
	"-",
	"-",
	"CLR",
	"Enter",
	"-",
	"-",
	"Shift",
	"CTL",
	"Menu",
	"Pause",
	"Caps Lock",
	"KAN",
	"-",
	"JUN",
	"FIN",
	"KAN",
	"-",
	"Escape",
	"CON",
	"NCO",
	"ACC",
	"MAD",
	"Space",
	"PGU",
	"PGD",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"SEL",
	"PRI",
	"EXE",
	"PRI",
	"INS",
	"Delete",
	"HEL",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"WIN",
	"WIN",
	"APP",
	"-",
	"SLE",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"MUL",
	"ADD",
	"SEP",
	"MIN",
	"Delete",
	"DIV",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"F16",
	"F17",
	"F18",
	"F19",
	"F20",
	"F21",
	"F22",
	"F23",
	"F24",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"NUM",
	"SCR",
	"EQU",
	"MAS",
	"TOY",
	"OYA",
	"OYA",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"Shift",
	"Shift",
	"Ctrl",
	"Ctrl",
	"Alt",
	"Alt"
};

void ImGui::HelpMarker(const char* szDescription)
{
	TextDisabled(CS_XOR("(?)"));
	if (IsItemHovered())
	{
		BeginTooltip();
		PushTextWrapPos(450.f);
		TextUnformatted(szDescription);
		PopTextWrapPos();
		EndTooltip();
	}
}

bool ImGui::HotKey(const char* szLabel, unsigned int* pValue)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* pWindow = g.CurrentWindow;

	if (pWindow->SkipItems)
		return false;

	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;
	const ImGuiID nIndex = pWindow->GetID(szLabel);

	const float flWidth = CalcItemWidth();
	const ImVec2 vecLabelSize = CalcTextSize(szLabel, nullptr, true);

	const ImRect rectFrame(pWindow->DC.CursorPos + ImVec2(vecLabelSize.x > 0.0f ? style.ItemInnerSpacing.x + GetFrameHeight() : 0.0f, 0.0f), pWindow->DC.CursorPos + ImVec2(flWidth, vecLabelSize.x > 0.0f ? vecLabelSize.y + style.FramePadding.y : 0.f));
	const ImRect rectTotal(rectFrame.Min, rectFrame.Max);

	ItemSize(rectTotal, style.FramePadding.y);
	if (!ItemAdd(rectTotal, nIndex, &rectFrame))
		return false;

	const bool bHovered = ItemHoverable(rectFrame, nIndex, ImGuiItemFlags_None);
	if (bHovered)
	{
		SetHoveredID(nIndex);
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	const bool bClicked = bHovered && io.MouseClicked[0];
	const bool bDoubleClicked = bHovered && io.MouseDoubleClicked[0];
	if (bClicked || bDoubleClicked)
	{
		if (g.ActiveId != nIndex)
		{
			CRT::MemorySet(io.MouseDown, 0, sizeof(io.MouseDown));
			CRT::MemorySet(io.KeysDown, 0, sizeof(io.KeysDown));
			*pValue = 0U;
		}

		SetActiveID(nIndex, pWindow);
		FocusWindow(pWindow);
	}

	bool bValueChanged = false;
	if (unsigned int nKey = *pValue; g.ActiveId == nIndex)
	{
		for (int n = 0; n < IM_ARRAYSIZE(io.MouseDown); n++)
		{
			if (IsMouseDown(n))
			{
				switch (n)
				{
				case 0:
					nKey = VK_LBUTTON;
					break;
				case 1:
					nKey = VK_RBUTTON;
					break;
				case 2:
					nKey = VK_MBUTTON;
					break;
				case 3:
					nKey = VK_XBUTTON1;
					break;
				case 4:
					nKey = VK_XBUTTON2;
					break;
				}

				bValueChanged = true;
				ClearActiveID();
			}
		}

		if (!bValueChanged)
		{
			for (int n = VK_BACK; n <= VK_RMENU; n++)
			{
				if (IsKeyDown((ImGuiKey)n))
				{
					nKey = n;
					bValueChanged = true;
					ClearActiveID();
				}
			}
		}

		if (IsKeyPressed(ImGuiKey_Escape))
		{
			*pValue = 0U;
			ClearActiveID();
		}
		else
			*pValue = nKey;
	}

	char szBuffer[64] = {};
	char* szBufferEnd = CRT::StringCopy(szBuffer, "  ");
	if (*pValue != 0 && g.ActiveId != nIndex)
		szBufferEnd = CRT::StringCat(szBufferEnd, arrKeyNames[*pValue]);
	else if (g.ActiveId == nIndex)
		szBufferEnd = CRT::StringCat(szBufferEnd, CS_XOR("press"));
	else
		szBufferEnd = CRT::StringCat(szBufferEnd, CS_XOR("none"));
	CRT::StringCat(szBufferEnd, "  ");

	// modified by LUNOWARE
	PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, -1));

	const ImVec2 vecBufferSize = CalcTextSize(szBuffer);
	RenderFrame(ImVec2(rectFrame.Max.x - vecBufferSize.x, rectTotal.Min.y), ImVec2(rectFrame.Max.x, rectTotal.Min.y + style.FramePadding.y + vecBufferSize.y), GetColorU32((bHovered || bClicked || bDoubleClicked) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
	pWindow->DrawList->AddText(ImVec2(rectFrame.Max.x - vecBufferSize.x, rectTotal.Min.y + style.FramePadding.y), GetColorU32(g.ActiveId == nIndex ? ImGuiCol_Text : ImGuiCol_TextDisabled), szBuffer);

	if (vecLabelSize.x > 0.f)
		RenderText(ImVec2(rectTotal.Min.x, rectTotal.Min.y + style.FramePadding.y), szLabel);

	PopStyleVar();
	return bValueChanged;
}

bool ImGui::HotKey(const char* szLabel, KeyBind_t* pKeyBind, const bool bAllowSwitch)
{
	const bool bValueChanged = HotKey(szLabel, &pKeyBind->uKey);

	if (bAllowSwitch)
	{
		char* szUniqueID = static_cast<char*>(MEM_STACKALLOC(CRT::StringLength(szLabel) + 6));
		CRT::StringCat(CRT::StringCopy(szUniqueID, CS_XOR("key##")), szLabel);

		if (IsItemClicked(ImGuiMouseButton_Right))
			OpenPopup(szUniqueID);

		if (BeginPopup(szUniqueID))
		{
			SetNextItemWidth(ImGui::GetWindowWidth() * 0.75f);
			if (Combo(CS_XOR("##keybind.mode"), reinterpret_cast<int*>(&pKeyBind->nMode), CS_XOR("Hold\0Toggle\0\0")))
				CloseCurrentPopup();

			EndPopup();
		}

		MEM_STACKFREE(szUniqueID);
	}

	return bValueChanged;
}

bool ImGui::MultiCombo(const char* szLabel, unsigned int* pFlags, const char* const* arrItems, int nItemsCount)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* pWindow = g.CurrentWindow;

	if (pWindow->SkipItems)
		return false;

	IM_ASSERT(nItemsCount < 32); // bitflags shift overflow, decrease items count or change variable type

	const ImGuiStyle& style = g.Style;
	const ImVec2 vecLabelSize = CalcTextSize(szLabel, nullptr, true);
	const float flActiveWidth = CalcItemWidth() - (vecLabelSize.x > 0.0f ? style.ItemInnerSpacing.x + GetFrameHeight() : 0.0f);

	std::vector<const char*> vecActiveItems = {};

	// collect active items
	for (int i = 0; i < nItemsCount; i++)
	{
		if (*pFlags & (1 << i))
			vecActiveItems.push_back(arrItems[i]);
	}

	// fuck it, stl still haven't boost::join, fmt::join replacement
	std::string strBuffer = {};
	for (std::size_t i = 0U; i < vecActiveItems.size(); i++)
	{
		strBuffer.append(vecActiveItems[i]);

		if (i < vecActiveItems.size() - 1U)
			strBuffer.append(", ");
	}

	if (strBuffer.empty())
		strBuffer.assign("none");
	else
	{
		const char* szWrapPosition = g.Font->CalcWordWrapPositionA(GetCurrentWindow()->FontWindowScale, strBuffer.data(), strBuffer.data() + strBuffer.length(), flActiveWidth - style.FramePadding.x * 2.0f);
		const std::size_t nWrapLength = szWrapPosition - strBuffer.data();

		if (nWrapLength > 0U && nWrapLength < strBuffer.length())
		{
			strBuffer.resize(nWrapLength);
			strBuffer.append("...");
		}
	}

	bool bValueChanged = false;
	if (BeginCombo(szLabel, strBuffer.c_str()))
	{
		for (int i = 0; i < nItemsCount; i++)
		{
			const int nCurrentFlag = (1 << i);
			if (Selectable(arrItems[i], (*pFlags & nCurrentFlag), ImGuiSelectableFlags_DontClosePopups))
			{
				// flip bitflag
				*pFlags ^= nCurrentFlag;
				bValueChanged = true;
			}
		}

		EndCombo();
	}

	return bValueChanged;
}

bool ImGui::BeginListBox(const char* szLabel, int nItemsCount, int nHeightInItems)
{
	float height = GetTextLineHeightWithSpacing() * ((nHeightInItems < 0 ? ImMin(nItemsCount, 7) : nHeightInItems) + 0.25f) + GetStyle().FramePadding.y * 2.0f;
	return BeginListBox(szLabel, ImVec2(0.0f, height));
}

bool ImGui::ColorEdit3(const char* szLabel, Color_t* pColor, ImGuiColorEditFlags flags)
{
	return ColorEdit4(szLabel, pColor, flags | ImGuiColorEditFlags_NoAlpha);
}

bool ImGui::ColorEdit4(const char* szLabel, Color_t* pColor, ImGuiColorEditFlags flags)
{
	float arrColor[4];
	pColor->BaseAlpha(arrColor);

	if (ColorEdit4(szLabel, &arrColor[0], flags))
	{
		*pColor = Color_t::FromBase4(arrColor);
		return true;
	}

	return false;
}

bool ImGui::ColorEdit3(const char* szLabel, ColorPickerVar_t* pColor, ImGuiColorEditFlags flags)
{
	return ColorEdit4(szLabel, pColor, flags | ImGuiColorEditFlags_NoAlpha);
}

bool ImGui::ColorEdit4(const char* szLabel, ColorPickerVar_t* pColorVar, ImGuiColorEditFlags flags)
{
	const bool bResult = ColorEdit4(szLabel, &pColorVar->colValue, flags);

	// switch rainbow mode on middle mouse click
	if (IsItemHovered())
	{
		// tooltip for turn on/off rainbow mode
		BeginTooltip();
		{
			PushTextWrapPos(450.f);
			TextUnformatted(CS_XOR("use mouse middle-click to turn on/off rainbow mode!"));
			PopTextWrapPos();
		}
		EndTooltip();

		if (IsMouseClicked(ImGuiMouseButton_Middle))
			pColorVar->bRainbow = !pColorVar->bRainbow;
	}

	// open the context popup
	OpenPopupOnItemClick(CS_XOR("context##color.picker"), ImGuiPopupFlags_MouseButtonRight);
	// @todo: cleaner code
	SetNextWindowSize(ImVec2((pColorVar->bRainbow ? 120.f : 60.f) * D::CalculateDPI(C_GET(int, Vars.nDpiScale)), 0.f));
	if (BeginPopup(CS_XOR("context##color.picker")))
	{
		if (Button(CS_XOR("copy##color.picker"), ImVec2(-1, 15 * D::CalculateDPI(C_GET(int, Vars.nDpiScale)))))
		{
			// @todo: im32 hex format is AARRGGBB, but we need RRGGBBAA
			CRT::String_t<64U> szBuffer(CS_XOR("#%X"), pColorVar->colValue.GetU32());
			SetClipboardText(szBuffer.Data());
			szBuffer.Clear();

			CloseCurrentPopup();
		}

		if (Button(CS_XOR("paste##color.picker"), ImVec2(-1, 15 * D::CalculateDPI(C_GET(int, Vars.nDpiScale)))))
		{
			const char* szClipboardText = GetClipboardText();
			// @note: +1U for '#' prefix skipping
			const ImU32 uConvertedColor = CRT::StringToInteger<ImU32>(szClipboardText + 1U, nullptr, 16);

			pColorVar->colValue = Color_t(uConvertedColor);
			CloseCurrentPopup();
		}

		if (pColorVar->bRainbow)
		{
			// @note: urgh padding moment idk
			SetNextItemWidth(ImGui::GetWindowWidth() * 0.90f + 1.f);
			SliderFloat(CS_XOR("##speed.color.picker"), &pColorVar->flRainbowSpeed, 0.f, 5.f, CS_XOR("speed: %.1f"), ImGuiSliderFlags_AlwaysClamp);
		}

		EndPopup();
	}

	return bResult;
}

#pragma endregion

// thread-safe draw data mutex
static SRWLOCK drawLock = {};

static void* __cdecl ImGuiAllocWrapper(const std::size_t nSize, [[maybe_unused]] void* pUserData = nullptr)
{
	return MEM::HeapAlloc(nSize);
}

static void __cdecl ImGuiFreeWrapper(void* pMemory, [[maybe_unused]] void* pUserData = nullptr) noexcept
{
	MEM::HeapFree(pMemory);
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../core/hooks.h"

// Simple helper function to load an image into a DX11 texture with common settings
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = image_width;
	desc.Height = image_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	I::Device->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	I::Device->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	*out_width = image_width;
	*out_height = image_height;
	stbi_image_free(image_data);

	return true;
}

int my_image_width = 170;
int my_image_height = 295;

bool D::Setup(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// check is it were already initialized
	if (bInitialized)
		return true;

	ImGui::SetAllocatorFunctions(ImGuiAllocWrapper, ImGuiFreeWrapper);

	ImGui::CreateContext();

	// setup platform and renderer bindings
	if (!ImGui_ImplWin32_Init(hWnd))
		return false;

	if (!ImGui_ImplDX11_Init(pDevice, pContext))
		return false;

	// create draw data containers
	pDrawListActive = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
	pDrawListSafe = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
	pDrawListRender = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());

	// setup styles
#pragma region draw_setup_style
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.WindowPadding = ImVec2(8, 8);
	style.WindowRounding = 4.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.ChildRounding = 4.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 4.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4, 2);
	style.FrameRounding = 4.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(8, 4);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.IndentSpacing = 6.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 6.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 0.0f;
	style.GrabRounding = 4.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 1.0f;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.5f);
	style.WindowShadowSize = 0.f;
	style.AntiAliasedLines = true;
	style.AntiAliasedFill = true;
	style.AntiAliasedLinesUseTex = true;
	style.ColorButtonPosition = ImGuiDir_Right;
#pragma endregion

	ImGuiIO& io = ImGui::GetIO();

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.FontDataOwnedByAtlas = false;

	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	ImFontConfig imVerdanaConfig;
	imVerdanaConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
	for (int i = 0; i < CS_ARRAYSIZE(FONT::pMenu); i++)
	{
		const float flFontSize = 12.f * CalculateDPI(i);
		FONT::pMenu[i] = io.Fonts->AddFontFromFileTTF(CS_XOR("C:\\Windows\\Fonts\\Verdana.ttf"), flFontSize, &imVerdanaConfig, io.Fonts->GetGlyphRangesCyrillic());
		io.Fonts->AddFontFromMemoryTTF((void*)fa_solid_900, sizeof(fa_solid_900), flFontSize, &icons_config, icons_ranges);
	}

	imVerdanaConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Bold;
	FONT::pExtra = io.Fonts->AddFontFromFileTTF(CS_XOR("C:\\Windows\\Fonts\\Verdana.ttf"), 14.f, &imVerdanaConfig, io.Fonts->GetGlyphRangesCyrillic());

	ImFontConfig imTahomaConfig;
	imTahomaConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
	FONT::pVisual = io.Fonts->AddFontFromFileTTF(CS_XOR("C:\\Windows\\Fonts\\Tahoma.ttf"), 14.f, &imTahomaConfig, io.Fonts->GetGlyphRangesCyrillic());
	FONT::WeaponFont = io.Fonts->AddFontFromMemoryTTF(WeaponImages, sizeof(WeaponImages), 15.0f);
	FONT::DefuseKitFont = io.Fonts->AddFontFromMemoryTTF(WeaponImages, sizeof(WeaponImages), 20.0f);
	FONT::pTab = io.Fonts->AddFontFromMemoryTTF(MuseoSans, sizeof(MuseoSans), 18.0f), nullptr, io.Fonts->GetGlyphRangesCyrillic();
	FONT::font = io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\segoeui.ttf"), 16.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	FONT::pMain = io.Fonts->AddFontFromMemoryTTF(PTRootUIMedium, sizeof(PTRootUIMedium), 20.0f), nullptr, io.Fonts->GetGlyphRangesCyrillic();
	FONT::big_icon = io.Fonts->AddFontFromMemoryTTF(&rawData, sizeof rawData, 35, NULL, io.Fonts->GetGlyphRangesCyrillic());

	ImFontConfig cfg;

	cfg.PixelSnapH = false;
	cfg.OversampleH = 5;
	cfg.OversampleV = 5;

	fonts.verdana = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 13.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	fonts.verdana_small = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	LoadTextureFromFile("C:\\Users\\Luzin\\Downloads\\photo_2024-10-19_00-32-13.jpg", &I::Maintexture, &my_image_width, &my_image_height);
	auto prev_flags = cfg.FontBuilderFlags;
	cfg.FontBuilderFlags = 1 << 4 | 1 << 7;
	fonts.onetap_pixel = io.Fonts->AddFontFromMemoryTTF(onetap_pixel_font, sizeof(onetap_pixel_font), 8.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	cfg.FontBuilderFlags = prev_flags;

	font::lexend_general_bold = io.Fonts->AddFontFromMemoryTTF(lexend_bold, sizeof(lexend_bold), 18.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::lexend_bold = io.Fonts->AddFontFromMemoryTTF(lexend_regular, sizeof(lexend_regular), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::lexend_regular = io.Fonts->AddFontFromMemoryTTF(lexend_regular, sizeof(lexend_regular), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::icomoon = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 20.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	font::icomoon_widget = io.Fonts->AddFontFromMemoryTTF(icomoon_widget, sizeof(icomoon_widget), 15.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	font::icomoon_widget2 = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 16.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	io.Fonts->FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
	bInitialized = io.Fonts->Build();
	return bInitialized;
}

	std::string pChar(std::string first_, std::string second_)
	{
		return (first_ + second_);
	}

	float D::Animate(const char* label, const char* second_label, bool if_, float Maximal_, float Speed_, int type)
	{
		auto ID = ImGui::GetID(pChar(label, second_label).c_str());

		static std::map<ImGuiID, float> pValue;

		auto this_e = pValue.find(ID);

		if (this_e == pValue.end())
		{
			pValue.insert({ ID, 0.f });
			this_e = pValue.find(ID);
		}

		switch (type)
		{
		case DYNAMIC:
		{
			if (if_) //do
				this_e->second += abs(Maximal_ - this_e->second) / Speed_;
			else
				this_e->second -= (0 + this_e->second) / Speed_;
		}
		break;

		case INTERP:
		{
			if (if_) //do
				this_e->second += (Maximal_ - this_e->second) / Speed_;
			else
				this_e->second -= (0 + this_e->second) / Speed_;
		}
		break;

		case STATIC:
		{
			if (if_) //do
				this_e->second += Speed_;
			else
				this_e->second -= Speed_;
		}
		break;
		}

		{
			if (this_e->second < 0.f)
				this_e->second = 0.f;
			else if (this_e->second > Maximal_)
				this_e->second = Maximal_;
		}

		return this_e->second;
	}

void D::Destroy()
{
	// check is it already destroyed or wasn't initialized at all
	if (!bInitialized)
		return;

	// free draw data containers
	IM_DELETE(pDrawListActive);
	IM_DELETE(pDrawListSafe);
	IM_DELETE(pDrawListRender);

	// shutdown imgui directx9 renderer binding
	ImGui_ImplDX11_Shutdown();

	// shutdown imgui win32 platform binding
	ImGui_ImplWin32_Shutdown();

	// destroy imgui context
	ImGui::DestroyContext();

	bInitialized = false;
}

#pragma region draw_callbacks

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool D::OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// check is drawing initialized
	if (!bInitialized)
		return false;

	IPT::OnWndProc(hWnd, uMsg, wParam, lParam);

	// switch menu state
	if (IPT::IsKeyReleased(C_GET(unsigned int, Vars.nMenuKeys)))
	{
		MENU::bMainWindowOpened = !MENU::bMainWindowOpened;
		// update animation
		//MENU::animMenuDimBackground.Switch();

		//// handle IsRelativeMouseMode original
		const auto oIsRelativeMouseMode = H::hkIsRelativeMouseMode.GetOriginal();
		oIsRelativeMouseMode(I::InputSystem, MENU::bMainWindowOpened ? false : MENU::bMainActive);
	}

	// handle ImGui's window messages and block game's input if menu is opened
	return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) || MENU::bMainWindowOpened;
}

void D::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void D::Render()
{
	ImGui::Render();
	RenderDrawData(ImGui::GetDrawData());
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

#pragma endregion

#pragma region draw_main

void D::RenderDrawData(ImDrawData* pDrawData)
{
	if (::TryAcquireSRWLockExclusive(&drawLock))
	{
		*pDrawListRender = *pDrawListSafe;
		::ReleaseSRWLockExclusive(&drawLock);
	}

	if (pDrawListRender->CmdBuffer.empty())
		return;

	// remove trailing command if unused
	// @note: equivalent to  pDrawList->_PopUnusedDrawCmd()
	if (const ImDrawCmd& lastCommand = pDrawListRender->CmdBuffer.back(); lastCommand.ElemCount == 0 && lastCommand.UserCallback == nullptr)
	{
		pDrawListRender->CmdBuffer.pop_back();
		if (pDrawListRender->CmdBuffer.empty())
			return;
	}

	ImGuiContext* pContext = ImGui::GetCurrentContext();
	ImGuiViewportP* pViewport = pContext->Viewports[0];
	ImVector<ImDrawList*>* vecDrawLists = pViewport->DrawDataBuilder.Layers[0];
	vecDrawLists->push_front(pDrawListRender); // this one being most background

	pDrawData->CmdLists.push_front(pDrawListRender);
	pDrawData->CmdListsCount = vecDrawLists->Size;
	pDrawData->TotalVtxCount += pDrawListRender->VtxBuffer.Size;
	pDrawData->TotalIdxCount += pDrawListRender->IdxBuffer.Size;
}

void D::ResetDrawData()
{
	pDrawListActive->_ResetForNewFrame();
	pDrawListActive->PushTextureID(ImGui::GetIO().Fonts->TexID);
	pDrawListActive->PushClipRectFullScreen();
}

void D::SwapDrawData()
{
	::AcquireSRWLockExclusive(&drawLock);

	IM_ASSERT(pDrawListActive->VtxBuffer.Size == 0 || pDrawListActive->_VtxWritePtr == pDrawListActive->VtxBuffer.Data + pDrawListActive->VtxBuffer.Size);
	IM_ASSERT(pDrawListActive->IdxBuffer.Size == 0 || pDrawListActive->_IdxWritePtr == pDrawListActive->IdxBuffer.Data + pDrawListActive->IdxBuffer.Size);

	if (!(pDrawListActive->Flags & ImDrawListFlags_AllowVtxOffset))
		IM_ASSERT(static_cast<int>(pDrawListActive->_VtxCurrentIdx) == pDrawListActive->VtxBuffer.Size);

	*pDrawListSafe = *pDrawListActive;

	::ReleaseSRWLockExclusive(&drawLock);
}

#pragma endregion

#pragma region draw_bindings
#include <algorithm>
std::string D::LowerText(std::string data)
{
	std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c)
	{ return std::tolower(c); });
	return data;
}

std::string D::UpperText(std::string data)
{
	std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c)
	{ return std::toupper(c); });
	return data;
}

void D::RenderText(const Vector_t& txt_pos, Color_t color, const int flags, const ImFont* font, std::string message, const int font_size)
{
	auto DrawList = ImGui::GetBackgroundDrawList();

	DrawList->PushTextureID(font->ContainerAtlas->TexID);

	if (flags & LowerCase)
		message = D::LowerText(message);

	if (flags & UpperCase)
		message = D::UpperText(message);

	const auto size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, message.data());
	auto pos = ImVec2(txt_pos.x, txt_pos.y);
	auto outline_clr = Color_t(0, 0, 0, static_cast<int>(color.a * 0.5f));

	if (flags & Centred)
		pos = ImVec2(txt_pos.x - size.x / 2.0f, txt_pos.y);

	if (flags & DropShadow)
		DrawList->AddText(font, font_size, ImVec2(pos.x + 1, pos.y + 1), outline_clr.GetU32(), message.data());

	if (flags & OutLine)
	{
		DrawList->AddText(font, font_size, ImVec2(pos.x + 1, pos.y - 1), outline_clr.GetU32(), message.data());
		DrawList->AddText(font, font_size, ImVec2(pos.x - 1, pos.y + 1), outline_clr.GetU32(), message.data());
		DrawList->AddText(font, font_size, ImVec2(pos.x - 1, pos.y - 1), outline_clr.GetU32(), message.data());
		DrawList->AddText(font, font_size, ImVec2(pos.x + 1, pos.y + 1), outline_clr.GetU32(), message.data());
		DrawList->AddText(font, font_size, ImVec2(pos.x, pos.y + 1), outline_clr.GetU32(), message.data());
		DrawList->AddText(font, font_size, ImVec2(pos.x, pos.y - 1), outline_clr.GetU32(), message.data());
		DrawList->AddText(font, font_size, ImVec2(pos.x + 1, pos.y), outline_clr.GetU32(), message.data());
		DrawList->AddText(font, font_size, ImVec2(pos.x - 1, pos.y), outline_clr.GetU32(), message.data());
	}

	DrawList->AddText(font, font_size, pos, color.GetU32(), message.data());
}

void D::Rect(Vector_t start_pos, Vector_t end_pos, Color_t color, float rounding)
{
	auto DrawList = ImGui::GetBackgroundDrawList();
	DrawList->AddRectFilled(ImVec2(start_pos.x, start_pos.y), ImVec2(end_pos.x, end_pos.y), color.GetU32(), rounding);
}

bool D::WorldToScreen(const Vector_t& vecOrigin, ImVec2* pvecScreen)
{
	const float flWidth = SDK::ViewMatrix[3][0] * vecOrigin.x + SDK::ViewMatrix[3][1] * vecOrigin.y + SDK::ViewMatrix[3][2] * vecOrigin.z + SDK::ViewMatrix[3][3];

	// check is point can't fit on screen, because it's behind us
	if (flWidth < 0.001f)
		return false;

	// compute the scene coordinates of a point in 3D
	const float flInverse = 1.0f / flWidth;
	pvecScreen->x = (SDK::ViewMatrix[0][0] * vecOrigin.x + SDK::ViewMatrix[0][1] * vecOrigin.y + SDK::ViewMatrix[0][2] * vecOrigin.z + SDK::ViewMatrix[0][3]) * flInverse;
	pvecScreen->y = (SDK::ViewMatrix[1][0] * vecOrigin.x + SDK::ViewMatrix[1][1] * vecOrigin.y + SDK::ViewMatrix[1][2] * vecOrigin.z + SDK::ViewMatrix[1][3]) * flInverse;

	// screen transform
	// get the screen position in pixels of given point
	const ImVec2 vecDisplaySize = ImGui::GetIO().DisplaySize;
	pvecScreen->x = (vecDisplaySize.x * 0.5f) + (pvecScreen->x * vecDisplaySize.x) * 0.5f;
	pvecScreen->y = (vecDisplaySize.y * 0.5f) - (pvecScreen->y * vecDisplaySize.y) * 0.5f;
	return true;
}

struct key_state
{
	ImVec4 background, text;
	bool active = false;
	bool hovered = false;
	float alpha = 0.f;
};

void D::RadialGradient3D(Vector_t pos, float radius, Color_t in, Color_t out, bool one)
{
	ImVec2 center;
	ImVec2 g_pos;
	ImDrawList* pDrawList = ImGui::GetBackgroundDrawList();
	// Use arc with automatic segment count
	static float m_flAnim = 0.f;
	m_flAnim += ImGui::GetIO().DeltaTime;
	if (m_flAnim > 1.f)
		m_flAnim = 0.f;
	D::WorldToScreen(Vector_t(pos), &g_pos);
	center = ImVec2(g_pos.x + (one ? sin(m_flAnim * (MATH::_PI * 2.0f)) * radius : radius / 2.f), g_pos.y + (one ? cos(m_flAnim * (MATH::_PI * 2.0f)) * radius : radius / 2.f));
	pDrawList->_PathArcToFastEx(center, radius, 0, 48, 0);
	const int count = pDrawList->_Path.Size - 1;
	float step = (MATH::_PI * 2.0f) / (count + 1);
	std::vector<ImVec2> point;
	for (float lat = 0.f; lat <= MATH::_PI * 2.0f; lat += step)
	{
		const auto& point3d = Vector_t(sin(lat), cos(lat), 0.f) * radius;
		ImVec2 point2d;
		if (D::WorldToScreen(Vector_t(pos) + point3d, &point2d))
			point.push_back(ImVec2(point2d.x, point2d.y));
	}
	if (in.a == 0 && out.a == 0 || radius < 0.5f || point.size() < count + 1)
		return;

	unsigned int vtx_base = pDrawList->_VtxCurrentIdx;
	pDrawList->PrimReserve(count * 3, count + 1);

	// Submit vertices
	const ImVec2 uv = pDrawList->_Data->TexUvWhitePixel;
	pDrawList->PrimWriteVtx(center, uv, ImColor(in.r, in.g, in.b, in.a));
	for (int n = 0; n < count; n++)
		pDrawList->PrimWriteVtx(point[n + 1], uv, ImColor(out.r, out.g, out.b, out.a));

	// Submit a fan of triangles
	for (int n = 0; n < count; n++)
	{
		pDrawList->PrimWriteIdx((ImDrawIdx)(vtx_base));
		pDrawList->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + n));
		pDrawList->PrimWriteIdx((ImDrawIdx)(vtx_base + 1 + ((n + 1) % count)));
	}
	pDrawList->_Path.Size = 0;
}

bool ImGui::Keybind(const char* label, int* key, int* mode, ImGuiComboFlags flags)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const float width = (GetContentRegionMax().x - style.WindowPadding.x);

	const ImRect rect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width, 22.f));

	ItemSize(ImRect(rect.Min, rect.Max - ImVec2(0, 2)));
	if (!ImGui::ItemAdd(rect, id))
		return false;

	char buf_display[64] = "None";

	bool value_changed = false;
	int k = *key;

	std::string active_key = "";
	active_key += keys[*key];

	if (*key != 0 && g.ActiveId != id)
	{
		strcpy_s(buf_display, active_key.c_str());
	}
	else if (g.ActiveId == id)
	{
		strcpy_s(buf_display, "...");
	}

	const ImVec2 label_size = CalcTextSize(buf_display, NULL, true);

	ImRect clickable(ImVec2(rect.Max.x - 10 - label_size.x, rect.Min.y), rect.Max);
	bool hovered = ItemHoverable(clickable, id,0);

	static std::map<ImGuiID, key_state> anim;
	auto it_anim = anim.find(id);

	if (it_anim == anim.end())
	{
		anim.insert({ id, key_state() });
		it_anim = anim.find(id);
	}

	ImVec4 _keyi_bg_hov = ImColor(87, 83, 78, 75);
	ImVec4 _keyii_bg = ImColor(87, 83, 78, 45);

	ImRect total_bb(window->DC.CursorPos + params::widgets_padding, window->DC.CursorPos + ImVec2(257, 33) + params::widgets_padding); // <- you can modify size from here
	ImRect combo_bb(total_bb.Max - ImVec2(99, 27), total_bb.Max - ImVec2(10, 6));
	ImVec4 text_hov = ImColor(255, 255, 255, 150);
	ImVec4 text_active = ImColor(255, 255, 255, 255);
	ImVec4 text = ImColor(255, 255, 255, 100);
	const ImU32 frame_col = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);


	it_anim->second.background = ImLerp(it_anim->second.background, g.ActiveId == id || hovered ? _keyi_bg_hov : _keyii_bg, ImGui::GetIO().DeltaTime * 6.f);
	it_anim->second.text = ImLerp(it_anim->second.text, g.ActiveId == id ? text_active : hovered ? text_hov :
																								   text,
	ImGui::GetIO().DeltaTime * 6.f);

	window->DrawList->AddRectFilled(clickable.Min, clickable.Max, GetColorU32(it_anim->second.background), 4);


	window->DrawList->AddText(rect.Min - ImVec2(0, 0), GetColorU32(it_anim->second.text), label);

	PushStyleColor(ImGuiCol_Text, text_active);
	RenderTextClipped(clickable.Min - ImVec2(0, 1), clickable.Max - ImVec2(0, 1), buf_display, NULL, &label_size, ImVec2(0.5f, 0.5f));
	PopStyleColor();

	if (hovered && io.MouseClicked[0])
	{
		if (g.ActiveId != id)
		{
			// Start edition
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			*key = 0;
		}
		ImGui::SetActiveID(id, window);
		ImGui::FocusWindow(window);
	}
	else if (io.MouseClicked[0])
	{
		// Release focus when we click outside
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	if (g.ActiveId == id)
	{
		for (auto i = 0; i < 5; i++)
		{
			if (io.MouseDown[i])
			{
				switch (i)
				{
				case 0:
					k = 0x01;
					break;
				case 1:
					k = 0x02;
					break;
				case 2:
					k = 0x04;
					break;
				case 3:
					k = 0x05;
					break;
				case 4:
					k = 0x06;
					break;
				}
				value_changed = true;
				ImGui::ClearActiveID();
			}
		}
		if (!value_changed)
		{
			for (auto i = 0x08; i <= 0xA5; i++)
			{
				if (io.KeysDown[i])
				{
					k = i;
					value_changed = true;
					ImGui::ClearActiveID();
				}
			}
		}

		if (IsKeyPressedMap(ImGuiKey_Escape))
		{
			*key = 0;
			ImGui::ClearActiveID();
		}
		else
		{
			*key = k;
		}
	}

	if (hovered && g.IO.MouseClicked[1] || it_anim->second.active && (g.IO.MouseClicked[0] || g.IO.MouseClicked[1]) && !it_anim->second.hovered)
		it_anim->second.active = !it_anim->second.active;

	it_anim->second.alpha = ImClamp(it_anim->second.alpha + (8.f * g.IO.DeltaTime * (it_anim->second.active ? 1.f : -1.f)), 0.f, 1.f);

	ImVec4 i_bg = ImColor(87, 83, 78, 45);

	PushStyleColor(ImGuiCol_WindowBg, i_bg);
	// PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.f );

	if (BeginPopup(label) && it_anim->second.alpha >= 0.01f)
	{
		BeginGroup();
		{
			it_anim->second.hovered = IsWindowHovered();

			if (Selectable("Hold", *mode == 0))
			{
				*mode = 0;
				it_anim->second.active = false;
			}
			if (Selectable("Toggle", *mode == 1))
			{
				*mode = 1;
				it_anim->second.active = false;
			}
			if (Selectable("Always", *mode == 2))
			{
				*mode = 2;
				it_anim->second.active = false;
			}
		}
		EndGroup();

		EndPopup();
	}

	if (hovered && io.MouseClicked[1])
		OpenPopup(label);

	PopStyleColor();

	return value_changed;
}



void D::RectOutline(Vector_t start_pos, Vector_t end_pos, Color_t color, float thickness, float rounding)
{
	auto DrawList = ImGui::GetBackgroundDrawList();

	DrawList->AddRect(ImVec2(start_pos.x, start_pos.y), ImVec2(end_pos.x, end_pos.y), color.GetU32(), rounding, 0, thickness);
}

float D::CalculateDPI(const int nScaleTarget)
{
	switch ((EMiscDpiScale)nScaleTarget)
	{
	case EMiscDpiScale::MISC_DPISCALE_DEFAULT:
		return 1.f;
	case EMiscDpiScale::MISC_DPISCALE_125:
		return 1.25f;
	case EMiscDpiScale::MISC_DPISCALE_150:
		return 1.5f;
	case EMiscDpiScale::MISC_DPISCALE_175:
		return 1.75f;
	case EMiscDpiScale::MISC_DPISCALE_200:
		return 2.f;
	default:
		return 1.f;
	}
}

void D::AddDrawListRect(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colRect, const unsigned int uFlags, const Color_t& colOutline, const float flRounding, const ImDrawFlags roundingCorners, float flThickness, const float flOutlineThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	const ImU32 colRectPacked = colRect.GetU32();
	const ImU32 colOutlinePacked = colOutline.GetU32();

	if (uFlags & DRAW_RECT_FILLED)
		pDrawList->AddRectFilled(vecMin, vecMax, colRectPacked, flRounding, roundingCorners);
	else
	{
		pDrawList->AddRect(vecMin, vecMax, colRectPacked, flRounding, roundingCorners, flThickness);
		flThickness *= 0.5f;
	}

	const float flHalfOutlineThickness = flOutlineThickness * 0.5f;
	const ImVec2 vecThicknessOffset = { flThickness + flHalfOutlineThickness, flThickness + flHalfOutlineThickness };

	if (uFlags & DRAW_RECT_BORDER)
		pDrawList->AddRect(vecMin + vecThicknessOffset, vecMax - vecThicknessOffset, colOutlinePacked, flRounding, roundingCorners, flOutlineThickness);

	if (uFlags & DRAW_RECT_OUTLINE)
		pDrawList->AddRect(vecMin - vecThicknessOffset, vecMax + vecThicknessOffset, colOutlinePacked, flRounding, roundingCorners, flOutlineThickness);
}

void D::AddDrawListRectMultiColor(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colUpperLeft, const Color_t& colUpperRight, const Color_t& colBottomRight, const Color_t& colBottomLeft)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	pDrawList->AddRectFilledMultiColor(vecMin, vecMax, colUpperLeft.GetU32(), colUpperRight.GetU32(), colBottomRight.GetU32(), colBottomLeft.GetU32());
}

void D::AddDrawListCircle(ImDrawList* pDrawList, const ImVec2& vecCenter, const float flRadius, const Color_t& colCircle, const int nSegments, const unsigned int uFlags, const Color_t& colOutline, float flThickness, const float flOutlineThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	const ImU32 colCirclePacked = colCircle.GetU32();

	if (uFlags & DRAW_CIRCLE_FILLED)
	{
		pDrawList->AddCircleFilled(vecCenter, flRadius, colCirclePacked, nSegments);
		flThickness = 0.0f;
	}
	else
		pDrawList->AddCircle(vecCenter, flRadius, colCirclePacked, nSegments, flThickness);

	if (uFlags & DRAW_CIRCLE_OUTLINE)
		pDrawList->AddCircle(vecCenter, flRadius + flOutlineThickness, colOutline.GetU32(), nSegments, flThickness + flOutlineThickness);
}

void D::AddDrawListArc(ImDrawList* pDrawList, const ImVec2& vecPosition, const float flRadius, const float flMinimumAngle, const float flMaximumAngle, const Color_t& colArc, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	pDrawList->PathArcTo(vecPosition, flRadius, M_DEG2RAD(flMinimumAngle), M_DEG2RAD(flMaximumAngle), 32);
	pDrawList->PathStroke(colArc.GetU32(), false, flThickness);
}

void D::AddDrawListLine(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const Color_t& colLine, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	pDrawList->AddLine(vecFirst, vecSecond, colLine.GetU32(), flThickness);
}

void D::AddDrawListTriangle(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const ImVec2& vecThird, const Color_t& colTriangle, const unsigned int uFlags, const Color_t& colOutline, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	const ImU32 colTrianglePacked = colTriangle.GetU32();

	if (uFlags & DRAW_TRIANGLE_FILLED)
		pDrawList->AddTriangleFilled(vecFirst, vecSecond, vecThird, colTrianglePacked);
	else
		pDrawList->AddTriangle(vecFirst, vecSecond, vecThird, colTrianglePacked, flThickness);

	if (uFlags & DRAW_TRIANGLE_OUTLINE)
		pDrawList->AddTriangle(vecFirst, vecSecond, vecThird, colOutline.GetU32(), flThickness + 1.0f);
}

void D::AddDrawListQuad(ImDrawList* pDrawList, const ImVec2& vecFirst, const ImVec2& vecSecond, const ImVec2& vecThird, const ImVec2& vecFourth, const Color_t& colQuad, const unsigned int uFlags, const Color_t& colOutline, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	const ImU32 colQuadPacked = colQuad.GetU32();

	if (uFlags & DRAW_QUAD_FILLED)
		pDrawList->AddQuadFilled(vecFirst, vecSecond, vecThird, vecFourth, colQuadPacked);
	else
		pDrawList->AddQuad(vecFirst, vecSecond, vecThird, vecFourth, colQuadPacked, flThickness);

	if (uFlags & DRAW_QUAD_OUTLINE)
		pDrawList->AddQuad(vecFirst, vecSecond, vecThird, vecFourth, colOutline.GetU32(), flThickness + 1.0f);
}

void D::AddDrawListPolygon(ImDrawList* pDrawList, const ImVec2* vecPoints, const int nPointsCount, const Color_t& colPolygon, unsigned int uFlags, const Color_t& colOutline, const bool bClosed, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	const ImU32 colPolygonPacked = colPolygon.GetU32();

	if (uFlags & DRAW_POLYGON_FILLED)
		pDrawList->AddConvexPolyFilled(vecPoints, nPointsCount, colPolygonPacked);
	else
		pDrawList->AddPolyline(vecPoints, nPointsCount, colPolygonPacked, bClosed, flThickness);

	if (uFlags & DRAW_POLYGON_OUTLINE)
		pDrawList->AddPolyline(vecPoints, nPointsCount, colOutline.GetU32(), bClosed, flThickness + 1.0f);
}

void D::AddDrawListText(ImDrawList* pDrawList, const ImFont* pFont, const ImVec2& vecPosition, const char* szText, const Color_t& colText, const unsigned int uFlags, const Color_t& colOutline, const float flThickness)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	// set font texture
	pDrawList->PushTextureID(pFont->ContainerAtlas->TexID);

	const ImU32 colOutlinePacked = colOutline.GetU32();

	if (uFlags & DRAW_TEXT_DROPSHADOW)
		pDrawList->AddText(pFont, pFont->FontSize, vecPosition + ImVec2(flThickness, flThickness), colOutlinePacked, szText);
	else if (uFlags & DRAW_TEXT_OUTLINE)
	{
		pDrawList->AddText(pFont, pFont->FontSize, vecPosition + ImVec2(flThickness, -flThickness), colOutlinePacked, szText);
		pDrawList->AddText(pFont, pFont->FontSize, vecPosition + ImVec2(-flThickness, flThickness), colOutlinePacked, szText);
	}

	pDrawList->AddText(pFont, pFont->FontSize, vecPosition, colText.GetU32(), szText);
	pDrawList->PopTextureID();
}

void D::AddDrawListShadowRect(ImDrawList* pDrawList, const ImVec2& vecMin, const ImVec2& vecMax, const Color_t& colShadow, float flThickness, float flRounding, ImDrawFlags roundingCorners)
{
	if (pDrawList == nullptr)
		pDrawList = pDrawListActive;

	pDrawList->AddShadowRect(vecMin, vecMax, colShadow.GetU32(), flThickness, ImVec2(0, 0), roundingCorners, flRounding);
}

#pragma endregion

#pragma region draw_structures

void AnimationHandler_t::Update(const float flDeltaTime, const float flDuration)
{
	if (fnEaseIn == nullptr)
		fnEaseIn = &EASING::InSine;

	if (fnEaseOut == nullptr)
		fnEaseOut = &EASING::OutSine;

	// Reset the elapsed time if the bool switches
	if (bSwitch != bLastSwitch)
		flElapsedTime = 0;

	flElapsedTime = MATH::Max(0.0f, MATH::Min(flElapsedTime, flDuration));
	float flTime = flElapsedTime / flDuration;

	// Determine the initial and target value based on the current state
	float flInitialValue = bSwitch ? 0.1f : flValue;
	float flTargetValue = bSwitch ? 1.0f : 0.1f; /*(1.0f is max value)*/

	// Select the appropriate easing function based on the current state
	EasingFunction_t fnCurrentEase = bSwitch ? fnEaseIn : fnEaseOut;

	// Apply the appropriate easing function based on fade-in or fade-out (with lerping, which is basically what's the math were doing)
	flValue = (flInitialValue + (flTargetValue - flInitialValue)) * (float)fnCurrentEase(flTime);
	flValue = MATH::Clamp(flValue, 0.1f, 1.0f);

	flElapsedTime += flDeltaTime;
	bLastSwitch = bSwitch;
}

static const char* notif_name;
static const char* notif_icon;
static int notif_state;
static float notif_rotate;

static float notif_offset;

static float notif_timer;

static float notif_width;

static void Message(const char* name, const char* icon)
{
	notif_name = name;
	notif_icon = icon;
	notif_state = 0;
	notif_rotate = 0;
	notif_offset = 150;
	notif_timer = 0;
}

#pragma endregion
