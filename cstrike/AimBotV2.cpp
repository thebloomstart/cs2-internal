#include "AimBot.h"
#include "Lagcomp.h"
#include "utilities/draw.h"
#include "AntiAim.h"
#include <iostream>
#include "utilities/inputsystem.h"
#include <DirectXMath.h>
#include "Pena.h"
#include "Utils.h"
#include "sdk/interfaces/iengineclient.h"
#include <format>
#include "sdk/interfaces/inetworkclientservice.h"
#include "features/misc/movement.h"
#include "pred.h"
#include "sdk/EntityList/EntityList.h"

float ScaleDamage(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, Vector_t start, Vector_t aim_point, float& dmg, bool& canHit)
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
using removed_aimfunction_t = void* (__fastcall*)(void*, QAngle_t*, float, bool);
inline static removed_aimfunction_t fnUpdateAimPunch_fn;



void RageBot::Hook()
{
	fnUpdateAimPunch_fn = reinterpret_cast<removed_aimfunction_t>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8D 4E ? 48 8D 54 24 ? E8 ? ? ? ? F2 0F 10 44 24")), 0x1));
}

QAngle_t RageBot::get_removed_aim_punch_angle(C_CSPlayerPawn* local_player)
{
	QAngle_t aim_punch = {};
	fnUpdateAimPunch_fn(local_player, &aim_punch, 0.0f, true);
	return aim_punch;
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
	c2 = std::cos(r4);
	s1 = std::sin(r2);
	s2 = std::sin(r4);

	// calculate spread vector.
	return {
		(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
		(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
		0.f
	};
}

int hitchance(C_CSPlayerPawn* pPawn, Vector_t vecTarget, int iHitChance)
{
	if (I::Cvar->Find(FNV1A::HashConst("weapon_accuracy_nospread"))->value.i1) // todo: nospread
		return 100;

	if (iHitChance == 0)
		return 0;

	C_CSWeaponBase* pWeapon = SDK::LocalPawn->GetWeaponActive();
	if (!pWeapon)
		return 0;
	
	const Vector_t vecSource = g_PredictionSystem->GetShootPos();

	const int nWantedHitCount = 255;
	Vector_t vecForward = Vector_t(0, 0, 0), vecRight = Vector_t(0, 0, 0), vecUp = Vector_t(0, 0, 0);
	g_Utils->CalcAngles(vecSource, vecTarget).ToDirections(&vecForward, &vecRight, &vecUp);

	int nHits = 0;
	const int nNeededHits = static_cast<int>(nWantedHitCount * (iHitChance / 100.f));

	pWeapon->update_accuracy();
	const float flWeaponSpread = g_PredictionSystem->GetSpread();
	const float flWeaponInaccuracy = g_PredictionSystem->GetAccuracy();

	TraceFilter_t filter(0x1C3003, SDK::LocalPawn, nullptr, 4);

	for (int i = 0; i < nWantedHitCount; i++)
	{
		float a = MATH::fnRandomFloat(0.f, 1.f);
		float b = MATH::fnRandomFloat(0.f, 2.f * DirectX::XM_PI);
		const float c = MATH::fnRandomFloat(0.f, 1.f);
		const float d = MATH::fnRandomFloat(0.f, 2.f * DirectX::XM_PI);

		const float flInaccuracy = a * flWeaponInaccuracy;
		const float flSpread = c * flWeaponSpread;

		if (SDK::LocalPawn->GetWeaponActive()->GetAttributeManager()->GetItem()->GetItemDefinitionIndex() == WEAPON_R8_REVOLVER)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector_t vecSpreadView((cos(b) * flInaccuracy) + (cos(d) * flSpread), (sin(b) * flInaccuracy) + (sin(d) * flSpread), 0), vecDirection = Vector_t(0, 0, 0);

		vecDirection.x = vecForward.x + (vecSpreadView.x * vecRight.x) + (vecSpreadView.y * vecUp.x);
		vecDirection.y = vecForward.y + (vecSpreadView.x * vecRight.y) + (vecSpreadView.y * vecUp.y);
		vecDirection.z = vecForward.z + (vecSpreadView.x * vecRight.z) + (vecSpreadView.y * vecUp.z);
		vecDirection = vecDirection.Normalized();

		QAngle_t angViewSpread{};
		g_Utils->VectorAngles(vecDirection, angViewSpread);
		angViewSpread = angViewSpread.Normalize();

		Vector_t vecViewForward;
		angViewSpread.ToDirections(&vecViewForward);
		vecViewForward.NormalizeInPlace();
		vecViewForward = vecSource + (vecViewForward * SDK::LocalPawn->GetWeaponActive()->GetWeaponVData()->m_flRange());

		Ray_t ray = Ray_t();
		GameTrace_t trace = GameTrace_t();

		I::GameTraceManager->ClipRayToEntity(&ray, vecSource, vecViewForward, pPawn, &filter, &trace);

		if (trace.m_pHitEntity == pPawn /*&& trace.m_pHitboxData->m_nHitGroup == hitgroup*/)
			nHits++;

		const int nHitChance = static_cast<int>((static_cast<float>(nHits) / nWantedHitCount) * 100.f);
		if (nHitChance >= iHitChance)
			return nHitChance;

		if ((nWantedHitCount - i + nHits) < nNeededHits)
			return 0;
	}
	return 0;
}

Vector_t ExtrapolatePosition(Vector_t pos, Vector_t vel) {
	float ticks = 1.f;
	const float intervalpertick = 1.f / 64.f;

	INetChannelInfo* networkGameClient = I::Engine->GetNetChannelInfo();
	if (!networkGameClient)
		return pos;

	float magic = static_cast<float>((networkGameClient->get_network_latency() * 64.f) / 1000.f);

	return (pos)+(vel * intervalpertick * static_cast<float>(ticks)) + (vel * magic);
}

//Skeet AutoRevolver
void AutoRevolver(CUserCmd* cmd) {
	static float revolverPrepareTime = 0.1f;
	static float readyTime;
	if (!SDK::LocalPawn)
		return;

	const auto activeWeapon = SDK::LocalPawn->GetWeaponActive();
	//if (activeWeapon && activeWeapon->GetAttributeManager() && activeWeapon->GetAttributeManager()->GetItem() && activeWeapon->GetAttributeManager()->GetItem()->GetItemDefinitionIndex() == WEAPON_R8_REVOLVER) {
	//	if (!readyTime)
	//		readyTime = I::GlobalVars->flCurTime + revolverPrepareTime;
	//	const auto ticksToReady = TIME_TO_TICKS(readyTime - I::GlobalVars->flCurTime - 0.5/*- interfaces::Engine->GetNetChannelInfo(0)->GetLatency(FLOW_OUTGOING)*/);
	//	if (ticksToReady > 0 && ticksToReady <= TIME_TO_TICKS(revolverPrepareTime))
	//		cmd->nButtons.nValue |= IN_ATTACK;
	//	else
	//		readyTime = 0.0f;
	//}
	C_CSPlayerPawn* local = SDK::LocalPawn;
	if (activeWeapon->GetClip1() > 0)
	{
		float v8 = TICKS_TO_TIME(SDK::LocalController->GetTickBase());
		if (local->CanAttack(v8))
		{
			if (v8 >= activeWeapon->GetNextPrimaryAttackTick()
				&& activeWeapon->GetAttributeManager()->GetItem()->GetItemDefinitionIndex() == WEAPON_R8_REVOLVER
				&& v8 > activeWeapon->m_nPostponeFireReadyTicks())
			{
				if (activeWeapon->GetNextSecondaryAttackTick() <= v8)
					goto LABEL_21;

				cmd->nButtons.nValue |= IN_SECOND_ATTACK;//IN_SECOND_ATTACK
			}
			else
				cmd->nButtons.nValue |= IN_ATTACK;//IN_ATTACK
		}
	}

LABEL_21:
	if (!(activeWeapon->GetClip1()) && (int)activeWeapon->GetReserveAmmo() > 0)
		cmd->nButtons.nValue = cmd->nButtons.nValue & ~(IN_RELOAD | IN_ATTACK);
}

std::optional<Lagcomp::LagRecord> findBestRecord(C_CSPlayerPawn* pawn) {
	auto entity = Lagcomp::g_LagCompensation->GetLagEntity(pawn);
	if (!entity)
		return std::nullopt;

	std::vector<Lagcomp::LagRecord> valid_records{};
	for (size_t i = 0; i < entity->m_arrLagRecords.size(); i++)
	{
		if (!entity->m_arrLagRecords[i].IsValidRecord())
			continue;

		valid_records.push_back(entity->m_arrLagRecords[i]);
	}

	if (valid_records.size() == 0)
		return std::nullopt;

	int nDeltaLimiter = C_GET(int, Vars.iMaxBackTrackTicks);

	int nBestTick = -1;
	float flBestSortingValue = 0.f;
	size_t nStartIndex = 0;

	if (C_GET(int, Vars.iMinBackTrackTicks))
		nStartIndex = C_GET(int, Vars.iMinBackTrackTicks);

	for (size_t i = nStartIndex; i < valid_records.size(); i++)
	{
		float flDeltaTime = pawn->GetSimulationTime() - valid_records[i].GetSimulationTime;
		int nDeltaTicks = TIME_TO_TICKS(flDeltaTime);

		if (nDeltaTicks > nDeltaLimiter)
			break;

		auto sBackupValue = valid_records[i].Apply(pawn);

		float flDamage = 0.;
		bool v = 0;
		Vector_t eye_pos = (SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin() - SDK::LocalPawn->GetAbsVelocity() * TICK_INTERVAL) + SDK::LocalPawn->GetViewOffset();

		Vector_t n = eye_pos;

		auto start = n;
		auto direction = (valid_records[i].m_boneData[BONEINDEX::Head].vecPosition - start).Normalized();
		direction *= n.DistTo(valid_records[i].m_boneData[BONEINDEX::Head].vecPosition) + 1.f;
		flDamage = ScaleDamage(pawn, SDK::LocalPawn,(C_CSWeaponBaseGun*)SDK::LocalPawn->GetWeaponActive(), g_PredictionSystem->GetShootPos(), valid_records[i].m_boneData[BONEINDEX::Head].vecPosition, flDamage, v);

		valid_records[i].Restore(pawn, sBackupValue);

		if (flDamage > flBestSortingValue /*&& nHitbox == 0*/) {
			nBestTick = (int)i;
			flBestSortingValue = flDamage;
		}
	}

	if (nBestTick == -1) {
		for (size_t i = nStartIndex; i < valid_records.size(); i++)
		{
			float flDeltaTime = pawn->GetSimulationTime() - valid_records[i].GetSimulationTime;
			int nDeltaTicks = TIME_TO_TICKS(flDeltaTime);

			if (nDeltaTicks > nDeltaLimiter)
				break;

			float flDistance = valid_records[i].m_boneData[BONEINDEX::Head].vecPosition.DistTo(g_PredictionSystem->GetShootPos());
			if (flDistance > flBestSortingValue) {
				nBestTick = (int)i;
				flBestSortingValue = flDistance;
			}
		}
	}

	if (nBestTick == -1) {
		return std::nullopt;
	}

	return valid_records[nBestTick];
}
bool can_shoot()
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


float getSelecetionSortingVal(float hitchance, float damage, float distance, ImVec2 screenpos) {
	Vector2D_t dista{};
	if (C_GET(int,Vars.iSelectTarget) == 0) // damage
		return damage;
	if (C_GET(int, Vars.iSelectTarget) == 1) // distance (world)
		return distance;
	if (C_GET(int, Vars.iSelectTarget) == 2) // distance (crosshair)
		return dista.dist(Vector2D_t{ screenpos.x, screenpos.y }, Vector2D_t{ ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f });
	if (C_GET(int, Vars.iSelectTarget) == 3) // hitchance
		return hitchance;
}

float getMinDmg() {
	/*if (CKeybindManager::GetKeybindKey(cfg.mindmgOverwrite) != "0" && CKeybindManager::IsKeybindActive(cfg.mindmgOverwrite))
		return *(float*)CKeybindManager::GetKeybindOverrideData(cfg.mindmgOverwrite);*/

	if(IPT::IsPressed(C_GET(int, Vars.bKeyMinDamage), C_GET(int, Vars.bKeyBindMinDamage)))
	{
		return C_GET(int, Vars.rage_minimum_damage);
	}
}

void AutoStop(C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, CUserCmd* cmd)
{
	if (!C_GET(bool,Vars.bAutoStop))
		return;

	if (cmd->nButtons.nValue & IN_DUCK || !pLocal || !weapon)
		return;
	if (!can_shoot())
		return;
	if (/*!pLocal->vali() || */!(SDK::LocalPawn->GetFlags() & FL_ONGROUND))
		return;

	CBaseUserCmdPB* pBaseCmd = cmd->csgoUserCmd.pBaseCmd;
	if (!pBaseCmd)
		return;

	pBaseCmd->SetBits(BASE_BITS_FORWARDMOVE);
	pBaseCmd->SetBits(BASE_BITS_LEFTMOVE);

	pBaseCmd->flSideMove = 0.0f;
	pBaseCmd->flForwardMove = pLocal->m_vecVelocity().Length2D() > 20.0f ? 1.0f : 0.0f;

	QAngle_t angViewAngles = g_AntiAim->StoreAngels;

	float flYaw = pLocal->m_vecVelocity().ToAngles().y + 180.0f;
	float flRotation = M_DEG2RAD(angViewAngles.y - flYaw);

	float flCosRotation = std::cos(flRotation);
	float flSinRotation = std::sin(flRotation);

	float flNewForwardMove = flCosRotation * pBaseCmd->flForwardMove - flSinRotation * pBaseCmd->flSideMove;
	float flNewSideMove = flSinRotation * pBaseCmd->flForwardMove + flCosRotation * pBaseCmd->flSideMove;

	pBaseCmd->flForwardMove = flNewForwardMove;
	pBaseCmd->flSideMove = -flNewSideMove;
}
void SetupHitboxes()
{
	g_RageBot->Hitbox.clear();
	for (int i = 0; i < HITBOX_COUNT; i++) {
		if (C_GET(unsigned int,Vars.iHitbox) & (1 << i)) {
			g_RageBot->Hitbox.push_back(BoneIndexMap[i]);
		}
	}
}

std::optional<SelectionResult> select() {
	std::vector<C_CSPlayerPawn*> players = g_Utils->GetPlayers(true, false, true);

	SelectionResult best{};
	float bestSortingVal = -1.f;

	C_CSWeaponBaseGun* weapon = (C_CSWeaponBaseGun*)SDK::LocalPawn->GetWeaponActive();
	if (!weapon)
		return std::nullopt;

	for (size_t p = 0; p < players.size(); p++)
	{
		if (!players[p])
			continue;

		players[p]->GetGameSceneNode()->GetSkeletonInstance()->calc_world_space_bones(0xfffffu);

		auto record = findBestRecord(players[p]);
		Lagcomp::LagRecordBackup record_backup{};
		if (record.has_value())
			record_backup = record->Apply(players[p]);

		if (!record.has_value()) {
			auto le = Lagcomp::g_LagCompensation->GetLagEntity(players[p]);
			if (le.has_value() && le->m_arrLagRecords.size() > 0) {
				record_backup = le->m_arrLagRecords[0].Apply(players[p]);
			}
			else {
				//
			}
		}
		
		//std::vector<CHitBox*> hbs = hitboxes(players[p]);

		//CHitBox* bestHitBox = nullptr;
		Vector_t bestMultipoint{};
		float bestDamage = 0.f;
		int pointhitchance = 0;
		int hitbox_indexs{};
		g_RageBot->svo = { 0,0 };
		for (int hitbox : g_RageBot->Hitbox)
		{
			hitbox_indexs = hitbox;
			//CHitBox* hitbox = hbs[hitbox_index];
			//if (!hitbox)
			//	continue;
			/*std::vector<AimPosition> */ Vector_t mps = players[p]->GetGameSceneNode()->GetSkeletonInstance()->GetBonePosition(hitbox); //multipoints(players[p], hitbox_index);
		//	L_PRINT(LOG_INFO) << hitbox;
			Vector_t bestPoint = {};
			float bestPointDamage = 0.f;

			//for (size_t mulitpoint_index = 0; mulitpoint_index < mps.size(); mulitpoint_index++)
			{
				if (mps.DistTo(g_PredictionSystem->GetShootPos()) > weapon->GetWeaponVData()->m_flRange())
					continue;

				if (&C_GET(bool,Vars.bPerfectSilent))
					if (g_Utils->GetFov(g_AntiAim->StoreAngels, g_Utils->CalcAngles(g_PredictionSystem->GetShootPos(), mps)) > 45.f)
						continue;

				float damage = 0.f;
				int hitbox = 0;
				bool s = false;
				damage = ScaleDamage(players[p], SDK::LocalPawn, SDK::LocalPawn->GetWeaponActive(), g_PredictionSystem->GetShootPos(), mps, damage,s);
	
				//AutoWall::calculate_damage(players[p], weapon, mps[mulitpoint_index].m_vecMultiPoints, &damage, &hitbox);

				bool bIsDamageOkay = false;

				if (damage > getMinDmg())
					bIsDamageOkay = true;
				else {
					if (players[p]->GetHealth() < 100.f)
						bIsDamageOkay = damage > players[p]->GetHealth();
				}
				if (bIsDamageOkay) {
					if (bestPointDamage < damage) {
						bestPoint = mps;
						D::WorldToScreen(mps, &g_RageBot->svo);
						bestPointDamage = damage;
					}
				}
			}
			if (bestDamage < bestPointDamage) {
				bestDamage = bestPointDamage;
				D::WorldToScreen(g_PredictionSystem->GetShootPos(), &g_RageBot->v);

			//    bestHitBox = hitbox_index;
				bestMultipoint = bestPoint;
				//if (!C_GET(bool,Vars.bNoSpread))
			//	pointhitchance = C_GET(bool, Vars.rage_hitchance) ? 100 : hitchance(players[p], bestMultipoint, C_GET(int, Vars.rage_minimum_hitchance));
			//	else*/
					//pointhitchance = HitChanceNS(bestMultipoint,SDK::LocalPawn->GetEyePosition(),players[p]);
				best.record = record;
				best.simtime = record.has_value() ? record->GetSimulationTime : players[p]->GetSimulationTime();
			}
		}

		if (record.has_value())
			record->Restore(players[p], record_backup);
		else {
			auto le = Lagcomp::g_LagCompensation->GetLagEntity(players[p]);
			if (le.has_value() && le->m_arrLagRecords.size() > 0) {
				le->m_arrLagRecords[0].Restore(players[p], record_backup);
			}
		}

		if (bestDamage == 0.f)
			continue;

		if (pointhitchance < C_GET(int, Vars.bKeyHitchance), C_GET(int, Vars.bKeyBindHitchance)) {
			bDidntSelectDueToHitchance = true;
			continue;
		}
		bDidntSelectDueToHitchance = false;
		// pointhitchance = hitchance(weapon, bestMultipoint, players[p], cheats::ragebotMinDmg, cheats::ragebotHitchance);

		ImVec2 sp{};
		D::WorldToScreen(g_PredictionSystem->GetShootPos(), &sp);
		float sortingVal = getSelecetionSortingVal(pointhitchance, bestDamage, g_PredictionSystem->GetShootPos().DistTo(players[p]->GetEyePosition()), sp);
		if (sortingVal > bestSortingVal) {
			best.entity = players[p];
			best.expectedDamage = bestDamage;
			best.hitbox = hitbox_indexs;
			best.hitchance = pointhitchance;
			best.record = record;
			best.point = bestMultipoint;
			bestSortingVal = sortingVal;
		}
		players.clear();
	}
	return bestSortingVal > 0.f ? std::make_optional<SelectionResult>(best) : std::nullopt;
}

std::string hitboxIndexToName(int index) {
	if (index == 0)
		return ("Head");
	if (index == 1)
		return ("Neck");
	if (index == 2)
		return ("Pelvis");
	if (index == 3)
		return ("Stomach");
	if (index == 4)
		return ("Chest");
	if (index == 7)
		return ("Left thigh");
	if (index == 8)
		return ("Right thigh");
	if (index == 9)
		return ("Left leg");
	if (index == 10)
		return ("Right leg");
	if (index == 15)
		return ("Left upper arm");
	if (index == 16)
		return ("Left lower arm");
	if (index == 17)
		return ("Right upper arm");
	if (index == 18)
		return ("Right lower arm");
	if (index == 11)
		return ("Left foot");
	if (index == 12)
		return ("Right foot");
	return ("unknonwn");
}


bool RageBot::runRagebot(CCSGOInput* pInput) {
	//interfaces::ragebotAiming = false;
	if (!SDK::Alive)
		return false;
	if (!C_GET(bool, Vars.rage_enable))
		return false;

	CUserCmd* usercmd = SDK::Cmd;
	if (!usercmd)
		return false;

	CBaseUserCmdPB* baseCmd = usercmd->csgoUserCmd.pBaseCmd;
	if (!baseCmd)
		return false;
	if (!SDK::LocalPawn)
		return false;
	if (!SDK::LocalController->IsPawnAlive())
		return false;
	if (!SDK::LocalPawn->GetWeaponActive())
		return false;

	//if (!isValidPawn(SDK::LocalPawn) || !isValidController(SDK::LocalController) || !SDK::LocalController->m_bPawnIsAlive() || !config.rage.enable || !SDK::LocalPawn->GetActiveWeapon() || m_bIsInFreezeTime) {
	//	quickpeek_retract = false;
	//	quickpeek_retract_shoot = false;
	//	return false;
	//}
	//quickpeek(baseCmd);
	//autoRevolver(pInput->GetUserCmd()->csgoUserCmd.pBaseCmd);

	if (SDK::LocalPawn->GetWeaponActive()->IsNade() || SDK::LocalPawn->GetWeaponActive()->IsInReload())
		return false;

	if (SDK::LocalPawn->GetWeaponActive()->IsKnife())
		return false;
	SetupHitboxes();

	if (SDK::LocalPawn->GetWeaponActive()->GetAttributeManager()->GetItem()->GetItemDefinitionIndex() == WEAPON_ZEUS_X27) {
		std::vector<C_CSPlayerPawn*> players = g_Utils->GetPlayers(true,false, false);

		C_CSWeaponBaseGun* weapon = (C_CSWeaponBaseGun*)SDK::LocalPawn->GetWeaponActive();

		C_CSPlayerPawn* target = nullptr;
		Vector_t targetPos = {};

		for (size_t p = 0; p < players.size(); p++) {
			CGameSceneNode* node = players[p]->GetGameSceneNode();
			if (!node)
				continue;

			CSkeletonInstance* skel = node->GetSkeletonInstance();
			if (!skel)
				continue;

			float dist = skel->GetBonePosition(BONEINDEX::Pelvis).DistTo(SDK::LocalPawn->GetEyePosition());

			if (dist > weapon->GetWeaponVData()->m_flRange())
				continue;

			Ray_t ray{};
			game_trace_t trace{};
			TraceFilter_t filter{ 0x1C3003, SDK::LocalPawn, NULL, 4 };

			if (!I::GameTraceManager->TraceShape(&ray, SDK::LocalPawn->GetEyePosition(), skel->GetBonePosition(BONEINDEX::Pelvis), &filter, &trace))
				continue;

			if (trace.HitEntity != players[p] || trace.DidHitWorld())
				continue;

			targetPos = skel->GetBonePosition(BONEINDEX::Pelvis);
			target = players[p];
		}

		if (target) {
			QAngle_t angle = g_Utils->CalcAngles(g_PredictionSystem->GetShootPos(), targetPos);

			int nTickCount = TIME_TO_TICKS(target->GetSimulationTime()) + 1;
			for (size_t i = 0; i < usercmd->csgoUserCmd.inputHistoryField.nCurrentSize; i++)
			{
				CCSGOInputHistoryEntryPB* iHistory = usercmd->csgoUserCmd.inputHistoryField.pRep->tElements[i];
				if (!iHistory) continue;

				//iHistory->setRenderTickCount(nTickCount + 1);
				//iHistory->setPlayerTickCount(SDK::LocalController->GetTickBase());

				//if (!iHistory->cl_interp)
				//	iHistory->cl_interp = iHistory->create_CLinterp_message();

				iHistory->setClInterp(nTickCount + 1, nTickCount, 0.999f);

				//if (!iHistory->sv_interp0)

				iHistory->setSVInterp0(nTickCount + 1, nTickCount, 0.999f);

				//if (!iHistory->sv_interp1)

			    iHistory->setSVInterp1(nTickCount + 1, nTickCount, 0.999f);

				//if (!iHistory->player_interp)
				//	iHistory->player_interp = usercmd->AddInputHistory()->player_interp;

				iHistory->setPlayerInterp(nTickCount + 1, nTickCount, 0.999f);

				//if (!iHistory->pViewAngles)
				//	iHistory->pViewAngles = iHistory->create_qangle_message();

				iHistory->SetViewAngles(angle);

				//if (!iHistory->pShootPosition)
				//	iHistory->pShootPosition = usercmd->AddInputHistory()->pShootPosition;
				//iHistory->SetShootPos(Vector4D_t{ SDK::LocalPawn->GetEyePosition().x, SDK::LocalPawn->GetEyePosition().y, SDK::LocalPawn->GetEyePosition().z });

				iHistory->nTargetEntIndex = target->GetRefEHandle().GetEntryIndex();
				iHistory->SetBits(INPUT_HISTORY_BITS_TARGETENTINDEX);
			}

			//baseCmd->markButtonStateModified();
			usercmd->nButtons.nValue |= IN_ATTACK;

			//CCSPlayerController* controller = interfaces::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(target->m_hController());

			//std::stringstream ss;
			//ss << CHAT_COLOR_WHITE "[" CHAT_COLOR_GRAY "OXYCODEINE.CC" CHAT_COLOR_WHITE "] " CHAT_COLOR_RED "RAGEBOT" CHAT_COLOR_WHITE": Zeusing \"";
			//ss << controller->m_sSanitizedPlayerName() << "\"";
			//MEM::SendMessageToLocalPlayer(ss.str().c_str());
			players.clear();
		}

		return false;
	}


	std::optional<SelectionResult> s = select();
	if (!s.has_value()) {

		if (bDidntSelectDueToHitchance) 
		{
			// predictive autoscope / autostop
			if (/*_config.autoStop ||*/ C_GET(bool, Vars.bAutoScope) && /*!_config.nospread &&*/ C_GET(bool, Vars.rage_enable)){
				std::vector<C_CSPlayerPawn*> players = g_Utils->GetPlayers(true, false, true);

				C_CSWeaponBaseGun* weapon = (C_CSWeaponBaseGun*)SDK::LocalPawn->GetWeaponActive();

				if (!weapon || !weapon->GetVData() || !weapon->GetWeaponVData())
					return false;

				float dmg = 0.f;
				for (size_t p = 0; p < players.size(); p++) {
					CGameSceneNode* node = players[p]->GetGameSceneNode();
					if (!node)
						continue;

					CSkeletonInstance* skel = node->GetSkeletonInstance();
					if (!skel)
						continue;

					auto entity = Lagcomp::g_LagCompensation->GetLagEntity(players[p]);
					if (entity.has_value()) {
						//for (size_t i = 0; i < C_GET(int,Vars.iMaxBackTrackTicks); i++)
						{
						//	if (i >= entity->m_arrLagRecords.size())
							//	break;
							
							//Lagcomp::LagRecordBackup backup = entity->m_arrLagRecords[i].Apply(players[p]);

							static float amount_extrapolate = 3.f; // <- idk
							for (int hitbox : Hitbox)
							{
								int _ = 0;
								Vector_t m = skel->GetBonePosition(hitbox);

								Vector_t eyePos = ExtrapolatePosition(g_PredictionSystem->GetShootPos(), SDK::LocalPawn->GetAbsVelocity() * amount_extrapolate);
								auto start = eyePos;
								auto direction = (m - start).Normalized();
								direction *= eyePos.DistTo(m) + 6.f;
								bool sdasd;
								dmg = ScaleDamage(players[p], SDK::LocalPawn, SDK::LocalPawn->GetWeaponActive(), eyePos, m, dmg, sdasd);

								//	AutoWall::run(players[p], eyePos, direction, dmg, _);
								//entity->m_arrLagRecords[i].Restore(players[p], backup);

								if (dmg > getMinDmg() / 2.f)
									break;
							}
						}
					}
					
					if (dmg > getMinDmg() / 2.f)
						break;
				}
				players.clear();
				if (C_GET(bool, Vars.bAutoScope) && weapon->HasScope() && weapon->GetZoomLevel() == 0) {
					//usercmd->csgoUserCmd.markBaseCmdAsModified();
					usercmd->nButtons.nValue &= ~IN_SECOND_ATTACK;
					//baseCmd->pInButtonState->removeButton(IN_SECOND_ATTACK);

					if (dmg > 0.f) {
						usercmd->nButtons.nValue |= IN_SECOND_ATTACK;
						//baseCmd->pInButtonState->addButtonValue(IN_SECOND_ATTACK);
						usercmd->csgoUserCmd.setAttack2StartHistoryIndex(0);
					}
				}
				//F::MISC::MOVEMENT::quick_stop(usercmd);
				AutoStop(SDK::LocalPawn, SDK::LocalPawn->GetWeaponActive(), usercmd);
				if (/*_config.autoStop &&*/ !(baseCmd->pInButtonState->nValue & IN_DUCK) && SDK::LocalPawn->GetWeaponActive()->GetNextSecondaryAttackTick() <= SDK::LocalController->GetTickBase()
					&& SDK::LocalPawn->GetAbsVelocity().Length2D() > 20.f) {

					if (dmg > getMinDmg() / 2.f && SDK::LocalPawn->GetFlags() & FL_ONGROUND && !(baseCmd->pInButtonState->nValue & IN_JUMP)) {
						CBaseUserCmdPB* pCmd = SDK::Cmd->csgoUserCmd.pBaseCmd;
						pCmd->flForwardMove = 1.f;
						pCmd->flSideMove = 0.f;
						Vector_t* move = (Vector_t*)&pCmd->flForwardMove;
						Vector_t move_backup = *move;
						const QAngle_t& current_angles = g_AntiAim->StoreAngels;

						QAngle_t angle = g_Utils->CalcAngles(SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin(),ExtrapolatePosition(SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin(), SDK::LocalPawn->GetAbsVelocity() * -5.f));

						const float delta = remainderf(angle.y - current_angles.y, 360.f);
						const float yaw = DirectX::XMConvertToRadians(delta);

						move->x = move_backup.x * std::cos(yaw) - move_backup.y * std::sin(yaw);
						move->y = move_backup.x * std::sin(yaw) + move_backup.y * std::cos(yaw);

						float mul = 1.f;

						if (std::fabsf(move->x) > 1.f)
							mul = 1.f / std::fabsf(move->x);
						else if (std::fabsf(move->y) > 1.f)
							mul = 1.f / std::fabsf(move->y);

						move->x *= mul;
						move->y *= mul;
						move->z = 0.f;

						pCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
						pCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
						pCmd->flForwardMove = move->x * mul;
						pCmd->flSideMove = move->y * mul;
					}
				}
			}
		}
		bDidntSelectDueToHitchance = false;

		return false;
	}

	CCSPlayerController* targetController = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(s->entity->GetControllerHandle());
	if (!targetController)
		return false;
	if (!can_shoot())
	    return false;

	if (C_GET(bool,Vars.bAutoFire) || baseCmd->pInButtonState->nValue & IN_ATTACK) {
		//interfaces::ragebotAiming = true;
		//SDK::fnConColorMsg(Color_t(255, 255, 255), "Targetting: ",targetController->GetPlayerName(), " Hitbox:" , hitboxIndexToName(s->hitbox) , " Damage:", (int)s->expectedDamage);
		AutoRevolver(usercmd);

		/*if (C_GET(bool,Vars.bNoSpread))
			SDK::fnConColorMsg(Color_t(255, 255, 255),"NoSpread");
		else
			SDK::fnConColorMsg(Color_t(255, 255, 255), "Hitchance");*/
		if (s->record.has_value()) {
			float delta = fabsf(s->entity->GetSimulationTime() - s->record->GetSimulationTime);
			/*if (TIME_TO_TICKS(delta) > 0)
				SDK::fnConColorMsg(Color_t(255, 255, 255), "Backtracking", TIME_TO_TICKS(delta) , " ticks");
			else
				SDK::fnConColorMsg(Color_t(255, 255, 255), ", No Backtrack");*/
		}
		else
		{

		}
		//SDK::fnConColorMsg(Color_t(255, 255, 255), ", No Backtrack");

		QAngle_t angle = g_Utils->CalcAngles(g_PredictionSystem->GetShootPos(), s->point);
		angle = angle - get_removed_aim_punch_angle(SDK::LocalPawn);
		// TODO: proper nospread
		//if (C_GET(bool, Vars.bNoSpread))
		//{
		//	s->hitchance = HitChanceNS(angle, SDK::LocalPawn->GetEyePosition(), s->entity, SDK::LocalPawn->GetWeaponActive(), SDK::LocalPawn);
		//	//NoSpread::NoSpreadResult res = NoSpread::nospread(angle);
		//	//if (!res.found) {
		//	//	L_PRINT(LOG_WARNING) << "Failed to find NS Seed!";
		//	//	return false;
		//	//}
		//	if (s->hitchance == 100)
		//		L_PRINT(LOG_INFO) << "NoSpread : " << s->hitchance;
		//	else
		//		return 0;
		//	//angle = res.adjusted;
		//}

		g_RageBot->shotInfo.pPawn = s->entity;
		g_RageBot->shotInfo.vecShootEyePosition = g_PredictionSystem->GetShootPos();
		g_RageBot->shotInfo.angShootAngle = angle;
		if (!s->record.has_value()) {
			auto le = Lagcomp::g_LagCompensation->GetLagEntity(s->entity);
			if (le.has_value() && le->m_arrLagRecords.size() > 0)
				g_RageBot->shotInfo.pRecord = le->m_arrLagRecords[0];
		}
		else
			g_RageBot->shotInfo.pRecord = s->record.value();
		//interfaces::ragebotTarget = s->entity;

		QAngle_t silentAngle = angle;

		int nTickCount = TIME_TO_TICKS(s->simtime) + 1;
		if (s->record.has_value()) {
			nTickCount = TIME_TO_TICKS(s->record->GetSimulationTime) + 1;
			/*std::string log = std::format("Backtracking: {:.1f} | delta: {:.1f}", nTickCount, abs((TIME_TO_TICKS(s->simtime) + 1) - (TIME_TO_TICKS(s->entity->GetSimulationTime()) + 1)));
			I::Engine->client_cmd_unrestricted(log);*/
			L_PRINT(LOG_INFO) << "Backtracking to " << nTickCount << " (delta: " << abs((TIME_TO_TICKS(s->simtime) + 1) - (TIME_TO_TICKS(s->entity->GetSimulationTime()) + 1)) << " ticks)";
		}

		for (int i = 0; i < usercmd->csgoUserCmd.inputHistoryField.nCurrentSize; i++)
	    {
		    CCSGOInputHistoryEntryPB* iHistory = usercmd->GetInputHistoryEntry(i);
			if (!iHistory)
	        	continue;
			if (iHistory->nRenderTickCount)
	    	    iHistory->setRenderTickCount(nTickCount + 1);

			if (iHistory->nPlayerTickCount)
	     		iHistory->setPlayerTickCount(g_PredictionSystem->predictionData.nTickBase);

	   //      if (!iHistory->cl_interp)
				//iHistory->cl_interp = iHistory->create_CLinterp_message();
			/*if (iHistory->cl_interp)
			    iHistory->setClInterp(nTickCount + 1, nTickCount, 0.999f);*/

			//if (!iHistory->sv_interp0)
			//	iHistory->sv_interp0 = iHistory->create_interp_message();
			/*if (iHistory->sv_interp0)
			    iHistory->setSVInterp0(nTickCount + 1, nTickCount, 0.999f);*/

			/*if (!iHistory->sv_interp1)
				iHistory->sv_interp1 = iHistory->create_interp_message();*/
			/*if (iHistory->sv_interp1)
			    iHistory->setSVInterp1(nTickCount + 1, nTickCount, 0.999f);*/

			//if (!iHistory->player_interp)
			//	iHistory->player_interp = iHistory->create_interp_message();

			/*if (iHistory->player_interp)
			    iHistory->setPlayerInterp(nTickCount + 1, nTickCount, 0.999f);*/

			//if (!iHistory->pViewAngles)
			//	iHistory->pViewAngles = iHistory->create_qangle_message();

			iHistory->SetViewAngles(silentAngle);

			/*if (!iHistory->pShootPosition)
				iHistory->pShootPosition = iHistory->create_vector_message();*/
			if (iHistory->pShootPosition)
			{
				L_PRINT(LOG_INFO) << iHistory->pShootPosition->vecValue.x;
				iHistory->SetShootPos(Vector4D_t{ SDK::LocalPawn->GetEyePosition().x, SDK::LocalPawn->GetEyePosition().y, SDK::LocalPawn->GetEyePosition().z });
			}

			iHistory->nTargetEntIndex = s->entity->GetRefEHandle().GetEntryIndex();
			iHistory->SetBits(INPUT_HISTORY_BITS_TARGETENTINDEX);
		}
		//usercmd->adjust_input_history(SDK::Cmd->GetInputHistoryEntry(0), silentAngle, nTickCount, s->entity->GetRefEHandle().GetEntryIndex(), SDK::LocalPawn->GetEyePosition(), SDK::LocalController->GetTickBase());
		//usercmd->SetSubTickAngle(silentAngle);
		//const bool is_safe_angle = g_Utils->GetFov(baseCmd->pViewAngles->angValue, angle) <= 45.f;
		//if (!is_safe_angle) {
		//	QAngle_t fix_angle = baseCmd->pViewAngles->angValue;
		//	fix_angle.x = 179.f;
		//	
		//	baseCmd->pViewAngles->angValue = fix_angle;
		//}
		silentAngle.z = 0;
		baseCmd->pViewAngles->angValue = (silentAngle);
		baseCmd->pViewAngles->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
			//usercmd->csgoUserCmd.markBaseCmdAsModified();
		usercmd->csgoUserCmd.setAttack1StartHistoryIndex(0);

		if (!(usercmd->nButtons.nValue & IN_ATTACK)) {
			//usercmd->csgoUserCmd.markBaseCmdAsModified();
			  usercmd->nButtons.nValue |= IN_ATTACK;
			//baseCmd->pInButtonState->addButtonValue(IN_ATTACK);
		}
		//if (config.rage.quickPeek && CKeybindManager::IsKeybindActive(config.rage.quickPeekKey))
		//	quickpeek_retract_shoot = true;

		// todo: subticks maybe
	}
}
