	#include "Events.h"
#include "../../BulletTracer.h"
#include "../../InvecntoryChanger.h"
#include "../../AimBot.h"
#include "../../utilities/notify.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../HitMarker.h"
#include "../../KillEffects.h"
#include "../../AntiAim.h"

CCSPlayerController* CGameEventHelper::GetPlayerController()
{
	if (!Event)
		return nullptr;

	int controller_id{};

	CBuffer buffer;
	buffer.name = "userid";

	Event->GetControllerId(controller_id, &buffer);

	if (controller_id == -1)
		return nullptr;

	return (CCSPlayerController*)I::GameResourceService->pGameEntitySystem->Get(controller_id + 1);
}

CCSPlayerController* CGameEventHelper::GetAttackerController()
{
	if (!Event)
		return nullptr;

	int controller_id{};

	CBuffer buffer;
	buffer.name = "attacker";

	Event->GetControllerId(controller_id, &buffer);

	if (controller_id == -1)
		return nullptr;

	return (CCSPlayerController*)I::GameResourceService->pGameEntitySystem->Get(controller_id + 1);
}

int CGameEventHelper::GetDamage()
{
	if (!Event)
		return -1;

	return Event->GetInt2("dmg_health", false);
}

//int CGameEventHelper::GetHealth()
//{
//	if (!Event)
//		return -1;
//
//	CBuffer buffer;
//	buffer.name = "health";
//
//	return Event->GetInt(&buffer);
//}

int CGameEvent::GetInt2(const char* Name, bool Unk)
{
	using GetEventInt_t = int(__fastcall*)(void*, const char*, bool);
	static GetEventInt_t GetEventInt = reinterpret_cast<GetEventInt_t>(MEM::FindPattern(CLIENT_DLL, "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 30 48 8B 01 41 8B F0 4C 8B F1 41 B0 01 48 8D 4C 24 20 48 8B DA 48 8B 78"));

	return GetEventInt(this, Name, Unk);
}

float CGameEvent::GetFloat2(const char* Name, bool Unk)
{
	using GetEventFloat_t = float(__fastcall*)(void*, const char*, bool);
	static GetEventFloat_t GetEventFloat = reinterpret_cast<GetEventFloat_t>(MEM::FindPattern(CLIENT_DLL, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B 01 48 8B F1 0F 29 74 24 30 48 8D 4C 24 20 0F 28 F2 48 8B DA 48 8B 78"));

	return GetEventFloat(this, Name, Unk);
}

bool CEvents::Intilization()
{
	I::GameEventManager->AddListener(this, "player_hurt", false);
	if (!I::GameEventManager->FindListener(this, "player_hurt"))
		return false;

	I::GameEventManager->AddListener(this, "player_death", false);
	if (!I::GameEventManager->FindListener(this, "player_death"))
		return false;

	//I::GameEventManager->AddListener(this, "add_bullet_hit_marker", false);
	//if (!I::GameEventManager->FindListener(this, "add_bullet_hit_marker"))
	//	return false;

	I::GameEventManager->AddListener(this, "bullet_impact", false);
	if (!I::GameEventManager->FindListener(this, "bullet_impact"))
		return false;

	I::GameEventManager->AddListener(this, "round_start", false);
	if (!I::GameEventManager->FindListener(this, "round_start"))
		return false;

	I::GameEventManager->AddListener(this, "round_freeze_end", false);
	if (!I::GameEventManager->FindListener(this, "round_freeze_end"))
		return false;


	I::GameEventManager->AddListener(this, "round_announce_warmup", false);
	if (!I::GameEventManager->FindListener(this, "round_announce_warmup"))
		return false;


	I::GameEventManager->AddListener(this, "round_officially_ended", false);
	if (!I::GameEventManager->FindListener(this, "round_officially_ended"))
		return false;

	I::GameEventManager->AddListener(this, "round_end", false);
	if (!I::GameEventManager->FindListener(this, "round_end"))
		return false;


	//I::GameEventManager->AddListener(this, "weapon_fire", false);
	//if (!I::GameEventManager->FindListener(this, "weapon_fire"))
	//	return false;

	return true;
}

void CEvents::FireGameEvent(CGameEvent* event)
{
	std::string name = event->GetName();

	if (name.find("bullet_impact") != std::string::npos)
		OnBulletImpact(event);

	if (name.find("round_start") != std::string::npos)
	{
		//I::Engine->client_cmd_unrestricted("buy ssg08");
		force_update = true;
	}
	//cheat->mShouldClearNotice = true;

	if (name.find("player_death") != std::string::npos)
		PlayerDeath(event);

	if (name.find("round_freeze_end") != std::string::npos)
		g_AntiAim->bFreezTime = false;

	if (name.find("round_announce_warmup") != std::string::npos)
		g_AntiAim->bFreezTime = false;

	if (name.find("round_end") != std::string::npos)
	{
		g_AntiAim->bFreezTime = false;
		force_update = true;
	}

	if (name.find("round_officially_ended") != std::string::npos)
		g_AntiAim->bFreezTime = true;

	if (name.find("player_hurt") != std::string::npos)
		OnPlayerHurt(event);

	if (name.find("localplayer_death") != std::string::npos)
	{
		LocalPlayerDeath(event);
		force_update = true;
	}
}

void CEvents::PlayerDeath(CGameEvent* event)
{
	auto player = event->GetEventHelper().GetPlayerController();
	auto attacker = event->GetEventHelper().GetAttackerController();

	if (!player || !attacker)
		return;

	if (!player->IsBasePlayerController() || !attacker->IsBasePlayerController())
		return;
	//if (player == SDK::LocalController->GetLocalPlayerController())
	//	cheat->mShouldClearNotice = true;

	//if (!attacker->is_local_player_controller())
	//    return;

	//auto damage = event->get_event_helper().get_damage();

	//if (damage < 1)
	//    return;

	//auto hitbox = event->get_int2(xorstr_("hitgroup"), false);

	//if (g_user->IsActive(xorstr_("misc_zues_kill_effect"), xorstr_("misc_blur"), 2))
	//{
	//    g_cs2->killed_target = player->get_player_pawn();
	//    g_cs2->target_killed = true;
	//}

	//if (g_user->IsActive(xorstr_("misc_hitlogs"), xorstr_("misc_hitlogs"), 0))
	//{
	//    std::string logMessage = tfm::format(xorstr_("Killed %s in the %s for %d"), player->name(), get_hitgroup(hitbox), damage);
	//    g_misc->add_notify(logMessage, xorstr_(""), 7.f);
	//}
}

void CEvents::LocalPlayerDeath(CGameEvent* event)
{
	auto player = event->GetEventHelper().GetPlayerController();
	auto attacker = event->GetEventHelper().GetAttackerController();

	if (!player || !attacker)
		return;

	auto pawn = SDK::LocalPawn;
	if (!pawn)
		return;

	if (!player->get_player_pawn() || !attacker->get_player_pawn())
		return;
}

void PlaySound(const char* nameSound)
{
	using function_t = void* (__fastcall*)(const char*); //xref: "C:\\buildworker\\csgo_rel_win64\\build\\src\\soundsystem\\snd_dma.cpp",
	static const function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(SOUND_DLL, CS_XOR("48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 55 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 8B")));
	CS_ASSERT(fn != nullptr);
	fn(nameSound);
}
int dmg = 0;

void CEvents::OnPlayerHurt(CGameEvent* event)
{
	CGameEventHelper EventHelper = event->GetEventHelper();

	auto pPlayer = EventHelper.GetPlayerController();
	auto attacker = EventHelper.GetAttackerController();


	if (!pPlayer || !attacker)
		return;
	auto pawn = SDK::LocalPawn;
	if (!pawn)
		return;

	auto damage = EventHelper.GetDamage();
	if (damage < 1)
		return;

	auto hitbox = event->GetInt2(CS_XOR("hitgroup"), false);
	dmg = event->GetInt2(CS_XOR("dmg_health"), false);

	g_Effect->ShokedEffect(pPlayer->get_player_pawn());

	if (C_GET(bool, Vars.bHealthBoost) || !C_GET(bool,Vars.bNoScope))
	{
		SDK::LocalPawn;
		if (I::GlobalVars)
			pawn->GetHealthShotBoostExpirationTime() = I::GlobalVars->flCurTime + 1;
	}

	if (C_GET(bool, Vars.bHitSound))
	{
		if (!pPlayer->get_player_pawn()->IsAlive())
			PlaySound("sounds/ui/panorama/mainmenu_press_shop_01.vsnd");///sounds/ui/coin_pickup_01.vsnd
		//else 
			//PlaySound("/sounds/physics/wood/wood_plank_impact_hard4.vsnd_c");
	}
	//on_shot_capsule(player->get_player_pawn());

	//C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
	//if (pPawn == nullptr)
	//	return;

	//CGameSceneNode* game_scene = pPawn->GetGameSceneNode();
	//if (!game_scene)
	//	return;

	//CSkeletonInstance* skeleton = game_scene->GetSkeletonInstance();

	//auto model_state = &skeleton->m_modelState();
	//CStrongHandle<CModel> model = model_state->m_hModel();
	//auto model_skelet = &model->m_modelSkeleton();

	//if (!model_skelet)
	//	return;

	//skeleton->calc_world_space_bones(0, bone_flags::FLAG_HITBOX);
	//auto bones = model_state->m_pBones;

	//for (int i{}; i < 19; i++)
	//{
	//	//auto hitbox_data = model->get_hitbox(i);

	//	if (!bones)
	//		continue;

	//	//if (hitbox_data->m_nGroupId == hitbox)
	//	{
	//		auto bone_index = player->get_bone_index(model_state->GetHitboxName(i));
	//		g_hitmarker->AddDamage(std::to_string(dmg), skeleton->m_modelState().m_pBones[hitbox].vecPosition);
	//		g_hitmarker->add(skeleton->m_modelState().m_pBones[hitbox].vecPosition, SDK::LocalPawn->GetEyePosition()); //bone_index
	//	}
	//}
}

void CEvents::OnBulletImpact(CGameEvent* event)
{
	if (!SDK::Alive)
		return;

	std::string_view token_name_x = CS_XOR("x");
	cUltStringToken tokenx(token_name_x.data());

	std::string_view token_name_y = CS_XOR("y");
	cUltStringToken tokeny(token_name_y.data());

	std::string_view token_name_z = CS_XOR("z");
	cUltStringToken tokenz(token_name_z.data());

	auto player = event->GetEventHelper().GetPlayerController();
	auto position = Vector_t(event->GetFloatNew(tokenx), event->GetFloatNew(tokeny), event->GetFloatNew(tokenz));
	C_CSPlayerPawn* pLocalPawn = SDK::LocalPawn;
	if (!pLocalPawn)
		return;
	CGameEventHelper EventHelper = event->GetEventHelper();

	auto pPlayer = EventHelper.GetPlayerController();

	if (!pPlayer)
		return;
	if (pPlayer->get_player_pawn() != SDK::LocalPawn)
		return;

	if (C_GET(bool,Vars.bBulletTrace))
	    g_BulletTrace->add_bullet_trace(SDK::LocalPawn->GetEyePosition(), position, C_GET(Color_t, Vars.colBulletTracer));

	//g_BulletTrace->AddHit(position, SDK::LocalPawn->GetEyePosition());
	if (dmg >= 1)
	    g_hitmarker->HitMarker(SDK::LocalPawn->GetEyePosition(), position, dmg);

	//if (C_GET(unsigned int, Vars.bBulletImpacts) & SERVER)
	//{
	//	auto debugOverlay = I::Client->GetSceneDebugOverlay();
	//	debugOverlay->addBox(position, Vector_t(-2, -2, -2), Vector_t(2, 2, 2), Vector_t(), Color_t(0, 0, 255, 127));
	//}

	//if (C_GET(unsigned int, Vars.bBulletImpacts) & LOCAL)
	//	F::VISUALS::WORLD::BulletImpact(pLocalPawn);

	//F::VISUALS::WORLD::ProcessTracers(pLocalPawn->GetEyePosition(), position);
}
