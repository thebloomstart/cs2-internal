#include "entity.h"

// used: convars
#include "../core/convars.h"
#include "interfaces/cgameentitysystem.h"
#include "interfaces/ienginecvar.h"
#include "interfaces/iengineclient.h"

// used: game's definitions, enums
#include "const.h"
#include "../InventoryManager.h"
#include "../core/hooks.h"
#include "../core/sdk.h"
#include "../sdk/interfaces/cgameentitysystem.h"
#include "../sdk/interfaces/igameresourceservice.h"
#include "../Pena.h"
#include "../Utils.h"

// global empty vector for when we can't get the origin
static Vector_t vecEmpty = Vector_t(0, 0, 0);

CCSPlayerController* CCSPlayerController::GetLocalPlayerController()
{
	const int nIndex = I::Engine->GetLocalPlayer();
	return I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(nIndex);
}

const Vector_t& CCSPlayerController::GetPawnOrigin()
{
	CBaseHandle hPawnHandle = this->GetPawnHandle();
	if (!hPawnHandle.IsValid())
		return vecEmpty;

	C_CSPlayerPawn* pPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(hPawnHandle);
	if (pPlayerPawn == nullptr)
		return vecEmpty;

	return pPlayerPawn->GetSceneOrigin();
}

C_BaseEntity* C_BaseEntity::GetLocalPlayer()
{
	const int nIndex = I::Engine->GetLocalPlayer();
	return I::GameResourceService->pGameEntitySystem->Get(nIndex);
}

const Vector_t& C_BaseEntity::GetSceneOrigin()
{
	if (this->GetGameSceneNode())
		return GetGameSceneNode()->GetAbsOrigin();

	return vecEmpty;
}

bool C_CSPlayerPawn::IsOtherEnemy(C_CSPlayerPawn* pOther)
{
	// check are other player is invalid or we're comparing against ourselves
	if (pOther == nullptr || this == pOther)
		return false;

	if (CONVAR::game_type->value.i32 == GAMETYPE_FREEFORALL && CONVAR::game_mode->value.i32 == GAMEMODE_FREEFORALL_SURVIVAL)
		// check is not teammate
		return (this->GetSurvivalTeam() != pOther->GetSurvivalTeam());

	// @todo: check is deathmatch
	if (CONVAR::mp_teammates_are_enemies->value.i1)
		return true;

	return this->GetAssociatedTeam() != pOther->GetAssociatedTeam();
}

int C_CSPlayerPawn::GetAssociatedTeam()
{
	const int nTeam = this->GetTeam();

	// @todo: check is coaching, currently cs2 doesnt have sv_coaching_enabled, so just let it be for now...
	//if (CONVAR::sv_coaching_enabled->GetBool() && nTeam == TEAM_SPECTATOR)
	//	return this->GetCoachingTeam();

	return nTeam;
}

bool C_CSPlayerPawn::CanAttack(const float flServerTime)
{
	// check is player ready to attack
	if (CCSPlayer_WeaponServices* pWeaponServices = this->GetWeaponServices(); pWeaponServices != nullptr)
		if (this->IsWaitForNoAttack() || pWeaponServices->GetNextAttack() > flServerTime)
			return false;

	return true;
}


std::uint32_t C_CSPlayerPawn::GetOwnerHandleIndex()
{
	std::uint32_t Result = -1;
	if (this && GetCollision() && !(GetCollision()->GetSolidFlags() & 4))
		Result = this->GetOwnerHandle().GetEntryIndex();

	return Result;
}

std::uint16_t C_CSPlayerPawn::GetCollisionMask()
{
	if (this && GetCollision())
		return GetCollision()->CollisionMask(); // Collision + 0x38

	return 0;
}

bool C_CSWeaponBaseGun::CanPrimaryAttack(const int nWeaponType, const float flServerTime)
{
	// check are weapon support burst mode and it's ready to attack
	if (this->IsBurstMode())
	{
		// check is it ready to attack
		if (this->GetBurstShotsRemaining() > 0 /*&& this->GetNextBurstShotTime() <= flServerTime*/)
			return true;
	}

		// check is weapon ready to attack
	if (this->GetNextPrimaryAttackTick() > TIME_TO_TICKS(flServerTime))
		return false;

	// we doesn't need additional checks for knives
	if (nWeaponType == WEAPONTYPE_KNIFE)
		return true;

	// check do weapon have ammo
	if (this->GetClip1() <= 0)
		return false;

	const ItemDefinitionIndex_t nDefinitionIndex = this->GetAttributeManager()->GetItem()->GetItemDefinitionIndex();

	// check for revolver cocking ready
	if (nDefinitionIndex == WEAPON_R8_REVOLVER && this->GetPostponeFireReadyFrac() > flServerTime)
		return false;

	return true;
}

bool C_CSWeaponBaseGun::CanSecondaryAttack(const int nWeaponType, const float flServerTime)
{
	// check is weapon ready to attack
	if (this->GetNextSecondaryAttackTick() > TIME_TO_TICKS(flServerTime))
		return false;

	// we doesn't need additional checks for knives
	if (nWeaponType == WEAPONTYPE_KNIFE)
		return true;

	// check do weapon have ammo
	if (this->GetClip1() <= 0)
		return false;

	// only revolver is allowed weapon for secondary attack
	if (this->GetAttributeManager()->GetItem()->GetItemDefinitionIndex() != WEAPON_R8_REVOLVER)
		return false;

	return true;
}
//START SKINCHANGER//

CGCClientSharedObjectTypeCache* CGCClientSharedObjectCache::CreateBaseTypeCache(
int nClassID)
{
	using fnCGCClientSharedObjectTypeCache = CGCClientSharedObjectTypeCache*(CS_FASTCALL*)(void*, int);
	static fnCGCClientSharedObjectTypeCache createbasetypecache = reinterpret_cast<fnCGCClientSharedObjectTypeCache>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, ("E8 ? ? ? ? 33 C9 8B D1")), 1, 0));
	CS_ASSERT(createbasetypecache != nullptr);
	if (createbasetypecache == nullptr)
	{
		L_PRINT(LOG_INFO) << "INVALID SIGNATURE createbasetypecache" << createbasetypecache;
	}
	return createbasetypecache(this, nClassID);
}

CGCClientSharedObjectCache* CGCClient::FindSOCache(SOID_t ID,
bool bCreateIfMissing)
{
	using fnFindSOCache = CGCClientSharedObjectCache*(CS_FASTCALL*)(void*, SOID_t, bool);
	static auto FindSOCache = reinterpret_cast<fnFindSOCache>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, ("E8 ? ? ? ? 48 8B F0 48 85 C0 74 0E 4C 8B C3")), 1));
	CS_ASSERT(FindSOCache != nullptr);
	if (FindSOCache == nullptr)
	{
		L_PRINT(LOG_INFO) << "INVALID SIGNATURE FindSOCache" << FindSOCache;
	}
	return FindSOCache(this, ID, bCreateIfMissing);
}

void CEconItem::SetDynamicAttributeValue(int index, void* value)
{
	CEconItemSchema* pItemSchema =
	I::Client->GetEconItemSystem()->GetEconItemSchema();
	if (!pItemSchema)
		return;

	void* pAttributeDefinitionInterface =
	pItemSchema->GetAttributeDefinitionInterface(index);
	if (!pAttributeDefinitionInterface)
		return;

	if (!MEM::fnSetDynamicAttributeValueUint)
		return;

	MEM::fnSetDynamicAttributeValueUint(this, pAttributeDefinitionInterface, value);
}

void CEconItem::SetDynamicAttributeValueString(int index, const char* value)
{
	// CS2FIXME: Function got inlined and cannot be sigscanned.
}

bool CEconItemDefinition::is_custom_player()
{
	auto CSGO_Type_Agents = FNV1A::Hash("#Type_CustomPlayer");

	if (FNV1A::Hash(this->m_pszItemBaseName) != CSGO_Type_Agents)
		return false;

	if (m_nDefIndex >= 5036 && m_nDefIndex < 5106)
		return false;

	if (m_nDefIndex >= 5200 && m_nDefIndex <= 5205)
		return false;

	if (m_nDefIndex >= 5300 && m_nDefIndex <= 5305)
		return false;

	if (m_nDefIndex == 5600)
		return false;

	return true;
}

bool CEconItemDefinition::IsWeapon()
{
	// Every gun supports at least 4 stickers.
	return GetStickersSupportedCount() >= 4;
}

bool CEconItemDefinition::IsKnife(bool excludeDefault)
{
	auto CSGO_Type_Knife = FNV1A::Hash("#CSGO_Type_Knife");

	if (FNV1A::Hash(this->m_pszItemBaseName) != CSGO_Type_Knife)
		return false;

	return excludeDefault ? m_nDefIndex >= 500 : true;
}

bool CEconItemDefinition::IsGlove(bool excludeDefault)
{
	auto Type_Hands = FNV1A::Hash("#Type_Hands");

	bool valid = FNV1A::Hash(this->m_pszItemBaseName) == Type_Hands;
	bool defaultGlove = valid && m_nDefIndex == 5028 || m_nDefIndex == 5029;

	return excludeDefault ? !defaultGlove : valid;
}

bool CEconItemDefinition::IsAgent()
{
	auto Type_CustomPlayer = FNV1A::Hash("#Type_CustomPlayer");

	bool valid = FNV1A::Hash(this->m_pszItemBaseName) == Type_CustomPlayer;

	if (valid || m_nDefIndex >= 5036 && m_nDefIndex < 5106)
		return false;

	if (valid || m_nDefIndex >= 5200 && m_nDefIndex <= 5205)
		return false;

	if (valid || m_nDefIndex >= 5300 && m_nDefIndex <= 5305)
		return false;

	if (valid || m_nDefIndex == 5600)
		return false;

	return true;
}

CEconItem* CEconItem::CreateInstance()
{
	// ida: // #STR: "Update(CEconItem)", "CEconItem", "Create(CEconItem)", "BuildCacheSubscribed(CEconItem)", "Update(CEconEquipSlot)", "CEconEquipSlot", "Create(CEconEquipSlot)", "BuildCacheSubscribed(CEconEquipSlot)", "Update(CEconPersonaDataPublic)", "CEconPersonaDataPublic"
	// ida:  sub_E0F420(
	//	1,
	//	(unsigned int)CEconItem::CreateInstance,
	//	0,
	//	(unsigned int)"CEconItem",
	//	(__int64)"BuildCacheSubscribed(CEconItem)",
	//	(__int64)"Create(CEconItem)",
	//	(__int64)"Update(CEconItem)");

	using fnCreateSharedObjectSubclassEconItem = CEconItem*(__cdecl*)();
	static fnCreateSharedObjectSubclassEconItem oCreateSharedObjectSubclassEconItem = reinterpret_cast<fnCreateSharedObjectSubclassEconItem>(MEM::FindPattern(CLIENT_DLL,("48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85")));

#ifdef CS_PARANOID
	CS_ASSERT(oCreateSharedObjectSubclassEconItem != nullptr);
#endif

	return oCreateSharedObjectSubclassEconItem();
}

CEconItem* C_EconItemView::GetSOCData(CCSPlayerInventory* sdfsdf)
{
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return nullptr;

	return pInventory->GetSOCDataForItem(GetItemIDCS2());
}

void CGameSceneNode::SetMeshGroupMask(uint64_t meshGroupMask)
{
	if (!MEM::SetMeshGroupMask)
	{
		L_PRINT(LOG_ERROR) << "error getting meshgroupmask";
	}

	return MEM::SetMeshGroupMask(this, meshGroupMask);
}

void C_BaseModelEntity::SetModel(const char* name)
{
	auto orig = H::hkSetModel.GetOriginal();
	return orig(this, name);
}
// END SKINCHAGER //
C_CSWeaponBase* C_CSPlayerPawn::GetWeaponActive()
{
	auto service = GetWeaponServices();

	if (!service)
		return nullptr;

	auto handle = service->GetActiveWeapon();

	auto weapon = reinterpret_cast<C_CSWeaponBase*>(I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(handle));

	if (!weapon)
		return nullptr;

	return weapon;
}


bool CCSPlayerController::IsThrowingGrenade(C_CSWeaponBase* pBaseWeapon)
{
	if (!pBaseWeapon)
		return false;

	if (!SDK::LocalController->IsPawnAlive() || !SDK::LocalController)
		return false;

	const float flServerTime = TIME_TO_TICKS(this->GetTickBase());
	const short nDefinitionIndex = pBaseWeapon->GetAttributeManager()->GetItem()->GetItemDefinitionIndex();
	CCSWeaponBaseVData* pWeaponBaseVData = pBaseWeapon->GetWeaponVData();
	if (!pWeaponBaseVData)
		return false;

	if (pWeaponBaseVData->GetWeaponType() == WEAPONTYPE_GRENADE)
	{
		C_BaseCSGrenade* pGrenade = reinterpret_cast<C_BaseCSGrenade*>(pBaseWeapon);
		if (pGrenade != nullptr)
		{
			return !pGrenade->IsPinPulled() && pGrenade->GetThrowTime() > 0.f && pGrenade->GetThrowTime() < flServerTime;
		}
	}

	return false;
}

C_CSWeaponBase* CCSPlayerController::GetPlayerWeapon(C_CSPlayerPawn* pPlayer)
{
	if (!pPlayer || !pPlayer->GetWeaponServices())
		return nullptr;

	CBaseHandle hActiveWeapon = pPlayer->GetWeaponServices()->GetActiveWeapon();

	if (!hActiveWeapon.IsValid())
		return nullptr;

	C_CSWeaponBase* pWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(hActiveWeapon);

	return pWeapon;
}

bool C_CSPlayerPawn::IsThorning()
{
	C_CSWeaponBase* active_weapon = this->GetWeaponActive();

	if (!active_weapon)
		return false;

	CBaseCSGreande* grenade = reinterpret_cast<CBaseCSGreande*>(active_weapon);
	CCSWeaponBaseVData* weapon_data = active_weapon->GetWeaponVData();

	if (!grenade || !weapon_data)
		return false;

	if (weapon_data->GetWeaponType() == WEAPONTYPE_GRENADE && grenade->m_throw_time() != 0.f)
		return true;

	return false;
}

int C_CSPlayerPawn::get_bone_index(const char* name)
{
	using function_t = int(__fastcall*)(void*, const char*);
	static function_t fn = reinterpret_cast<function_t>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ?? ?? ?? ?? 85 C0 78 11")), 0x1));
	CS_ASSERT(fn != nullptr);
	return fn(this, name);
}

//////////////////////////////////////////////////////////////////////////
// #STR: "C:\\buildworker\\csgo_rel_win64\\build\\src\\game\\shared\, "Bone merge bones from parent were invalid: parent model '%, "CalcWorldSpaceBones"
void CS_FASTCALL CSkeletonInstance::calc_world_space_bones(uint32_t mask)
{ // cHoca

	using fnNewCalcWSsBones = void(CS_FASTCALL)(void*, uint32_t);
	static auto bone_new = reinterpret_cast<fnNewCalcWSsBones*>(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC D0")));
	if (bone_new == nullptr)
		L_PRINT(LOG_WARNING) << "FAIL calc_world_space_bones";
	CS_ASSERT(bone_new != nullptr);

	return bone_new(this, mask);
}

uint32_t CModel::GetHitboxesNum()
{
	using fnHitboxNum = uint32_t(CS_FASTCALL*)(void*);
	static auto HitboxNum = reinterpret_cast<fnHitboxNum>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 85 C0 7E ? 83 7F 20 00")), 1, 0));
	CS_ASSERT(HitboxNum != nullptr);

	return HitboxNum(this);
}

uint32_t CModel::GetHitboxFlags(uint32_t index)
{
	using fnHitboxFlags = uint32_t(CS_FASTCALL*)(void*, uint32_t);
	static auto HitboxFlags = reinterpret_cast<fnHitboxFlags>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 78 16 3B 91")));
	CS_ASSERT(HitboxFlags != nullptr);

	return HitboxFlags(this, index);
}

uint32_t CModel::GetHitboxParent(uint32_t index)
{
	using fnHitboxParent = uint32_t(CS_FASTCALL*)(void*, uint32_t);
	static auto HitboxParent = reinterpret_cast<fnHitboxParent>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 78 17 3B 91 78")));
	CS_ASSERT(HitboxParent != nullptr);
	return HitboxParent(this, index);
}

const char* CModel::GetHitboxName(uint32_t index)
{
	using fnHitboxName = const char*(CS_FASTCALL*)(void*, uint32_t);
	static auto HitboxName = reinterpret_cast<fnHitboxName>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 78 25 3B 91")));
	CS_ASSERT(HitboxName != nullptr);

	return HitboxName(this, index);
}

C_CSPlayerPawn* CCSPlayerController::get_player_pawn()
{
	if (!GetPlayerPawnHandle().IsValid())
		return nullptr;

	int index = GetPlayerPawnHandle().GetEntryIndex();
	return I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(index);
}

C_CSWeaponBase* C_CSPlayerPawn::GetActiveWeapon()
{
	auto service = GetWeaponServices();

	if (!service)
		return nullptr;

	auto handle = service->GetActiveWeapon();

	if (!handle.IsValid())
		return nullptr;

	auto weapon = reinterpret_cast<C_CSWeaponBase*>(I::GameResourceService->pGameEntitySystem->Get(handle.GetEntryIndex()));

	if (!weapon)
		return nullptr;

	return weapon;
}

C_CSPlayerPawn* CCSPlayerController::get_observer_pawn()
{
	if (!GetPlayerPawnObserver().IsValid())
		return nullptr;

	int index = GetPlayerPawnObserver().GetEntryIndex();
	return I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(index);
}

bool C_CSPlayerPawn::hasArmour(const int hitgroup)
{
	if (!this->GetItemServices())
		return false;

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return this->GetItemServices()->m_bHasHelmet();
	case HITGROUP_GENERIC:
	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		return true;
	default:
		return false;
	}
}

bool C_CSWeaponBase::HasScope() {
	auto defidx = GetAttributeManager()->GetItem()->GetItemDefinitionIndex();
	return defidx == WEAPON_AWP || defidx == WEAPON_G3SG1 || defidx == WEAPON_SCAR_20 || defidx == WEAPON_SSG_08;
}

bool C_CSWeaponBase::IsNade() {
	auto defidx = GetWeaponVData()->GetWeaponType();
	return defidx == WEAPONTYPE_GRENADE;
}
bool C_CSWeaponBase::IsKnife() {
	auto defidx = GetWeaponVData()->GetWeaponType();
	return defidx == WEAPONTYPE_KNIFE;
}


CHitBoxSet* C_CSPlayerPawn::GetHitboxSet(int i)
{
	if (!GetHitboxSet_fn)
	    GetHitboxSet_fn = reinterpret_cast<fnGetHitboxSet>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 ?? ?? ?? 48 89 ?? ?? ?? 57 48 ?? ?? ?? ?? ?? ?? 8B DA 48 8B F9 E8 ?? ?? ?? ?? 48 8B F0 48 85 C0 0F ?? ?? ?? ?? ?? 48 8D")));

	return GetHitboxSet_fn(this, i);
}

int C_CSPlayerPawn::HitboxToWorldTransforms(CHitBoxSet* hitBoxSet, CTransform* hitboxToWorld)
{
	if (!HitboxToWorldTransforms_fn)
    	HitboxToWorldTransforms_fn = reinterpret_cast<fnHitboxToWorldTransforms>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 ?? ?? ?? 55 57 41 54 41 56 41 57 48 ?? ?? ?? 45")));

	return HitboxToWorldTransforms_fn(this, hitBoxSet, hitboxToWorld, 1024);
}

hitboxes_data C_CSPlayerPawn::GetHitboxRadMinMax(int i)
{
	hitboxes_data return_data{};
	constexpr int MAX_HITBOXES = 64;

	CHitBoxSet* hitboxSet = GetHitboxSet(0);
	if (!hitboxSet)
		return return_data;

	CHitBox* hitbox = &hitboxSet->m_HitBoxes()[i];
	if (!hitbox)
		return return_data;

	CTransform hitBoxTransforms[MAX_HITBOXES];
	int hitBoxCount = HitboxToWorldTransforms(hitboxSet, hitBoxTransforms);
	if (!hitBoxCount)
		return return_data;

	const Matrix3x4_t hitBoxMatrix = hitBoxTransforms[i].ToMatrix();
	Vector_t worldMins, worldMaxs;

	g_Utils->TransformAABB(hitBoxMatrix, hitbox->m_vMinBounds(), hitbox->m_vMaxBounds(), worldMins, worldMaxs);

	return_data.rad = hitbox->m_flShapeRadius();
	return_data.min = worldMins;
	return_data.max = worldMaxs;

	return return_data;
}