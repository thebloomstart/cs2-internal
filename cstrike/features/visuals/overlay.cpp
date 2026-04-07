// used: [stl] vector
#include <vector>
// used: [stl] sort
#include <algorithm>

#include "overlay.h"

// used: cheat variables
#include "../../core/variables.h"

// used: entity
#include "../../sdk/entity.h"
#include "../../utilities/draw.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/cgametracemanager.h"

// used: sdk variables
#include "../../core/sdk.h"

// used: l_print
#include "../../utilities/log.h"
// used: inputsystem
#include "../../utilities/inputsystem.h"
// used: draw system
#include "../../utilities/draw.h"

// used: mainwindowopened
#include "../../core/menu.h"
#include <map>
#include "../../sdk/EntityList/EntityList.h"
#include "../../BulletTracer.h"	
#include "../../sdk/entity.h"
#include "../../AntiAim.h"
#include "../../AimBot.h"
#include "../../MovementRecorder.h"
#include "../../GrenadeVisuals.h"

Box GetEntityBoundingBox(C_CSPlayerPawn* pEntity)
{
	CCollisionProperty* pCollision = pEntity->GetCollision();
	if (pCollision == nullptr)
		return {};

	CGameSceneNode* pGameSceneNode = pEntity->GetGameSceneNode();
	if (pGameSceneNode == nullptr)
		return {};

	CTransform nodeToWorldTransform = pGameSceneNode->GetNodeToWorld();
	const Matrix3x4_t matTransform = nodeToWorldTransform.quatOrientation.ToMatrix(nodeToWorldTransform.vecPosition);

	const Vector_t vecMins = pCollision->GetMins();
	const Vector_t vecMaxs = pCollision->GetMaxs();
	Box out{};
	out.x = out.y = std::numeric_limits<float>::max();
	out.width = out.width = -std::numeric_limits<float>::max();

	for (int i = 0; i < 8; ++i)
	{
		const Vector_t vecPoint{ 
			i & 1 ? vecMaxs.x : vecMins.x, 
			i & 2 ? vecMaxs.y : vecMins.y,
			i & 4 ? vecMaxs.z : vecMins.z 
		};
		ImVec2 vecScreen;	
		if (!D::WorldToScreen(vecPoint.Transform(matTransform), &vecScreen))
			return {};

		out.x = MATH::Min(out.x, vecScreen.x);
		out.y = MATH::Min(out.y, vecScreen.y);
		out.height = MATH::Max(out.height, vecScreen.x);
		out.width = MATH::Max(out.width, vecScreen.y);
		out.m_found = true;
		return out;

	}


	return {};
}

inline const char* GunIcon(const std::string& weapon)
{
	std::map<std::string, const char*> gunIcons = {
		{ "weapon_p90", "P" },
		{ "weapon_mp9", "O" },
		{ "weapon_mp5sd", "O" },
		{ "weapon_m4a4", "M" },
		{ "weapon_knife", "]" },
		{ "weapon_knife_ct", "]" },
		{ "weapon_knife_t", "]" },
		{ "weapon_deage", "A" },
		{ "weapon_eite", "B" },
		{ "weapon_fiveseven", "C" },
		{ "weapon_gock", "D" },
		{ "weapon_revover", "J" },
		{ "weapon_hkp2000", "E" },
		{ "weapon_p250", "F" },
		{ "weapon_usp_siencer", "G" },
		{ "weapon_tec9", "H" },
		{ "weapon_cz75a", "I" },
		{ "weapon_mac10", "K" },
		{ "weapon_ump45", "" },
		{ "weapon_bizon", "M" },
		{ "weapon_mp7", "N" },
		{ "weapon_gaiar", "Q" },
		{ "weapon_famas", "R" },
		{ "weapon_m4a1_siencer", "T" },
		{ "weapon_m4a1", "S" },
		{ "weapon_aug", "U" },
		{ "weapon_sg556", "V" },
		{ "weapon_ak47", "W" },
		{ "weapon_g3sg1", "X" },
		{ "weapon_scar20", "Y" },
		{ "weapon_awp", "Z" },
		{ "weapon_ssg08", "a" },
		{ "weapon_xm1014", "b" },
		{ "weapon_sawedoff", "c" },
		{ "weapon_mag7", "d" },
		{ "weapon_nova", "e" },
		{ "weapon_negev", "f" },
		{ "weapon_m249", "g" },
		{ "weapon_taser", "h" },
		{ "weapon_fashbang", "i" },
		{ "weapon_hegrenade", "j" },
		{ "weapon_smokegrenade", "k" },
		{ "weapon_mootov", "" },
		{ "weapon_decoy", "m" },
		{ "weapon_incgrenade", "n" },
		{ "weapon_c4", "o" },
		{ "weapon_bayonet", "]" },
		{ "weapon_knife_surviva_bowie", "]" },
		{ "weapon_knife_butterfy", "]" },
		{ "weapon_knife_canis", "]" },
		{ "weapon_knife_cord", "]" },
		{ "weapon_knife_css", "]" },
		{ "weapon_knife_fachion", "]" },
		{ "weapon_knife_fip", "]" },
		{ "weapon_knife_gut", "]" },
		{ "weapon_knife_karambit", "]" },
		{ "weapon_knife_twinbade", "]" },
		{ "weapon_knife_kukri", "]" },
		{ "weapon_knife_m9_bayonet", "]" },
		{ "weapon_knife_outdoor", "]" },
		{ "weapon_knife_push", "]" },
		{ "weapon_knife_skeeton", "]" },
		{ "weapon_knife_stietto", "]" },
		{ "weapon_knife_tactica", "]" },
		{ "weapon_knife_widowmaker", "]" },
		{ "weapon_knife_ursus", "]" }
	};

	gunIcons[CS_XOR("P90")] = "P";
	gunIcons[CS_XOR("MP9")] = "O";
	gunIcons[CS_XOR("Mp5sd")] = "x";
	gunIcons[CS_XOR("M4a4")] = "M";
	gunIcons[CS_XOR("knife")] = "]";
	gunIcons[CS_XOR("DEagle")] = "A";
	gunIcons[CS_XOR("Elite")] = "B";
	gunIcons[CS_XOR("FiveSeven")] = "C";
	gunIcons[CS_XOR("Glock")] = "D";
	gunIcons[CS_XOR("Revolver")] = "J";
	gunIcons[CS_XOR("HKP2000")] = "E";
	gunIcons[CS_XOR("P250")] = "F";
	gunIcons[CS_XOR("Tec9")] = "H";
	gunIcons[CS_XOR("Cz75a")] = "I";
	gunIcons[CS_XOR("MAC10")] = "K";
	gunIcons[CS_XOR("Ump45")] = "L";
	gunIcons[CS_XOR("Bizon")] = "M";
	gunIcons[CS_XOR("MP7")] = "N";
	gunIcons[CS_XOR("GalilAR")] = "Q";
	gunIcons[CS_XOR("Famas")] = "R";
	gunIcons[CS_XOR("M4A1")] = "S";
	gunIcons[CS_XOR("Aug")] = "U";
	gunIcons[CS_XOR("SG556")] = "V";
	gunIcons[CS_XOR("AK47")] = "W";
	gunIcons[CS_XOR("G3SG1")] = "X";
	gunIcons[CS_XOR("SCAR20")] = "Y";
	gunIcons[CS_XOR("AWP")] = "Z";
	gunIcons[CS_XOR("SSG08")] = "a";
	gunIcons[CS_XOR("XM1014")] = "b";
	gunIcons[CS_XOR("Sawedoff")] = "c";
	gunIcons[CS_XOR("Mag7")] = "d";
	gunIcons[CS_XOR("NOVA")] = "e";
	gunIcons[CS_XOR("Negev")] = "f";
	gunIcons[CS_XOR("M249")] = "g";
	gunIcons[CS_XOR("Taser")] = "h";
	gunIcons[CS_XOR("Flashbang")] = "i";
	gunIcons[CS_XOR("hegrenade")] = "j";
	gunIcons[CS_XOR("SmokeGrenade")] = "k";
	gunIcons[CS_XOR("MolotovGrenade")] = "l";
	gunIcons[CS_XOR("DecoyGrenade")] = "m";
	gunIcons[CS_XOR("C4")] = "o";
	gunIcons[CS_XOR("CBaseAnimGraph")] = "r";
	gunIcons[CS_XOR("IncendiaryGrenade")] = "n";

	auto it = gunIcons.find(weapon);

	if (it != gunIcons.end())
		return it->second;

	return "";
}


//void PlayerESP::CalculateSkeleton(C_CSPlayerPawn* player)
//{
//	if (C_GET(bool, Vars.bSkeleton))
//	{
//		if (!player)
//			return;
//
//		auto game_scene_node = player->GetGameSceneNode();
//		if (!game_scene_node)
//			return;
//
//		auto skeleton = game_scene_node->GetSkeletonInstance();
//		if (!skeleton)
//			return;
//
//		auto model_state = &skeleton->m_modelState();
//		CStrongHandle<CModel> model = model_state->m_hModel();
//		auto model_skelet = &model->m_modelSkeleton();
//
//		if (!model_skelet)
//			return;
//
//		skeleton->calc_world_space_bones(bone_flags::FLAG_HITBOX);
//
//		const auto num_bones = model->GetHitboxesNum();
//		auto bones = model_state->m_pBones;
//
//		/* this method is not proper fuck it*/
//		/*CTransform* boneToWorldTransform = model_state->BoneTransform();
//		const Matrix3x4_t boneMatTransform = boneToWorldTransform->quatOrientation.ToMatrix(boneToWorldTransform->vecPosition);
//		/////////////////////////////////////////////////////////////////////////////////*/
//
//		//for (size_t entityIndex = 0; entityIndex < player->GetRefEHandle().GetEntryIndex(); entityIndex++)
//		{
//			for (uint32_t i = 0; i < num_bones; i++)
//			{
//				if (!(model->GetHitboxFlags(i) & bone_flags::FLAG_HITBOX))
//				{
//					continue;
//				}
//
//				auto parent_index = model->GetHitboxParent(i);
//				if (parent_index == -1)
//					continue;
//
//				ImVec2 start_scr, end_scr;
//				if (!D::WorldToScreen(bones[i].vecPosition, &start_scr) || !D::WorldToScreen(bones[parent_index].vecPosition, &end_scr))
//					continue;
//
//				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(start_scr.x, start_scr.y), ImVec2(end_scr.x, end_scr.y), Color_t(255, 255, 255).GetU32(), 2.f);
//				ImGui::GetBackgroundDrawList()->AddLine(ImVec2(start_scr.x, start_scr.y), ImVec2(end_scr.x, end_scr.y), Color_t(255, 255, 255).GetU32(), 1.f);
//				/*
//				char buf[16];
//				sprintf_s(buf, "%d", i); // Convert bone ID to a string
//				ImVec2 textPos = ImVec2(start_scr.x + (end_scr.x - start_scr.x) * 0.5f, start_scr.y + (end_scr.y - start_scr.y) * 0.5f);
//				ImGui::GetBackgroundDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), buf);
//		*/
//			}
//		}
//	}
//}


void PlayerESP::store_players()
{
	if (!I::Engine->IsInGame())
		return;

	const std::unique_lock<std::mutex> m(m_player_mutex);

	C_CSPlayerPawn* local_player = SDK::LocalPawn;

	std::erase_if(m_player_map, [](const auto& kv)
	{
			auto& player_info = kv.second;
			if (player_info.m_valid)
			{
				int current_index = player_info.m_handle == 0xffffffff ? 0x7fff : player_info.m_handle & 0x7fff;

				// in case when handle is still valid
				// entity will be found
				// otherwise it will be fucked OR it will be find wrong entity
				C_BaseEntity* entity = I::GameResourceService->pGameEntitySystem->Get(current_index);

				if (entity)
				{
					CCSPlayerController* player_controller = reinterpret_cast<CCSPlayerController*>(entity);

					if (!player_controller)
						return true;

					// if entity is wrong (for example it magically became some unknown prop)
					// erase it
					return !player_controller->IsPlayer();
				}
				else
					return true;
			}

			return false; });

	auto entitass = g_EntityList->get(CS_XOR("CCSPlayerController"));

	for (auto entity : entitass)
	{
		CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(entity);
		if (entity == 0)
			continue;

		if (!controller)
			continue;

		C_CSPlayerPawn* player_pawn = reinterpret_cast<C_CSPlayerPawn*>(I::GameResourceService->pGameEntitySystem->Get(controller->GetPawnHandle().GetEntryIndex()));
		if (!player_pawn)
			continue;
		if (!local_player)
			continue;

		int handle = player_pawn->GetRefEHandle().ToInt();
		int team_num = player_pawn->GetTeam();
		auto player_ptr = m_player_map.find(handle);

		bool is_local_player = (local_player && (player_pawn == local_player));
		bool is_teammate = /*(!C_GET(bool,Vars.bTeamCheck) &&*/ (local_player->GetTeam() == team_num);

		if (is_local_player || is_teammate || !player_pawn->IsAlive())
		{
			if (player_ptr != m_player_map.end())
				m_player_map.erase(player_ptr);

			continue;
		}

		if (player_ptr == m_player_map.end())
		{
			m_player_map.insert_or_assign(handle, player_info_t{});
			continue;
		}
		//g_rage_bot->debug(player_pawn);

		player_info_t& player_info = player_ptr->second;
		player_info.m_valid = true;
		player_info.m_handle = handle;
		player_info.m_bbox = calculate_bbox(player_pawn);
		player_info.m_health = player_pawn->GetHealth();
		player_info.m_name = controller->GetPlayerName();
		player_info.pawn = player_pawn;

		C_CSWeaponBase* active_weapon = player_pawn->GetWeaponActive();
		if (active_weapon)
		{
			CCSWeaponBaseVData* weapon_data = active_weapon->GetWeaponVData();
			if (weapon_data)
			{
				player_info.m_ammo = active_weapon->GetClip1();
				player_info.m_max_ammo = weapon_data->GetMaxClip1();

				const char* m_name = weapon_data->m_szName();
				//const char* name_length = strstr(m_name, "weapon_");
				//std::string weapon_name = name_length ? name_length + strlen("weapon_") : m_name;
				//std::transform(weapon_name.begin(), weapon_name.end(), weapon_name.begin(), ::toupper);

				player_info.m_weapon_name = std::move(m_name);
			}
		}
	}
}

Box PlayerESP::calculate_bbox(C_CSPlayerPawn* entity)
{
	if (!entity || !entity->IsPlayer())
		return {};

	Box box{};

	CGameSceneNode* scene_node = entity->GetGameSceneNode();
	if (!scene_node)
		return {};

	CPlayer_MovementServices* movement_services = entity->MovementServices();
	if (!movement_services)
		return {};


	ImVec2 flb, brt, blb, frt, frb, brb, blt, flt = {};
	float left, top, right, bottom = 0.f;

	Vector_t Min = entity->GetCollision()->GetMins() + scene_node->GetAbsOrigin();
	Vector_t Max = entity->GetCollision()->GetMaxs() + scene_node->GetAbsOrigin();

	std::array<Vector_t, 8> points = {
		Vector_t(Min.x, Min.y, Min.z),
		Vector_t(Min.x, Min.y, Max.z),
		Vector_t(Min.x, Max.y, Min.z),
		Vector_t(Min.x, Max.y, Max.z),
		Vector_t(Max.x, Min.y, Min.z),
		Vector_t(Max.x, Min.y, Max.z),
		Vector_t(Max.x, Max.y, Min.z),
		Vector_t(Max.x, Max.y, Max.z)
	};

	if (!D::WorldToScreen(points[3], &flb) || !D::WorldToScreen(points[5], &brt) || !D::WorldToScreen(points[0], &blb) || !D::WorldToScreen(points[4], &frt) || !D::WorldToScreen(points[2], &frb) || !D::WorldToScreen(points[1], &brb) || !D::WorldToScreen(points[6], &blt) || !D::WorldToScreen(points[7], &flt))
		return {};

	std::array<ImVec2, 8> arr = { flb, brt, blb, frt, frb, brb, blt, flt };

	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	for (std::int32_t i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;

		if (bottom < arr[i].y)
			bottom = arr[i].y;

		if (right < arr[i].x)
			right = arr[i].x;

		if (top > arr[i].y)
			top = arr[i].y;
	}

	box.x = left;
	box.y = top;
	box.width = right - left;
	box.height = bottom - top;
	box.m_found = true;
	return box;
}

void DrawLine(ImVec2 start, ImVec2 end, Color_t color, float thickness = 1.0f)
{
	ImU32 sd = color.GetU32();
	auto DrawList = ImGui::GetBackgroundDrawList();
	DrawList->AddLine(start, end, sd, thickness);
}

void DrawCube(const Vector_t& origin, float size, Color_t color)
{
	// Определяем 8 вершин куба в мировых координатах
	Vector_t vertices[8] = {
		{ origin.x - size, origin.y - size, origin.z - size }, // 0
		{ origin.x + size, origin.y - size, origin.z - size }, // 1
		{ origin.x + size, origin.y + size, origin.z - size }, // 2
		{ origin.x - size, origin.y + size, origin.z - size }, // 3
		{ origin.x - size, origin.y - size, origin.z + size }, // 4
		{ origin.x + size, origin.y - size, origin.z + size }, // 5
		{ origin.x + size, origin.y + size, origin.z + size }, // 6
		{ origin.x - size, origin.y + size, origin.z + size }, // 7
	};

	// Массив для хранения экранных координат
	ImVec2 screenPoints[8];

	// Преобразуем мировые координаты вершин в экранные координаты
	for (int i = 0; i < 8; i++)
	{
		if (!D::WorldToScreen(vertices[i], &screenPoints[i]))
		{
			return; // Если хотя бы одна точка вне экрана, не рисуем куб
		}
	}

	// Рисуем линии для каждой грани куба
	DrawLine(screenPoints[0], screenPoints[1], color);
	DrawLine(screenPoints[1], screenPoints[2], color);
	DrawLine(screenPoints[2], screenPoints[3], color);
	DrawLine(screenPoints[3], screenPoints[0], color);

	DrawLine(screenPoints[4], screenPoints[5], color);
	DrawLine(screenPoints[5], screenPoints[6], color);
	DrawLine(screenPoints[6], screenPoints[7], color);
	DrawLine(screenPoints[7], screenPoints[4], color);

	DrawLine(screenPoints[0], screenPoints[4], color);
	DrawLine(screenPoints[1], screenPoints[5], color);
	DrawLine(screenPoints[2], screenPoints[6], color);
	DrawLine(screenPoints[3], screenPoints[7], color);

	// Отображаем изображение на передней грани куба (грань 0-1-2-3)
	ImVec2 uvMin = ImVec2(0.0f, 0.0f); // Начальные UV-координаты изображения
	ImVec2 uvMax = ImVec2(1.0f, 1.0f); // Конечные UV-координаты изображения

	// Рисуем изображение внутри передней грани куба
	ImGui::GetForegroundDrawList()->AddImage((void*)I::Maintexture, screenPoints[0], screenPoints[2], uvMin, uvMax);
}

float lerpSpeed = 0.0050f; 
Vector_t cubePosition = { 0.0f, 0.0f, 0.0f }; 
Vector_t previousCameraPosition = { 0.0f, 0.0f, 0.0f };

Vector_t offsetFromCamera = { 0.0f, 0.0f, -10.0f }; 

Vector_t Lerp(const Vector_t& start, const Vector_t& end, float t)
{
	return {
		start.x + (end.x - start.x) * t,
		start.y + (end.y - start.y) * t,
		start.z + (end.z - start.z) * t
	};
}

void UpdateCubePosition(C_CSPlayerPawn* pLocalPawn)
{
	if (!SDK::Alive)
		return;

	if (!pLocalPawn)
		return;

	auto game_scene_node = pLocalPawn->GetGameSceneNode();
	if (!game_scene_node)
		return;

	auto skeleton = game_scene_node->GetSkeletonInstance();
	if (!skeleton)
		return;
	if (!skeleton->m_modelState().m_pBones)
		return;

	Vector_t cameraPosition = skeleton->m_modelState().m_pBones[6].vecPosition;

	// Рассчитываем направление движения игрока (сглаженное)
	Vector_t direction = {
		cameraPosition.x - previousCameraPosition.x,
		cameraPosition.y - previousCameraPosition.y,
		cameraPosition.z - previousCameraPosition.z
	};

	// Используем Lerp для плавного изменения направления куба, чтобы смещение было мягким
	Vector_t smoothDirection = Lerp({ 0, 0, 0 }, direction, 0.1f); // Сглаживаем направление

	// Целевая позиция куба с учетом плавного движения
	Vector_t targetPosition = {
		cameraPosition.x + offsetFromCamera.x - smoothDirection.x, // Плавное смещение по X
		cameraPosition.y + offsetFromCamera.y, // По Y остается на заданной высоте
		cameraPosition.z + offsetFromCamera.z - smoothDirection.z // Плавное смещение по Z
	};

	// Плавная интерполяция позиции куба к целевой позиции
	cubePosition = Lerp(cubePosition, targetPosition, lerpSpeed);

	// Обновляем предыдущую позицию камеры для вычисления направления в следующем кадре
	previousCameraPosition = cameraPosition;

	// Рисуем куб в новой позиции
	Color_t color = Color_t(230, 230, 250); // Цвет куба (красный)
	float cubeSize = 5.0f;
	DrawCube(cubePosition, cubeSize, color);
}

inline void angle_vectors(const Vector_t& angles, Vector_t& forward)
{
	float sp, sy, cp, cy;

	MATH::sin_cos(MATH::deg2rad(angles[1]), &sy, &cy);
	MATH::sin_cos(MATH::deg2rad(angles[0]), &sp, &cp);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void rotate_triangle(std::array<ImVec2 , 3>& points, float rotation)
{
	const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
	for (auto& point : points)
	{
		point -= points_center;

		const auto temp_x = point.x;
		const auto temp_y = point.y;

		const auto theta = MATH::deg2rad(rotation);
		const auto c = cosf(theta);
		const auto s = sinf(theta);

		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;

		point += points_center;
	}
}

ImVec2 RotateVertex(const ImVec2& p, const ImVec2& v, float angle)
{
	float c = std::cos(MATH::deg2rad(angle));
	float s = std::sin(MATH::deg2rad(angle));

	return {
		p.x + (v.x - p.x) * c - (v.y - p.y) * s,
		p.y + (v.x - p.x) * s + (v.y - p.y) * c
	};
}

//void draw_fov(C_CSPlayerPawn* player)
//{
//	if (C_GET(bool, Vars.bEnableFovVisualize))
//		return;
//
//	if (!SDK::Alive)
//		return;
//
//	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
//	ImVec2 screen_pos, offscreen_pos;
//	float leeway_x, leeway_y, radius, offscreen_rotation;
//	bool is_on_screen;
//	
//	leeway_x = SDK::ScreenSize.x / 18.f;
//	leeway_y = SDK::ScreenSize.y / 18.f;
//
//	float radius1 = C_GET(float, Vars.flAimRange) / 90.f * leeway_x / 2;
//
//	draw_list->AddCircle(screen_pos, C_GET(float, Vars.flAimRange), Color_t(255, 255, 255).GetU32(), leeway_x / 2, leeway_y / 2);
//}

void draw_oof(C_CSPlayerPawn* player)
{
	if (!SDK::LocalPawn)
		return;
	if (!SDK::Alive)
		return;
	if (!player)
		return;
	if (!I::GlobalVars)
		return;
	if (!player->GetGameSceneNode())
		return;
	Vector_t view_origin, target_pos, delta;
	ImVec2 screen_pos, offscreen_pos;
	float leeway_x, leeway_y, radius, offscreen_rotation;
	bool is_on_screen;
	std::array<ImVec2, 3> verts;

	auto get_offscreen_data = [](const Vector_t& delta, float radius, ImVec2& out_offscreen_pos, float& out_rotation)
	{
		Vector_t view_angles = { g_AntiAim->StoreAngels.x,g_AntiAim->StoreAngels.y,g_AntiAim->StoreAngels.z };
		Vector_t fwd, right, up(0.f, 0.f, 1.f);
		float front, side, yaw_rad, sa, ca;

		angle_vectors(view_angles, fwd);
		fwd.z = 0.f;
		fwd.normalize();

		right = up.cross(fwd);
		front = delta.dot(fwd);
		side = delta.dot(right);

		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		out_rotation = MATH::rad2deg(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + MATH::_PI);

		yaw_rad = MATH::deg2rad(-out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		out_offscreen_pos.x = (int)((SDK::ScreenSize.x / 2.f) + (radius * sa));
		out_offscreen_pos.y = (int)((SDK::ScreenSize.y / 2.f) - (radius * ca));
	};
	
	target_pos = player->GetGameSceneNode()->GetAbsOrigin();
	if (target_pos.IsZero())
		return;

	is_on_screen = D::WorldToScreen(target_pos, &screen_pos);

	leeway_x = SDK::ScreenSize.x / 18.f;
	leeway_y = SDK::ScreenSize.y / 18.f;

	if (!is_on_screen || screen_pos.x < -leeway_x || screen_pos.x > (SDK::ScreenSize.x + leeway_x) || screen_pos.y < -leeway_y || screen_pos.y > (SDK::ScreenSize.y + leeway_y))
	{
		view_origin = SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin();
		delta = (target_pos - view_origin).normalized();

		radius = 200.f * (SDK::ScreenSize.y / 480.f);

		get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

		offscreen_rotation = -offscreen_rotation;

		verts[0] = { offscreen_pos.x, offscreen_pos.y };
		verts[1] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f };
		verts[2] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f };

		verts[0] = RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
		verts[1] = RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
		verts[2] = RotateVertex(offscreen_pos, verts[2], offscreen_rotation);
		static auto swap = false;
		static float alpha = 255.f;
		static auto last_framecount = 0;

		if (alpha == 255.f)
		{
			swap = false;
		}
		if (alpha == 0.f)
		{
			swap = true;
		}
		if (last_framecount != I::GlobalVars->iFrameCount)
		{
			if (swap)
				alpha += 255.f / 0.73f * I::GlobalVars->flFrametime;
			else
				alpha -= 255.f / 0.73f * I::GlobalVars->flFrametime;
			alpha = ImClamp(alpha, 0.f, 255.f);
			last_framecount = I::GlobalVars->iFrameCount;
		}
		Color_t color = { 255,255,255,(int)alpha };

		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
		draw_list->AddTriangleFilled(
		ImVec2(verts[0].x, verts[0].y),
		ImVec2(verts[1].x, verts[1].y),
		ImVec2(verts[2].x, verts[2].y),
		color.GetU32());
	}
}



void PlayerESP::handle_players()
{
	const std::unique_lock<std::mutex> m(m_player_mutex);

	if (!I::Engine->IsInGame())
		return;

	if (SDK::LocalPawn)
	{
		if (C_GET(bool, Vars.bDrawCube))
			UpdateCubePosition(SDK::LocalPawn);
		//GrenadeVisuals::present();

	}

	for (auto it = m_player_map.begin(); it != m_player_map.end(); it = std::next(it))
	{
		player_info_t& player_info = it->second;
		if (!player_info.m_valid)
			continue;
		if (C_GET(bool, Vars.bOof))
			draw_oof(player_info.pawn);

		if (player_info.m_bbox.m_found)
		{
			Box bbox = player_info.m_bbox;
			ragebot::debug_multipoints();
			ImGui::GetBackgroundDrawList()->AddShadowCircle(g_RageBot->v, 20, Color_t(255, 255, 255).GetU32(), 10,ImVec2(5,5));
			ImGui::GetBackgroundDrawList()->AddShadowCircle(g_RageBot->svo, 20, Color_t(255,0 , 0).GetU32(), 10, ImVec2(5, 5));

			static const Color_t outline_color = Color_t{ 0.f, 0.f, 0.f, 1.f };
			if (C_GET(FrameOverlayVar_t, Vars.overlayBox).bEnable)
			{
				const Color_t box_color = C_GET(Color_t, Vars.colBox);

				if (C_GET(int, Vars.BoxType) == FilledBox)
				{
					ImGui::GetBackgroundDrawList()->AddShadowRect({ bbox.x - 1, bbox.y - 1 }, { bbox.x + bbox.width + 1, bbox.y + bbox.height + 1 }, box_color.GetU32(), 4.f, ImVec2(2, 2));
					ImGui::GetBackgroundDrawList()->AddShadowRect({ bbox.x, bbox.y }, { bbox.x + bbox.width, bbox.y + bbox.height }, box_color.GetU32(), 4.f, ImVec2(2, 2));
					ImGui::GetBackgroundDrawList()->AddShadowRect({ bbox.x + 1, bbox.y + 1 }, { bbox.x + bbox.width - 1, bbox.y + bbox.height - 1 }, box_color.GetU32(), 4.f, ImVec2(2, 2));
				}

				if (C_GET(int, Vars.BoxType) == DefaultBox)
				{
					D::RectOutline({ bbox.x - 1, bbox.y - 1 }, { bbox.x + bbox.width + 1, bbox.y + bbox.height + 1 }, outline_color, 1, 0);
					D::RectOutline({ bbox.x, bbox.y }, { bbox.x + bbox.width, bbox.y + bbox.height }, box_color, 1, 0);
					D::RectOutline({ bbox.x + 1, bbox.y + 1 }, { bbox.x + bbox.width - 1, bbox.y + bbox.height - 1 }, outline_color, 1, 0);
				}

				if (C_GET(int, Vars.BoxType) == CornerBox)
				{


					//const ImVec4& vecBox();
					//ImDrawList* pDrawList;
					//float flThickness;
					//const Color_t& colPrimary();
					//const Color_t& colOutline();

					//flThickness = std::floorf(flThickness);
					//const ImVec2 vecThicknessOffset = { flThickness, flThickness };



					//// corner part of the whole line
					//constexpr float flPartRatio = 0.25f;

					//const float flCornerWidth = ((vecBox[SIDE_RIGHT] - vecBox[SIDE_LEFT]) * flPartRatio);
					//const float flCornerHeight = ((vecBox[SIDE_BOTTOM] - vecBox[SIDE_TOP]) * flPartRatio);

					//const  arrCornerPoints[4][3] = {
					//	// top-left
					//	{ bbox.x, vecBox[SIDE_TOP] + flCornerHeight) + vecThicknessOffset, ImVec2(vecBox[SIDE_LEFT], vecBox[SIDE_TOP]) + vecThicknessOffset, ImVec2(vecBox[SIDE_LEFT] + flCornerWidth, vecBox[SIDE_TOP]) + vecThicknessOffset },

					//	// top-right
					//	{ bbox.y - flCornerWidth - vecThicknessOffset.x, vecBox[SIDE_TOP] + vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_RIGHT] - vecThicknessOffset.x, vecBox[SIDE_TOP] + vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_RIGHT] - vecThicknessOffset.x, vecBox[SIDE_TOP] + flCornerHeight + vecThicknessOffset.y * 2.0f) },

					//	// bottom-left
					//	{ ImVec2(vecBox[SIDE_LEFT] + flCornerWidth + vecThicknessOffset.x, vecBox[SIDE_BOTTOM] - vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_LEFT] + vecThicknessOffset.x, vecBox[SIDE_BOTTOM] - vecThicknessOffset.y * 2.0f), ImVec2(vecBox[SIDE_LEFT] + vecThicknessOffset.x, vecBox[SIDE_BOTTOM] - flCornerHeight - vecThicknessOffset.y * 2.0f) },

					//	// bottom-right
					//	{ ImVec2(vecBox[SIDE_RIGHT], vecBox[SIDE_BOTTOM] - flCornerHeight) - vecThicknessOffset, ImVec2(vecBox[SIDE_RIGHT], vecBox[SIDE_BOTTOM]) - vecThicknessOffset, ImVec2(vecBox[SIDE_RIGHT] - flCornerWidth, vecBox[SIDE_BOTTOM]) - vecThicknessOffset }
					//};

					//for (std::size_t i = 0U; i < CS_ARRAYSIZE(arrCornerPoints); i++)
					//{
					//	const auto& arrLinePoints = arrCornerPoints[i];
					//	const ImVec2 vecHalfPixelOffset = ((i & 1U) == 1U ? ImVec2(-0.5f, -0.5f) : ImVec2(0.5f, 0.5f));

					//	// @todo: we can even do not clear path and reuse it
					//	ImGui::GetBackgroundDrawList()->PathLineTo(arrLinePoints[0] + vecHalfPixelOffset);
					//	ImGui::GetBackgroundDrawList()->PathLineTo(arrLinePoints[1] + vecHalfPixelOffset);
					//	ImGui::GetBackgroundDrawList()->PathLineTo(arrLinePoints[2] + vecHalfPixelOffset);
					//	ImGui::GetBackgroundDrawList()->PathStroke(main_color.GetU32(), false, flThickness);

					//	ImGui::GetBackgroundDrawList()->PathLineTo(arrLinePoints[0] + vecHalfPixelOffset);
					//	ImGui::GetBackgroundDrawList()->PathLineTo(arrLinePoints[1] + vecHalfPixelOffset);
					//	ImGui::GetBackgroundDrawList()->PathLineTo(arrLinePoints[2] + vecHalfPixelOffset);
					//	ImGui::GetBackgroundDrawList()->PathStroke(main_color.GetU32(), false, flThickness);
					//}
				}
			}
			if (C_GET(bool, Vars.bHealthBar))
			{
				Color_t full_hp_color = C_GET(Color_t, Vars.colFullHealthBar);

				Color_t low_hp_color = C_GET(Color_t, Vars.colLowHealthBar);

				int esp_health = std::clamp(player_info.m_health, 0, 100);
				float esp_step = std::clamp(static_cast<float>(esp_health) / 100.f, 0.f, 1.f);

				float height = (bbox.height - ((bbox.height * static_cast<float>(esp_health)) / 100.f));
				height = std::max<float>(std::min<float>(height, bbox.height), 0.f);

				Vector_t bb_outline_min{ bbox.x - 5.f, bbox.y - 1.f };
				Vector_t bb_outline_max{ 3.f, bbox.height + 2.f };

				Vector_t bb_min{ bbox.x - 4.f, bbox.y + height };
				Vector_t bb_max{ 2.f, bbox.height - height };

				D::Rect(bb_outline_min, bb_outline_min + bb_outline_max, outline_color, 0);
				D::Rect(bb_min, bb_min + bb_max, low_hp_color.lerp(full_hp_color, esp_step), 0);

				// TO-DO: fix position and color (@evj_k)
				if (esp_health > 0 && esp_health < 100)
				{
					ImGui::PushFont(D::fonts.onetap_pixel);
					auto text_size = ImGui::CalcTextSize(std::to_string(esp_health).data());
					Vector_t text_pos{ bbox.x - 7.f, bbox.y + bbox.height / 2.f - height - text_size.y };

					D::RenderText(text_pos, Color_t{ 1.f, 1.f, 1.f, 1.f }, font_flags_center | font_flags_dropshadow,
					D::fonts.onetap_pixel, std::to_string(esp_health).data(), 1);

					ImGui::PopFont();
				}
			}

			if (C_GET(bool, Vars.bSkeleton))
			{
				if (!player_info.pawn)
					return;

				auto game_scene_node = player_info.pawn->GetGameSceneNode();
				if (!game_scene_node)
					return;

				auto skeleton = game_scene_node->GetSkeletonInstance();
				if (!skeleton)
					return;

				auto model_state = &skeleton->m_modelState();
				CStrongHandle<CModel> model = model_state->m_hModel();
				auto model_skelet = &model->m_modelSkeleton();

				if (!model_skelet)
					return;

				skeleton->calc_world_space_bones(bone_flags::FLAG_HITBOX);

				const auto num_bones = model->GetHitboxesNum();
				auto bones = model_state->m_pBones;

				/* this method is not proper fuck it*/
				/*CTransform* boneToWorldTransform = model_state->BoneTransform();
				const Matrix3x4_t boneMatTransform = boneToWorldTransform->quatOrientation.ToMatrix(boneToWorldTransform->vecPosition);
				/////////////////////////////////////////////////////////////////////////////////*/

				//for (size_t entityIndex = 0; entityIndex < player->GetRefEHandle().GetEntryIndex(); entityIndex++)
				{
					for (uint32_t i = 0; i < num_bones; i++)
					{
						if (!(model->GetHitboxFlags(i) & bone_flags::FLAG_HITBOX))
						{
							continue;
						}

						auto parent_index = model->GetHitboxParent(i);
						if (parent_index == -1)
							continue;

						ImVec2 start_scr, end_scr;
						if (!D::WorldToScreen(bones[i].vecPosition, &start_scr) || !D::WorldToScreen(bones[parent_index].vecPosition, &end_scr))
							continue;

						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(start_scr.x, start_scr.y), ImVec2(end_scr.x, end_scr.y), C_GET(Color_t, Vars.colSkeleton).GetU32(), 2.f);
						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(start_scr.x, start_scr.y), ImVec2(end_scr.x, end_scr.y), C_GET(Color_t, Vars.colSkeleton).GetU32(), 1.f);
						/*
						char buf[16];
						sprintf_s(buf, "%d", i); // Convert bone ID to a string
						ImVec2 textPos = ImVec2(start_scr.x + (end_scr.x - start_scr.x) * 0.5f, start_scr.y + (end_scr.y - start_scr.y) * 0.5f);
						ImGui::GetBackgroundDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), buf);
				*/
					}
				}
			}

			if (C_GET(bool, Vars.bNameEsp))
			{
				const Color_t name_color = C_GET(Color_t, Vars.colName);

				ImGui::PushFont(D::fonts.verdana_small);
				auto text_size = ImGui::CalcTextSize(player_info.m_name.c_str());
				D::RenderText(Vector_t(bbox.x + bbox.width / 2.f, bbox.y - text_size.y - 3.f),
				name_color, font_flags_center | font_flags_dropshadow,
				D::fonts.verdana_small, player_info.m_name,
				D::fonts.verdana_small->FontSize);
				ImGui::PopFont();
			}
			if (C_GET(bool, Vars.bAmmoBar))
			{
				const auto ammobar_color = C_GET(Color_t, Vars.colAmmobar);

				auto full_ammo_color = C_GET(Color_t, Vars.colAmmobar);
				auto low_ammo_color = C_GET(Color_t, Vars.colAmmobar);

				float text_offset = 0.f;
				if (player_info.m_ammo > 0)
				{
					float esp_step = std::clamp(static_cast<float>(player_info.m_ammo) / static_cast<float>(player_info.m_max_ammo), 0.f, 1.f);

					float width = ((bbox.width * static_cast<float>(player_info.m_ammo)) / static_cast<float>(player_info.m_max_ammo));
					width = std::max<float>(std::min<float>(width, bbox.width), 0.f);

					Vector_t bb_outline_min{ bbox.x - 1.f, bbox.y + bbox.height + 2.f };
					Vector_t bb_outline_max{ bbox.width + 2.f, 2.f };

					Vector_t bb_min{ bbox.x, bbox.y + bbox.height + 2.f };
					Vector_t bb_max{ width, 2.f };

					D::Rect(bb_outline_min, bb_outline_min + bb_outline_max, outline_color, 0);
					D::Rect(bb_min, bb_min + bb_max, low_ammo_color.lerp(full_ammo_color, esp_step), 0);

					text_offset += 5.f;
				}
			}

			if (C_GET(bool, Vars.bWeaponName))
			{
				const Color_t weaponname_color = C_GET(Color_t, Vars.colWeaponName);

				Vector_t weaponname_pos{ bbox.x + bbox.width / 2.f, bbox.y + bbox.height + 6.f };

				ImGui::PushFont(D::fonts.verdana_small);

				D::RenderText(weaponname_pos, weaponname_color, font_flags_center | font_flags_dropshadow,
					D::fonts.verdana_small, player_info.m_weapon_name,
					D::fonts.verdana_small->FontSize);
				ImGui::PopFont();
			}

			if (C_GET(bool, Vars.bWeaponIcon))
			{
				const Color_t weaponicon_color = C_GET(Color_t, Vars.colWeaponIcon);

				if (C_GET(bool, Vars.bWeaponName))
				{
					Vector_t weapon_pos{ bbox.x + bbox.width / 2.f, bbox.y + bbox.height + 18.f };

					D::RenderText(weapon_pos, weaponicon_color, font_flags_center | font_flags_dropshadow,
						FONT::WeaponFont, GunIcon(player_info.m_weapon_name), 15);
				}

				else if (!C_GET(bool, Vars.bWeaponName))
				{
					Vector_t weapon_pos{ bbox.x + bbox.width / 2.f, bbox.y + bbox.height + 6.f };

					D::RenderText(weapon_pos, weaponicon_color, font_flags_center | font_flags_dropshadow,
						FONT::WeaponFont, GunIcon(player_info.m_weapon_name), 15);
				}
			}

			const Color_t main_color = Color_t{ 1.f, 1.f, 1.f, 1.f };
			Vector_t flags_pos{ bbox.x + bbox.height + 0.f, bbox.y + bbox.width + -100.f };

			//if (C_GET(unsigned int, Vars.pEspFlags) & FLAGS_ARMOR && player_info.pawn->GetArmorValue() > 0)
			//{
			//	ImGui::PushFont(D::fonts.verdana_small);
			//	D::RenderText(flags_pos, main_color, font_flags_center | font_flags_outline,
			//	D::fonts.verdana_small, "K",
			//	D::fonts.verdana_small->FontSize);
			//	//ImGui::PopFont();
			//}

			
		}
	}
}

//void OVERLAY::Player(CCSPlayerController* pLocal, CCSPlayerController* pPlayer, const float flDistance)
//{
//	C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocal->GetPawnHandle());
//	C_CSPlayerPawn* pPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
//
//	if (pLocalPawn == nullptr || pPlayerPawn == nullptr)
//		return;
//
//
//	bool bIsEnemy = (pLocalPawn->IsOtherEnemy(pPlayerPawn));
//
//	// @note: only enemy overlay for now
//	if (!bIsEnemy)
//		return;
//
//	ImVec4 vecBox = {};
//	//if (!GetEntityBoundingBox(pPlayerPawn, &vecBox))
//	//	return;
//
//	//Context_t context;
//
//	//if (const auto& frameOverlayConfig = C_GET(FrameOverlayVar_t, Vars.overlayBox); frameOverlayConfig.bEnable)
//	//	context.AddBoxComponent(ImGui::GetBackgroundDrawList(), vecBox, 1, frameOverlayConfig.flThickness, frameOverlayConfig.flRounding, frameOverlayConfig.colPrimary, frameOverlayConfig.colOutline);
//
//	//if (const auto& nameOverlayConfig = C_GET(TextOverlayVar_t, Vars.overlayName); nameOverlayConfig.bEnable)
//	//{
//	//	const char* szPlayerName = pPlayer->GetPlayerName();
//	//	context.AddComponent(new CTextComponent(false, SIDE_TOP, DIR_TOP, FONT::pVisual, szPlayerName, Vars.overlayName));
//	//}
//
//	//if (const auto& healthOverlayConfig = C_GET(BarOverlayVar_t, Vars.overlayHealthBar); healthOverlayConfig.bEnable)
//	//{
//	//	// @note: pPlayerPawn->GetMaxHealth() sometime return 0.f
//	//	const float flHealthFactor = pPlayerPawn->GetHealth() / 100.f;
//	//	context.AddComponent(new CBarComponent(false, SIDE_LEFT, vecBox, flHealthFactor, Vars.overlayHealthBar));
//	//}
//
//	//if (const auto& armorOverlayConfig = C_GET(BarOverlayVar_t, Vars.overlayArmorBar); armorOverlayConfig.bEnable)
//	//{
//	//	const float flArmorFactor = pPlayerPawn->GetArmorValue() / 100.f;
//	//	context.AddComponent(new CBarComponent(false, SIDE_BOTTOM, vecBox, flArmorFactor, Vars.overlayArmorBar));
//	//}
//
//	// render all the context
////	context.Render(ImGui::GetBackgroundDrawList(), vecBox);
//}
