#pragma once
#include <cstdint>
#include "windows.h"

class IGameEvent;
class CEconItem;
class C_BaseModelEntity;
class CCSPlayerInventory;
class C_CSPlayerPawn;
class CEconItemDefinition;
class C_BaseViewModel;
class C_EconItemView;

inline bool force_update = false;

namespace skin_changer
{

	enum material_magic_number : uint32_t
	{
		material_magic_number__gloves = 0xf143b82a,
		material_magic_number__unknown1 = 0x1b52829c,
		material_magic_number__unknown2 = 0xa6ebe9b9,
		material_magic_number__unknown3 = 0x423b2ed4,
		material_magic_number__unknown4 = 0xc8d7255e
	};

	struct material_record
	{
		uint32_t ui32_unknwonstatic;
		material_magic_number identifer;
		uint32_t ui32_handle;
		uint32_t ui32_type_index;
	};

	inline std::uint64_t hash_agent;

	void OnFrameStageNotify(int frameStage);
	void OnPreFireEvent(IGameEvent* pEvent);
	void OnRoundReset(IGameEvent* pEvent);

	void OnEquipItemInLoadout(int team, int slot, uint64_t itemID);
	void OnSetModel(C_BaseModelEntity* pEntity, const char*& model);

	void AddEconItemToList(CEconItem* pItem);
	void Shutdown();
	inline bool dumpitems;
} // namespace skin_changer
