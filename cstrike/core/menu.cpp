#include "menu.h"

// used: config variables
#include "variables.h"
// used: entity stuff for skinchanger etc
#include "../cstrike/sdk/entity.h"
// used: iinputsystem
#include "interfaces.h"
#include "../sdk/interfaces/iengineclient.h"
#include "../sdk/interfaces/inetworkclientservice.h"
#include "../InvecntoryChanger.h"
#include "../sdk/interfaces/iglobalvars.h"
#include "../sdk/interfaces/ienginecvar.h"
#include "../sdk/datatypes/utlmap.h"
#include "../utilities/inputsystem.h"
// used: overlay's context
#include "../features/visuals/overlay.h"
#include "../Spectator.h"
// used: notifications
#include "../resources/font_awesome_5.h"
#include "../utilities/notify.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "imgui/imgui_edited.hpp"
#include "../skindata.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#pragma region menu_array_entries
static void RenderInventoryWindow();
int page = 0;
float tab_alpha = 0.f;
float tab_add = 0.f;
int active_tab = 0;
static constexpr const char* arrMiscDpiScale[] = {
	"100%",
	"125%",
	"150%",
	"175%",
	"200%"
};
int subtab;
static const std::pair<const char*, const std::size_t> arrColors[] = {
	{ "[accent] - main", Vars.colAccent0 },
	{ "[accent] - dark (hover)", Vars.colAccent1 },
	{ "[accent] - darker (active)", Vars.colAccent2 },
	{ "[primitive] - text", Vars.colPrimtv0 },
	{ "[primitive] - background", Vars.colPrimtv1 },
	{ "[primitive] - disabled", Vars.colPrimtv2 },
	{ "[primitive] - frame background", Vars.colPrimtv3 },
	{ "[primitive] - border", Vars.colPrimtv4 },
};

static constexpr const char* arrMenuAddition[] = {
	"dim",
	"particle",
	"glow"
};
static constexpr const char* arrEspFlags[] = {
	"Armor",
	"KIT"
};
static constexpr const char* arrLegitCond[] = {
	"In air",
	"Flashed",
	"Thru smoke",
	"Delay on kill"
};
static constexpr const char* arrMovementStrafer[] = {
	"Adjust mouse",
	"Directional"
};

static const char* hitboxNames[] = {
	"Head", "Neck", "Chest",
	"Stomach", "Left Arm", "Right Arm",
	"Left Leg", "Right Leg", "Left Foot", "Right Foot"
};

static const std::unordered_map<std::string, EHitBoxes> hitboxNameToEnum = {
	{ "Head", EHitBoxes::HITBOX_HEAD },
	{ "Neck", EHitBoxes::HITBOX_NECK },
	{ "Chest", EHitBoxes::HITBOX_CHEST },
	{ "Stomach", EHitBoxes::HITBOX_STOMACH },
	{ "Left Arm", EHitBoxes::HITBOX_LEFT_FOREARM },
	{ "Right Arm", EHitBoxes::HITBOX_RIGHT_FOREARM },
	{ "Left Leg", EHitBoxes::HITBOX_LEFT_THIGH },
	{ "Right Leg", EHitBoxes::HITBOX_RIGHT_THIGH },
	{ "Left Foot", EHitBoxes::HITBOX_LEFT_FOOT },
	{ "Right Foot", EHitBoxes::HITBOX_RIGHT_FOOT }
};

static constexpr const char* arrRemovalsNames[] = {
	"SMOKE",
	"LEGS",
	"TEAM INTRO",
	"FOG",
	/*"3DSky",*/
	"Shake",
	"Flash"
};

bool info_bar = true;

enum TAB : int {
	rage = 0,
	legit = 1,
	visuals = 2,
	misc = 3,
	skinchanger = 4,
	cloud = 5,
	scripting = 6,
};

enum SUBTAB : int {
	first = 0,
	seccond = 1,
	third = 4,
	fifth = 5,
};

#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*_ARR)))       // Size of a static C-style array. Don't use on pointers!

// Function to extract the unique identifier from the itemBaseName
std::string ExtractIdentifier(const std::string& itemBaseName, const std::string& modelName) {
	// Find the position of the modelName
	size_t modelPos = itemBaseName.find(modelName);

	// If modelName is found, extract the substring after it
	if (modelPos != std::string::npos) {
		// Find the next "/"
		size_t nextSlashPos = itemBaseName.find("/", modelPos + modelName.length());

		// Extract the substring after modelName until the next "/"
		return itemBaseName.substr(modelPos + modelName.length(), nextSlashPos - (modelPos + modelName.length()));
	}

	// If modelName is not found, return an empty string
	return "";
}
ImTextureID CreateTextureFromMemory(void* imageData, int width, int height) {
	ID3D11Texture2D* pTexture = nullptr;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = imageData;
	initData.SysMemPitch = width * 4; // Assuming 4 channels (R8G8B8A8)

	if (FAILED(I::Device->CreateTexture2D(&desc, &initData, &pTexture))) {
		// Handle creation failure
		return 0;
	}

	return (ImTextureID)pTexture;
}

void CricleProgress(const char* name, float progress, float max, float radius, const ImVec2& size)
{
	const float tickness = 3.f;
	static float position = 0.f;

	position = progress / max * 6.28f;

	ImGui::GetForegroundDrawList()->PathClear();
	ImGui::GetForegroundDrawList()->PathArcTo(ImGui::GetCursorScreenPos() + size, radius, 0.f, 2.f * IM_PI, 120.f);
	ImGui::GetForegroundDrawList()->PathStroke(ImGui::GetColorU32(c::elements::background_widget), 0, tickness);

	ImGui::GetForegroundDrawList()->PathClear();
	ImGui::GetForegroundDrawList()->PathArcTo(ImGui::GetCursorScreenPos() + size, radius, IM_PI * 1.5f, IM_PI * 1.5f + position, 120.f);
	ImGui::GetForegroundDrawList()->PathStroke(ImGui::GetColorU32(c::accent), 0, tickness);

	int procent = progress / (int)max * 100;

	std::string procent_str = std::to_string(procent) + "%";

}

static int notify_select = 0;
const char* notify_items[2]{ "Circle", "Line" };

struct Notification {
	int id;
	std::string message;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point endTime;
};

class NotificationSystem {
public:
	NotificationSystem() : notificationIdCounter(0) {}

	void AddNotification(const std::string& message, int durationMs) {
		auto now = std::chrono::steady_clock::now();
		auto endTime = now + std::chrono::milliseconds(durationMs);
		notifications.push_back({ notificationIdCounter++, message, now, endTime });
	}

	void DrawNotifications() {
		auto now = std::chrono::steady_clock::now();

		std::sort(notifications.begin(), notifications.end(),
			[now](const Notification& a, const Notification& b) -> bool {
				float durationA = std::chrono::duration_cast<std::chrono::milliseconds>(a.endTime - a.startTime).count();
				float elapsedA = std::chrono::duration_cast<std::chrono::milliseconds>(now - a.startTime).count();
				float percentageA = (durationA - elapsedA) / durationA;

				float durationB = std::chrono::duration_cast<std::chrono::milliseconds>(b.endTime - b.startTime).count();
				float elapsedB = std::chrono::duration_cast<std::chrono::milliseconds>(now - b.startTime).count();
				float percentageB = (durationB - elapsedB) / durationB;

				return percentageA < percentageB;
			}
		);

		int currentNotificationPosition = 0;

		for (auto& notification : notifications) {
			if (now < notification.endTime) {
				float duration = std::chrono::duration_cast<std::chrono::milliseconds>(notification.endTime - notification.startTime).count();
				float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - notification.startTime).count();
				float percentage = (duration - elapsed) / duration * 100.0f;

				ShowNotification(currentNotificationPosition, notification.message, percentage);
				currentNotificationPosition++;
			}
		}

		notifications.erase(std::remove_if(notifications.begin(), notifications.end(),
			[now](const Notification& notification) { return now >= notification.endTime; }),
			notifications.end());
	}

private:
	std::vector<Notification> notifications;
	int notificationIdCounter;

	void ShowNotification(int position, const std::string& message, float percentage) {

		float duePercentage = 100.0f - percentage;
		float alpha = percentage > 10.0f ? 1.0f : percentage / 10.0f;
		ImGui::GetStyle().WindowPadding = ImVec2(15, 10);

		ImGui::SetNextWindowPos(ImVec2(duePercentage < 15.f ? duePercentage : 15.f, 15 + position * 90));

		ImGui::Begin(("##NOTIFY" + std::to_string(position)).c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

		ImVec2 pos = ImGui::GetWindowPos(), spacing = ImGui::GetStyle().ItemSpacing, region = ImGui::GetContentRegionMax();

		ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(pos, pos + region, ImGui::VGetColorU32(c::background::filling, alpha), ImGui::VGetColorU32(c::accent, 0.01f), ImGui::VGetColorU32(c::accent, 0.01f), ImGui::VGetColorU32(c::background::filling, alpha));
		ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + region, ImGui::VGetColorU32(c::background::filling, 0.4f), c::elements::rounding);
		ImGui::GetBackgroundDrawList()->AddRect(pos, pos + region, ImGui::VGetColorU32(c::background::stroke, alpha), c::elements::rounding);

		if (notify_select == 0)
			CricleProgress("##NOTIFY", percentage, 100, 7.f, ImVec2(ImGui::GetContentRegionMax().x - 40, 11));
		if (notify_select == 1)
			ImGui::GetBackgroundDrawList()->AddRectFilled(pos + ImVec2(0, region.y - 3), pos + ImVec2(region.x * (duePercentage / 100.0f), region.y), ImGui::VGetColorU32(c::accent, alpha), c::elements::rounding);

		ImGui::PushFont(font::lexend_bold);
		ImGui::TextColored(ImColor(ImGui::VGetColorU32(c::accent, alpha)), "%s", "[Notification]");
		ImGui::TextColored(ImColor(ImGui::VGetColorU32(c::elements::text_active, alpha)), "%s", message.c_str());
		ImGui::Dummy(ImVec2(ImGui::CalcTextSize(message.c_str()).x + 15, 5));
		ImGui::PopFont();

		ImGui::End();
	}
};

NotificationSystem notificationSystem;


//
//void spec1()
//{
//	if (!C_GET(bool, Vars.bSpectatorList))
//		return;
//
//	std::vector<std::string> spects;
//	ImGui::Begin(CS_XOR("Spectators"), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
//	{
//		static bool keybinds[5], spectating[5];
//		const char* names[5] = { "Jonh", "Oscar", "Kate", "Insultik", "Zavodnoy krokodil" };
//		for (int i = 0; i < 5; i++)
//		{
//			if (spectating[i])
//				spects.push_back(names[i]);
//		}
//		static int current_size; int max_size; max_size = 30 + 20 * spects.size();
//		if (current_size < max_size)
//			current_size++;
//		else if (current_size > max_size)
//			current_size--;
//		ImGui::SetWindowSize(ImVec2(160, current_size));
//		int spacing = 0;
//		ImGui::SetCursorPosY(35);
//		ImGui::BeginGroup();
//		ImGui::PushFont(FONT::pExtra);
//		for (int i = 0; i < spects.size(); i++)
//		{
//			ImGui::Text(spects.at(i).c_str());
//		}
//		ImGui::PopFont();
//		ImGui::EndGroup();
//	}
//	ImGui::End();
//}

//void MENU::spectators() {
//	std::vector <c_specs> spectators = F::MISC::SPECTATORLIST::Run();
//
//	if (!(C_GET(bool, Vars.bSpectatorList) && (MENU::bMainWindowOpened || spectators.size() > 0)))
//		return;
//
//	ImGui::SetNextWindowSize({ 200, 200 });
//	ImGui::Begin(CS_XOR("Spectator"), NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
//
//
//	for (auto it = spectators.begin(); it != spectators.end(); it++)
//	{
//		auto steam_id = it->steam_id;
//
//		if (std::find_if(specs_.begin(), specs_.end(), [&](const auto& x) { return x.steam_id == steam_id; }) != specs_.end())
//			continue;
//
//		specs_.push_back(*it);
//	}
//
//	auto position = ImGui::GetWindowPos();
//	std::string name = CS_XOR("Spectator");
//	//auto calc_text = g_render->get_font(0)->calc_text_size(13.f, FLT_MAX, FLT_MAX, name.c_str());
//	//auto center = position + (ImGui::GetWindowSizeNew() / 2) - (calc_text / 2);
//
//	float size_y = 25 * std::count_if(specs_.begin(), specs_.end(), [](const auto& x) {
//		return x.m_show;
//		});
//
//	animation_lerp[FNV1A::HashConst("spectators_animation_y")] = std::lerp(animation_lerp[FNV1A::HashConst("spectators_animation_y")], size_y, 0.1f);
//	auto draw = ImGui::GetWindowDrawList();
//
//	draw->AddRectFilled(position, ImGui::GetWindowSize(), 20, 20, 20, 0, 0, 0, 5.f, render_rounding_flags_::ROUND_RECT_TOP);
//	draw->AddRectFilled(position, ImGui::GetWindowSize(), 13, 13, 14, 0, 5.f, render_rounding_flags_::ROUND_RECT_TOP);
//	draw->AddText("LUNOWARE", position + (ImGui::GetWindowSize() / 2) - (render_size(58 * 0.65f, 25 * 0.65f) / 2), render_color::Orange(), g_render->get_font(0), 17.f);
//	//g_render->add_texture(position + (ImGui::GetWindowSizeNew() / 2) - (render_size(58 * 0.65f, 25 * 0.65f) / 2), render_size(58 * 0.65f, 25 * 0.65f), g_gui->m_texture[1]);
//
//	g_render->set_limit(position + render_position(0, 35), render_size(ImGui::GetWindowSizeNew().x, animation_lerp[FNV1A::HashConst("spectators_animation_y")]));
//	g_render->rect_filled(position + render_position(0, 35), render_size(ImGui::GetWindowSizeNew().x, animation_lerp[FNV1A::HashConst("spectators_animation_y")]), render_color::black().override_alpha(0.4f), 0, 3.f, render_rounding_flags_::ROUND_RECT_BOT);
//
//	position.y += 35;
//
//	float high_size = 200.f;
//	float temp_size = 0.f;
//
//	for (auto it = specs_.begin(); it != specs_.end();)
//	{
//		if (!it->m_show && it->m_animation.base <= 0.f)
//		{
//			it = specs_.erase(it);
//			continue;
//		}
//
//		it->m_animation.run(it->m_show);
//		it->m_show = std::find_if(spectators.begin(), spectators.end(), [&](const auto& x) { return x.steam_id == it->steam_id; }) != spectators.end();
//
//		auto calc_text = g_render->get_font(0)->calc_text_size(11.f, FLT_MAX, FLT_MAX, it->name.c_str());
//
//		auto size_window = 200.f + (calc_text.x - 35);
//
//		if (size_window > high_size)
//			high_size = std::max(size_window, 200.f);
//
//
//		g_render->add_texture_circle(position + render_position(7, 2), 10, it->texture_id);
//		g_render->add_text(it->name, position + render_position(32, 5), render_color::white().override_alpha(it->m_animation.base), g_render->get_font(0), 11.f);
//		position.y += 25;
//
//		it++;
//	}
//
//	g_render->get_buffer()->pop_clip_rect();
//
//	float animation_lerp = ImGui::GetWindowSizeNew().x;
//
//	animation_lerp = std::lerp(ImGui::GetWindowSizeNew().x, high_size, 0.1f);
//
//	ImGui::SetWindowSize(ImVec2(animation_lerp, ImGui::GetWindowSizeNew().y));
//
//	ImGui::End();
//}

bool parseSkinDataFromFile(const std::string& filePath, std::vector<SkinData>& skinList) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		L_PRINT(LOG_ERROR) << "Failed to open the file!";
		return false;
	}

	SkinData currentSkin = {};
	std::string line;

	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '/' || line.find_first_not_of(" \t") == std::string::npos)
			continue;

		std::istringstream iss(line);
		std::string key, value;
		bool bSetSkin = false;

		if (std::getline(iss, key, ':') && std::getline(iss, value)) {
			key = key.substr(0, key.find_last_not_of(" \t") + 1);
			value = value.substr(value.find_first_not_of(" \t"));

			if (key == "Weapon ID")
				currentSkin.weaponID = std::stoi(value);
			if (key == "Quality")
				currentSkin.quality = std::stoi(value);
			if (key == "Rarity")
				currentSkin.rarity = std::stoi(value);
			if (key == "Skin ID")
				currentSkin.skinID = std::stoi(value);
			if (key == "Paint Seed")
				currentSkin.paintSeed = std::stoi(value);
			if (key == "Wear") {
				currentSkin.wear = std::stof(value);
				bSetSkin = true;
			}

		}

		if (bSetSkin) {
			skinList.push_back(currentSkin);
			currentSkin = {};
		}
	}

	file.close();
	return !skinList.empty();
}
void addSkinFromData(const SkinData& skinData) {
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	CEconItem* pItem = CEconItem::CreateInstance();
	if (!pItem)
		return;

	auto highestIDs = pInventory->GetHighestIDs();

	pItem->m_ulID = highestIDs.first + 1;
	pItem->m_unInventory = highestIDs.second + 1;
	pItem->m_unAccountID = uint32_t(pInventory->GetOwner().m_id);
	pItem->m_unDefIndex = skinData.weaponID;
	pItem->m_nQuality = skinData.quality;
	pItem->m_nRarity = skinData.rarity;

	pItem->SetPaintKit(static_cast<float>(skinData.skinID));
	pItem->SetPaintSeed(static_cast<int32_t>(skinData.paintSeed)); // Paint Seed передаётся напрямую
	pItem->SetPaintWear(skinData.wear);

	L_PRINT(LOG_NONE) << "[DEBUG] Adding skin to inventory: Weapon ID = " << skinData.weaponID
		<< ", Paint Seed = " << skinData.paintSeed
		<< ", Wear = " << skinData.wear;

	if (pInventory->AddEconItem(pItem)) {
		skin_changer::AddEconItemToList(pItem);
	}
}

void logSkinDetailsToFile(int weaponID, int quality, int rarity, int skinID, int paintSeed, float wear) {
	if (skinID <= 0) {
		std::cerr << "Invalid skin ID for logging: " << skinID << std::endl;
		return;
	}

	std::string path = R"(C:\\UNDERAGER_cfgs\\skins.txt)";
	std::ofstream outFile(path, std::ios::app);

	if (!outFile.is_open()) {
		L_PRINT(LOG_ERROR) << "Failed to open log file: ";
		return;
	}

	outFile << "Weapon ID: " << weaponID << "\n";
	outFile << "Quality: " << quality << "\n";
	outFile << "Rarity: " << rarity << "\n";
	outFile << "Skin ID: " << skinID << "\n";
	outFile << "Paint Seed: " << paintSeed << "\n";
	outFile << "Wear: " << wear << "\n\n";

	outFile.close();
}

void addSkin() {
	std::vector<SkinData> skinList;
	std::string filePath = R"(C:\\UNDERAGER_cfgs\\skins.txt)";

	if (parseSkinDataFromFile(filePath, skinList)) {
		for (const auto& skin : skinList) {
			addSkinFromData(skin);
		}
		L_PRINT(LOG_INFO) << "All skins have been added successfully.\n";
	}
	else {
		L_PRINT(LOG_ERROR) << "Failed to load skins from file.\n";
	}
}

void MENU::Hotkeys()
{
	if (C_GET(bool, Vars.bHotkeys))
	{
		ImGui::SetNextWindowBgAlpha(0.f);
		ImGui::SetNextWindowSize(ImVec2(140, 125));
		ImGui::Begin("Hotkeys", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
		{
			auto background_list = ImGui::GetBackgroundDrawList();
			auto draw_list = ImGui::GetWindowDrawList();
			ImVec2 xy = ImGui::GetWindowPos();

			ImVec2 rect_min = ImGui::GetCursorScreenPos(); // Начальная позиция
			ImVec2 rect_max = ImVec2(rect_min.x + 140, rect_min.y + 30); // Конечная позиция

			// Устанавливаем радиус закругления
			float rounding = 7.0f; // Радиус закругления углов

			// Рисуем заполненный прямоугольник с закругленными углами
			draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(17, 17, 18, 255), rounding); // Черно-серый цвет (RGBA)
			background_list->AddShadowRect(xy, ImVec2(xy.x + 140, xy.y + 30), ImColor(26, 28, 96), 100.f, ImVec2(0, 0)); // Glow effect

			// Рисуем текст, сдвинув его еще на 2 пикселя вправо
			draw_list->AddText(FONT::pMain, 15.f, ImVec2(xy.x + 47, xy.y + 8), ImColor(255, 255, 255, 255), "Hotkeys"); // Name

			ImGui::Dummy(ImVec2(10.f, 0.f));ImGui::Dummy(ImVec2(10.f, 0.f));ImGui::Dummy(ImVec2(10.f, 0.f));ImGui::Dummy(ImVec2(10.f, 0.f));

			if (IPT::IsPressed(C_GET(int, Vars.bKeyMinDamage), C_GET(int, Vars.bKeyBindMinDamage)))
			{
				ImGui::Text("MinDamage [ON]");
			};

			if (IPT::IsPressed(C_GET(int, Vars.bKeyNoSpread), C_GET(int, Vars.bKeyBindNoSpread)))
			{
				ImGui::Text("NoSpread [ON]");
			};

			if (IPT::IsPressed(C_GET(int, Vars.bKeyHitchance), C_GET(int, Vars.bKeyBindHitchance)))
			{
				ImGui::Text("Hitchance [ON]");
			};

			if (IPT::IsPressed(C_GET(int, Vars.nLegitbotActivationKeysoso), C_GET(int, Vars.nLegitbotActivationKey)))
			{
				ImGui::Text("Legit aim [ON]");
			};

			if (IPT::IsPressed(C_GET(int, Vars.iThirdKey), C_GET(int, Vars.iThirdKeyBind)))
			{
				ImGui::Text("Thirdperson [ON]");
			};
		
		}
		ImGui::End();
	}
};

void MENU::menu()
{
	static constexpr float windowWidth = 540.f;

	struct DumpedSkin_t
	{
		std::string m_name = "";
		int m_ID = 0;
		int m_rarity = 0;
	};
	struct DumpedItem_t
	{
		std::string m_name = "";
		uint16_t m_defIdx = 0;
		int m_rarity = 0;
		void* m_image = nullptr;
		ImTextureID m_textureID = nullptr;
		bool m_unusualItem = false;
		std::vector<DumpedSkin_t> m_dumpedSkins{};
		DumpedSkin_t* pSelectedSkin = nullptr;
	};

	static std::vector<DumpedItem_t> vecDumpedItems;
	static DumpedItem_t* pSelectedItem = nullptr;

	CEconItemSchema* pItemSchema =
		I::Client->GetEconItemSystem()->GetEconItemSchema();



	// Render the ImGui draw data using the DirectX 11 blur shader
	//blurShader.Render(drawData);
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	// @test: we should always update the animation?
	animMenuDimBackground.Update(io.DeltaTime, style.AnimationSpeed);
	if (!bMainWindowOpened)
		return;

	const ImVec2 vecScreenSize = io.DisplaySize;
	flDpiScale = 1.50f;

	// @note: we call this every frame because we utilizing rainbow color as well! however it's not really performance friendly?
	UpdateStyle(&style);

	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing = ImVec2(10 * D::CalculateDPI(C_GET(int, Vars.nDpiScale)), 10 * D::CalculateDPI(C_GET(int, Vars.nDpiScale)));
	style.WindowBorderSize = 0;
	style.ScrollbarSize = 3.f * D::CalculateDPI(C_GET(int, Vars.nDpiScale));
	c::accent = C_GET(Color_t, Vars.color).GetVec4(1.f);

	ImGui::SetNextWindowSize(c::background::size* D::CalculateDPI(C_GET(int, Vars.nDpiScale)));

	// render main window
	ImGui::Begin(CS_XOR("t.me/LUNOWARECS2"), &bMainWindowOpened, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
	{
		const ImVec2& pos = ImGui::GetWindowPos();
		const ImVec2& region = ImGui::GetContentRegionMax();
		const ImVec2& spacing = style.ItemSpacing;

		ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + c::background::size * D::CalculateDPI(C_GET(int, Vars.nDpiScale)), ImGui::GetColorU32(c::background::filling), c::background::rounding);
		ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + ImVec2(200.f * D::CalculateDPI(C_GET(int, Vars.nDpiScale)), c::background::size.y * D::CalculateDPI(C_GET(int, Vars.nDpiScale))), ImGui::GetColorU32(c::tab::border), c::background::rounding, ImDrawFlags_RoundCornersLeft);
		ImGui::GetBackgroundDrawList()->AddLine(pos + ImVec2(200.f * D::CalculateDPI(C_GET(int, Vars.nDpiScale)), 0.f), pos + ImVec2(200.f, c::background::size.y * D::CalculateDPI(C_GET(int, Vars.nDpiScale))), ImGui::GetColorU32(c::background::stroke), 1.f);

		ImGui::GetBackgroundDrawList()->AddRect(pos, pos + c::background::size * D::CalculateDPI(C_GET(int, Vars.nDpiScale)), ImGui::GetColorU32(c::background::stroke), c::background::rounding);

		ImGui::SetCursorPos({ 5, 10 });
		ImGui::BeginGroup();
		{
			std::vector<std::vector<std::string>> tab_columns = {
				{ "c", "b", "f", "b", "o", "e" },
				{ "Aimbot", "Antiaim", "Visuals", "Chams", "Skins", "Misc" },
				{ "Legit aims and agressively at targets...",  "Accuracy assistance...", "Visualisation", "Chams visualisation", "Items customization...", "Save/Load configs, engine..." },
				{ "Have you switched to the Aimbot tab? You're just crazy!", "Have you switched to the Visuals tab, do you want to get banned?", "You switched to the skins tab, why do you need self-deception??", "You switched to the skins tab, why do you need self-deception??", "You switched over.. And yes, to hell with it, come up with a script yourself.", "You switched over.. And yes, to hell with it, come up with a script yourself." }
			};

			const int num_tabs = tab_columns[0].size();

			for (int i = 0; i < num_tabs; ++i)
				if (edited::Tab(page == i, tab_columns[0][i].c_str(), tab_columns[1][i].c_str(), tab_columns[2][i].c_str(), ImVec2(180, 50))) {
					page = i;

					notificationSystem.AddNotification(tab_columns[3][i], 1000);
				}
		}
		ImGui::EndGroup();

		tab_alpha = ImLerp(tab_alpha, (page == active_tab) ? 1.f : 0.f, 15.f * ImGui::GetIO().DeltaTime);
		if (tab_alpha < 0.01f && tab_add < 0.01f) active_tab = page;

		ImGui::SetCursorPos(ImVec2(200, 100 - (tab_alpha * 100)));
		auto current_weapon = C_GET(int, Vars.rage_weapon_selection);

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style.Alpha);
		{
			if (active_tab == 0)
			{
				edited::BeginChild(CS_XOR("##Container0"), ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), CS_XOR("Ragebot"));

					auto current_weapon = C_GET(int, Vars.rage_weapon_selection);
					edited::Checkbox(CS_XOR("Rage"), CS_XOR("Rage aimbot for HvH"), &C_GET(bool, Vars.rage_enable));
					ImGui::BeginDisabled(!C_GET(bool, Vars.rage_enable));
					{
						//ImGui::MultiCombo(CS_XOR("Select Hitbox"), &C_GET(unsigned int, Vars.iCurHitbox), Bones, IM_ARRAYSIZE(Bones));
						edited::Checkbox(CS_XOR("Auto Fire"), CS_XOR("The function shoots for you"), &C_GET(bool, Vars.bAutoFire));
						/*elements::Checkbox(CS_XOR("PSilent"), &C_GET(bool, Vars.bSilent));*/
						edited::Keybind(CS_XOR("NoSpread key"), CS_XOR("The key that NoSpread will work"), &C_GET(int, Vars.bKeyNoSpread), &C_GET(int, Vars.bKeyBindNoSpread));
						edited::Checkbox(CS_XOR("AutoScope"), CS_XOR("The function will scoped for you"), &C_GET(bool, Vars.bAutoScope));
						edited::Checkbox(CS_XOR("AutoStop"), CS_XOR("The function will stoped for you"), &C_GET(bool, Vars.bAutoStop));
						edited::Checkbox(CS_XOR("PredictLethal"), CS_XOR("The function will enable predict"), &C_GET(bool, Vars.bPredictLethal));
						ImGui::BeginDisabled(!C_GET(bool, Vars.bPredictLethal));
						{
							edited::Keybind(CS_XOR("Predict key"), CS_XOR("The key that predictlethal will work"), &C_GET(int, Vars.bKeyPredict), &C_GET(int, Vars.bKeyBindPredict));
						}
						ImGui::EndDisabled();
						edited::Checkbox(CS_XOR("PerfectSilent"), CS_XOR("Its exploit for beautiful ragebot"), &C_GET(bool, Vars.bPerfectSilent));
						if (edited::Combo(CS_XOR("Hitbox"), CS_XOR("Choose hitbox for aim"), &C_GET(int, Vars.iHitbox), hitboxNames, CS_ARRAYSIZE(hitboxNames)))
						{
							EHitBoxes selectedHitbox = hitboxNameToEnum.at(hitboxNames[C_GET(int, Vars.iHitbox)]);
							if (pointscan_config.find(selectedHitbox) == pointscan_config.end()) {
								pointscan_config[selectedHitbox] = {};
							}
						}

						edited::SliderInt(CS_XOR("MinDamage"), CS_XOR("If the enemy takes less damage than you chose, then the cheat will not fire."), &C_GET(int, Vars.rage_minimum_damage), 0, 100);
						edited::Keybind(CS_XOR("MinDamage key"), CS_XOR("The key that mindamage will work"), &C_GET(int, Vars.bKeyMinDamage), &C_GET(int, Vars.bKeyBindMinDamage));
						edited::SliderInt(CS_XOR("HitChance"), CS_XOR("The chance of hitting the enemy"), &C_GET(int, Vars.rage_minimum_hitchance), 0, 100);
						edited::Keybind(CS_XOR("HitChance key"), CS_XOR("The key that hitchance will work"), &C_GET(int, Vars.bKeyHitchance), &C_GET(int, Vars.bKeyBindHitchance));
						edited::Combo(CS_XOR("Select Target"), CS_XOR("What will ragebot shoot through"), &C_GET(int, Vars.iSelectTarget), new const char* [] {"Damage", "Distance (world)", "Distande (crosshair)", "Hitchance"}, 4);
						edited::Combo(CS_XOR("Scan Type"), CS_XOR("How will ragebot search for an opponent"), &C_GET(int, Vars.iScanType), new const char* [] {"Strict", "Adaptive"}, 2);
						edited::Combo(CS_XOR("AutoStop Mode"), CS_XOR("Fullstop - for nospread, Dynamic - for normal use"), &C_GET(int, Vars.iAutoStopMode), new const char* [] {"Fullstop", "Dynamic"}, 2);
						if (C_GET(int, Vars.iScanType) == mindamage_type::adaptive_scan)
							edited::SliderInt(CS_XOR("Lethal Modifer"), CS_XOR("???????"), &C_GET(int, Vars.iLethalModifer), 0, 20);
						EHitBoxes currentHitbox = hitboxNameToEnum.at(hitboxNames[C_GET(int, Vars.iHitbox)]);
						auto& multipointSets = pointscan_config[currentHitbox];

						if (edited::Button("Add Multipoint Set", ImVec2(100, 25), 0))
						{
							multipointSets.push_back(multipoint_set{});
						}

						for (int i = 0; i < multipointSets.size(); ++i) {
							ImGui::PushID(i);
							ImGui::BeginChild(("set_" + std::to_string(i)).c_str(), ImVec2(0, 160), true);

							multipoint_set& set = multipointSets[i];
							ImGui::Columns(3);

							edited::Checkbox("Top Left", CS_XOR(""), &C_GET(bool, Vars.btop_left));
							edited::Checkbox("Left", CS_XOR(""), &set.left);
							edited::Checkbox("Bottom Left", CS_XOR(""), &set.bottom_left);

							ImGui::NextColumn();

							edited::Checkbox("Top", CS_XOR(""), &set.top);
							edited::Checkbox("Middle", CS_XOR(""), &C_GET(bool, Vars.bmiddle));
							edited::Checkbox("Bottom", CS_XOR(""), &set.bottom);
							ImGui::NextColumn();

							edited::Checkbox("Top Right", CS_XOR(""), &set.top_right);
							edited::Checkbox("Right", CS_XOR(""), &set.right);
							edited::Checkbox("Bottom Right", CS_XOR(""), &C_GET(bool, Vars.bbottom_right));


							ImGui::Columns(1);
							edited::SliderFloat("Pointscale", CS_XOR(""), &set.pointscale, 1.0f, 100.0f, "%.2f");

							if (edited::Button("Remove Set", ImVec2(100, 25), 0)) {
								multipointSets.erase(multipointSets.begin() + i);
								i--;
							}

							ImGui::EndChild();
							ImGui::PopID();
						}
					}
					ImGui::EndDisabled();

				}
				edited::EndChild();
				ImGui::SameLine(0, 0);

				edited::BeginChild("##Container1", ImVec2((c::background::size.x - 200 * dpi) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Legitbot");

					edited::Checkbox(CS_XOR("Enable LegitBot##aimbot"), CS_XOR("Legit assistance with a aim"), &C_GET(bool, Vars.bLegitbot));
					if (edited::Checkbox(CS_XOR("Enable Vizualisation FOV"), CS_XOR("AIM Fov circle"), &C_GET(bool, Vars.AimVof)));
					{
						edited::Color(CS_XOR("FOV Visualization color"), CS_XOR("Circle color"), &C_GET(Color_t, Vars.colVof), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
					}
					edited::SliderFloat(CS_XOR("Aim FOV"), CS_XOR("Aim range for legitbot"), &C_GET(float, Vars.flAimRange), 1.f, 135.f);
					edited::SliderFloat(CS_XOR("Smoothing"), CS_XOR("Aim's accuracy"), &C_GET(float, Vars.flSmoothing), 1.f, 100.f);

					ImGui::NewLine();
					// Key
					edited::Checkbox(CS_XOR("Always on##aimbot"), CS_XOR("The legitbot will always be on"), &C_GET(bool, Vars.bLegitbotAlwaysOn));
					ImGui::BeginDisabled(C_GET(bool, Vars.bLegitbotAlwaysOn));
					{
						edited::Keybind(CS_XOR("Keybind"), CS_XOR("The key that legitbot will trigger on"), &C_GET(int, Vars.nLegitbotActivationKeysoso), &C_GET(int, Vars.nLegitbotActivationKey));
					}
					ImGui::EndDisabled();
				}
				edited::EndChild();

			}
			else if (active_tab == 1)
			{
				edited::BeginChild("##Container0", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Antiaim");

					edited::Checkbox(CS_XOR("Enable##AntiAim"), CS_XOR("Antiaim's will make it difficult to hit you."), &C_GET(bool, Vars.bAntiAim));
					edited::Combo("JitterType", CS_XOR("How do you want to spin?"), &C_GET(int, Vars.iJitterType), new const char* [] {"None", "Center", "3Way", "Random", "Spin", "Dance"}, 6);
					edited::SliderFloat(CS_XOR("Jitter"), CS_XOR("Rotation speed"), &C_GET(float, Vars.flJitter), 1.f, 100.f);
					edited::Combo("YAW", CS_XOR("So far, only backward"), &C_GET(int, Vars.iYaw), new const char* [] {"None", "Backward"}, 2);
					edited::Keybind(CS_XOR("Left"), CS_XOR("You can turn to the left by pressing the button"), &C_GET(int, Vars.iLeftKey), &C_GET(int, Vars.iLeftKeyBind));
					edited::Keybind(CS_XOR("Right"), CS_XOR("You can turn to the кшпре by pressing the button"), &C_GET(int, Vars.iRightKey), &C_GET(int, Vars.iRightKeyBind));
					edited::SliderFloat(CS_XOR("YawOffset"), CS_XOR("Which way do you want to turn and by how many degrees?"), &C_GET(float, Vars.flYawOffset), -180.f, 180.f);
					edited::Checkbox(CS_XOR("AtTarget##AntiAim"), CS_XOR("The Antiaim will be aimed at where the nearest enemy is"), &C_GET(bool, Vars.bAtTarget));
					edited::Checkbox(CS_XOR("Pitch##AntiAim"), CS_XOR("Includes different types of antiaim"), &C_GET(bool, Vars.bPitch));
					edited::Combo("Pitch Type", CS_XOR("Choose the type of antiaim"), &C_GET(int, Vars.iPitchType), new const char* [] {"None", "Down", "Up", "Jitter"}, 4);
				}
				edited::EndChild();
			}
			else if (active_tab == 2) {
				edited::BeginChild("##Container0", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{

					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Players");

					edited::Checkbox(CS_XOR("Enable"), CS_XOR("Enable ESP"), &C_GET(bool, Vars.bVisualOverlay));
					if (C_GET(bool, Vars.bVisualOverlay))
					{
						edited::Checkbox(CS_XOR("Bounding box"), CS_XOR("Shows player bounding box"), &C_GET(FrameOverlayVar_t, Vars.overlayBox).bEnable);
						if (C_GET(FrameOverlayVar_t, Vars.overlayBox).bEnable)
						{
							const char* BoxTypes[2]{ CS_XOR("Default"), CS_XOR("Filled") };
							edited::Combo(CS_XOR("Box type"), CS_XOR("Box type for bounding box"), &C_GET(int, Vars.BoxType), BoxTypes, IM_ARRAYSIZE(BoxTypes), 2);
						}
						edited::Checkbox(CS_XOR("Player Name"), CS_XOR("Shows player name"), &C_GET(bool, Vars.bNameEsp));
						edited::Checkbox(CS_XOR("Player Health bar"), CS_XOR("Shows player health bar"), &C_GET(bool, Vars.bHealthBar));
						edited::Checkbox(CS_XOR("Player Skeleton"), CS_XOR("Shows player bones"), &C_GET(bool, Vars.bSkeleton));
						edited::Checkbox(CS_XOR("Player Ammo bar"), CS_XOR("Shows player ammo"), &C_GET(bool, Vars.bAmmoBar));
						edited::Checkbox(CS_XOR("Player Weapon name"), CS_XOR("Shows player weapon name"), &C_GET(bool, Vars.bWeaponName));
						edited::Checkbox(CS_XOR("Player Weapon icon"), CS_XOR("Shows player weapon icon"), &C_GET(bool, Vars.bWeaponIcon));

						edited::Checkbox(CS_XOR("Enable Other Visuals"), CS_XOR("Enable Other Visuals"), &C_GET(bool, Vars.bOtherVisuals));

						if (C_GET(bool, Vars.bOtherVisuals))
						{
							edited::Checkbox(CS_XOR("BulletTrace"), CS_XOR("Enable Other Visuals"), &C_GET(bool, Vars.bBulletTrace));
							if (C_GET(bool, Vars.bBulletTrace))
							{
								edited::Color(CS_XOR("BulletTrace Color"), CS_XOR("Change bullettrace color"), &C_GET(Color_t, Vars.colBulletTracer), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
							}
							edited::Checkbox(CS_XOR("DrawCube"), CS_XOR("Draw pet CUBE :3"), &C_GET(bool, Vars.bDrawCube));
							edited::Checkbox(CS_XOR("Grenade Visuals"), CS_XOR("Enable grenade trajectory"), &C_GET(bool, Vars.bGrenadeVisuals));
							edited::Checkbox(CS_XOR("OOF"), CS_XOR("Enable Out Of View Arrow"), &C_GET(bool, Vars.bOof));
							edited::SliderFloat(CS_XOR("AspectRatio"), CS_XOR("Change your aspect ratio"), &C_GET(float, Vars.flAspectRatio), 1.f, 3.f);
						}
					}
				}
				edited::EndChild();

				ImGui::SameLine(0, 0);

				edited::BeginChild("##Container1", ImVec2((c::background::size.x - 200 * dpi) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Colors");

					edited::Checkbox(CS_XOR("Enable color editor"), CS_XOR("Edit ESP colors"), &C_GET(bool, Vars.bColorEditor));
					if (C_GET(bool, Vars.bColorEditor))
					{
						if (C_GET(FrameOverlayVar_t, Vars.overlayBox).bEnable) {
							edited::Color(CS_XOR("Bounding box color"), CS_XOR("Color for bounding box element"), &C_GET(Color_t, Vars.colBox), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
						if (C_GET(bool, Vars.bNameEsp)) {
							edited::Color(CS_XOR("Player name color"), CS_XOR("Color for player name element"), &C_GET(Color_t, Vars.colName), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
						if (C_GET(bool, Vars.bHealthBar)) {
							edited::Color(CS_XOR("Player full health bar color"), CS_XOR("Color for player full health bar element"), &C_GET(Color_t, Vars.colFullHealthBar), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
							edited::Color(CS_XOR("Player low health bar color"), CS_XOR("Color for player low health bar element"), &C_GET(Color_t, Vars.colLowHealthBar), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
						if (C_GET(bool, Vars.bSkeleton)) {
							edited::Color(CS_XOR("Player skeleton color"), CS_XOR("Color for player skeleton element"), &C_GET(Color_t, Vars.colSkeleton), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
						if (C_GET(bool, Vars.bAmmoBar)) {
							edited::Color(CS_XOR("Player ammo bar color"), CS_XOR("Color for player ammo bar element"), &C_GET(Color_t, Vars.colAmmobar), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
						if (C_GET(bool, Vars.bWeaponName)) {
							edited::Color(CS_XOR("Player weapon name color"), CS_XOR("Color for weapon name element"), &C_GET(Color_t, Vars.colWeaponName), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
						if (C_GET(bool, Vars.bWeaponIcon)) {
							edited::Color(CS_XOR("Player weapon icon color"), CS_XOR("Color for player name element"), &C_GET(Color_t, Vars.colWeaponIcon), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
					}

					edited::Checkbox("Enable Modulation", CS_XOR("Change your world colors"), &C_GET(bool, Vars.bWorldModulation));
					if (C_GET(bool, Vars.bWorldModulation))
					{
						edited::Color(CS_XOR("World color"), CS_XOR("Change your walls color"), &C_GET(Color_t, Vars.colWorld), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						edited::Color(CS_XOR("Light Color"), CS_XOR("Change your lighting color"), &C_GET(Color_t, Vars.colLightning), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						edited::Color(CS_XOR("Sky Color"), CS_XOR("Change your sky color"), &C_GET(Color_t, Vars.colSky), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						edited::Color(CS_XOR("Sun Color"), CS_XOR("Change your sun color"), &C_GET(Color_t, Vars.colClouds), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						edited::SliderFloat(CS_XOR("Intensive"), CS_XOR("Change your intensive count"), &C_GET(float, Vars.flLightingIntensity), 0, 10);
						edited::SliderFloat(CS_XOR("Exposure"), CS_XOR("Change your exspousre count"), &C_GET(float, Vars.flExposure), 0, 100);
					}
				}
				edited::EndChild();
			}

			else if (active_tab == 3)
			{
				edited::BeginChild("##Container0", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{
					edited::Checkbox(CS_XOR("Enable Chams##chams"), CS_XOR("Enable the chams models"), &C_GET(bool, Vars.bVisualChams));
					if (C_GET(bool, Vars.bVisualChams))
					{
						edited::Checkbox(CS_XOR("Enemy##Enemy"), CS_XOR("Chams for enemy players"), &C_GET(bool, Vars.bEnemyChams));
						if (C_GET(bool, Vars.bEnemyChams))
						{
							edited::Combo(CS_XOR("Materials"), CS_XOR("Chams materials"), &C_GET(int, Vars.nVisualChamMaterial), CS_XOR("white\0illuminate\0Latex\0Glow\0Glow2\0Metalic\0"));
							edited::Checkbox(CS_XOR("Enable invisible chams##chams"), CS_XOR("Invisible chams color"), &C_GET(bool, Vars.bVisualChamsIgnoreZ));
							edited::Color(CS_XOR("Visible color"), CS_XOR("Visible chams color"), &C_GET(Color_t, Vars.colVisualChams), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
							if (C_GET(bool, Vars.bVisualChamsIgnoreZ))
							{
								edited::Color(CS_XOR("Invisible color"), CS_XOR(""), &C_GET(Color_t, Vars.colVisualChamsIgnoreZ), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
							}
						}

						edited::Checkbox(CS_XOR("Weapon##chams"), CS_XOR("Chams for local weapon"), &C_GET(bool, Vars.bWeaponChams));
						if (C_GET(bool, Vars.bWeaponChams))
						{
							edited::Combo(CS_XOR("Material"), CS_XOR("Material for local weapon"), &C_GET(int, Vars.nWeaponChamMaterial), CS_XOR("white\0illuminate\0Latex\0Glow\0Glow2\0Metalic\0"));
							edited::Color(CS_XOR("Weapon color"), CS_XOR("Color for local weapon chams"), &C_GET(Color_t, Vars.colWeapon), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}

						edited::Checkbox(CS_XOR("ViewModel##chams"), CS_XOR("Enable chams for hands"), &C_GET(bool, Vars.bViewModelChams));
						if (C_GET(bool, Vars.bViewModelChams))
						{
							edited::Combo(CS_XOR("Material##ViewModel"), CS_XOR("Material for hands"), &C_GET(int, Vars.nViewModelChamMaterial), CS_XOR("white\0illuminate\0Latex\0Glow\0Glow2\0Metalic\0"));
							edited::Color(CS_XOR("ViewModel color"), CS_XOR("Color for local hands chams"), &C_GET(Color_t, Vars.colViewModel), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
						}
					}
				}
				edited::EndChild();
			}

			else if (active_tab == 5) {
				edited::BeginChild("##Container0", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{

					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "Misc");

					edited::Checkbox(CS_XOR("Keybinds list"), ("See, what the keybind is active"), &C_GET(bool, Vars.bHotkeys));
					/*edited::Checkbox(CS_XOR("Anti untrusted"), CS_XOR("Disables dangerous functions that can lower the trust factor"), &C_GET(bool, Vars.bAntiUntrusted));*/
					edited::Checkbox(CS_XOR("Unlock inventory"), ("Unlock the inventory im MM games"), &C_GET(bool, Vars.bUnlockINVENTORY));
					/*edited::Checkbox(CS_XOR("Watermark"), ("Shows watermark always"), &C_GET(bool, Vars.bWatermark1));*/
					edited::Checkbox(CS_XOR("Health Boost"), CS_XOR("?????"), &C_GET(bool, Vars.bHealthBoost));
					edited::Checkbox(CS_XOR("Hit Marker"), CS_XOR("The marker is played when hit"), &C_GET(bool, Vars.bHitmarker));
					edited::Checkbox(CS_XOR("Hit Effect"), CS_XOR("The effect is played when hit"), &C_GET(bool, Vars.bHiteffect));
					edited::Checkbox(CS_XOR("Hit Sound"), CS_XOR("The sound is played when hit"), &C_GET(bool, Vars.bHitSound));
					if (edited::SliderFloat(CS_XOR("Volume"), CS_XOR("The volume of the hit sound"), &C_GET(float, Vars.flVolumeHit), 0, 1))
						I::Cvar->Find(FNV1A::Hash("snd_toolvolume"))->value.fl = C_GET(float, Vars.flVolumeHit);
					edited::Checkbox(CS_XOR("Hit Logs"), CS_XOR("Logs of player hits"), &C_GET(bool, Vars.bHitLogs));
					//edited::Checkbox(CS_XOR("Spectator List"), CS_XOR("Displays who's watching you"), &C_GET(bool, Vars.bSpectatorList));
					//edited::Checkbox(CS_XOR("Keybinds List"), CS_XOR("Displays your's keybinds"), &C_GET(bool, Vars.bKeybindsList));
					//edited::Checkbox(CS_XOR("Bomb Timer"), CS_XOR("Timer before the bomb explodes"), &C_GET(bool, Vars.bBombTimer));
					edited::Checkbox(CS_XOR("No Scope"), CS_XOR("Removes the scope in the game"), &C_GET(bool, Vars.bNoScope));
					edited::Checkbox(CS_XOR("Jump Bug"), CS_XOR("???????"), &C_GET(bool, Vars.bJumpBug));
					edited::MultiCombo(CS_XOR("Removals"), &C_GET(unsigned int, Vars.bRemovalsNames), arrRemovalsNames, CS_ARRAYSIZE(arrRemovalsNames));
					edited::Checkbox(CS_XOR("Fov"), CS_XOR("Field Of View on the map"), &C_GET(bool, Vars.bFov));
					edited::SliderFloat(CS_XOR("FovValue"), CS_XOR("How strong do you want to make the FOV?"), &C_GET(float, Vars.flFov), 0, 170);
					edited::SliderFloat(CS_XOR("FovViewModel"), CS_XOR("How far do you want to put your hands?"), &C_GET(float, Vars.flViewModelFov), 0, 180);
					edited::Checkbox(CS_XOR("Auto BHope"), CS_XOR("He jumps in your place"), &C_GET(bool, Vars.bAutoBHop));
					if (C_GET(bool, Vars.bAutoBHop))
						edited::SliderInt(CS_XOR("chance"), CS_XOR("A chance to jump"), &C_GET(int, Vars.nAutoBHopChance), 0, 100, CS_XOR("%d%%"));
					edited::Checkbox(CS_XOR("auto strafe"), CS_XOR("It's AutoStrafer"), &C_GET(bool, Vars.bAutoStrafe));
					edited::Checkbox(CS_XOR("Subtick Strafe"), CS_XOR("It's Subtick Strafer"), &C_GET(bool, Vars.bSubtickStrafe));
					edited::SliderFloat(CS_XOR("Smooth Strafe"), CS_XOR("Smoothness of the strafes"), &C_GET(float, Vars.flStrafeSmoothing), 0, 100);
					edited::Keybind("ThirdPerson", "ThirdPerson bind", &C_GET(int, Vars.iThirdKey), &C_GET(int, Vars.iThirdKeyBind));
					edited::SliderInt(CS_XOR("Distance"), CS_XOR("Third-person distance"), &C_GET(int, Vars.iDistanceThird), 1, 180, CS_XOR("%d%%"));
					edited::SliderFloat(CS_XOR("View X"), CS_XOR("Hand position on the X axis"), &C_GET(float, Vars.flViX), -10, 10);
					edited::SliderFloat(CS_XOR("View Y"), CS_XOR("Hand position on the Y axis"), &C_GET(float, Vars.flViY), -10, 10);
					edited::SliderFloat(CS_XOR("View Z"), CS_XOR("Hand position on the Z axis"), &C_GET(float, Vars.flViZ), -10, 10);
				}
				edited::EndChild();
				ImGui::SameLine(0, 0);
				edited::BeginChild("##Container1", ImVec2((c::background::size.x - 200) / 2, c::background::size.y), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), CS_XOR("Configs"));
					{
						{
							{
								ImGui::Columns(2, CS_XOR("#CONFIG"), false);
								{
									ImGui::PushItemWidth(-1);

									// check selected configuration for magic value
									if (T::nSelectedConfig == ~1U)
									{
										// set default configuration as selected on first use
										for (std::size_t i = 0U; i < C::vecFileNames.size(); i++)
										{
											if (CRT::StringCompare(C::vecFileNames[i], CS_XOR(CS_CONFIGURATION_DEFAULT_FILE_NAME CS_CONFIGURATION_FILE_EXTENSION)) == 0)
												T::nSelectedConfig = i;
										}
									}

									if (ImGui::BeginListBox(CS_XOR("##config.list"), C::vecFileNames.size(), 5))
									{
										for (std::size_t i = 0U; i < C::vecFileNames.size(); i++)
										{
											// @todo: imgui cant work with wstring
											const wchar_t* wszFileName = C::vecFileNames[i];

											char szFileName[MAX_PATH] = {};
											CRT::StringUnicodeToMultiByte(szFileName, CS_ARRAYSIZE(szFileName), wszFileName);

											if (ImGui::Selectable(szFileName, (T::nSelectedConfig == i)))
												T::nSelectedConfig = i;
										}

										ImGui::EndListBox();
									}

									ImGui::PopItemWidth();
								}
								ImGui::NextColumn();
								{
									ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));
									ImGui::PushItemWidth(-1);
									if (ImGui::InputTextWithHint(CS_XOR("##config.file"), CS_XOR("create new..."), T::szConfigFile, sizeof(T::szConfigFile), ImGuiInputTextFlags_EnterReturnsTrue))
									{
										// check if the filename isn't empty
										if (const std::size_t nConfigFileLength = CRT::StringLength(T::szConfigFile); nConfigFileLength > 0U)
										{
											CRT::WString_t wszConfigFile(T::szConfigFile);

											if (C::CreateFile(wszConfigFile.Data()))
												// set created config as selected @todo: dependent on current 'C::CreateFile' behaviour, generally it must be replaced by search
												T::nSelectedConfig = C::vecFileNames.size() - 1U;

											// clear string
											CRT::MemorySet(T::szConfigFile, 0U, sizeof(T::szConfigFile));
										}
									}
									if (ImGui::IsItemHovered())
										ImGui::SetTooltip(CS_XOR("press enter to create new configuration"));

									if (edited::Button(CS_XOR("save"), ImVec2(-1, 15 * MENU::flDpiScale), 0))
										C::SaveFile(T::nSelectedConfig);

									if (edited::Button(CS_XOR("load"), ImVec2(-1, 15 * MENU::flDpiScale), 0))
										C::LoadFile(T::nSelectedConfig);

									if (edited::Button(CS_XOR("remove"), ImVec2(-1, 15 * MENU::flDpiScale), 0))
										ImGui::OpenPopup(CS_XOR("confirmation##config.remove"));

									if (edited::Button(CS_XOR("refresh"), ImVec2(-1, 15 * MENU::flDpiScale), 0))
										C::Refresh();

									ImGui::PopItemWidth();
									ImGui::PopStyleVar();
								}
								ImGui::Columns(1);

								if (ImGui::BeginPopupModal(CS_XOR("confirmation##config.remove"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
								{
									CRT::String_t<MAX_PATH> szCurrentConfig(C::vecFileNames[T::nSelectedConfig]);

									ImGui::Text(CS_XOR("are you sure you want to remove \"%s\" configuration?"), szCurrentConfig);
									ImGui::Spacing();

									if (edited::Button(CS_XOR("no"), ImVec2(ImGui::GetContentRegionAvail().x / 2.f, 0), 0))
										ImGui::CloseCurrentPopup();

									ImGui::SameLine();

									if (edited::Button(CS_XOR("yes"), ImVec2(ImGui::GetContentRegionAvail().x, 0), 0))
									{
										C::RemoveFile(T::nSelectedConfig);

										// reset selected configuration index
										T::nSelectedConfig = ~0U;

										ImGui::CloseCurrentPopup();
									}

									ImGui::EndPopup();
								}
							}
						}
						edited::Color(CS_XOR("Menu accent color"), CS_XOR("Color for menu"), &C_GET(Color_t, Vars.color), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
					}
				}
				edited::EndChild();
			}
			else if (active_tab == 4) {
				edited::BeginChild(CS_XOR("##Container6"), ImVec2(((c::background::size.x * D::CalculateDPI(C_GET(int, Vars.nDpiScale)) - 200) / 1), c::background::size.y * D::CalculateDPI(C_GET(int, Vars.nDpiScale))), NULL);
				{
					ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), CS_XOR("Inventory Changer"));
					{
						if (vecDumpedItems.empty() && skin_changer::dumpitems)
						{
							const CUtlMap2<int, CEconItemDefinition*>& vecItems =
								pItemSchema->GetSortedItemDefinitionMap();
							CUtlMap2<int, CPaintKit*>& vecPaintKits =
								pItemSchema->GetPaintKits();
							static const CUtlMap2<uint64_t, AlternateIconData_t>& vecAlternateIcons =
								pItemSchema->GetAlternateIconsMap();

							for (const auto& it : vecItems)
							{
								CEconItemDefinition* pItem = it.m_value;
								if (!pItem)
									continue;

								const bool isWeapon = pItem->IsWeapon();
								const bool isKnife = pItem->IsKnife(true);
								const bool isGloves = pItem->IsGlove(true);
								const bool IsAgent = pItem->IsAgent();

								const char* itemBaseName = pItem->m_pszItemBaseName;

								if (!itemBaseName || itemBaseName[0] == '\0')
									continue;

								const uint16_t defIdx = pItem->m_nDefIndex;

								DumpedItem_t dumpedItem;
								dumpedItem.m_name = I::Localize->FindSafe(pItem->GetName());

								//dumpedItem.m_image = pItem->m_pKVItem;
								dumpedItem.m_defIdx = defIdx;
								dumpedItem.m_rarity = pItem->m_nItemRarity;
								if (isKnife || isGloves || isWeapon || IsAgent)
								{
									dumpedItem.m_unusualItem = true;
								}
								dumpedItem.m_dumpedSkins.emplace_back("Vanilla", 0, IR_ANCIENT);
								// Load the image and set the texture ID.
								//if (dumpedItem.m_image)
								//{
								//	dumpedItem.m_textureID = CreateTextureFromMemory(dumpedItem.m_image, 120, 280);
								//}

								// We filter skins by guns.
								for (const auto& it : vecPaintKits)
								{
									CPaintKit* pPaintKit = it.m_value;
									if (!pPaintKit || pPaintKit->PaintKitId() == 0 || pPaintKit->PaintKitId() == 9001)
										continue;

									const uint64_t skinKey =
										Helper_GetAlternateIconKeyForWeaponPaintWearItem(
											defIdx, pPaintKit->PaintKitId(), 0);
									if (vecAlternateIcons.FindByKey(skinKey))
									{
										DumpedSkin_t dumpedSkin;
										dumpedSkin.m_name = I::Localize->FindSafe(
											pPaintKit->PaintKitDescriptionTag());
										dumpedSkin.m_ID = pPaintKit->PaintKitId();
										dumpedSkin.m_rarity = std::clamp(pItem->GetRarity() + pPaintKit->PaintKitRarity() - 1, 0, (pPaintKit->PaintKitRarity() == 7) ? 7 : 6) + 1;
										dumpedItem.m_dumpedSkins.emplace_back(dumpedSkin);
									}
								}

								//Sort skins by rarity.
								if (!dumpedItem.m_dumpedSkins.empty() && isWeapon)
								{
									std::sort(dumpedItem.m_dumpedSkins.begin(),
										dumpedItem.m_dumpedSkins.end(),
										[](const DumpedSkin_t& a, const DumpedSkin_t& b)
										{
											return a.m_rarity > b.m_rarity;
										});
								}

								vecDumpedItems.emplace_back(dumpedItem);
							}
						}
						static char IconFilterText[128] = "";

						if (!vecDumpedItems.empty())
						{
							if (edited::Button("Add all items", ImVec2(540, 50), 0))
							{
								for (const auto& item : vecDumpedItems)
								{
									for (const auto& skin : item.m_dumpedSkins)
									{
										CEconItem* pItem = CEconItem::CreateInstance();
										L_PRINT(LOG_INFO) << "item addr:" << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << reinterpret_cast<uintptr_t>(pItem);
										if (pItem)
										{
											CCSPlayerInventory* pInventory =
												CCSPlayerInventory::GetInstance();
											auto highestIDs = pInventory->GetHighestIDs();
											L_PRINT(LOG_INFO) << "uid:" << pItem->m_ulID << " id:" << pItem->m_unAccountID << "idx:" << pItem->m_unDefIndex;
											pItem->m_ulID = highestIDs.first + 1;
											pItem->m_unInventory = highestIDs.second + 1;
											pItem->m_unAccountID =
												uint32_t(pInventory->GetOwner().m_id);
											pItem->m_unDefIndex = item.m_defIdx;
											if (item.m_unusualItem)
												pItem->m_nQuality = IQ_UNUSUAL;
											pItem->m_nRarity =
												std::clamp(item.m_rarity + skin.m_rarity - 1, 0,
													(skin.m_rarity == 7) ? 7 : 6);

											pItem->SetPaintKit((float)skin.m_ID);
											pItem->SetPaintSeed(1.f);
											if (pInventory->AddEconItem(pItem))
												skin_changer::AddEconItemToList(pItem);
										}
									}
								}
							}
						}

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Will cause lag on weaker computers.");
						if (!vecDumpedItems.empty())
						{
							static ImGuiTextFilter itemFilter;
							itemFilter.Draw("Type here to filter Items...", 540);

							// ...

							// Modify the loop for items to check against the item filter.
							if (ImGui::BeginListBox("##items", { 540, 110.f }))
							{
								for (auto& item : vecDumpedItems)
								{
									if (!itemFilter.PassFilter(item.m_name.c_str()))
										continue;

									ImGui::PushID(&item);
									if (ImGui::Selectable(item.m_name.c_str(), pSelectedItem == &item))
									{
										if (pSelectedItem == &item)
											pSelectedItem = nullptr;
										else
											pSelectedItem = &item;
									}
									ImGui::PopID();
								}
								ImGui::EndListBox();
							}
							static char skinFilterText[128] = "";

							if (pSelectedItem)
							{
								if (!pSelectedItem->m_dumpedSkins.empty())
								{
									static ImGuiTextFilter skinFilter;
									skinFilter.Draw("Type here to filter Skins...", 999);
									if (ImGui::BeginListBox("##skins", { 540, 110.f }))
									{
										for (auto& skin : pSelectedItem->m_dumpedSkins)
										{
											if (!skinFilter.PassFilter(skin.m_name.c_str()))
												continue;

											ImGui::PushID(&skin);
											if (ImGui::Selectable(
												skin.m_name.c_str(),
												pSelectedItem->pSelectedSkin == &skin))
											{
												if (pSelectedItem->pSelectedSkin == &skin)
													pSelectedItem->pSelectedSkin = nullptr;
												else
													pSelectedItem->pSelectedSkin = &skin;
											}
											ImGui::PopID();
										}
										ImGui::EndListBox();
									}
								}

								char buttonLabel[128];
								snprintf(buttonLabel, 128, "Add every %s skin",
									pSelectedItem->m_name.c_str());

								if (edited::Button(buttonLabel, ImVec2(270, 55), 0))
								{
									for (const auto& skin : pSelectedItem->m_dumpedSkins)
									{
										CEconItem* pItem = CEconItem::CreateInstance();
										if (pItem)
										{
											CCSPlayerInventory* pInventory =
												CCSPlayerInventory::GetInstance();

											auto highestIDs = pInventory->GetHighestIDs();

											pItem->m_ulID = highestIDs.first + 1;
											pItem->m_unInventory = highestIDs.second + 1;
											pItem->m_unAccountID =
												uint32_t(pInventory->GetOwner().m_id);
											pItem->m_unDefIndex = pSelectedItem->m_defIdx;
											if (pSelectedItem->m_unusualItem)
												pItem->m_nQuality = IQ_UNUSUAL;
											pItem->m_nRarity = std::clamp(
												pSelectedItem->m_rarity + skin.m_rarity - 1, 0,
												(skin.m_rarity == 7) ? 7 : 6);

											pItem->SetPaintKit((float)skin.m_ID);
											pItem->SetPaintSeed(1.f);
											if (pInventory->AddEconItem(pItem))
												skin_changer::AddEconItemToList(pItem);
										}
									}
								}
								ImGui::SameLine();
								if (pSelectedItem->pSelectedSkin)
								{
									static float kitWear = 0.f;
									static int kitSeed = 1;
									static int gunKills = -1;
									static char gunName[32];

									bool vanillaSkin = pSelectedItem->pSelectedSkin->m_ID == 0;
									snprintf(
										buttonLabel, 128, "Add %s%s%s",
										pSelectedItem->m_name.c_str(), vanillaSkin ? "" : " | ",
										vanillaSkin ? "" : pSelectedItem->pSelectedSkin->m_name.c_str());

									if (edited::Button(buttonLabel, ImVec2(260, 55), 0))
									{
										CEconItem* pItem = CEconItem::CreateInstance();
										if (pItem)
										{
											CCSPlayerInventory* pInventory =
												CCSPlayerInventory::GetInstance();

											auto highestIDs = pInventory->GetHighestIDs();
											L_PRINT(LOG_INFO) << "item addr:" << L::AddFlags(LOG_MODE_INT_FORMAT_HEX | LOG_MODE_INT_SHOWBASE) << reinterpret_cast<uintptr_t>(pItem);
											L_PRINT(LOG_INFO) << "uid:" << pItem->m_ulID << " id:" << pItem->m_unAccountID << "idx:" << pItem->m_unDefIndex;

											pItem->m_ulID = highestIDs.first + 1;
											pItem->m_unInventory = highestIDs.second + 1;
											pItem->m_unAccountID =
												uint32_t(pInventory->GetOwner().m_id);
											pItem->m_unDefIndex = pSelectedItem->m_defIdx;

											if (pSelectedItem->m_unusualItem)
												pItem->m_nQuality = IQ_UNUSUAL;

											// I don't know nor do care why the rarity is calculated
											// like this. [Formula]
											pItem->m_nRarity = std::clamp(
												pSelectedItem->m_rarity +
												pSelectedItem->pSelectedSkin->m_rarity - 1,
												0,
												(pSelectedItem->pSelectedSkin->m_rarity == 7) ? 7 : 6);

											pItem->SetPaintKit(
												(float)pSelectedItem->pSelectedSkin->m_ID);
											pItem->SetPaintSeed((float)kitSeed);
											pItem->SetPaintWear(kitWear);

											if (gunKills >= 0)
											{
												pItem->SetStatTrak(gunKills);
												pItem->SetStatTrakType(0);

												// Applied automatically on knives.
												if (pItem->m_nQuality != IQ_UNUSUAL)
													pItem->m_nQuality = IQ_STRANGE;
											}

											if (pInventory->AddEconItem(pItem))
												skin_changer::AddEconItemToList(pItem);

											kitWear = 0.f;
											kitSeed = 1;
											gunKills = -1;
											memset(gunName, '\0', IM_ARRAYSIZE(gunName));
											pItem->SetCustomName(gunName);
										}
									}
								}
							}
						}
					}	edited::EndChild();
				}
			}
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();

}

const char* cheat_name = "UNDERAGER";
const char* game_status = "Counter-Strike: 2";
const char* developer = "KEGLYA";
const char* role = "Dev";
const char* fps = "210fps";
const char* ping = "45ms";
const char* world_time = "12:20am";

void MENU::RenderWatermark()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	UpdateStyle(&style);


	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing = ImVec2(10 * dpi, 10 * dpi);
	style.WindowBorderSize = 0;
	style.ScrollbarSize = 3.f * dpi;

	if (!C_GET(bool, Vars.bWatermark1))
		return;

	ImGui::PushFont(font::lexend_bold);

	const float cheat_name_size = ImGui::CalcTextSize(cheat_name).x;
	const float developer_size = ImGui::CalcTextSize(developer).x;
	const float role_size = ImGui::CalcTextSize(role).x;
	const float ping_size = ImGui::CalcTextSize(ping).x;
	const float fps_size = ImGui::CalcTextSize(fps).x;
	const float world_time_size = ImGui::CalcTextSize(world_time).x;
	const float bar_size = ImGui::CalcTextSize("|").x;

	const float ibar_size = cheat_name_size + bar_size + developer_size + bar_size + role_size + bar_size + ping_size + fps_size + developer_size + bar_size + world_time_size * 2 + bar_size * 3;

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - (ibar_size + 15), 15));
	ImGui::SetNextWindowSize(ImVec2(ibar_size, 45));

	ImGui::Begin("info-bar", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
	{
		const ImVec2& pos = ImGui::GetWindowPos(), spacing = style.ItemSpacing, region = ImGui::GetContentRegionMax();

		ImGui::GetBackgroundDrawList()->AddRectFilled(pos, pos + region, ImGui::GetColorU32(c::background::filling), c::elements::rounding);
		ImGui::GetBackgroundDrawList()->AddRect(pos, pos + region, ImGui::GetColorU32(c::background::stroke), c::elements::rounding);

		const char* info_set[6] = { cheat_name, developer, role, ping, fps, world_time };

		ImGui::SetCursorPos(ImVec2(spacing.x, (45 - ImGui::CalcTextSize(developer).y) / 2));
		ImGui::BeginGroup();
		{
			for (int i = 0; i < ARRAYSIZE(info_set); i++) {
				ImGui::TextColored(i < 1 ? ImColor(ImGui::GetColorU32(c::accent)) : ImColor(ImGui::GetColorU32(c::elements::text)), info_set[i]);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(ImGui::GetColorU32(c::elements::text)), "|");
				ImGui::SameLine();
			}
		}
		ImGui::EndGroup();
	}
	ImGui::End();

	ImGui::PopFont();
}

void MENU::UpdateStyle(ImGuiStyle* pStyle)
{
	ImGuiStyle& style = pStyle != nullptr ? *pStyle : ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.87f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.26f, 0.26f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = c::tab::tab_active;
	style.Colors[ImGuiCol_ScrollbarGrab] = c::tab::tab_active;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = c::accent;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = c::accent;
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(c::accent.x, c::accent.y, c::accent.z, 0.70f);
	style.Colors[ImGuiCol_SliderGrabActive] = c::accent;
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 0.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0);
	style.Colors[ImGuiCol_Tab] = ImLerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.80f);
	style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_HeaderHovered];
	style.Colors[ImGuiCol_TabActive] = ImLerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.60f);
	style.Colors[ImGuiCol_TabUnfocused] = ImLerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.80f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImLerp(style.Colors[ImGuiCol_TabActive], style.Colors[ImGuiCol_TitleBg], 0.40f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


	C_GET(ColorPickerVar_t, Vars.colPrimtv0).UpdateRainbow(); // (text)
	C_GET(ColorPickerVar_t, Vars.colPrimtv1).UpdateRainbow(); // (background)
	C_GET(ColorPickerVar_t, Vars.colPrimtv2).UpdateRainbow(); // (disabled)
	C_GET(ColorPickerVar_t, Vars.colPrimtv3).UpdateRainbow(); // (control bg)
	C_GET(ColorPickerVar_t, Vars.colPrimtv4).UpdateRainbow(); // (border)

	C_GET(ColorPickerVar_t, Vars.colAccent0).UpdateRainbow(); // (main)
	C_GET(ColorPickerVar_t, Vars.colAccent1).UpdateRainbow(); // (dark)
	C_GET(ColorPickerVar_t, Vars.colAccent2).UpdateRainbow(); // (darker)

	// update animation speed
	style.AnimationSpeed = C_GET(float, Vars.flAnimationSpeed) / 10.f;
}

#pragma region menu_tabs

void T::Render(const char* szTabBar, const CTab* arrTabs, const unsigned long long nTabsCount, int* nCurrentTab, ImGuiTabBarFlags flags)
{
	if (ImGui::BeginTabBar(szTabBar, flags))
	{
		for (std::size_t i = 0U; i < nTabsCount; i++)
		{
			// add tab
			if (ImGui::BeginTabItem(arrTabs[i].szName))
			{
				// set current tab index
				*nCurrentTab = (int)i;
				ImGui::EndTabItem();
			}
		}

		// render inner tab
		if (arrTabs[*nCurrentTab].pRenderFunction != nullptr)
			arrTabs[*nCurrentTab].pRenderFunction();

		ImGui::EndTabBar();
	}
}


#pragma endregion

#pragma region menu_particle

void MENU::ParticleContext_t::Render(ImDrawList* pDrawList, const ImVec2& vecScreenSize, const float flAlpha)
{
	if (this->vecParticles.empty())
	{
		for (int i = 0; i < 100; i++)
			this->AddParticle(ImGui::GetIO().DisplaySize);
	}

	for (auto& particle : this->vecParticles)
	{
		this->DrawParticle(pDrawList, particle, C_GET(ColorPickerVar_t, Vars.colAccent0).colValue.Set<COLOR_A>(flAlpha * 255));
		this->UpdatePosition(particle, vecScreenSize);
		this->FindConnections(pDrawList, particle, C_GET(ColorPickerVar_t, Vars.colAccent2).colValue.Set<COLOR_A>(flAlpha * 255), 200.f);
	}
}

void MENU::ParticleContext_t::AddParticle(const ImVec2& vecScreenSize)
{
	// exceeded limit
	if (this->vecParticles.size() >= 200UL)
		return;

	// @note: random speed value
	static constexpr float flSpeed = 100.f;
	this->vecParticles.emplace_back(
		ImVec2(MATH::fnRandomFloat(0.f, vecScreenSize.x), MATH::fnRandomFloat(0.f, vecScreenSize.y)),
		ImVec2(MATH::fnRandomFloat(-flSpeed, flSpeed), MATH::fnRandomFloat(-flSpeed, flSpeed)));
}

void MENU::ParticleContext_t::DrawParticle(ImDrawList* pDrawList, ParticleData_t& particle, const Color_t& colPrimary)
{
	D::AddDrawListCircle(pDrawList, particle.vecPosition, 2.f, colPrimary, 12, DRAW_CIRCLE_OUTLINE | DRAW_CIRCLE_FILLED);
}

void MENU::ParticleContext_t::FindConnections(ImDrawList* pDrawList, ParticleData_t& particle, const Color_t& colPrimary, float flMaxDistance)
{
	for (auto& currentParticle : this->vecParticles)
	{
		// skip current particle
		if (&currentParticle == &particle)
			continue;

		/// @note: calcuate length distance 2d, return FLT_MAX if failed
		const float flDistance = ImLength(particle.vecPosition - currentParticle.vecPosition, FLT_MAX);
		if (flDistance <= flMaxDistance)
			this->DrawConnection(pDrawList, particle, currentParticle, (flMaxDistance - flDistance) / flMaxDistance, colPrimary);
	}
}

void MENU::ParticleContext_t::DrawConnection(ImDrawList* pDrawList, ParticleData_t& particle, ParticleData_t& otherParticle, float flAlpha, const Color_t& colPrimary) const
{
	D::AddDrawListLine(pDrawList, particle.vecPosition, otherParticle.vecPosition, colPrimary.Set<COLOR_A>(flAlpha * 255), 1.f);
}

void MENU::ParticleContext_t::UpdatePosition(ParticleData_t& particle, const ImVec2& vecScreenSize) const
{
	this->ResolveScreenCollision(particle, vecScreenSize);

	ImGuiStyle& style = ImGui::GetStyle();

	// move particle
	particle.vecPosition.x += (particle.vecVelocity.x * style.AnimationSpeed * 10.f) * ImGui::GetIO().DeltaTime;
	particle.vecPosition.y += (particle.vecVelocity.y * style.AnimationSpeed * 10.f) * ImGui::GetIO().DeltaTime;
}

void MENU::ParticleContext_t::ResolveScreenCollision(ParticleData_t& particle, const ImVec2& vecScreenSize) const
{
	if (particle.vecPosition.x + particle.vecVelocity.x > vecScreenSize.x || particle.vecPosition.x + particle.vecVelocity.x < 0)
		particle.vecVelocity.x = -particle.vecVelocity.x;

	if (particle.vecPosition.y + particle.vecVelocity.y > vecScreenSize.y || particle.vecPosition.y + particle.vecVelocity.y < 0)
		particle.vecVelocity.y = -particle.vecVelocity.y;
}

#pragma endregion
