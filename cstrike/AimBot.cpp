#include "AimBot.h"
#include "sdk/interfaces/ccsgoinput.h"
#include "Pena.h"
#include "sdk/EntityList/EntityList.h"
#include "sdk/interfaces/ccsgoinput.h"
#include "features/misc/movement.h"
#include "core/menu.h"
#include "Utilities/inputsystem.h"
#include "core/hooks.h"
#include <algorithm>
#include <mutex>
#include "Lagcomp.h"
#include "Utils.h"

Vector_t NoSpreadCalculate_spread(unsigned int index, int bullets, float accuracy, float spread, unsigned short item_index, int recoil_index)
{
	Vector_t spread_;

	MATH::fnRandomSeed(index + 1);

	float r1 = MATH::fnRandomFloat(0.0f, 1.0f);
	float r2 = MATH::fnRandomFloat(0.0f, 6.2831855f);

	float r3{};
	float r4{};
	float c1{};
	float c2{};
	float s1{};
	float s2{};

	//if (/*g_cs2->weapon_accuracy_shotgun_spread_patterns->GetValue<bool>()*/true)
	{
		//	get_spread_pattern(item_index, bullets, recoil_index, &r3, &r4);
	}
	//else
	{
		r3 = r1;
		r4 = r2;
	}

	if (item_index == WEAPON_R8_REVOLVER)
	{
		r1 = 1.f - (r1 * r1);
		r3 = 1.f - (r3 * r3);
	}
	else if (item_index == WEAPON_NEGEV && recoil_index < 3)
	{
		for (int i{ 3 }; i > recoil_index; --i)
		{
			r1 *= r1;
			r3 *= r3;
		}

		r1 = 1.f - r1;
		r3 = 1.f - r3;
	}

	// get needed sine / cosine values.
	c1 = std::cosf(r2);
	s1 = std::sinf(r4);
	c2 = std::cosf(r2);
	s2 = std::sinf(r4);

	float acc = r1 * accuracy;
	float sp = r3 * spread;

	return {
		(c1 * acc) + (c2 * sp),
		(s1 * acc) + (s2 * sp),
		0.f
	};
}

__forceinline Vector_t CalculateSpread(C_CSWeaponBase* weapon, int seed, float inaccuracy, float spread, bool revolver2 = false)
{
	const char* item_def_index;
	float recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

	if (!weapon)
		return {};
	// if we have no bullets, we have no spread.
	auto wep_info = weapon->GetWeaponVData();
	if (!wep_info)
		return {};

	// get some data for later.
	item_def_index = wep_info->m_szName();
	recoil_index = weapon->m_iRecoilIndex();

	MATH::fnRandomSeed((seed & 0xff) + 1);

	// generate needed floats.
	r1 = MATH::fnRandomFloat(0.f, 1.f);
	r2 = MATH::fnRandomFloat(0.f, 3.14159265358979323846264338327950288f * 2);
	r3 = MATH::fnRandomFloat(0.f, 1.f);
	r4 = MATH::fnRandomFloat(0.f, 3.14159265358979323846264338327950288f * 2);

	// revolver secondary spread.
	if (item_def_index == CS_XOR("weapon_revoler") && revolver2)
	{
		r1 = 1.f - (r1 * r1);
		r3 = 1.f - (r3 * r3);
	}

	// negev spread.
	else if (item_def_index == CS_XOR("weapon_negev") && recoil_index < 3.f)
	{
		for (int i{ 3 }; i > recoil_index; --i)
		{
			r1 *= r1;
			r3 *= r3;
		}

		r1 = 1.f - r1;
		r3 = 1.f - r3;
	}

	// get needed sine / cosine values.
	c1 = std::cos(r2);
	s1 = std::sin(r4);
	c2 = std::cos(r2);
	s2 = std::sin(r4);

	// calculate spread vector.
	return {
		(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
		(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
		0.f
	};
}

unsigned int get_hash_seed(C_CSPlayerPawn* player, QAngle_t angle, int attack_tick)
{
	return MEM::GetHashSeed(player, angle, attack_tick);
}

int RageBot::HitChanceNS(QAngle_t& best_point, Vector_t start, C_CSPlayerPawn* player, C_CSWeaponBase* weapon, C_CSPlayerPawn* local)
{
	auto weapon_data = weapon->GetWeaponVData();
	if (!weapon_data)
		return false;

	weapon = local->GetWeaponActive();

	auto recoil_index = weapon->m_iRecoilIndex();

	Vector_t dir{};
	Vector_t end{};
	Vector_t fwd{};
	Vector_t right{};
	Vector_t up{};

	int hits{};
	int aw_hits{};
	float range = weapon_data->m_flRange();
	weapon->update_accuracy();
	auto accuracy = weapon->get_inaccuracyHitChance();
	auto spread = weapon->get_spread();

	auto EconItemView = weapon->GetEconItemView();
	auto weapon_index = EconItemView->GetItemDefinitionIndex();
	g_Utils->angle_vectors(best_point, &fwd, &right, &up);

	//if (weapon_is_at_max_accuracy(weapon_data, accuracy))
	//	return 100;

	TraceFilter_t filter(0x1C3003, local, nullptr, 4);

	Ray_t ray{};
	game_trace_t trace{};

	for (int i = 0; i < 256; i++)
	{
		//auto spread_calc = CalculateSpread(weapon, i, accuracy, spread);
		auto spread_calc = NoSpreadCalculate_spread(get_hash_seed(local, best_point, SDK::prediction_tick + 1), weapon->GetWeaponVData()->m_nNumBullets(), accuracy, spread, weapon_index, recoil_index);

		// get spread direction.
		dir = Vector_t(fwd + (right * spread_calc.x) + (up * spread_calc.y));

		end = local->GetEyePosition() + dir * range;

		I::GameTraceManager->ClipRayToEntity(&ray, local->GetEyePosition(), end, player, &filter, &trace);

		if (trace.HitEntity == player)
			hits++;
	}

	return static_cast<int>((static_cast<float>(hits / 256)) * 100.f);
}

float RageBot::ScaleDamage(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, Vector_t start, Vector_t aim_point, float& dmg, bool& canHit)
{
	if (!pLocal || !weapon || !target)
		return 0.f;

	if (pLocal->GetHealth() <= 0)
		return 0.f;

	auto vdata = weapon->GetWeaponVData();
	if (!vdata)
		return 0.f;

	auto entity = I::GameResourceService->pGameEntitySystem->Get(target->GetRefEHandle());
	if (!entity)
		return 0.f;

	auto localent = I::GameResourceService->pGameEntitySystem->Get(pLocal->GetRefEHandle());
	if (!localent)
		return 0.f;

	float damage = 0.f;
	F::AUTOWALL::c_auto_wall::data_t data;
	F::AUTOWALL::g_auto_wall->pen(data, start, aim_point, entity, localent, pLocal, target, vdata, damage, canHit);
	return data.m_can_hit ? data.m_dmg : 0.f;
}

bool RageBot::can_shoot()
{
	auto pBaseWeapon = SDK::LocalPawn->GetWeaponActive();
	auto pPawn = SDK::LocalPawn;
	auto pController = SDK::LocalController;
	const float flServerTime = TICKS_TO_TIME(pController->GetTickBase());
	if (!pPawn)
		return false;

	CCSPlayer_WeaponServices* pPlayerWeaponServices = static_cast<CCSPlayer_WeaponServices*>(pPawn->GetWeaponServices());
	if (!pPlayerWeaponServices)
		return false;

	// check is have ammo
	if (pBaseWeapon->GetClip1() <= 0)
		return false;

	// is player ready to shoot
	if (pPlayerWeaponServices->GetNextAttack() > flServerTime)
		return false;

	const short nDefinitionIndex = pBaseWeapon->GetAttributeManager()->GetItem()->GetItemDefinitionIndex();

	// is weapon ready to shoot
	if (pBaseWeapon->GetNextPrimaryAttackTick() > static_cast<int>(pController->GetTickBase()))
		return false;

	// check for revolver cocking ready
	if (nDefinitionIndex == WEAPON_R8_REVOLVER && pBaseWeapon->m_nPostponeFireReadyTicks() > flServerTime)
		return false;

	return true;
}

bool RageBot::weapon_is_at_max_accuracy(CCSWeaponBaseVData* weapon_data, float inaccuracy)
{

	constexpr auto round_accuracy = [](float accuracy)
		{
			return floorf(accuracy * 170.f) / 170.f;
		};
	constexpr auto round_duck_accuracy = [](float accuracy)
		{
			return floorf(accuracy * 300.f) / 300.f;
		};

	float speed = SDK::LocalPawn->GetAbsVelocity().Length();

	bool is_scoped = ((weapon_data->GetWeaponType() == WEAPONTYPE_SNIPER_RIFLE) && !(SDK::Cmd->nButtons.nValue & IN_ZOOM) && !SDK::LocalPawn->IsScoped());

	bool is_ducking = ((SDK::LocalPawn->GetFlags() & FL_DUCKING) || (SDK::Cmd->nButtons.nValue & IN_DUCK));

	float rounded_accuracy = round_accuracy(inaccuracy);
	float rounded_duck_accuracy = round_duck_accuracy(inaccuracy);

	if (is_ducking && is_scoped && (SDK::LocalPawn->GetFlags() & FL_ONGROUND) && (rounded_duck_accuracy < SDK::WepAccuracy))
		return true;

	if (speed <= 0 && is_scoped && (SDK::LocalPawn->GetFlags() & FL_ONGROUND) && (rounded_accuracy < SDK::WepAccuracy))
		return true;

	return false;
}



bool RageBot::hitchancev2(QAngle_t& pos, C_CSWeaponBase* weapon, C_CSPlayerPawn* Target)
{

	if (!weapon)
		return false;

	auto weapon_data = weapon->GetWeaponVData();

	if (!weapon_data)
		return false;

	auto controller = CCSPlayerController::GetLocalPlayerController();
	if (!controller)
		return 0;

	auto pawn = SDK::LocalPawn;
	if (!pawn)
		return 0;

	Vector_t dir{};
	Vector_t end{};
	Vector_t fwd{};
	Vector_t right{};
	Vector_t up{};

	int hits{};
	int aw_hits{};
	float range = weapon_data->m_flRange();

	weapon->update_accuracy();
	float inaccuracy = weapon->get_inaccuracyHitChance();
	float spread = weapon->get_spread();

	auto needed_hits{ (size_t)std::ceil((C_GET(int, Vars.rage_minimum_hitchance) * 255) / 100.f) };
	g_Utils->angle_vectors(pos, &fwd, &right, &up);
	//if (g_RageBot->weapon_is_at_max_accuracy(weapon_data, inaccuracy))
	//	return true;
	float damage = 0;
	TraceFilter_t filter = {};
	I::GameTraceManager->Init(filter, pawn, 0x1C3003, 3, 7);
	Vector_t eye = pawn->GetEyePosition();
	for (int i = 0; i < 255; i++)
	{
		auto spread_calc = CalculateSpread(weapon, i, inaccuracy, spread);

		// get spread direction.
		dir = Vector_t(fwd - (right * spread_calc.x) + (up * spread_calc.y));

		end = eye + dir * range;
		float max;
		float lenght;
		float min;
		Ray_t ray{};
		game_trace_t trace{};
		I::GameTraceManager->ClipTraceToPlayers(eye, end, &filter, &trace, 0.F, 60.F, (1.F / (eye - end).Length()) * (trace.m_end_pos - eye).Length()); //->clip_ray_to_entity(&ray, pawn->m_vecLastClipCameraPos(), end, Target, &filter, &trace);

		if (trace.HitEntity == Target)
			hits++;
	}
	return hits >= needed_hits;
}


C_CSPlayerPawn* FindBestPlayer(const std::vector<C_CSPlayerPawn*>& players) {
	std::vector<PlayerInfo> playerInfos;

	if (!SDK::LocalPawn)
		return nullptr;
	if (players.empty())
		return nullptr;

	for (auto player : players) {
		if (player && player->IsOtherEnemy(SDK::LocalPawn) && player->GetHealth() > 0) {
			playerInfos.emplace_back(player);
		}
	}

	for (auto& info : playerInfos) {
		info.Update();
	}

	if (playerInfos.empty())
		return nullptr;

	std::sort(playerInfos.begin(), playerInfos.end(), [](const PlayerInfo& a, const PlayerInfo& b) {
		if (a.distance != b.distance) return a.distance < b.distance;
		if (a.health != b.health) return a.health > b.health;
		return a.velocity > b.velocity;
		});

	return playerInfos.front().player;
}

std::mutex damageMutex;
std::pair<Vector_t, float> RageBot::FindBestHitbox(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon) {
	std::vector<float> damages;
	std::vector<Vector_t> hitboxPositions;

	for (BONEINDEX bone : BoneIndexMap) {
		Vector_t aim_point = target->GetGameSceneNode()->GetSkeletonInstance()->GetBonePosition(bone);

		float damage = 0.f;
		bool canHit = false;
		std::lock_guard<std::mutex> lock(damageMutex);
		damage = ScaleDamage(target, pLocal, weapon, pLocal->GetEyePosition(), aim_point, damage, canHit);

		damages.push_back(damage);
		hitboxPositions.push_back(aim_point);
	}

	auto maxDamageIt = std::max_element(damages.begin(), damages.end());
	if (maxDamageIt != damages.end()) {

		size_t index = std::distance(damages.begin(), maxDamageIt);
		float maxDamage = *maxDamageIt;
		Vector_t bestHitboxPosition = hitboxPositions[index];
		return std::make_pair(bestHitboxPosition, maxDamage);
	}
	return std::make_pair(Vector_t{}, 0.f);
}

void autoRevolver(CUserCmd* cmd) {
	static float revolverPrepareTime = 0.1f;

	static float readyTime;
	if (!SDK::LocalPawn || true)
		return;

	const auto activeWeapon = SDK::LocalPawn->GetWeaponActive();
	if (activeWeapon && activeWeapon->GetAttributeManager() && activeWeapon->GetAttributeManager()->GetItem() && activeWeapon->GetAttributeManager()->GetItem()->GetItemDefinitionIndex() == WEAPON_R8_REVOLVER) {
		if (!readyTime)
			readyTime = I::GlobalVars->flCurTime + revolverPrepareTime;
		const auto ticksToReady = TIME_TO_TICKS(readyTime - I::GlobalVars->flCurTime - 0.8/*- interfaces::Engine->GetNetChannelInfo(0)->GetLatency(FLOW_OUTGOING)*/);
		if (ticksToReady > 0 && ticksToReady <= TIME_TO_TICKS(revolverPrepareTime))
			cmd->nButtons.nValue |= IN_ATTACK;
		else
			readyTime = 0.0f;
	}
}

void RageBot::RageBotAim(CUserCmd* pCmd, CCSGOInput* pInput)
{
	if (!C_GET(bool, Vars.rage_enable))
		return;
	if (!SDK::Alive)
		return;
	C_CSPlayerPawn* pLocalPawn = SDK::LocalPawn;
	players.clear();

	auto entitass = g_EntityList->get(CS_XOR("CCSPlayerController"));
	for (auto entity : entitass) {
		CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(entity);
		if (!controller) continue;
		C_CSPlayerPawn* player_pawn = reinterpret_cast<C_CSPlayerPawn*>(I::GameResourceService->pGameEntitySystem->Get(controller->GetPawnHandle().GetEntryIndex()));
		if (!player_pawn) continue;
		if (player_pawn) {
			players.push_back(player_pawn);
		}
	}
	if (players.empty())
		return;

	Vector_t ShootPos = pLocalPawn->GetEyePosition();
	C_CSWeaponBase* active_weapon = pLocalPawn->GetWeaponActive();

	bestPlayer = FindBestPlayer(players);
	if (!bestPlayer)
		return;
	auto lagrecord = Lagcomp::g_LagCompensation->GetLagEntity(bestPlayer);
	BestHitbox = FindBestHitbox(bestPlayer, pLocalPawn, active_weapon);

	autoRevolver(pCmd);
	//for (size_t entityIndex = 0; entityIndex < Lagcomp::g_LagCompensation->m_arrEntities.size(); entityIndex++)
	{
		/*Lagcomp::LagEntity sEntity = Lagcomp::g_LagCompensation->m_arrEntities[entityIndex];
		if (!sEntity.m_pEntity)
			continue;
		if (!sEntity.m_pEntity->IsOtherEnemy(SDK::LocalPawn) || sEntity.m_arrLagRecords.size() == 0)
			continue;*/

		if (BestHitbox.second >= C_GET(int, Vars.rage_minimum_damage))
		{
			int nTick = C_GET(int, Vars.iMaxBackTrackTicks);

			/*if (nTick >= sEntity.m_arrLagRecords.size())
				nTick = sEntity.m_arrLagRecords.size() - 1;*/

			//Vector_t BestVector = sEntity.m_arrLagRecords[nTick].m_boneData[6].vecPosition;// BestHitbox.first;
			D::WorldToScreen(BestHitbox.first, &v);

			QAngle_t pos = g_Utils->CalcAngles(ShootPos, BestHitbox.first);
			pos -= get_removed_aim_punch_angle(pLocalPawn);

			QAngle_t nig = { pos.x,pos.y,pos.z };
			if (C_GET(bool, Vars.bSilent))
				pCmd->SetSubTickAngle(nig);
			else
				pInput->SetViewAngle(nig);

			//int nTickCount = TIME_TO_TICKS(bestPlayer->GetSimulationTime()) + 1;
			//for (size_t i = 0; i < SDK::Cmd->csgoUserCmd.inputHistoryField.nCurrentSize; i++)
			//{
			//	CCSGOInputHistoryEntryPB* iHistory = pCmd->csgoUserCmd.inputHistoryField.pRep->tElements[i];
			//	if (!iHistory) continue;

			//	iHistory->setRenderTickCount(nTickCount + 1);
			//	iHistory->setPlayerTickCount(SDK::LocalController->GetTickBase());

			//	//if (!iHistory->cl_interp)
			//	//	iHistory->cl_interp = pCmd->AddInputHistory()->cl_interp;

			//	//iHistory->setClInterp(nTickCount + 1, nTickCount, 0.999f);

			//	/*if (!iHistory->sv_interp0)
			//		iHistory->sv_interp0 = pCmd->AddInputHistory()->sv_interp0;*/

			//	//iHistory->setSVInterp0(nTickCount + 1, nTickCount, 0.999f);

			//	//if (!iHistory->sv_interp1)
			//	//	iHistory->sv_interp1 = pCmd->AddInputHistory()->sv_interp1;

			//	//iHistory->setSVInterp1(nTickCount + 1, nTickCount, 0.999f);

			//	//if (!iHistory->player_interp)
			//	//	iHistory->player_interp = pCmd->AddInputHistory()->player_interp;

			////	iHistory->setPlayerInterp(nTickCount + 1, nTickCount, 0.999f);

			//	//if (!iHistory->pViewAngles)
			//	//	iHistory->pViewAngles = pCmd->AddInputHistory()->pViewAngles;

			//	iHistory->SetViewAngles(nig);

			//	/*if (!iHistory->pShootPosition)
			//		iHistory->pShootPosition = pCmd->AddInputHistory()->pShootPosition;*/
			//	iHistory->SetShootPos(Vector4D_t{ SDK::LocalPawn->GetEyePosition().x, SDK::LocalPawn->GetEyePosition().y, SDK::LocalPawn->GetEyePosition().z });

			//	iHistory->nTargetEntIndex = bestPlayer->GetRefEHandle().GetEntryIndex();
			//	iHistory->SetBits(INPUT_HISTORY_BITS_TARGETENTINDEX);
			//}

		//	auto s = sEntity.m_arrLagRecords[nTick].Apply(bestPlayer);

			if (IPT::IsPressed(C_GET(int, Vars.bKeyNoSpread), C_GET(int, Vars.bKeyBindNoSpread)))
			{

				if (HitChanceNS(pos, ShootPos, bestPlayer, active_weapon, pLocalPawn) < C_GET(int, Vars.rage_minimum_hitchance))
					return;
			}
			else if (!hitchancev2(nig, active_weapon, bestPlayer))
				return;
			//sEntity.m_arrLagRecords[nTick].Restore(bestPlayer, s);
			//if (sEntity) 
			/*{
				nTickCount = TIME_TO_TICKS(sEntity.m_arrLagRecords[nTick].GetSimulationTime) + 1;
				L_PRINT(LOG_INFO) << "Backtracking to " << nTickCount << " (delta: " << abs((TIME_TO_TICKS(bestPlayer->GetSimulationTime()) + 1) - (TIME_TO_TICKS(bestPlayer->GetSimulationTime()) + 1)) << " ticks)";
			}

			for (int i = 0; i < pCmd->csgoUserCmd.inputHistoryField.nCurrentSize; i++)
			{
				CCSGOInputHistoryEntryPB* iHistory = pCmd->GetInputHistoryEntry(i);
				if (!iHistory)
					continue;

				iHistory->setRenderTickCount(nTickCount + 1);
				if (!iHistory->pViewAngles)
					iHistory->pViewAngles = pCmd->AddInputHistory()->pViewAngles;

				iHistory->SetViewAngles(nig);
			}*/
			if (!can_shoot())
				return;

			if (C_GET(bool, Vars.bAutoFire))
				pCmd->nButtons.nValue |= IN_ATTACK;
		}
	}
}
