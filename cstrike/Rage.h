#pragma once
#include "sdk/datatypes/vector.h"
#include "lagcompensation.h"
#include "EntityCache.h"
#include <vector>
#include "Utils.h"
#include <future>
#include "Utils.h"
#include "AntiAim.h"
#include "sdk/EntityList/AutoWall.h"
#include <DirectXMath.h>

enum EHitBoxes : std::int32_t
{
	HITBOX_INVALID = -1,
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_THORAX,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_MAX
};

enum bone_indexes : uint32_t
{
	HEAD = 6,
	NECK = 5,
	CHEST = 4,
	RIGHT_CHEST = 8,
	LEFT_CHEST = 13,
	STOMACH = 3,
	PELVIS = 2,
	CENTER = 1,
	L_LEG = 23,
	L_FEET = 24,
	R_LEG = 26,
	R_FEET = 27
};


struct multipoint_set {
	bool top = false;
	bool bottom = false;
	bool left = false;
	bool right = false;
	bool top_right = false;
	bool bottom_left = false;
	float pointscale = 35.f;
};
static Vector_t DUCKPEEKASSIST_EYEPOS{};
inline std::unordered_map<EHitBoxes, std::vector<multipoint_set>> pointscan_config;

class scan {

public:
	struct target_info
	{
		float damage;
		Vector_t point;
		EntityCache::cached_entity target;
		lag_record_t record;
		float hitbox_radius;
		Vector_t hitbox_min;
		Vector_t hitbox_max;
		int bone_index;
		int hitgroup;
		float delta_time;
	};

	static std::string getHitgroupString(int hitbox)
	{
		// Map enum values to string in a static array for fast lookup
		static const std::array<std::string, HITBOX_MAX> hitboxStrings = {
			"head", "neck", "pelvis", "stomach", "thorax", "chest", "upper_chest",
			"left_thigh", "right_thigh", "left_calf", "right_calf", "left_foot", "right_foot",
			"left_hand", "right_hand", "left_upper_arm", "left_forearm", "right_upper_arm", "right_forearm"
		};

		// Check for valid hitbox enum, otherwise return "invalid"
		if (hitbox < 0 || hitbox >= HITBOX_MAX) {
			return hitboxStrings[0]; // "invalid"
		}

		return hitboxStrings[hitbox];
	}

	static constexpr int MapHitgroupToBone(int group)
	{
		switch (group)
		{
		case HITBOX_HEAD:
			return bone_indexes::HEAD;
		case HITBOX_NECK:
			return bone_indexes::NECK;
		case HITBOX_CHEST:
		case HITBOX_THORAX:
		case HITBOX_UPPER_CHEST:
			return bone_indexes::CHEST;
		case HITBOX_PELVIS:
			return bone_indexes::PELVIS;
		case HITBOX_STOMACH:
			return bone_indexes::STOMACH;
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
			return bone_indexes::L_LEG;
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
			return bone_indexes::R_LEG;
		case HITBOX_LEFT_FOOT:
			return bone_indexes::L_FEET;
		case HITBOX_RIGHT_FOOT:
			return bone_indexes::R_FEET;
		default:
			return bone_indexes::CENTER;
		}
	}

	std::vector<Vector_t> aimpoints_;
	static void process_player(EntityCache::cached_entity& entity, C_CSPlayerPawn* pLocalPawn, C_BaseEntity* local_ent_base, CUserCmd* pCmd, target_info& bestInfo);
	static bool calculate_best_point(EntityCache::cached_entity& entity, C_CSPlayerPawn* local_pawn, C_BaseEntity* local_ent_base, C_BaseEntity* target_ent_base, const lag_record_t& lag_record, target_info& best_info, bool has_heavy_armor, int team, int health);
	static std::array<Vector_t, 9> GenerateMultipointPositions(const Vector_t& center, float rs, const Vector_t& forward);
	static std::vector<Vector_t> GetMultipointPositions(const lag_record_t& lagRecord, int group, float rad, const Vector_t& min, const Vector_t& max);
};

class ragebot {
public:

	//static void check_min_key() {

	//	GLOBAL_MINDAMAGE = qd_data::active_weapon_specific_config->mindamage;
	//	HOLDING_MIN = false;
	//	if (GetAsyncKeyState(0x0005) & 0x8000)
	//	{
	//		GLOBAL_MINDAMAGE = qd_data::active_weapon_specific_config->mindamage_override;
	//		HOLDING_MIN = true;
	//	}
	//	return;
	//}

	using removed_aimfunction_t = void* (__fastcall*)(void*, QAngle_t*, float, bool);
	inline static removed_aimfunction_t fnUpdateAimPunch_fn;

	static QAngle_t get_removed_aim_punch_angle(C_CSPlayerPawn* local_player)
	{
		QAngle_t aim_punch = {};
		fnUpdateAimPunch_fn(local_player, &aim_punch, 0.0f, true);
		return aim_punch;
	}
	
	static Vector_t calculate_spread_internal(int seed, float accuracy, float spread, float recoil_index, int item_def_idx);
	static void Hook();
	static void debug_multipoints();
	float calculate_roll_compensation(float desired_yaw, float desired_pitch, float current_yaw, float current_pitch);
	static void autostop();
	static bool hitchance(Vector_t aim_point, QAngle_t angle, float hitbox_rad, C_CSWeaponBase* weapon);
	static void run();
};