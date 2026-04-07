#pragma once

#include "../../common.h"

// used: draw system
#include "../../utilities/draw.h"
#include <unordered_map>
#include <memory>
#include <mutex>

class CCSPlayerController;
class C_BaseEntity;
class C_CSPlayerPawn;
class C_CSWeaponBase;
class SchemaClassInfoData_t;
namespace F::VISUALS::OVERLAY
{
	/* @section: main */
	// draw box, bars, text infos, etc at player position
}

struct WeaponEntity
{
	C_CSWeaponBase* CSWeaponBase = nullptr;
	Vector_t WorldPosition{ 0, 0, 0 };
	std::string Name;
	bool isWeapon = false;
};

inline std::unique_ptr<WeaponEntity> cWeaponEntity = std::make_unique<WeaponEntity>();

inline std::vector<WeaponEntity> WeaponListA, WeaponListB;
inline std::vector<WeaponEntity>*CurrentWeaponList = &WeaponListA, *NextWeaponList = &WeaponListB;

static const std::unordered_map<std::string, size_t> ClassTrimMap = {
	{ "C_Knife", 2 },
	{ "C_Weapon", 8 },
	{ "C_DEagle", 2 },
	{ "C_Flashbang", 2 },
	{ "C_SmokeGrenade", 2 },
	{ "C_HEGrenade", 2 },
	{ "C_IncendiaryGrenade", 2 },
	{ "C_DecoyGrenade", 2 },
	{ "C_WeaponSCAR20", 8 },
	{ "C_C4", 2 },
	{ "C_AK47", 2 },
	{ "C_MolotovGrenade", 2 },
	{ "C_PlantedC4", 9 },
	{ "CBaseAnimGraph", 0 }

};
/////////////////////////////////
struct Box
{
	bool m_found = false;

	float x;
	float y;
	float width;
	float height;
	float offset_xui = 43.f;
	float offset_yaw;
};

enum FontFlags
{
	font_flags_center = 1 << 0,
	font_flags_outline = 1 << 1,
	font_flags_dropshadow = 1 << 2
};

class PlayerESP final
{
	enum EspType
	{
		EnemyType,
		TeamType,
		LocalType,
	};

	struct player_info_t
	{
		bool m_valid = false;

		int m_handle;
		int m_health;
		int m_ammo;
		int m_max_ammo;

		Box m_bbox;
		ImVec4 position;
		std::string m_name;
		std::string m_weapon_name;
		C_CSPlayerPawn* pawn;
		CCSPlayerController* controller;
	};

	int Type{};

public:
	void store_players();
	Box calculate_bbox(C_CSPlayerPawn* entity);
	void handle_players();
	std::mutex m_player_mutex;
	std::unordered_map<int, player_info_t> m_player_map;

};

inline const std::unique_ptr<PlayerESP> g_Esp{ new PlayerESP() };
