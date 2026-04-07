#pragma once
#include "sdk/entity.h"
#include "core/variables.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/datatypes/usercmd.h"
#include "sdk/EntityList/AutoWall.h"
#include "core/sdk.h"
#include <deque>
#include <unordered_map>
#include <optional>
#include "Lagcomp.h"
#include "pred.h"

enum contents_t
{
	CONTENTS_EMPTY = 0,
	CONTENTS_SOLID = 0x1,
	CONTENTS_WINDOW = 0x2,
	CONTENTS_AUX = 0x4,
	CONTENTS_GRATE = 0x8,
	CONTENTS_SLIME = 0x10,
	CONTENTS_WATER = 0x20,
	CONTENTS_BLOCKLOS = 0x40,
	CONTENTS_OPAQUE = 0x80,
	CONTENTS_TESTFOGVOLUME = 0x100,
	CONTENTS_UNUSED = 0x200,
	CONTENTS_BLOCKLIGHT = 0x400,
	CONTENTS_TEAM1 = 0x800,
	CONTENTS_TEAM2 = 0x1000,
	CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
	CONTENTS_MOVEABLE = 0x4000,
	CONTENTS_AREAPORTAL = 0x8000,
	CONTENTS_PLAYERCLIP = 0x10000,
	CONTENTS_MONSTERCLIP = 0x20000,
	CONTENTS_CURRENT_0 = 0x40000,
	CONTENTS_CURRENT_90 = 0x80000,
	CONTENTS_CURRENT_180 = 0x100000,
	CONTENTS_CURRENT_270 = 0x200000,
	CONTENTS_CURRENT_UP = 0x400000,
	CONTENTS_CURRENT_DOWN = 0x800000,
	CONTENTS_ORIGIN = 0x1000000,
	CONTENTS_MONSTER = 0x2000000,
	CONTENTS_DEBRIS = 0x4000000,
	CONTENTS_DETAIL = 0x8000000,
	CONTENTS_TRANSLUCENT = 0x10000000,
	CONTENTS_LADDER = 0x20000000,
	CONTENTS_HITBOX = 0x40000000,
};

enum trace_masks_t
{
	MASK_ALL = 0xFFFFFFFF,
	MASK_SOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_PLAYERSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_NPCSOLID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_NPCFLUID = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE,
	MASK_WATER = CONTENTS_WATER | CONTENTS_MOVEABLE | CONTENTS_SLIME,
	MASK_OPAQUE = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE,
	MASK_OPAQUE_AND_NPCS = MASK_OPAQUE | CONTENTS_MONSTER,
	MASK_BLOCKLOS = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_BLOCKLOS,
	MASK_BLOCKLOS_AND_NPCS = MASK_BLOCKLOS | CONTENTS_MONSTER,
	MASK_VISIBLE = MASK_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE,
	MASK_VISIBLE_AND_NPCS = MASK_OPAQUE_AND_NPCS | CONTENTS_IGNORE_NODRAW_OPAQUE,
	MASK_SHOT = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE | CONTENTS_HITBOX,
	MASK_SHOT_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_DEBRIS,
	MASK_SHOT_HULL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE,
	MASK_SHOT_PORTAL = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER,
	MASK_SOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE,
	MASK_PLAYERSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_PLAYERCLIP | CONTENTS_GRATE,
	MASK_NPCSOLID_BRUSHONLY = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
	MASK_NPCWORLDSTATIC = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE,
	MASK_NPCWORLDSTATIC_FLUID = CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP,
	MASK_SPLITAREPORTAL = CONTENTS_WATER | CONTENTS_SLIME,
	MASK_CURRENT = CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN,
	MASK_DEADSOLID = CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_GRATE,
};

enum BONEINDEX : DWORD
{
	Pelvis = 0,
	Spine_1 = 2,
	Spine_2 = 3,
	Spine_3 = 4,
	Neck_0 = 5,
	Head = 6,
	Arm_Upper_L = 8,
	Arm_Lower_L = 9,
	Hand_L = 10,
	Arm_Upper_R = 13,
	Arm_Lower_R = 14,
	Hand_R = 15,
	Leg_Upper_L = 22,
	Leg_Lower_L = 23,
	Ankle_L = 24,
	Leg_Upper_R = 25,
	Leg_Lower_R = 26,
	Ankle_R = 27,
	HITBOX_COUNT
};
inline const char* arrHit[] = {
	"Head",
	"Neck",
	"Spine 1",
	"Spine 2",
	"Pelvis",
	"Left Upper Arm",
	"Left Lower Arm",
	"Left Hand",
	"Right Upper Arm",
	"Right Lower Arm",
	"Right Hand",
	"Left Upper Leg",
	"Left Lower Leg",
	"Left Ankle",
	"Right Upper Leg",
	"Right Lower Leg",
	"Right Ankle",
};

inline BONEINDEX BoneIndexMap[] = {
	BONEINDEX::Head, // 0 "Head" maps to BONEINDEX::Head (6)
	BONEINDEX::Neck_0, // 1 "Neck" maps to BONEINDEX::Neck_0 (5)
	BONEINDEX::Spine_1, // 2 "Spine 1" maps to BONEINDEX::Spine_1 (4)
	BONEINDEX::Spine_2, // 3 "Spine 2" maps to BONEINDEX::Spine_2 (2)
	BONEINDEX::Pelvis, // 4 "Pelvis" maps to BONEINDEX::Pelvis (0)
	BONEINDEX::Arm_Upper_L, // 5 "Left Upper Arm" maps to BONEINDEX::Arm_Upper_L (8)
	BONEINDEX::Arm_Lower_L, // 6 "Left Lower Arm" maps to BONEINDEX::Arm_Lower_L (9)
	BONEINDEX::Hand_L, // 7 "Left Hand" maps to BONEINDEX::Hand_L (10)
	BONEINDEX::Arm_Upper_R, // 8 "Right Upper Arm" maps to BONEINDEX::Arm_Upper_R (13)
	BONEINDEX::Arm_Lower_R, // 9 "Right Lower Arm" maps to BONEINDEX::Arm_Lower_R (14)
	BONEINDEX::Hand_R, // 10 "Right Hand" maps to BONEINDEX::Hand_R (15)
	BONEINDEX::Leg_Upper_L, // 11 "Left Upper Leg" maps to BONEINDEX::Leg_Upper_L (22)
	BONEINDEX::Leg_Lower_L, // 12 "Left Lower Leg" maps to BONEINDEX::Leg_Lower_L (23)
	BONEINDEX::Ankle_L, // 13 "Left Ankle" maps to BONEINDEX::Ankle_L (24)
	BONEINDEX::Leg_Upper_R, // 14 "Right Upper Leg" maps to BONEINDEX::Leg_Upper_R (25)
	BONEINDEX::Leg_Lower_R, // 15 "Right Lower Leg" maps to BONEINDEX::Leg_Lower_R (26)
	BONEINDEX::Ankle_R // 16 "Right Ankle" maps to BONEINDEX::Ankle_R (27)
};

struct PlayerInfo
{
	C_CSPlayerPawn* player;
	float distance;
	int health;
	float velocity;

	PlayerInfo(C_CSPlayerPawn* p)
		: player(p),
		distance(SDK::LocalPawn->GetEyePosition().DistTo(p->GetEyePosition())),
		health(p->GetHealth()),
		velocity(p->m_vecVelocity().Length())
	{}
	void Update() {
		if (player) {
			distance = SDK::LocalPawn->GetEyePosition().DistTo(player->GetEyePosition());
			health = player->GetHealth();
			velocity = player->m_vecVelocity().Length();
		}
	}
};

struct ShotInfo_t {
	C_CSPlayerPawn* pPawn;
	Lagcomp::LagRecord pRecord;
	Vector_t vecShootEyePosition;
	QAngle_t angShootAngle;
};


static bool bDidntSelectDueToHitchance;
struct SelectionResult {
	C_CSPlayerPawn* entity;
	Vector_t point;
	int hitbox;
	float expectedDamage;
	float hitchance;
	std::optional<Lagcomp::LagRecord> record;
	float simtime;
};

//enum HITBOX_DEFINITION {
//	HEAD = 1 << 1,
//	CHEST = 1 << 2,
//	STOMACH = 1 << 3,
//	LEGS = 1 << 4,
//	ARMS = 1 << 5,
//	FEET = 1 << 6
//};


class RageBot
{
public:
	ImVec2 v{};
	ImVec2 svo{};

	ShotInfo_t shotInfo = {};
	std::vector<C_CSPlayerPawn*> players{};
	C_CSPlayerPawn* bestPlayer{};
	std::pair<Vector_t, float> BestHitbox{};
	std::optional<Lagcomp::LagRecord> record;
	std::vector<C_CSPlayerPawn*> pawns{};
	inline static std::vector<int> Hitbox;
	int HitChanceNS(QAngle_t& best_point, Vector_t start, C_CSPlayerPawn* player, C_CSWeaponBase* weapon, C_CSPlayerPawn* local);
	float ScaleDamage(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, Vector_t start, Vector_t aim_point, float& dmg, bool& canHit);
	bool can_shoot();
	bool weapon_is_at_max_accuracy(CCSWeaponBaseVData* weapon_data, float inaccuracy);
	bool hitchancev2(QAngle_t& pos, C_CSWeaponBase* weapon, C_CSPlayerPawn* Target);
	std::pair<Vector_t, float> FindBestHitbox(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon);
	void Hook();
	QAngle_t get_removed_aim_punch_angle(C_CSPlayerPawn* local_player);
	void RageBotAim(CUserCmd* pCmd, CCSGOInput* pInput);
	bool runRagebot(CCSGOInput* pInput);
	void RunAim(CUserCmd* pCmd);
};
inline std::unique_ptr<RageBot> g_RageBot = std::make_unique<RageBot>();

