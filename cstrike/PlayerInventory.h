#pragma once
#include <utility>
#include "../cstrike/sdk/datatypes/utlvector.h"
#include "InventoryManager.h"
#include <cstdint>
#include "../cstrike/sdk/entity.h"
#include "../cstrike/utilities/memory.h"

class CEconItem;

struct SOID_t
{
	uint64_t m_id;
	uint32_t m_type;
	uint32_t m_padding;
};

class CSharedObject;
class C_EconItemView;

enum ESOCacheEvent
{
	/// Dummy sentinel value
	eSOCacheEvent_None = 0,

	/// We received a our first update from the GC and are subscribed
	eSOCacheEvent_Subscribed = 1,

	/// We lost connection to GC or GC notified us that we are no longer
	/// subscribed. Objects stay in the cache, but we no longer receive updates
	eSOCacheEvent_Unsubscribed = 2,

	/// We received a full update from the GC on a cache for which we were
	/// already subscribed. This can happen if connectivity is lost, and then
	/// restored before we realized it was lost.
	eSOCacheEvent_Resubscribed = 3,

	/// We received an incremental update from the GC about specific object(s)
	/// being added, updated, or removed from the cache
	eSOCacheEvent_Incremental = 4,

	/// A lister was added to the cache
	/// @see CGCClientSharedObjectCache::AddListener
	eSOCacheEvent_ListenerAdded = 5,

	/// A lister was removed from the cache
	/// @see CGCClientSharedObjectCache::RemoveListener
	eSOCacheEvent_ListenerRemoved = 6,
};

class CCSPlayerInventory
{
public:
	static CCSPlayerInventory* GetInstance();

	auto SOCreated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent)
	{
		return MEM::CallVFunc<void, 0u>(this, owner, pObject, eEvent);
	}

	auto SOUpdated(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent)
	{
		return MEM::CallVFunc<void, 1u>(this, owner, pObject, eEvent);
	}

	auto SODestroyed(SOID_t owner, CSharedObject* pObject, ESOCacheEvent eEvent)
	{
		return MEM::CallVFunc<void, 2u>(this, owner, pObject, eEvent);
	}

	auto GetItemInLoadout(int iClass, int iSlot)
	{
		return MEM::CallVFunc<C_EconItemView*, 8u>(this, iClass, iSlot);
	}

	bool AddEconItem(CEconItem* pItem);
	void RemoveEconItem(CEconItem* pItem);
	std::pair<uint64_t, uint32_t> GetHighestIDs();
	C_EconItemView* GetItemViewForItem(uint64_t itemID);
	CEconItem* GetSOCDataForItem(uint64_t itemID);

	auto GetOwner()
	{
		return *reinterpret_cast<SOID_t*>((uintptr_t)(this) + 0x10);
	}

	auto& GetItemVector()
	{
		return *reinterpret_cast<CUtlVector<C_EconItemView*>*>(
			(uintptr_t)(this) + 0x20);
	}
};
