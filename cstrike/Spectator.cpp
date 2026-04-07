#include "core/sdk.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/interfaces/igameresourceservice.h"
#include "sdk/interfaces/iengineclient.h"
#include "sdk/EntityList/EntityList.h"
#include "Spectator.h"

void Spec::spectators()
{
	if (!SDK::LocalPawn)
		return;

	auto local_player = SDK::LocalPawn;

	if (local_player && local_player->IsAlive() && I::Engine->IsConnected() && I::Engine->IsInGame())
	{
		auto entitass = g_EntityList->get(CS_XOR("CCSPlayerController"));

		for (auto entity : entitass)
		{
			CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(entity);
			if (!controller)
				continue;

			if (!controller->find_class("CCSPlayerController"))
				continue;

			auto pawn = controller->get_player_pawn();
			if (!pawn || pawn->IsAlive() || !pawn->IsPlayer())
				continue;

			if (pawn->GetFlags() & 0x100)
				continue;

			auto obs_pawn = controller->get_observer_pawn();
			if (!obs_pawn || !obs_pawn->find_class("C_CSObserverPawn"))
				continue;

			auto obs_service = obs_pawn->GetObserverServices();
			if (!obs_service)
				return;
			if (obs_service->GetObserverMode() != ObserverMode_t::OBS_MODE_IN_EYE)
				return;

			auto name = std::string(controller->GetPlayerName(), 16);
			std::string Spec = name + std::string(" [First Person]");
			ImGui::Text(Spec.c_str());
		}
	}
}
