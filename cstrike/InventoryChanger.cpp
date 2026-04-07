#include <vector>

#include "InvecntoryChanger.h"
#include "sdk/interfaces/iengineclient.h"
#include "core/config.h"
#include "core/variables.h"
#include "sdk/datatypes/usercmd.h"
#include "core/sdk.h"
#include "sdk/entity.h"
#include "sdk/interfaces/iglobalvars.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "../cstrike/sdk/interfaces/iengineclient.h"
#include "sdk\datatypes\qangle.h"
#include "sdk\datatypes\vector.h"
#include "../cstrike/sdk/interfaces/inetworkclientservice.h"
#include "../cstrike/sdk/interfaces/ccsgoinput.h"
#include "sdk/interfaces/ienginecvar.h"
#include <iostream>
#include <memoryapi.h>
#include <mutex>
#include <array>
#include "../cstrike/sdk/interfaces/iengineclient.h"
#include "InventoryManager.h"
#include "PlayerInventory.h"
#include "../cstrike/core/hooks.h"
#include "../cstrike/sdk/interfaces/ccsgoinput.h"
#include "sdk/datatypes/utlmap.h"
#include "sdk/interfaces/iengineclient.h"

static std::vector<uint64_t> g_vecAddedItemsIDs;

static int glove_frame = 0;

struct GloveInfo
{
	int itemId;
	uint64_t itemHighId;
	uint64_t itemLowId;
	int itemDefId;
};

static GloveInfo addedGloves;

// Define a struct to hold glove information

#pragma pack(push, 8)
#define STRINGTOKEN_MURMURHASH_SEED 0x31415926

class CUtlStringToken
{
public:
	explicit CUtlStringToken(const char* szKeyName)
	{
		uHashCode = FNV1A::Hash(szKeyName, STRINGTOKEN_MURMURHASH_SEED);
		szDebugName = szKeyName;
	}

	constexpr CUtlStringToken(const FNV1A_t uHashCode, const char* szKeyName) :
		uHashCode(uHashCode), szDebugName(szKeyName) { }

	CS_INLINE bool operator==(const CUtlStringToken& other) const
	{
		return (other.uHashCode == uHashCode);
	}

	CS_INLINE bool operator!=(const CUtlStringToken& other) const
	{
		return (other.uHashCode != uHashCode);
	}

	CS_INLINE bool operator<(const CUtlStringToken& other) const
	{
		return (uHashCode < other.uHashCode);
	}

public:
	FNV1A_t uHashCode = 0U; // 0x00
	const char* szDebugName = nullptr; // 0x08 //   @Todo: for some reason retards keep this even for non-debug builds, it can be changed later
};

#pragma pack(pop)
// helper to create a string token at compile-time
CS_INLINE consteval CUtlStringToken MakeStringToken(const char* szKeyName)
{
	return { FNV1A::HashConst(szKeyName, STRINGTOKEN_MURMURHASH_SEED), szKeyName };
}

inline uint32_t MurmurHash2(const void* key, int len, uint32_t seed)
{
	/* 'm' and 'r' are mixing constants generated offline.
	   They're not really 'magic', they just happen to work well.  */

	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	/* Initialize the hash to a 'random' value */

	uint32_t h = seed ^ len;

	/* Mix 4 bytes at a time into the hash */

	const unsigned char* data = (const unsigned char*)key;

	while (len >= 4)
	{
		uint32_t k = *(uint32_t*)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	/* Handle the last few bytes of the input array  */

	switch (len)
	{
	case 3:
		h ^= data[2] << 16;
	case 2:
		h ^= data[1] << 8;
	case 1:
		h ^= data[0];
		h *= m;
	};

	/* Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.  */

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

#define TOLOWERU(c) ((uint32_t)(((c >= 'A') && (c <= 'Z')) ? c + 32 : c))

inline uint32_t MurmurHash2LowerCaseA1(const char* pString, int len, uint32_t nSeed)
{
	char* p = (char*)malloc(len + 1);
	for (int i = 0; i < len; i++)
	{
		p[i] = TOLOWERU(pString[i]);
	}
	return MurmurHash2(p, len, nSeed);
}

#define DEBUG_STRINGTOKENS 0

class CUtlStringToken1
{
public:
	std::uint32_t m_nHashCode;
#if DEBUG_STRINGTOKENS
	const char* m_pDebugName;
#endif

	CUtlStringToken1(const char* szString)
	{
		this->SetHashCode1(this->MakeStringToken1(szString));
	}

	bool operator==(const CUtlStringToken1& other) const
	{
		return (other.m_nHashCode == m_nHashCode);
	}

	bool operator!=(const CUtlStringToken1& other) const
	{
		return (other.m_nHashCode != m_nHashCode);
	}

	bool operator<(const CUtlStringToken1& other) const
	{
		return (m_nHashCode < other.m_nHashCode);
	}

	/// access to the hash code for people who need to store thse as 32-bits, regardless of the
	/// setting of DEBUG_STRINGTOKENS (for instance, for atomic operations).
	uint32_t GetHashCode1(void) const
	{
		return m_nHashCode;
	}

	void SetHashCode1(uint32_t nCode)
	{
		m_nHashCode = nCode;
	}

	__forceinline std::uint32_t MakeStringToken1(const char* szString, int nLen)
	{
		std::uint32_t nHashCode = MurmurHash2LowerCaseA1(szString, nLen, STRINGTOKEN_MURMURHASH_SEED);
		return nHashCode;
	}

	__forceinline std::uint32_t MakeStringToken1(const char* szString)
	{
		return MakeStringToken1(szString, (int)strlen(szString));
	}

	//__forceinline std::uint32_t MakeStringToken(CUtlString& str)
	//{
	//    return MakeStringToken(str.Get(), str.Length());
	//}

	CUtlStringToken1()
	{
		m_nHashCode = 0;
	}
};

struct material_info
{
	skin_changer::material_record* p_mat_records;
	uint32_t ui32_count;
};

void invalidate_glove_material(C_BaseViewModel* viewmodel)
{
	using function_t = void(__fastcall*)(void*, uintptr_t);
	static function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(CLIENT_DLL, CS_XOR("89 54 24 10 48 83 EC 28 48 81 C1 ? ? ? ? 85 D2 75 09 48 83 C4 28 E9")));
	CS_ASSERT(fn != nullptr);
	return fn(viewmodel, skin_changer::material_magic_number::material_magic_number__gloves);
}

void UpdateKnife(int stage, C_CSPlayerPawn* player)
{
	if (stage != 6)
		return;

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	auto steam_id = pInventory->GetOwner().m_id;

	auto pViewModelServices = player->GetViewModelServices();
	if (!pViewModelServices)
		return;

	C_CSGOViewModel* pViewModel = reinterpret_cast<C_CSGOViewModel*>(I::GameResourceService->pGameEntitySystem->Get(pViewModelServices->m_hViewModel().GetEntryIndex()));

	int highestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();

	for (int i = 64 + 1; i <= highestIndex; ++i)
	{
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get<C_BaseEntity>(i);
		if (!pEntity || !pEntity->IsWeapon())
			continue;

		C_CSWeaponBase* pWeapon = reinterpret_cast<C_CSWeaponBase*>(pEntity);

		if (pWeapon->GetWeaponVData()->GetWeaponType() == WEAPONTYPE_KNIFE == WEAPONTYPE_C4 == WEAPONTYPE_GRENADE)
			return;
		if (pWeapon->GetOriginalOwnerXuid() != steam_id)
			continue;

		if (pWeapon->GetWeaponVData() && pWeapon->GetWeaponVData()->GetWeaponType() != WEAPONTYPE_KNIFE)
		{
			continue;
		}

		C_AttributeContainer* pAttributeContainer = pWeapon->GetAttributeManager();
		if (!pAttributeContainer)
			continue;

		C_EconItemView* pWeaponItemView = pAttributeContainer->GetItem();
		if (!pWeaponItemView)
			continue;

		CEconItemDefinition* pWeaponDefinition = pWeaponItemView->GetStaticData();
		if (!pWeaponDefinition)
			continue;

		CGameSceneNode* pWeaponSceneNode = pWeapon->GetGameSceneNode();
		if (!pWeaponSceneNode)
			continue;

		C_EconItemView* pWeaponInLoadoutItemView = nullptr;

		if (pWeaponDefinition->GetStickersSupportedCount() >= 4)
		{
			for (int i = 0; i <= 56; ++i)
			{
				C_EconItemView* pItemView = pInventory->GetItemInLoadout(
					pWeapon->m_iOriginalTeamNumber(), i);
				if (!pItemView)
					continue;

				if (pItemView->GetItemDefinitionIndex() == pWeaponDefinition->m_nDefIndex)
				{
					pWeaponInLoadoutItemView = pItemView;
					break;
				}
			}
		}
		else
		{
			pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(
				pWeapon->m_iOriginalTeamNumber(),
				pWeaponDefinition->GetLoadoutSlot());
		}
		if (!pWeaponInLoadoutItemView)
			continue;

		auto it = std::find(g_vecAddedItemsIDs.cbegin(), g_vecAddedItemsIDs.cend(), pWeaponInLoadoutItemView->GetItemIDCS2());
		if (it == g_vecAddedItemsIDs.cend())
			continue;

		CEconItemDefinition* pWeaponInLoadoutDefinition = pWeaponInLoadoutItemView->GetStaticData();
		if (!pWeaponInLoadoutDefinition)
			continue;

		if (!pWeaponInLoadoutDefinition->IsKnife(false) && pWeaponInLoadoutDefinition->m_nDefIndex != pWeaponDefinition->m_nDefIndex)
			continue;

		pWeaponItemView->m_bDisallowSOC() = false;
		pWeaponItemView->GetItemIDCS2() = pWeaponInLoadoutItemView->GetItemIDCS2();
		pWeaponItemView->GetItemIDHigh() = pWeaponInLoadoutItemView->GetItemIDHigh();
		pWeaponItemView->GetItemIDLow() = pWeaponInLoadoutItemView->GetItemIDLow();
		pWeaponItemView->GetAccountID() = uint32_t(steam_id);
		pWeaponItemView->GetItemDefinitionIndex() = pWeaponInLoadoutDefinition->m_nDefIndex;
		// *(bool*)(std::uintptr_t(pWeaponItemView) + 0x1e9) = false;

		std::uint32_t hash_ = CUtlStringToken1(std::to_string(pWeaponInLoadoutDefinition->m_nDefIndex).c_str()).GetHashCode1();
		pWeapon->m_nSubclassID() = hash_;
		pWeapon->UpdateSubClass();

		pWeapon->GetWeaponVData()->m_szName() = pWeaponInLoadoutDefinition->GetSimpleWeaponName();
		pWeapon->UpdateVData();

		const char* knifeModel = pWeaponInLoadoutDefinition->GetModelName();
		pWeapon->SetModel(knifeModel);
		CBaseHandle hWeapon = pWeapon->GetRefEHandle();

		if (pViewModel && (pViewModel->m_hWeapon().GetEntryIndex() == hWeapon.GetEntryIndex()))
		{
			pViewModel->SetModel(knifeModel);
			pViewModel->pAnimationGraphInstance->pAnimGraphNetworkedVariables = nullptr;
		}
	}
}

void update_agent(C_CSPlayerPawn* player)
{
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	if (!player)
		return;

	CEconItemDefinition* item_view_loadout = nullptr;
	for (int i = 0; i <= 56; ++i)
	{
		C_EconItemView* item_view_ = pInventory->GetItemInLoadout(player->GetTeam(), i);
		if (!item_view_)
			continue;

		auto p = item_view_->GetStaticData();

		if (p->is_custom_player())
		{
			item_view_loadout = p;
			break;
		}
	}

	if (!item_view_loadout)
		return;

	if (FNV1A::Hash(item_view_loadout->GetModelName()) == skin_changer::hash_agent)
		return;

	skin_changer::hash_agent = FNV1A::Hash(item_view_loadout->GetModelName());

	player->SetModel(item_view_loadout->GetModelName());
}

void UpdateGlove(int stage, C_CSPlayerPawn* player)
{
	if (stage != 6)
		return;

	auto pLocalPawn = player;

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	auto steamID = pInventory->GetOwner().m_id;
	C_CSGOViewModel* pViewModel = (C_CSGOViewModel*)I::GameResourceService->pGameEntitySystem->Get(pLocalPawn->GetViewModelServices()->m_hViewModel().GetEntryIndex());

	C_EconItemView* pGloveItemView = &pLocalPawn->m_EconGloves();
	if (!pGloveItemView)
		return;

	auto pItemViewLoadout = pInventory->GetItemInLoadout(pLocalPawn->GetTeam(), 41);
	if (!pItemViewLoadout)
		return;

	auto pGloveDefinition = pGloveItemView->GetStaticData();
	if (!pGloveDefinition)
		return;

	static std::uint8_t uUpdateFrames = 0U;
	/* Detect if we have to change gloves */
	if (pGloveItemView->GetItemIDCS2() != pItemViewLoadout->GetItemIDCS2() || force_update)
	{
		uUpdateFrames = 3U;
		L_PRINT(LOG_INFO) << "UPDATE";
		pGloveItemView->GetItemDefinitionIndex() = pItemViewLoadout->GetItemDefinitionIndex();
		pGloveItemView->GetItemIDCS2() = pItemViewLoadout->GetItemIDCS2();
		pGloveItemView->GetItemIDHigh() = pItemViewLoadout->GetItemIDHigh();
		pGloveItemView->GetItemIDLow() = pItemViewLoadout->GetItemIDLow();
		pGloveItemView->GetAccountID() = pItemViewLoadout->GetAccountID();
		pGloveItemView->m_bDisallowSOC() = false;
		force_update = false;
	}
	/* Detect if we have to change pGloveItemView */
	if (uUpdateFrames)
	{
		invalidate_glove_material(pViewModel);

		pGloveItemView->m_bInitialized() = true;
		pLocalPawn->m_bNeedToReApplyGloves() = true;

		uUpdateFrames--;
	}
}

void skin_changer::OnFrameStageNotify(int frameStage)
{
	if (frameStage != 6 and frameStage != 3)
		return;

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();
	if (!pLocalPlayerController)
		return;

	C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
	if (!pLocalPawn)
		return;

	if (pLocalPawn->GetHealth() <= 0)
		return;

	CGameEntitySystem* pEntitySystem = I::GameResourceService->pGameEntitySystem;
	if (!pEntitySystem)
		return;

	const uint64_t steamID = pInventory->GetOwner().m_id;

	CCSPlayer_ViewModelServices* pViewModelServices = pLocalPawn->GetViewModelServices();
	if (!pViewModelServices)
		return;

	C_CSGOViewModel* pViewModel = I::GameResourceService->pGameEntitySystem->Get<C_CSGOViewModel>(pViewModelServices->m_hViewModel());
	if (!pViewModel)
		return;

	UpdateGlove(frameStage, pLocalPawn);
	update_agent(pLocalPawn);
	UpdateKnife(frameStage, pLocalPawn);
	int highestIndex = pEntitySystem->GetHighestEntityIndex();
	for (int i = 64 + 1; i <= highestIndex; ++i)
	{
		C_BaseEntity* pEntity = pEntitySystem->Get(i);
		if (!pEntity || !pEntity->IsWeapon())
			continue;

		C_CSWeaponBase* pWeapon = reinterpret_cast<C_CSWeaponBase*>(pEntity);
		if (pWeapon->GetOriginalOwnerXuid() != steamID)
			continue;

		C_AttributeContainer* pAttributeContainer = pWeapon->GetAttributeManager();
		if (!pAttributeContainer)
			continue;

		C_EconItemView* pWeaponItemView = &pAttributeContainer->m_Item();
		if (!pWeaponItemView)
			continue;

		CEconItemDefinition* pWeaponDefinition =
			pWeaponItemView->GetStaticData();
		if (!pWeaponDefinition)
			continue;

		CGameSceneNode* pWeaponSceneNode = pWeapon->GetGameSceneNode();
		if (!pWeaponSceneNode)
			continue;

		// No idea how to check this faster with the new loadout system.
		C_EconItemView* pWeaponInLoadoutItemView = nullptr;

		if (pWeaponDefinition->IsWeapon())
		{
			for (int i = 0; i <= 56; ++i)
			{
				C_EconItemView* pItemView = pInventory->GetItemInLoadout(
					pWeapon->m_iOriginalTeamNumber(), i);
				if (!pItemView)
					continue;

				if (pItemView->GetItemDefinitionIndex() ==
					pWeaponDefinition->m_nDefIndex)
				{
					pWeaponInLoadoutItemView = pItemView;
					break;
				}
			}
		}
		else
		{
			pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(
				pWeapon->m_iOriginalTeamNumber(),
				pWeaponDefinition->GetLoadoutSlot());
		}

		if (!pWeaponInLoadoutItemView)
			continue;

		// Check if skin is added by us.
		auto it = std::find(g_vecAddedItemsIDs.cbegin(), g_vecAddedItemsIDs.cend(), pWeaponInLoadoutItemView->GetItemIDCS2());
		if (it == g_vecAddedItemsIDs.cend())
			continue;

		CEconItemDefinition* pWeaponInLoadoutDefinition = pWeaponInLoadoutItemView->GetStaticData();
		if (!pWeaponInLoadoutDefinition)
			continue;

		// Example: Will not equip FiveSeven skin on CZ. Not applies for knives.
		const bool isKnife = pWeaponInLoadoutDefinition->IsKnife(false);
		pWeaponItemView->m_bDisallowSOC() = false;
		pWeaponInLoadoutItemView->m_bDisallowSOC() = false;

		pWeaponItemView->GetItemIDCS2() = pWeaponInLoadoutItemView->GetItemIDCS2();
		pWeaponItemView->GetItemIDHigh() = pWeaponInLoadoutItemView->GetItemIDHigh();
		pWeaponItemView->GetItemIDLow() = pWeaponInLoadoutItemView->GetItemIDLow();
		pWeaponItemView->GetAccountID() = uint32_t(pInventory->GetOwner().m_id);
		pWeaponItemView->GetItemDefinitionIndex() = pWeaponInLoadoutDefinition->m_nDefIndex;

		// pWeaponItemView->m_bIsStoreItem() = true;
		//pWeaponItemView->m_bIsTradeItem() = true;

		// Displays nametag and stattrak on the gun.
		// Found by: https://www.unknowncheats.me/forum/members/2377851.html
		/*   if (!pWeapon->m_bUIWeapon()) {
			pWeapon->AddStattrakEntity();
			pWeapon->AddNametagEntity();
		}*/
		CBaseHandle hWeapon = pWeapon->GetRefEHandle();
		if (isKnife)
		{
			if (pViewModel && (pViewModel->m_hWeapon().GetEntryIndex() == hWeapon.GetEntryIndex() || pViewModel->m_hWeapon() == hWeapon))
			{
				if (pWeapon->GetWeaponVData()->GetWeaponType() == 9 or pWeapon->GetWeaponVData()->GetWeaponType() == 7)
					continue;
				pWeaponItemView->GetItemDefinitionIndex() = pWeaponInLoadoutDefinition->m_nDefIndex;

				const char* knifeModel = pWeaponInLoadoutDefinition->GetModelName();

				std::uint32_t Hash = CUtlStringToken1(std::to_string(pWeaponInLoadoutDefinition->m_nDefIndex).c_str()).GetHashCode1();
				pWeapon->m_nSubclassID() = Hash;
				pWeapon->UpdateSubClass();
				pWeapon->GetWeaponVData()->m_szName() = pWeaponInLoadoutDefinition->GetSimpleWeaponName();
				pWeapon->UpdateVData();

				CGameSceneNode* pViewModelSceneNode = pViewModel->GetGameSceneNode();
				if (pViewModelSceneNode)
				{
					pWeaponSceneNode->SetMeshGroupMask(2);
					pViewModelSceneNode->SetMeshGroupMask(2);
				}
				pWeapon->SetModel(knifeModel);
				pViewModel->SetModel(knifeModel);
				pViewModel->pAnimationGraphInstance->pAnimGraphNetworkedVariables = nullptr;
			}
		}
		else
		{
			// Use legacy weapon models only for skins that require them.
			// Probably need to cache this if you really care that much about
			// performance
			//const char* model = pWeaponInLoadoutDefinition->GetModelName();
			//pWeapon->SetModel(model);
			//if (pViewModel && pViewModel->m_hWeapon() == hWeapon)
			//{
			//	pViewModel->SetModel(model);
			//}

			CEconItemSystem* EconItemSystem = I::Client->GetEconItemSystem();
			CEconItemSchema* pItemSchema = EconItemSystem->GetEconItemSchema();

			auto paint_kit = pItemSchema->GetPaintKits().FindByKey(pWeaponInLoadoutItemView->GetCustomPaintKitIndex());
			bool uses_old_model = paint_kit.has_value() && paint_kit.value()->UsesOldModel();

			pWeaponSceneNode->SetMeshGroupMask(1 + uses_old_model);
			if (pViewModel && pViewModel->m_hWeapon().GetEntryIndex() == hWeapon.GetEntryIndex())
			{
				CGameSceneNode* pViewModelSceneNode = pViewModel->GetGameSceneNode();

				pViewModelSceneNode->SetMeshGroupMask(1 + uses_old_model);
			}
		}
	}
}

void skin_changer::OnEquipItemInLoadout(int team, int slot, uint64_t itemID)
{
	auto it =
		std::find(g_vecAddedItemsIDs.begin(), g_vecAddedItemsIDs.end(), itemID);
	if (it == g_vecAddedItemsIDs.end())
		return;

	CCSInventoryManager* pInventoryManager = CCSInventoryManager::GetInstance();
	if (!pInventoryManager)
		return;

	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	C_EconItemView* pItemViewToEquip = pInventory->GetItemViewForItem(*it);
	if (!pItemViewToEquip)
		return;

	C_EconItemView* pItemInLoadout = pInventory->GetItemInLoadout(team, slot);
	if (!pItemInLoadout)
		return;

	CEconItemDefinition* pItemInLoadoutStaticData = pItemInLoadout->GetStaticData();
	if (!pItemInLoadoutStaticData)
		return;

	// Equip default item. If you would have bought Deagle and you previously
	// had R8 equipped it will now give you a Deagle.
	const uint64_t defaultItemID = (std::uint64_t(0xF) << 60) | pItemViewToEquip->GetItemDefinitionIndex();
	pInventoryManager->EquipItemInLoadout(team, slot, defaultItemID);

	CEconItem* pItemInLoadoutSOCData = pItemInLoadout->GetSOCData(nullptr);
	if (!pItemInLoadoutSOCData)
		return;

	CEconItemDefinition* toequipdata = pItemViewToEquip->GetStaticData();
	if (!toequipdata)
		return;

	if (toequipdata->IsWeapon() && !toequipdata->IsKnife(false) && !toequipdata->IsGlove(false))
	{
		pInventory->SOUpdated(pInventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
		return;
	}
	else if (toequipdata->IsGlove(false))
	{
		const uint64_t steamID = pInventory->GetOwner().m_id;

		CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();
		if (!pLocalPlayerController)
			return;

		C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
		if (!pLocalPawn)
			return;

		if (pLocalPawn->GetHealth() <= 0)
			return;

		CCSPlayer_ViewModelServices* pViewModelServices = pLocalPawn->GetViewModelServices();
		if (!pViewModelServices)
			return;

		C_BaseViewModel* pViewModel = I::GameResourceService->pGameEntitySystem->Get<C_BaseViewModel>(pViewModelServices->m_hViewModel());
		if (!pViewModel)
			return;

		// Create a struct to store glove information
		addedGloves.itemId = pItemViewToEquip->GetItemIDCS2();
		addedGloves.itemHighId = pItemViewToEquip->GetItemIDHigh();
		addedGloves.itemLowId = pItemViewToEquip->GetItemIDLow();
		addedGloves.itemDefId = pItemViewToEquip->GetItemDefinitionIndex();

		C_EconItemView* pGloves = &pLocalPawn->m_EconGloves();
		if (!pGloves)
			return;

		CEconItemDefinition* pGlovesDefinition = pGloves->GetStaticData();
		if (!pGlovesDefinition)
			return;
		pLocalPawn->m_bNeedToReApplyGloves() = true;
		pInventory->SOUpdated(pInventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
		return;
	}
	else if (toequipdata->IsKnife(false))
	{
		pInventory->SOUpdated(pInventory->GetOwner(), (CSharedObject*)pItemInLoadoutSOCData, eSOCacheEvent_Incremental);
		return;
	}
}

void skin_changer::OnSetModel(C_BaseModelEntity* pEntity, const char*& model)
{
	// When you're lagging you may see the default knife for one second and this
	// function fixes that.
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return;

	CCSPlayerController* pLocalPlayerController = CCSPlayerController::GetLocalPlayerController();
	if (!pLocalPlayerController)
		return;

	C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalPlayerController->GetPawnHandle());
	if (!pLocalPawn)
		return;

	if (pLocalPawn->GetHealth() <= 0)
		return;

	if (!pEntity)
		return;

	C_BaseViewModel* pViewModel = (C_BaseViewModel*)pEntity;
	if (!pViewModel)
		return;
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	const uint64_t steamID = pInventory->GetOwner().m_id;

	C_CSWeaponBase* pWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(pViewModel->m_hWeapon());
	if (!pWeapon || !pWeapon->IsBasePlayerWeapon())
		return;

	if (!pWeapon->IsWeapon() ||
		pWeapon->GetOriginalOwnerXuid() != steamID)
		return;

	if (pWeapon->GetWeaponVData()->GetWeaponType() == WEAPONTYPE_KNIFE == WEAPONTYPE_C4 == WEAPONTYPE_GRENADE)
		return;

	CAttributeManager* pAttributeContainer = pWeapon->GetAttributeManager();
	if (!pAttributeContainer)
		return;

	C_EconItemView* pWeaponItemView = &pAttributeContainer->m_Item();
	if (!pWeaponItemView)
		return;

	CEconItemDefinition* pWeaponDefinition = pWeaponItemView->GetStaticData();
	if (!pWeaponDefinition)
		return;

	C_EconItemView* pWeaponInLoadoutItemView = pInventory->GetItemInLoadout(
		pWeapon->m_iOriginalTeamNumber(), pWeaponDefinition->GetLoadoutSlot());
	if (!pWeaponInLoadoutItemView)
		return;

	// Check if skin is added by us.
	auto it = std::find(g_vecAddedItemsIDs.cbegin(), g_vecAddedItemsIDs.cend(),
		pWeaponInLoadoutItemView->GetItemIDCS2());
	if (it == g_vecAddedItemsIDs.cend())
		return;
	if (pWeapon->GetWeaponVData()->GetWeaponType() == 9 or pWeapon->GetWeaponVData()->GetWeaponType() == 7)
		return;
	CEconItemDefinition* pWeaponInLoadoutDefinition =
		pWeaponInLoadoutItemView->GetStaticData();

	if (!pWeaponInLoadoutDefinition) // ||
		return;

	model = pWeaponInLoadoutDefinition->GetModelName();
}

void skin_changer::AddEconItemToList(CEconItem* pItem)
{
	g_vecAddedItemsIDs.emplace_back(pItem->m_ulID);
}

void skin_changer::Shutdown()
{
	CCSPlayerInventory* pInventory = CCSPlayerInventory::GetInstance();
	if (!pInventory)
		return;

	for (uint64_t id : g_vecAddedItemsIDs)
	{
		pInventory->RemoveEconItem(pInventory->GetSOCDataForItem(id));
	}
}
