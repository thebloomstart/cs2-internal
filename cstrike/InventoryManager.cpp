#include "InventoryManager.h"

#include "../cstrike/utilities/memory.h"
#include "../cstrike/core/interfaces.h"
#include "../cstrike/core/hooks.h"

CCSInventoryManager* CCSInventoryManager::GetInstance()
{
	if (!MEM::fnGetInventoryManager)
		return nullptr;
	return MEM::fnGetInventoryManager();
}
