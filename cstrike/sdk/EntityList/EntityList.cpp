#include "EntityList.h"
#include "../../core/sdk.h"
#include "../../SDK/Entity.h"
#include <functional>
#include "..\..\SDK\Interfaces\igameresourceservice.h"
#include "..\..\SDK\Interfaces\cgameentitysystem.h"
#include <vector>
#include "../../Lagcomp.h"
#include "../../AimBot.h"
#include "../../Utils.h"
class c_entity_system;

void c_entity_system::add(CEntityInstance* entity, CBaseHandle handle)
{
	const char* className = entity->GetEntityClassName();
	entities[FNV1A::Hash(className)].emplace_back(entity);

/*	if (FNV1A::Hash(className) == FNV1A::HashConst("C_CSPlayerPawn"))
	{
		g_RageBot->pawns.push_back((C_CSPlayerPawn*)entity);

		if (Lagcomp::g_LagCompensation)
			Lagcomp::g_LagCompensation->AddEntity((C_CSPlayerPawn*)entity);
	}
	else */if (FNV1A::Hash(className) == FNV1A::HashConst("CCSPlayerController")) {
		// Call original before modifying our data to maintain proper game state

		auto* controller = static_cast<CCSPlayerController*>(entity);
		auto* pawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(controller->GetPawnHandle());

		// Use write_players to safely modify the cache
		EntityCache::g_entity_cache->write_players([&](auto& players) {
			players.emplace_back(EntityCache::cached_entity{ handle, controller, pawn });
			});
	}
}

void c_entity_system::init()
{
	L_PRINT(LOG_INFO) << ("[ENTITY LISTENER] cache entities");
	for (int i{ 1 }; i <= I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex(); ++i)
	{
		C_BaseEntity* ent = static_cast<C_BaseEntity*>(I::GameResourceService->pGameEntitySystem->Get(i));
		if (!ent)
			continue;
		add(ent, ent->GetRefEHandle());
	}
}

void c_entity_system::on_level_init()
{
	entities.clear();
	EntityCache::g_entity_cache->clear_players();
	SDK::LocalController = nullptr;
	SDK::LocalPawn = nullptr;
	init();
}

void c_entity_system::level_shutdown()
{
	entities.clear();
	EntityCache::g_entity_cache->clear_players();
}

void c_entity_system::remove(CEntityInstance* entity, CBaseHandle handle)
{
	const char* class_name = entity->GetEntityClassName();


	if (FNV1A::Hash(class_name) == FNV1A::HashConst("CCSPlayerController")) {
		// Use write_players to safely modify the cache
		EntityCache::g_entity_cache->write_players([&](auto& players) {
			auto it = std::ranges::find_if(players, [&](const EntityCache::cached_entity& e) {
				return e.handle == handle;
				});

			if (it != players.end()) {
				players.erase(it);
			}
			});
	}
	auto it = entities.find(FNV1A::Hash(class_name));
	if (it == entities.end())
		return;

	std::vector<CEntityInstance*>& entities = it->second;
	for (auto iter = entities.begin(); iter != entities.end(); ++iter)
	{
		if ((*iter) == entity)
		{
			entities.erase(iter);
			break;
		}
	}
}

void c_entity_system::ForceUpdateEntityList()
{
	// clear
	entities.clear();
	EntityCache::g_entity_cache->clear_players();

	for (int i = 0; i < I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex(); i++)
	{
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(i);
		if (!pEntity || !pEntity->GetIdentity() || !pEntity->GetIdentity()->pInstance || !pEntity->GetRefEHandle().IsValid())
			continue;

		add(pEntity->GetIdentity()->pInstance, pEntity->GetRefEHandle());
	}
}

std::vector<CEntityInstance*> c_entity_system::get(const char* name)
{
	return entities[FNV1A::Hash(name)];
}

std::vector<C_CSPlayerPawn*> c_entity_system::get_players(int index)
{
	std::vector<C_CSPlayerPawn*> enteties;

	C_CSPlayerPawn* local_player = SDK::LocalPawn;
	if (!local_player)
		return enteties;

	int local_team_num = local_player->GetTeam();

	std::vector<CEntityInstance*> entities = get("C_CSPlayerPawn");
	for (CEntityInstance* entity : entities)
	{
		C_CSPlayerPawn* player_pawn = reinterpret_cast<C_CSPlayerPawn*>(entity);

		int team_num = player_pawn->GetTeam();

		if (index == ENTITY_ALL || ((index == ENTITY_ENEMIES_ONLY && local_team_num != team_num) || (index == ENTITY_TEAMMATES_ONLY && local_team_num == team_num && local_player != player_pawn)))
			enteties.emplace_back(player_pawn);
	}

	return enteties;
}

std::vector<player_controller_pair> c_entity_system::get_players_with_controller(int index)
{
	std::vector<player_controller_pair> return_enteties;

	C_CSPlayerPawn* local_player = SDK::LocalPawn;
	if (!local_player)
		return return_enteties;

	int local_team_num = local_player->GetTeam();

	std::vector<CEntityInstance*> entities = get("CCSPlayerController");
	for (auto entity : entities)
	{
		CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(entity);
		if (!controller)
			continue;

		C_CSPlayerPawn* player_pawn = reinterpret_cast<C_CSPlayerPawn*>(I::GameResourceService->pGameEntitySystem->Get(controller->GetPawnHandle().GetEntryIndex()));
		if (!player_pawn)
			continue;

		int team_num = player_pawn->GetTeam();

		if (index == ENTITY_ALL || ((index == ENTITY_ENEMIES_ONLY && local_team_num != team_num) || (index == ENTITY_TEAMMATES_ONLY && local_team_num == team_num && local_player != player_pawn)))
			return_enteties.emplace_back(std::make_pair(controller, player_pawn));
	}

	return return_enteties;
}
