#include <unordered_map>
#include <vector>
#include "../../SDK/Entity.h"

#define ENTITY_ALL 1
#define ENTITY_ENEMIES_ONLY 2
#define ENTITY_TEAMMATES_ONLY 3

using player_controller_pair = std::pair<CCSPlayerController*, C_CSPlayerPawn*>;

class c_entity_system
{
public:
	void init();
	void on_level_init();
	void level_shutdown();
	void add(CEntityInstance* entity, CBaseHandle handle);
	void remove(CEntityInstance* entity, CBaseHandle handle);
	void ForceUpdateEntityList();
	std::vector<CEntityInstance*> get(const char* name);
	std::vector<C_CSPlayerPawn*> get_players(int);
	std::vector<player_controller_pair> get_players_with_controller(int);	
	std::unordered_map<uint64_t, std::vector<CEntityInstance*>> entities;
};

inline const std::unique_ptr<c_entity_system> g_EntityList{ new c_entity_system() };
