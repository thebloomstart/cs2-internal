#pragma once

// @test: using interfaces in the header | not critical but could blow up someday with thousands of errors or affect to compilation time etc
// used: cgameentitysystem, ischemasystem
#include "../core/interfaces.h"
#include "interfaces/igameresourceservice.h"
#include "interfaces/ischemasystem.h"

// used: schema field
#include "../core/schema.h"

// used: l_print
#include "../utilities/log.h"
// used: vector_t
#include "datatypes/vector.h"
// used: qangle_t
#include "datatypes/qangle.h"
// used: ctransform
#include "datatypes/transform.h"

// used: cbasehandle
#include "entity_handle.h"
// used: game's definitions
#include "const.h"
// used: entity vdata
#include "vdata.h"
#include "../sdk/datatypes/utlmap.h"
#include "../InventoryManager.h"
#include "../PlayerInventory.h"
#include "algorithm"
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <iostream>


using GameTime_t = std::float_t;
using GameTick_t = std::int32_t;

class CEntityInstance;

class CEntityIdentity
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityIdentity);

	// @note: handle index is not entity index
	SCHEMA_ADD_OFFSET(std::uint32_t, GetIndex, 0x10);
	SCHEMA_ADD_FIELD(const char*, GetDesignerName, "CEntityIdentity->m_designerName");
	SCHEMA_ADD_FIELD(std::uint32_t, GetFlags, "CEntityIdentity->m_flags");

	[[nodiscard]] bool IsValid()
	{
		return GetIndex() != INVALID_EHANDLE_INDEX;
	}

	[[nodiscard]] int GetEntryIndex()
	{
		if (!IsValid())
			return ENT_ENTRY_MASK;
		
		return GetIndex() & ENT_ENTRY_MASK;
	}

	[[nodiscard]] int GetSerialNumber()
	{
		return GetIndex() >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	CEntityInstance* pInstance; // 0x00
};

class CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityInstance);

	void GetSchemaClassInfo(SchemaClassInfoData_t** pReturn)
	{
		return MEM::CallVFunc<void, 38U>(this, pReturn);
	}

	bool find_class(const char* name)
	{
		static SchemaClassInfoData_t* csDlass = nullptr;
		if (csDlass == nullptr)
			I::SchemaSystem->FindTypeScopeForModule(CS_XOR("client.dll"))->FindDeclaredClass(&csDlass, name);

		return (csDlass->InheritsFrom(csDlass));
	}
	SchemaClassInfoData_t* GetSchemaNoCrash()
	{
		SchemaClassInfoData_t* class_info = nullptr;
		this->GetSchemaClassInfo(&class_info);
		if (!class_info)
			return nullptr;

		return class_info;
	}

	const char* GetEntityClassName()
	{
		SchemaClassInfoData_t* class_info = GetSchemaNoCrash();

		return class_info->szName;
	}
	[[nodiscard]] CBaseHandle GetRefEHandle()
	{
		CEntityIdentity* pIdentity = GetIdentity();
		if (pIdentity == nullptr)
			return CBaseHandle();

		return CBaseHandle(pIdentity->GetEntryIndex(), pIdentity->GetSerialNumber() - (pIdentity->GetFlags() & 1));
	}

	SCHEMA_ADD_FIELD(CEntityIdentity*, GetIdentity, "CEntityInstance->m_pEntity");
};

class CCollisionProperty
{
public:
	std::uint16_t CollisionMask()
	{
		return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);
	}

	CS_CLASS_NO_INITIALIZER(CCollisionProperty);

	SCHEMA_ADD_FIELD(Vector_t, GetMins, "CCollisionProperty->m_vecMins");
	SCHEMA_ADD_FIELD(Vector_t, GetMaxs, "CCollisionProperty->m_vecMaxs");

	SCHEMA_ADD_FIELD(std::uint8_t, GetSolidFlags, "CCollisionProperty->m_usSolidFlags");
	SCHEMA_ADD_FIELD(std::uint8_t, GetCollisionGroup, "CCollisionProperty->m_CollisionGroup");
};

class CSkeletonInstance;
class CGameSceneNode
{
public:
	CS_CLASS_NO_INITIALIZER(CGameSceneNode);

	SCHEMA_ADD_FIELD(CTransform, GetNodeToWorld, "CGameSceneNode->m_nodeToWorld");
	SCHEMA_ADD_FIELD(CEntityInstance*, GetOwner, "CGameSceneNode->m_pOwner");
	SCHEMA_ADD_FIELD(Vector_t, m_vecOrigin, "CGameSceneNode->m_vecOrigin");

	SCHEMA_ADD_FIELD(Vector_t, GetAbsOrigin, "CGameSceneNode->m_vecAbsOrigin");
	SCHEMA_ADD_FIELD(Vector_t, GetRenderOrigin, "CGameSceneNode->m_vRenderOrigin");

	SCHEMA_ADD_FIELD(QAngle_t, GetAngleRotation, "CGameSceneNode->m_angRotation");
	SCHEMA_ADD_FIELD(QAngle_t, GetAbsAngleRotation, "CGameSceneNode->m_angAbsRotation");

	SCHEMA_ADD_FIELD(bool, IsDormant, "CGameSceneNode->m_bDormant");

	CSkeletonInstance* GetSkeletonInstance()
	{
		return MEM::CallVFunc<CSkeletonInstance*, 8U>(this);
	}

	void SetMeshGroupMask(uint64_t meshGroupMask);
	Vector_t GetBonePosition(const std::int32_t eBoneIndex);
};
class C_BaseEntity : public CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseEntity);

	[[nodiscard]] bool IsBasePlayerController()
	{
		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			return false;

		return FNV1A::Hash(pClassInfo->szName) == FNV1A::HashConst("C_CSPlayerController");
	}

	[[nodiscard]] bool IsWeapon()
	{
		static SchemaClassInfoData_t* pWeaponBaseClass = nullptr;
		if (pWeaponBaseClass == nullptr)
		I::SchemaSystem->FindTypeScopeForModule(CS_XOR("client.dll"))->FindDeclaredClass(&pWeaponBaseClass, CS_XOR("C_CSWeaponBase"));


		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			return false;

		return (pClassInfo->InheritsFrom(pWeaponBaseClass));
	}

	[[nodiscard]] bool IsPlayer()
	{
		return find_class(CS_XOR("C_CSPlayerPawn"));
	}
	void UpdateVData()
	{
		MEM::CallVFunc<void*, 182>(this);
	}

	int GetHandleEntity()
	{
		using fnGetPlayer = int(__thiscall*)(void*);
		static auto GetPlayerPawnf = reinterpret_cast<fnGetPlayer>(MEM::FindPattern(CLIENT_DLL, ("48 85 c9 74 ?? 48 8b 41 ?? 48 85 c0 74 ?? 44"))); // 48 85 C9 74 32 48 8B 41 10 48 85 C0 74 29 44        48 85 c9 74 ?? 48 8b 41 ?? 48 85 c0 74 ?? 44 8b 40 ?? ba ?? ?? ?? ?? 8b 48 ?? 41 8b c0 83 e1
		return GetPlayerPawnf(this);
	}
	void UpdateSubClass()
	{
		using fnGetPlayerPawn = void(__fastcall*)(void*);
		static auto GetPlayerPawnf = reinterpret_cast<fnGetPlayerPawn>(MEM::FindPattern(CLIENT_DLL, ("40 53 48 83 EC 30 48 8B 41 10 48 8B D9 8B 50 30")));
		GetPlayerPawnf(this);
	}

	SCHEMA_ADD_FIELD(std::uint64_t, m_nSubclassID, "C_BaseEntity->m_nSubclassID");

	static C_BaseEntity* GetLocalPlayer();

	// get entity origin on scene
	[[nodiscard]] const Vector_t& GetSceneOrigin();
	SCHEMA_ADD_FIELD(Vector_t, m_vecVelocity, "C_BaseEntity->m_vecVelocity");
	SCHEMA_ADD_FIELD(CGameSceneNode*, GetGameSceneNode, "C_BaseEntity->m_pGameSceneNode");
	SCHEMA_ADD_FIELD(CCollisionProperty*, GetCollision, "C_BaseEntity->m_pCollision");
	SCHEMA_ADD_FIELD(std::uint8_t, GetTeam, "C_BaseEntity->m_iTeamNum");
	SCHEMA_ADD_FIELD(CBaseHandle, GetOwnerHandle, "C_BaseEntity->m_hOwnerEntity");
	SCHEMA_ADD_FIELD(Vector_t, GetBaseVelocity, "C_BaseEntity->m_vecBaseVelocity");
	SCHEMA_ADD_FIELD(Vector_t, GetAbsVelocity, "C_BaseEntity->m_vecAbsVelocity");
	SCHEMA_ADD_FIELD(bool, IsTakingDamage, "C_BaseEntity->m_bTakesDamage");
	SCHEMA_ADD_FIELD(std::uint32_t, GetFlags, "C_BaseEntity->m_fFlags");
	SCHEMA_ADD_FIELD(std::int32_t, GetEflags, "C_BaseEntity->m_iEFlags");
	SCHEMA_ADD_FIELD(std::uint8_t, GetMoveType, "C_BaseEntity->m_nActualMoveType"); // m_nActualMoveType returns CSGO style movetype, m_nMoveType returns bitwise shifted move type
	SCHEMA_ADD_FIELD(std::uint8_t, GetLifeState, "C_BaseEntity->m_lifeState");
	SCHEMA_ADD_FIELD(std::int32_t, GetHealth, "C_BaseEntity->m_iHealth");
	SCHEMA_ADD_FIELD(std::int32_t, GetMaxHealth, "C_BaseEntity->m_iMaxHealth");
	SCHEMA_ADD_FIELD(float, GetWaterLevel, "C_BaseEntity->m_flWaterLevel");
	SCHEMA_ADD_FIELD_OFFSET(void*, GetVData, "C_BaseEntity->m_nSubclassID", 0x8);
	SCHEMA_ADD_FIELD(std::float_t, GetSimulationTime, "C_BaseEntity->m_flSimulationTime");
	SCHEMA_ADD_FIELD(std::int32_t, nActualMoveType, "C_BaseEntity->m_nActualMoveType");
};

class CGlowProperty
{
public:
	SCHEMA_ADD_FIELD(Vector_t, GetGlowColor , "CGlowProperty->m_fGlowColor"); // Vector
	SCHEMA_ADD_FIELD(int, GetGlowType, "CGlowProperty->m_iGlowType")
	SCHEMA_ADD_FIELD(int, GetGlowTeam, "CGlowProperty->m_iGlowTeam");
	SCHEMA_ADD_FIELD(int, GetGlowRange, "CGlowProperty->m_nGlowRange");
	SCHEMA_ADD_FIELD(int, GetGlowRangeMin, "CGlowProperty->m_nGlowRangeMin");
	SCHEMA_ADD_FIELD(Color_t, GetGlowColorOverride, "CGlowProperty->m_glowColorOverride");
	SCHEMA_ADD_FIELD(bool, GetFlashing, "CGlowProperty->m_bFlashing");
	SCHEMA_ADD_FIELD(float, GetflGlowTime, "CGlowProperty->m_flGlowTime");
	SCHEMA_ADD_FIELD(float, GetflGlowStartTime, "CGlowProperty->m_flGlowStartTime");
	SCHEMA_ADD_FIELD(bool, GetbEligibleForScreenHighlight, "CGlowProperty->m_bEligibleForScreenHighlight");
	SCHEMA_ADD_FIELD(bool, GetbGlowing, "CGlowProperty->m_bGlowing");
};

class C_BaseModelEntity : public C_BaseEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseModelEntity);

	SCHEMA_ADD_FIELD(CCollisionProperty, GetCollisionInstance, "C_BaseModelEntity->m_Collision");
	SCHEMA_ADD_FIELD(CGlowProperty, GetGlowProperty, "C_BaseModelEntity->m_Glow");
	SCHEMA_ADD_FIELD(Vector_t, GetViewOffset, "C_BaseModelEntity->m_vecViewOffset");
	SCHEMA_ADD_FIELD(GameTime_t, GetCreationTime, "C_BaseModelEntity->m_flCreateTime");
	SCHEMA_ADD_FIELD(GameTick_t, GetCreationTick, "C_BaseModelEntity->m_nCreationTick");
	SCHEMA_ADD_FIELD(CBaseHandle, GetMoveParent, "C_BaseModelEntity->m_hOldMoveParent");
	SCHEMA_ADD_FIELD(std::float_t, GetAnimTime, "C_BaseModelEntity->m_flAnimTime");
	void SetModel(const char* name);
};

class CPlayer_ItemServices
{
public:
	SCHEMA_ADD_FIELD(bool, m_bHasDefuser, "CCSPlayer_ItemServices->m_bHasDefuser");
	SCHEMA_ADD_FIELD(bool, m_bHasHelmet, "CCSPlayer_ItemServices->m_bHasHelmet");
	SCHEMA_ADD_FIELD(bool, m_bHasHeavyArmor, "CCSPlayer_ItemServices->m_bHasHeavyArmor");
};

class CPlayer_CameraServices
{
public:
	SCHEMA_ADD_FIELD(CBaseHandle, m_PostProcessingVolumes, "CPlayer_CameraServices->m_PostProcessingVolumes");
	SCHEMA_ADD_FIELD(Vector_t, ViewPunchAngle, "CPlayer_CameraServices->m_vecCsViewPunchAngle");

};

class CPlayer_WeaponServices
{
public:
	SCHEMA_ADD_FIELD(CBaseHandle, GetActiveWeapon, "CPlayer_WeaponServices->m_hActiveWeapon");
};

class CCSPlayer_WeaponServices : public CPlayer_WeaponServices
{
public:
	__forceinline Vector_t get_shoot_pos()
	{
		Vector_t temp;
		MEM::CallVFunc<void, 26>(this, &temp); //23
		return temp;
	}
	SCHEMA_ADD_FIELD(GameTime_t, GetNextAttack, "CCSPlayer_WeaponServices->m_flNextAttack");
};
enum class ObserverMode_t : uint8_t
{
	OBS_MODE_NONE = 0x0,
	OBS_MODE_FIXED = 0x1,
	OBS_MODE_IN_EYE = 0x2,
	OBS_MODE_CHASE = 0x3,
	OBS_MODE_ROAMING = 0x4,
	OBS_MODE_DIRECTED = 0x5,
	NUM_OBSERVER_MODES = 0x6,
};
class CPlayer_ObserverServices
{
public:
	SCHEMA_ADD_FIELD(ObserverMode_t, GetObserverMode, "CPlayer_ObserverServices->m_iObserverMode");
	SCHEMA_ADD_FIELD(CBaseHandle, GetObserverTarget, "CPlayer_ObserverServices->m_hObserverTarget");
	SCHEMA_ADD_FIELD(int, GetObserverLastMode, "CPlayer_ObserverServices->m_iObserverLastMode");
	SCHEMA_ADD_FIELD(int, GetForcedObserverMode, "CPlayer_ObserverServices->m_bForcedObserverMode");
};


class C_BasePlayerPawn : public C_BaseModelEntity

{
public:
	CS_CLASS_NO_INITIALIZER(C_BasePlayerPawn);

	SCHEMA_ADD_FIELD(CBaseHandle, GetControllerHandle, "C_BasePlayerPawn->m_hController");
	SCHEMA_ADD_FIELD(CCSPlayer_WeaponServices*, GetWeaponServices, "C_BasePlayerPawn->m_pWeaponServices");
	SCHEMA_ADD_FIELD(CPlayer_ItemServices*, GetItemServices, "C_BasePlayerPawn->m_pItemServices");
	SCHEMA_ADD_FIELD(CPlayer_CameraServices*, GetCameraServices, "C_BasePlayerPawn->m_pCameraServices");
	SCHEMA_ADD_FIELD(Vector_t, GetOrigin, "C_BasePlayerPawn->m_vOldOrigin");
	SCHEMA_ADD_FIELD(CPlayer_ObserverServices*, GetObserverServices, "C_BasePlayerPawn->m_pObserverServices");

	[[nodiscard]] Vector_t GetEyePosition()
	{
		Vector_t vecEyePosition = Vector_t(0.0f, 0.0f, 0.0f);
		MEM::CallVFunc<void, 169U>(this, &vecEyePosition);
		return vecEyePosition;
	}
};

class CCSPlayer_ViewModelServices
{
public:
	SCHEMA_ADD_FIELD(CBaseHandle, m_hViewModel, "CCSPlayer_ViewModelServices->m_hViewModel");
};

class C_CSPlayerPawnBase : public C_BasePlayerPawn
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawnBase);

	SCHEMA_ADD_FIELD(CCSPlayer_ViewModelServices*, GetViewModelServices, "C_CSPlayerPawnBase->m_pViewModelServices");
	SCHEMA_ADD_FIELD(float, GetLowerBodyYawTarget, "C_CSPlayerPawnBase->m_flLowerBodyYawTarget");
	SCHEMA_ADD_FIELD(float, GetFlashMaxAlpha, "C_CSPlayerPawnBase->m_flFlashMaxAlpha");
	SCHEMA_ADD_FIELD(float, GetFlashDuration, "C_CSPlayerPawnBase->m_flFlashDuration");
	SCHEMA_ADD_FIELD(Vector_t, GetLastSmokeOverlayColor, "C_CSPlayerPawnBase->m_vLastSmokeOverlayColor");
	SCHEMA_ADD_FIELD(int, GetSurvivalTeam, "C_CSPlayerPawnBase->m_nSurvivalTeam"); // danger zone
	SCHEMA_ADD_FIELD(bool, GetHasImmunity, "C_CSPlayerPawnBase->m_bGunGameImmunity"); // danger zone

};

class C_EconItemView;

class C_CSWeaponBase;

class CPlayer_MovementServices;
class hitboxes_data;
class CHitBoxSet;

class C_CSPlayerPawn : public C_CSPlayerPawnBase
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawn);

	[[nodiscard]] C_CSWeaponBase* GetActiveWeapon();
	[[nodiscard]] bool IsOtherEnemy(C_CSPlayerPawn* pOther);
	[[nodiscard]] int GetAssociatedTeam();
	[[nodiscard]] bool CanAttack(const float flServerTime);
	[[nodiscard]] std::uint32_t GetOwnerHandleIndex();
	[[nodiscard]] std::uint16_t GetCollisionMask();
	SCHEMA_ADD_FIELD(GameTime_t, GetHealthShotBoostExpirationTime, "C_CSPlayerPawn->m_flHealthShotBoostExpirationTime");

	C_CSWeaponBase* GetWeaponActive();

	bool IsThorning();

	int get_bone_index(const char* name);

	bool hasArmour(const int hitgroup);

	CHitBoxSet* GetHitboxSet(int i);

	int HitboxToWorldTransforms(CHitBoxSet* hitBoxSet, CTransform* hitboxToWorld);

	hitboxes_data GetHitboxRadMinMax(int i);

	bool IsAlive()
	{
		if (!this)
			return false;

		return GetHealth() > 0;
	}

	SCHEMA_ADD_FIELD(CPlayer_MovementServices*, MovementServices, "C_BasePlayerPawn->m_pMovementServices");


	SCHEMA_ADD_FIELD(bool, IsScoped, "C_CSPlayerPawn->m_bIsScoped");
	SCHEMA_ADD_FIELD(bool, IsDefusing, "C_CSPlayerPawn->m_bIsDefusing");
	SCHEMA_ADD_FIELD(bool, IsGrabbingHostage, "C_CSPlayerPawn->m_bIsGrabbingHostage");
	SCHEMA_ADD_FIELD(bool, IsWaitForNoAttack, "C_CSPlayerPawn->m_bWaitForNoAttack");
	SCHEMA_ADD_FIELD(int, GetShotsFired, "C_CSPlayerPawn->m_iShotsFired");
	SCHEMA_ADD_FIELD(float, GetArmorValue, "C_CSPlayerPawn->m_ArmorValue");
	SCHEMA_ADD_FIELD(QAngle_t, GetAimPuchAngle, "C_CSPlayerPawn->m_aimPunchAngle");
	SCHEMA_ADD_FIELD(C_EconItemView, m_EconGloves, ("C_CSPlayerPawn->m_EconGloves"));
	SCHEMA_ADD_FIELD(bool, m_bNeedToReApplyGloves, ("C_CSPlayerPawn->m_bNeedToReApplyGloves"));
};

class CBasePlayerController : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerController);

	SCHEMA_ADD_FIELD(std::uint64_t, GetSteamId, "CBasePlayerController->m_steamID");
	SCHEMA_ADD_FIELD(std::uint32_t, GetTickBase, "CBasePlayerController->m_nTickBase");
	SCHEMA_ADD_FIELD(CBaseHandle, GetPawnHandle, "CBasePlayerController->m_hPawn");
	SCHEMA_ADD_FIELD(bool, IsLocalPlayerController, "CBasePlayerController->m_bIsLocalPlayerController");
};

class CUserCmd;
class CPlayer_MovementServices
{
public:
	CS_CLASS_NO_INITIALIZER(CPlayer_MovementServices);

	SCHEMA_ADD_FIELD(float, m_flMaxspeed, ("CPlayer_MovementServices->m_flMaxspeed"));
	SCHEMA_ADD_FIELD(float, m_flForwardMove, ("CPlayer_MovementServices->m_flForwardMove"));
	SCHEMA_ADD_FIELD(float, m_flLeftMove, ("CPlayer_MovementServices->m_flLeftMove"));
	SCHEMA_ADD_OFFSET(float, m_flSurfaceFriction, 0x1FC);
	SCHEMA_ADD_FIELD(bool, IsDucked, "CPlayer_MovementServices_Humanoid->m_bDucking");

	void RunCommand(CUserCmd* pCmd)
	{
		MEM::CallVFunc<void, 23U>(this, pCmd);
	}

	void SetPredictionCommand(CUserCmd* pCmd)
	{
		MEM::CallVFunc<void, 37U>(this, pCmd);
	}

	void ResetPredictionCommand()
	{
		MEM::CallVFunc<void, 38U>(this);
	}

};
// forward decleration
class C_CSWeaponBaseGun;
class C_BasePlayerWeapon;
class CCSPlayerController : public CBasePlayerController
{
public:
	CS_CLASS_NO_INITIALIZER(CCSPlayerController);

	[[nodiscard]] static CCSPlayerController* GetLocalPlayerController();

	// @note: always get origin from pawn not controller
	[[nodiscard]] const Vector_t& GetPawnOrigin();

	bool IsThrowingGrenade(C_CSWeaponBase* pBaseWeapon);

	C_CSWeaponBase* GetPlayerWeapon(C_CSPlayerPawn* pPlayer);

	C_CSPlayerPawn* get_player_pawn();

	C_CSPlayerPawn* get_observer_pawn();

	SCHEMA_ADD_FIELD(std::uint32_t, GetPing, "CCSPlayerController->m_iPing");
	SCHEMA_ADD_FIELD(const char*, GetPlayerName, "CCSPlayerController->m_sSanitizedPlayerName");
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnHealth, "CCSPlayerController->m_iPawnHealth");
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnArmor, "CCSPlayerController->m_iPawnArmor");
	SCHEMA_ADD_FIELD(bool, IsPawnHasDefuser, "CCSPlayerController->m_bPawnHasDefuser");
	SCHEMA_ADD_FIELD(bool, IsPawnHasHelmet, "CCSPlayerController->m_bPawnHasHelmet");
	SCHEMA_ADD_FIELD(bool, IsPawnAlive, "CCSPlayerController->m_bPawnIsAlive");
	SCHEMA_ADD_FIELD(CBaseHandle, GetPlayerPawnHandle, "CCSPlayerController->m_hPlayerPawn");
	SCHEMA_ADD_FIELD(CBaseHandle, GetPlayerPawnObserver, "CCSPlayerController->m_hObserverPawn");
	SCHEMA_ADD_FIELD_OFFSET(CUserCmd*, GetCurrentCommand, "CBasePlayerController->m_steamID", -0x8);

};

class CBaseAnimGraph : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(CBaseAnimGraph);

	SCHEMA_ADD_FIELD(bool, IsClientRagdoll, "CBaseAnimGraph->m_bClientRagdoll");
};

class C_BaseFlex : public CBaseAnimGraph
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseFlex);
	/* not implemented */
};
// START VIEWMODEL //
class CAnimGraphNetworkedVariables;

class CAnimationGraphInstance
{
public:
	char pad_0x0000[0x2E0]; //0x0000
	CAnimGraphNetworkedVariables* pAnimGraphNetworkedVariables; //0x02E0
};

class C_BaseViewModel : public C_BaseModelEntity
{
public:
	SCHEMA_ADD_FIELD(CBaseHandle, m_hWeapon, "C_BaseViewModel->m_hWeapon");
};

class C_CSGOViewModel : public C_BaseViewModel
{
public:
	char pad_0x0000[0xD60]; //0x0000
	CAnimationGraphInstance* pAnimationGraphInstance; //0x0D08
};

class C_PlantedC4 : public CBaseAnimGraph
{
public:
	__forceinline float GetTimer(const float flServerTime)
	{
		return std::clamp(this->m_flC4Blow() - flServerTime, 0.0f, this->m_flTimerLength());
	}

	__forceinline float GetDefuseTimer(const float flServerTime)
	{
		return std::clamp(this->m_flDefuseCountDown() - flServerTime, 0.0f, this->m_flDefuseLength());
	}

public:
	SCHEMA_ADD_FIELD(bool, m_bBombTicking, "C_PlantedC4->m_bBombTicking");
	SCHEMA_ADD_FIELD(bool, m_bCannotBeDefused, "C_PlantedC4->m_bCannotBeDefused");
	SCHEMA_ADD_FIELD(bool, m_bBeingDefused, "C_PlantedC4->m_bBeingDefused");

	SCHEMA_ADD_FIELD(std::int32_t, m_nBombSite, "C_PlantedC4->m_nBombSite");

	SCHEMA_ADD_FIELD(std::float_t, m_flTimerLength, "C_PlantedC4->m_flTimerLength");
	SCHEMA_ADD_FIELD(std::float_t, m_flDefuseLength, "C_PlantedC4->m_flDefuseLength");

	SCHEMA_ADD_FIELD(GameTime_t, m_flC4Blow, "C_PlantedC4->m_flC4Blow");
	SCHEMA_ADD_FIELD(GameTime_t, m_flDefuseCountDown, "C_PlantedC4->m_flDefuseCountDown");

	SCHEMA_ADD_FIELD(CBaseHandle, m_hBombDefuser, "C_PlantedC4->m_hBombDefuser");
};

// END VIEWMODEL //
class CEconItem;
class CCSPlayerInventory;
class CEconItemDefinition;

class C_EconItemView
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconItemView);

	SCHEMA_ADD_FIELD(std::uint16_t, GetItemDefinitionIndex, "C_EconItemView->m_iItemDefinitionIndex");
	SCHEMA_ADD_FIELD(std::uint64_t, GetItemIDCS2, "C_EconItemView->m_iItemID");
	SCHEMA_ADD_FIELD(std::uint32_t, GetItemIDHigh, "C_EconItemView->m_iItemIDHigh");
	SCHEMA_ADD_FIELD(std::uint32_t, GetItemIDLow, "C_EconItemView->m_iItemIDLow");
	SCHEMA_ADD_FIELD(std::uint32_t, GetAccountID, "C_EconItemView->m_iAccountID");
	SCHEMA_ADD_FIELD(bool, m_bDisallowSOC, "C_EconItemView->m_bDisallowSOC");
	SCHEMA_ADD_FIELD(bool, m_bInitialized, "C_EconItemView->m_bInitialized");
	SCHEMA_ADD_FIELD(char[161], GetCustomName, "C_EconItemView->m_szCustomName");
	SCHEMA_ADD_FIELD(char[161], GetCustomNameOverride, "C_EconItemView->m_szCustomNameOverride");
	CEconItem* GetSOCData(CCSPlayerInventory* sdfsdf);

	auto GetCustomPaintKitIndex()
	{
		return MEM::CallVFunc<int, 2u>(this);
	}

	auto GetStaticData()
	{
		return MEM::CallVFunc<CEconItemDefinition*, 13u>(this);
	}
};

class CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER(CAttributeManager);
	SCHEMA_ADD_OFFSET(C_EconItemView, m_Item, 0x50);

	virtual ~CAttributeManager() = 0;
};
static_assert(sizeof(CAttributeManager) == 0x8);

class C_AttributeContainer : public CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER(C_AttributeContainer);

	SCHEMA_ADD_PFIELD(C_EconItemView, GetItem, "C_AttributeContainer->m_Item");
};

class C_EconEntity : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconEntity);

	SCHEMA_ADD_PFIELD(C_AttributeContainer, GetAttributeManager, "C_EconEntity->m_AttributeManager");
	SCHEMA_ADD_FIELD(std::uint32_t, GetOriginalOwnerXuidLow, "C_EconEntity->m_OriginalOwnerXuidLow");
	SCHEMA_ADD_FIELD(std::uint32_t, GetOriginalOwnerXuidHigh, "C_EconEntity->m_OriginalOwnerXuidHigh");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackPaintKit, "C_EconEntity->m_nFallbackPaintKit");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackSeed, "C_EconEntity->m_nFallbackSeed");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackWear, "C_EconEntity->m_flFallbackWear");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackStatTrak, "C_EconEntity->m_nFallbackStatTrak");
	SCHEMA_ADD_FIELD(CBaseHandle, GetViewModelAttachmentHandle, "C_EconEntity->m_hViewmodelAttachment");

	auto IsBasePlayerWeapon()
	{
		return MEM::CallVFunc<bool, 156u>(this);
	}

	uint64_t GetOriginalOwnerXuid()
	{
		return ((uint64_t)(GetOriginalOwnerXuidHigh()) << 32) |
		GetOriginalOwnerXuidLow();
	}
};

class C_EconWearable : public C_EconEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconWearable);

	SCHEMA_ADD_FIELD(std::int32_t, GetForceSkin, "C_EconWearable->m_nForceSkin");
	SCHEMA_ADD_FIELD(bool, IsAlwaysAllow, "C_EconWearable->m_bAlwaysAllow");
};

class C_BasePlayerWeapon : public C_EconEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BasePlayerWeapon);

	SCHEMA_ADD_FIELD(GameTick_t, GetNextPrimaryAttackTick, "C_BasePlayerWeapon->m_nNextPrimaryAttackTick");
	SCHEMA_ADD_FIELD(float, GetNextPrimaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextPrimaryAttackTickRatio");
	SCHEMA_ADD_FIELD(GameTick_t, GetNextSecondaryAttackTick, "C_BasePlayerWeapon->m_nNextSecondaryAttackTick");
	SCHEMA_ADD_FIELD(float, GetNextSecondaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextSecondaryAttackTickRatio");
	SCHEMA_ADD_FIELD(std::int32_t, GetClip1, "C_BasePlayerWeapon->m_iClip1");
	SCHEMA_ADD_FIELD(std::int32_t, GetClip2, "C_BasePlayerWeapon->m_iClip2");
	SCHEMA_ADD_FIELD(std::int32_t[2], GetReserveAmmo, "C_BasePlayerWeapon->m_pReserveAmmo");
};

class C_CSWeaponBase : public C_BasePlayerWeapon
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSWeaponBase);

	SCHEMA_ADD_FIELD(bool, IsInReload, "C_CSWeaponBase->m_bInReload");
	SCHEMA_ADD_FIELD(int, m_iOriginalTeamNumber, ("C_CSWeaponBase->m_iOriginalTeamNumber"));
	SCHEMA_ADD_FIELD(int, m_iRecoilIndex, "C_CSWeaponBase->m_iRecoilIndex");
	SCHEMA_ADD_FIELD(GameTick_t, m_nPostponeFireReadyTicks, "C_CSWeaponBase->m_nPostponeFireReadyTicks");

	C_EconItemView* GetEconItemView()
	{
		return reinterpret_cast<C_EconItemView*>(std::uintptr_t(this) + 0x10E8);
	}

	bool IsGrenade()
	{
		auto weapon_data = GetWeaponVData();

		if (!weapon_data)
			return false;

		return weapon_data->GetWeaponType() == WEAPONTYPE_GRENADE;
	}

	float get_inaccuracyHitChance()
	{
		using fn_get_inaccuracy_t = float(__fastcall*)(void*);
		static fn_get_inaccuracy_t fn = reinterpret_cast<fn_get_inaccuracy_t>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ?? 55 56 57 48 81 EC ?? ?? ?? ?? 44 0F 29 84 24")));

		return fn(this);
	}

	float get_spread()
	{
		using fn_get_spread_t = float(__fastcall*)(void*);
		static fn_get_spread_t fn = reinterpret_cast<fn_get_spread_t>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 83 EC ? 48 63 91")));

		return fn(this);
	}

	void update_accuracy()
	{
		using function_t = void(__fastcall*)(void*);
		static function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 44 0F 29 44 24")));
		if (fn == nullptr)
		{
			L_PRINT(LOG_WARNING) << CS_XOR("update_accuracy Invalid signature!");
		}
		fn(this);
	}
	CCSWeaponBaseVData* GetWeaponVData()
	{
		return static_cast<CCSWeaponBaseVData*>(GetVData());
	}
	bool HasScope();
	bool IsNade();
	bool IsKnife();
};

class C_CSWeaponBaseGun : public C_CSWeaponBase
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSWeaponBaseGun);

	SCHEMA_ADD_FIELD(std::int32_t, GetZoomLevel, "C_CSWeaponBaseGun->m_zoomLevel");
	SCHEMA_ADD_FIELD(std::int32_t, GetBurstShotsRemaining, "C_CSWeaponBaseGun->m_iBurstShotsRemaining");
	SCHEMA_ADD_FIELD(bool, IsBurstMode, "C_CSWeaponBase->m_bBurstMode");
	SCHEMA_ADD_FIELD(float, GetPostponeFireReadyFrac, "C_CSWeaponBase->m_flPostponeFireReadyFrac");

	[[nodiscard]] bool CanPrimaryAttack(const int nWeaponType, const float flServerTime);
	[[nodiscard]] bool CanSecondaryAttack(const int nWeaponType, const float flServerTime);
};

class C_BaseCSGrenade : public C_CSWeaponBase
{
public:
	SCHEMA_ADD_FIELD(bool, IsHeldByPlayer, "C_BaseCSGrenade->m_bIsHeldByPlayer");
	SCHEMA_ADD_FIELD(bool, IsPinPulled, "C_BaseCSGrenade->m_bPinPulled");
	SCHEMA_ADD_FIELD(GameTime_t, GetThrowTime, "C_BaseCSGrenade->m_fThrowTime");
	SCHEMA_ADD_FIELD(float, GetThrowStrength, "C_BaseCSGrenade->m_flThrowStrength");
};

class C_BaseGrenade : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseGrenade);
};

enum bone_flags : uint32_t
{
	FLAG_NO_BONE_FLAGS = 0x0,
	FLAG_BONEFLEXDRIVER = 0x4,
	FLAG_CLOTH = 0x8,
	FLAG_PHYSICS = 0x10,
	FLAG_ATTACHMENT = 0x20,
	FLAG_ANIMATION = 0x40,
	FLAG_MESH = 0x80,
	FLAG_HITBOX = 0x100,
	FLAG_BONE_USED_BY_VERTEX_LOD0 = 0x400,
	FLAG_BONE_USED_BY_VERTEX_LOD1 = 0x800,
	FLAG_BONE_USED_BY_VERTEX_LOD2 = 0x1000,
	FLAG_BONE_USED_BY_VERTEX_LOD3 = 0x2000,
	FLAG_BONE_USED_BY_VERTEX_LOD4 = 0x4000,
	FLAG_BONE_USED_BY_VERTEX_LOD5 = 0x8000,
	FLAG_BONE_USED_BY_VERTEX_LOD6 = 0x10000,
	FLAG_BONE_USED_BY_VERTEX_LOD7 = 0x20000,
	FLAG_BONE_MERGE_READ = 0x40000,
	FLAG_BONE_MERGE_WRITE = 0x80000,
	FLAG_ALL_BONE_FLAGS = 0xfffff,
	BLEND_PREALIGNED = 0x100000,
	FLAG_RIGIDLENGTH = 0x200000,
	FLAG_PROCEDURAL = 0x400000,
};

#include "../sdk/datatypes/stronghandle.h"
#include "../sdk/datatypes/vector.h"

class CModel;

struct BoneData_t
{
	Vector_t vecPosition;
	float flScale;
	Vector4D_t vecRotation;
};

class CModelSkeleton
{
public:
	CModelSkeleton() = delete;
	CModelSkeleton(CModelSkeleton&&) = delete;
	CModelSkeleton(const CModelSkeleton&) = delete;

public:
	SCHEMA_ADD_OFFSET(CUtlVector<const char*>, vecBoneNames, 0x4);
	SCHEMA_ADD_OFFSET(CUtlVector<std::uint16_t>, vecBoneParent, 0x18);
	SCHEMA_ADD_OFFSET(CUtlVector<float>, m_boneSphere, 0x30);
	SCHEMA_ADD_OFFSET(CUtlVector<std::uint32_t>, m_nFlag, 0x48);
	SCHEMA_ADD_OFFSET(CUtlVector<Vector_t>, m_bonePosParent, 0x60);
	SCHEMA_ADD_OFFSET(CUtlVector<QuaternionAligned_t>, m_boneRotParent, 0x78);
	SCHEMA_ADD_OFFSET(CUtlVector<float>, m_boneScaleParent, 0x90);
};

class CModel
{
public:
	SCHEMA_ADD_OFFSET(const char*, szName, 0x8);
	SCHEMA_ADD_OFFSET(CModelSkeleton, m_modelSkeleton, 0x188);


	uint32_t GetHitboxesNum();
	uint32_t GetHitboxFlags(uint32_t index);
	uint32_t GetHitboxParent(uint32_t index);
	const char* GetHitboxName(uint32_t index);
};

class CModelState
{
public:
	Vector_t GetHitboxPos(uint32_t index)
	{
		auto hitbox = this->m_pBones;
		if (!hitbox)
			return nullptr;

		if (!(this->m_hModel()->GetHitboxFlags(index) & bone_flags::FLAG_HITBOX))
			return nullptr;

		auto parent_index = this->m_hModel()->GetHitboxParent(index);
		if (parent_index == -1)
			return nullptr;

		return hitbox[index].vecPosition;
	}

	Vector4D_t GetHitboxRotation(uint32_t index)
	{
		auto hitbox = this->m_pBones;
		if (!hitbox)
			return 0;

		if (!(this->m_hModel()->GetHitboxFlags(index) & bone_flags::FLAG_HITBOX))
			return 0;

		auto parent_index = this->m_hModel()->GetHitboxParent(index);
		if (parent_index == -1)
			return 0;

		return hitbox[index].vecRotation;
	}

	const char* GetHitboxName(uint32_t index)
	{
		auto hitbox = this->m_pBones;
		if (!hitbox)
			return nullptr;

		if (!(this->m_hModel()->GetHitboxFlags(index) & bone_flags::FLAG_HITBOX))
			return nullptr;

		auto parent_index = this->m_hModel()->GetHitboxParent(index);
		if (parent_index == -1)
			return nullptr;

		return this->m_hModel()->GetHitboxName(index);
	}

	SCHEMA_ADD_FIELD(CStrongHandle<CModel>, m_hModel, "CModelState->m_hModel");
	MEM_PAD(0x80);
	BoneData_t* m_pBones;
};

class CSkeletonInstance : public CGameSceneNode
{
public:
	MEM_PAD(0x1CC); //0x0000
	int nBoneCount; //0x01CC
	MEM_PAD(0x18); //0x01D0
	int nMask; //0x01E8
	MEM_PAD(0x4); //0x01EC
	Matrix2x4_t* pBoneCache; //0x01F0
	SCHEMA_ADD_FIELD(CModelState, m_modelState, "CSkeletonInstance->m_modelState");
	void CS_FASTCALL calc_world_space_bones(uint32_t mask);
};

//START SKINCHANGER//
class CSharedObject;

class CGCClientSharedObjectTypeCache
{
public:
	auto AddObject(CSharedObject* pObject)
	{
		return MEM::CallVFunc<bool, 1u>(this, pObject);
	}

	auto RemoveObject(CSharedObject* soIndex)
	{
		return MEM::CallVFunc<CSharedObject*, 3u>(this, soIndex);
	}

	template <typename T>
	auto& GetVecObjects()
	{
		return *reinterpret_cast<CUtlVector<T>*>((uintptr_t)(this) + 0x8);
	}
};

class CGCClientSharedObjectCache
{
public:
	CGCClientSharedObjectTypeCache* CreateBaseTypeCache(int nClassID);
};

struct SOID_t;

class CGCClient
{
public:
	CGCClientSharedObjectCache* FindSOCache(SOID_t ID,
	bool bCreateIfMissing = true);
};

class CGCClientSystem
{
public:
	CGCClientSystem* GetInstance();

	CGCClient* GetCGCClient()
	{
		return reinterpret_cast<CGCClient*>((uintptr_t)(this) + 0xB8);
	};
};

class CEconItemDefinition;

class CEconItem
{
	void SetDynamicAttributeValue(int index, void* value);
	void SetDynamicAttributeValueString(int index, const char* value);

public:
	static CEconItem* CreateInstance();
	CEconItemDefinition* get_attribute_definition_by_name(const char* name);
	void set_sticker(int index, int id);

	void Destruct()
	{
		return MEM::CallVFunc<void, 1U>(this);
	}

	void SetPaintKit(float kit)
	{
		SetDynamicAttributeValue(6, &kit);
	}

	void SetPaintSeed(float seed)
	{
		SetDynamicAttributeValue(7, &seed);
	}

	void SetPaintWear(float wear)
	{
		SetDynamicAttributeValue(8, &wear);
	}

	void SetStatTrak(int count)
	{
		SetDynamicAttributeValue(80, &count);
	}

	void SetStatTrakType(int type)
	{
		SetDynamicAttributeValue(81, &type);
	}

	void SetCustomName(const char* pName)
	{
		SetDynamicAttributeValueString(111, pName);
	}
	void SetKeychains(int id) { SetDynamicAttributeValue(299, &id); }
	char pad0[0x10]; // 2 vtables
	uint64_t m_ulID;
	uint64_t m_ulOriginalID;
	void* m_pCustomDataOptimizedObject;
	uint32_t m_unAccountID;
	uint32_t m_unInventory;
	uint16_t m_unDefIndex;
	uint16_t m_unOrigin : 5;
	uint16_t m_nQuality : 4;
	uint16_t m_unLevel : 2;
	uint16_t m_nRarity : 4;
	uint16_t m_dirtybitInUse : 1;
	int16_t m_iItemSet;
	int m_bSOUpdateFrame;
	uint8_t m_unFlags;
};

class CEconItemDefinition
{
public:
	bool is_custom_player();
	bool IsWeapon();
	bool IsKnife(bool excludeDefault);
	bool IsGlove(bool excludeDefault);
	bool IsAgent();

	uint8_t GetRarity()
	{
		return *reinterpret_cast<uint8_t*>((uintptr_t)(this) + 0x42);
	}

	auto GetModelName()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0xD8);
	}

	auto GetStickersSupportedCount()
	{
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x100); // 0x118
	}

	auto GetSimpleWeaponName()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x210);
	}

	auto GetName()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x70);
	}

	auto GetLoadoutSlot()
	{
		return *reinterpret_cast<int*>((uintptr_t)(this) + 0x2E8);
	}

	char pad0[0x8]; // vtable
	void* m_pKVItem;
	uint16_t m_nDefIndex;
	CUtlVector<uint16_t> m_nAssociatedItemsDefIndexes;
	bool m_bEnabled;
	const char* m_szPrefab;
	uint8_t m_unMinItemLevel;
	uint8_t m_unMaxItemLevel;
	uint8_t m_nItemRarity;
	uint8_t m_nItemQuality;
	uint8_t m_nForcedItemQuality;
	uint8_t m_nDefaultDropItemQuality;
	uint8_t m_nDefaultDropQuantity;
	CUtlVector<void*> m_vecStaticAttributes;
	uint8_t m_nPopularitySeed;
	void* m_pPortraitsKV;
	const char* m_pszItemBaseName;
	bool m_bProperName;
	const char* m_pszItemTypeName;
	uint32_t m_unItemTypeID;
	const char* m_pszItemDesc;
};

struct AlternateIconData_t
{
public:
	const char* sSimpleName;
	const char* sLargeSimpleName;

private:
	char pad0[0x8]; // no idea
	char pad1[0x8]; // no idea
};

class CPaintKit
{
public:
	char pad_0x0000[0xE0]; //0x0000

	int64_t PaintKitId()
	{
		return *reinterpret_cast<int64_t*>((uintptr_t)(this));
	}

	const char* PaintKitName()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x8);
	}

	const char* PaintKitDescriptionString()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x10);
	}

	const char* PaintKitDescriptionTag()
	{
		return *reinterpret_cast<const char**>((uintptr_t)(this) + 0x18);
	}

	int32_t PaintKitRarity()
	{
		return *reinterpret_cast<int32_t*>((uintptr_t)(this) + 0x44);
	}

	bool UsesOldModel()
	{
		return *reinterpret_cast<bool*>((uintptr_t)(this) + 0xB2);
	}
};

class CEconItemSchema
{
public:
	auto GetAttributeDefinitionInterface(int iAttribIndex)
	{
		return MEM::CallVFunc<void*, 27U>(this, iAttribIndex);
	}

	auto& GetSortedItemDefinitionMap()
	{
		return *reinterpret_cast<CUtlMap2<int, CEconItemDefinition*>*>(
		(uintptr_t)(this) + 0x128);
	}

	auto& GetAlternateIconsMap()
	{
		return *reinterpret_cast<CUtlMap2<uint64_t, AlternateIconData_t>*>(
		(uintptr_t)(this) + 0x278);
	}

	auto& GetPaintKits()
	{
		return *reinterpret_cast<CUtlMap2<int, CPaintKit*>*>((uintptr_t)(this) +
		0x2F0);
	}
};

enum EEconItemQuality
{
	IQ_UNDEFINED = -1,
	IQ_NORMAL,
	IQ_GENUINE,
	IQ_VINTAGE,
	IQ_UNUSUAL,
	IQ_UNIQUE,
	IQ_COMMUNITY,
	IQ_DEVELOPER,
	IQ_SELFMADE,
	IQ_CUSTOMIZED,
	IQ_STRANGE,
	IQ_COMPLETED,
	IQ_HAUNTED,
	IQ_TOURNAMENT,
	IQ_FAVORED
};

enum EEconItemRarity
{
	IR_DEFAULT,
	IR_COMMON,
	IR_UNCOMMON,
	IR_RARE,
	IR_MYTHICAL,
	IR_LEGENDARY,
	IR_ANCIENT,
	IR_IMMORTAL
};

enum EEconTypeID
{
	k_EEconTypeItem = 1,
	k_EEconTypePersonaDataPublic = 2,
	k_EEconTypeGameAccountClient = 7,
	k_EEconTypeGameAccount = 8,
	k_EEconTypeEquipInstance = 31,
	k_EEconTypeDefaultEquippedDefinitionInstance = 42,
	k_EEconTypeDefaultEquippedDefinitionInstanceClient = 43,
	k_EEconTypeCoupon = 45,
	k_EEconTypeQuest = 46,
};
//END SKINCHANGER//
enum class EGrenadeThrowState : uint32_t
{
	NotThrowing = 0x0,
	Throwing = 0x1,
	ThrowComplete = 0x2,
};

class C_EnvSky
{
public:
	SCHEMA_ADD_FIELD(bool, m_bStartDisabled, "C_EnvSky->m_bStartDisabled");
	SCHEMA_ADD_FIELD(ByteColor, m_vTintColor, "C_EnvSky->m_vTintColor");
	SCHEMA_ADD_FIELD(Color_t, m_vTintColorLightingOnly, "C_EnvSky->m_vTintColorLightingOnly");
	SCHEMA_ADD_FIELD(float, m_flBrightnessScale, "C_EnvSky->m_flBrightnessScale");
	SCHEMA_ADD_FIELD(int, m_nFogType, "C_EnvSky->m_nFogType");
	SCHEMA_ADD_FIELD(float, m_flFogMinStart, "C_EnvSky->m_flFogMinStart");
	SCHEMA_ADD_FIELD(float, m_flFogMinEnd, "C_EnvSky->m_flFogMinEnd");
	SCHEMA_ADD_FIELD(float, m_flFogMaxStart, "C_EnvSky->m_flFogMaxStart");
	SCHEMA_ADD_FIELD(float, m_flFogMaxEnd, "C_EnvSky->m_flFogMaxEnd");
	SCHEMA_ADD_FIELD(bool, m_bEnabled, "C_EnvSky->m_bEnabled");
};

class CMaterial
{
public:
	const char* GetName()
	{
		return MEM::CallVFunc<const char*, 1>(this);
	}
};

class CSceneObject
{
public:
	char pad_0000[184]; //0x0000
	uint8_t r; //0x00B8
	uint8_t g; //0x00B9
	uint8_t b; //0x00BA
	uint8_t a; //0x00BB
	char pad_00BC[196]; //0x00BC
}; //Size: 0x0180
class CMaterial2;
class CBaseSceneData
{	
public:
	char pad_0000[24]; //0x0000
	CSceneObject* sceneObject; //0x0018
	CMaterial* material; //0x0020
	CMaterial2* material2; //0x0028
	char pad_0030[16]; //0x0030
	uint8_t r; //0x0040
	uint8_t g; //0x0041
	uint8_t b; //0x0042
	uint8_t a; //0x0043
	char pad_0044[36]; //0x0044
}; //Size: 0x0068

class C_AggregateSceneObjectData
{
private:
	char pad_0000[0x38]; // 0x0
public:
	std::uint8_t r; // 0x38
	std::uint8_t g; // 0x39
	std::uint8_t b; // 0x3A
private:
	char pad_0038[0x9];
};

class C_AggregateSceneObject
{
private:
	char pad_0000[0x120];

public:
	int m_nCount; // 0x120
private:
	char pad_0120[0x4];

public:
	C_AggregateSceneObjectData* m_pData; // 0x128
};

class C_SceneLightObject
{
public:
	char pad_0000[0xE4]; // 0x0
	float r; // 0xE4
	float g; // 0xE4
	float b; // 0xE4
};

class C_PostProcessingVolume
{
public:
	SCHEMA_ADD_FIELD(bool, ExposureControl, "C_PostProcessingVolume->m_bExposureControl");
	SCHEMA_ADD_FIELD(float, MinExposure, "C_PostProcessingVolume->m_flMinExposure");
	SCHEMA_ADD_FIELD(float, MaxExposure, "C_PostProcessingVolume->m_flMaxExposure");
	SCHEMA_ADD_FIELD(float, FadeSpeedUp, "C_PostProcessingVolume->m_flExposureFadeSpeedUp");
	SCHEMA_ADD_FIELD(float, FadeSpeedDown, "C_PostProcessingVolume->m_flExposureFadeSpeedDown");
};

class CBaseCSGreande : public C_CSWeaponBase
{
public:
	SCHEMA_ADD_FIELD(bool, m_held_by_player, "C_BaseCSGrenade->m_bIsHeldByPlayer");
	SCHEMA_ADD_FIELD(bool, m_pin_pulled, "C_BaseCSGrenade->m_bPinPulled");
	SCHEMA_ADD_FIELD(float, m_throw_time, "C_BaseCSGrenade->m_fThrowTime");
	SCHEMA_ADD_FIELD(float, m_throw_strength, "C_BaseCSGrenade->m_flThrowStrength");
};

inline Vector_t CGameSceneNode::GetBonePosition(const std::int32_t eBoneIndex)
{
	CSkeletonInstance* pSkeleton = GetSkeletonInstance();
	if (pSkeleton == nullptr)
		return Vector_t(0, 0, 0);

	const CModelState modelState = pSkeleton->m_modelState();
	const BoneData_t* pBoneArray = modelState.m_pBones;
	if (!modelState.m_pBones)
		return Vector_t(0, 0, 0);
	if (pBoneArray == nullptr)
		return Vector_t(0, 0, 0);

	return pBoneArray[eBoneIndex].vecPosition;
}

class CEffectData // CEffectData in schema
{
public:
	void* vftable = {}; // 0x0
	Vector_t origin = {}; // 0x8
	Vector_t start = {}; // 0x14
	Vector_t normal = {}; // 0x20
	QAngle_t angles = {}; // 0x2c
	uint32_t handle_entity = {}; // 0x38
	uint32_t handle_other_entity = {}; // 0x3c
	float scale = {}; // 0x40
	float magnitude = {}; // 0x44
	float radius = {}; // 0x48
	int surface_prop = {}; // 0x4c
	void* effect_index = {}; // 0x50
	uint32_t damage_type = {}; // 0x58
	uint8_t penetrate = {}; // 0x5c
	uint8_t __pad005d[0x1] = {}; // 0x5d
	uint16_t material = {}; // 0x5e
	uint16_t hitbox = {}; // 0x60
	uint8_t color = {}; // 0x62
	uint8_t flags = {}; // 0x63
	uint8_t attachment_index = {}; // 0x64
	uint8_t __pad0065[0x3] = {}; // 0x65
	int attachment_name = {}; // 0x68
	uint16_t effect_name = {}; // 0x6c
	uint8_t explosion_type = {}; // 0x6e
	uint8_t __pad006f[0x9] = {};
};

enum ModelType_t : int
{
	MODEL_SUN,
	MODEL_CLOUDS,
	MODEL_EFFECTS,
	MODEL_OTHER
};

inline int GetModelType(const std::string_view& name)
{
	if (name.find(CS_XOR("sun")) != std::string::npos || name.find(CS_XOR("clouds")) != std::string::npos)
		return MODEL_SUN;

	if (name.find(CS_XOR("effects")) != std::string::npos)
		return MODEL_EFFECTS;

	return MODEL_OTHER;
}

class C_Inferno : public C_BaseEntity {
public:
	CS_CLASS_NO_INITIALIZER(C_Inferno);

	SCHEMA_ADD_FIELD(Vector_t[64], m_firePositions, "C_Inferno->m_firePositions");
	SCHEMA_ADD_FIELD(Vector_t[64], m_fireParentPositions, "C_Inferno->m_fireParentPositions");
	SCHEMA_ADD_FIELD(bool[64], m_bFireIsBurning, "C_Inferno->m_bFireIsBurning");
	SCHEMA_ADD_FIELD(Vector_t[64], m_BurnNormal, "C_Inferno->m_BurnNormal");
	SCHEMA_ADD_FIELD(int32_t, m_fireCount, "C_Inferno->m_fireCount");
	SCHEMA_ADD_FIELD(int32_t, m_nInfernoType, "C_Inferno->m_nInfernoType");
	SCHEMA_ADD_FIELD(float, m_nFireLifetime, "C_Inferno->m_nFireLifetime");
	SCHEMA_ADD_FIELD(bool, m_bInPostEffectTime, "C_Inferno->m_bInPostEffectTime");
	SCHEMA_ADD_FIELD(int32_t, m_lastFireCount, "C_Inferno->m_lastFireCount");
	SCHEMA_ADD_FIELD(int32_t, m_nFireEffectTickBegin, "C_Inferno->m_nFireEffectTickBegin");
	SCHEMA_ADD_FIELD(int32_t, m_drawableCount, "C_Inferno->m_drawableCount");
	SCHEMA_ADD_FIELD(bool, m_blosCheck, "C_Inferno->m_blosCheck");
	SCHEMA_ADD_FIELD(int32_t, m_nlosperiod, "C_Inferno->m_nlosperiod");
	SCHEMA_ADD_FIELD(float, m_maxFireHalfWidth, "C_Inferno->m_maxFireHalfWidth");
	SCHEMA_ADD_FIELD(float, m_maxFireHeight, "C_Inferno->m_maxFireHeight");
	SCHEMA_ADD_FIELD(Vector_t, m_minBounds, "C_Inferno->m_minBounds");
	SCHEMA_ADD_FIELD(Vector_t, m_maxBounds, "C_Inferno->m_maxBounds");
	SCHEMA_ADD_FIELD(float, m_flLastGrassBurnThink, "C_Inferno->m_flLastGrassBurnThink");
	SCHEMA_ADD_FIELD(GameTime_t, flCreateTime, "C_BaseEntity->m_flCreateTime");
};

class CHitBox
{
public:

	Vector_t m_vMinBounds() {
		return *reinterpret_cast<Vector_t*>(reinterpret_cast<uintptr_t>(this) + 0x18);
	}

	Vector_t m_vMaxBounds() {
		return *reinterpret_cast<Vector_t*>(reinterpret_cast<uintptr_t>(this) + 0x24);
	}

	float m_flShapeRadius() {
		return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + 0x30);
	}


private:
	// Size of 'CHitBox' class. Can be obtainted through the SchemaSystem.
	// Must have this here or we can't iterate the 'm_HitBoxes' vec3 that stores
	// CHitBox directly and not by a pointer.
	char pad[0x70];
};

class CHitBoxSet
{
public:

	CUtlVector<CHitBox>& m_HitBoxes() {
		return *reinterpret_cast<CUtlVector<CHitBox>*>(reinterpret_cast<uintptr_t>(this) + 0x10);
	}

};

struct hitboxes_data {
	Vector_t min;
	Vector_t max;
	float rad;
};
using fnHitboxToWorldTransforms = int(__fastcall*)(void*, CHitBoxSet*, CTransform*, int);
static fnHitboxToWorldTransforms HitboxToWorldTransforms_fn;
using fnGetHitboxSet = CHitBoxSet * (__fastcall*)(void*, int);
static fnGetHitboxSet GetHitboxSet_fn;

namespace fuck_this_shit {

	static void find_gethitboxset_pttr() {
		GetHitboxSet_fn = reinterpret_cast<fnGetHitboxSet>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 ?? ?? ?? 48 89 ?? ?? ?? 57 48 ?? ?? ?? ?? ?? ?? 8B DA 48 8B F9 E8 ?? ?? ?? ?? 48 8B F0 48 85 C0 0F ?? ?? ?? ?? ?? 48 8D")));
	}

	static void find_hitboxtoworldtransforms_pptr() {
		HitboxToWorldTransforms_fn = reinterpret_cast<fnHitboxToWorldTransforms>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 ?? ?? ?? 55 57 41 54 41 56 41 57 48 ?? ?? ?? 45")));
	}
}