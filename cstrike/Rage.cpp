#include <cstdint>
#include "utilities/memory.h"
#include "utilities/notify.h"
#include "sdk/datatypes/vector.h"
#include "Rage.h"
#include "Pena.h"
#include "utilities/inputsystem.h"
#include <format>
typedef void(__fastcall* FnCalcSpread)(int16_t, int, int, std::uint32_t, float, float, float, float*, float*);

static FnCalcSpread fnCalcSpread;

namespace fuck_this_shit {
	static void fin_spread_pttr() {
		fnCalcSpread = reinterpret_cast<FnCalcSpread>(MEM::FindPattern(CLIENT_DLL, "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA"));
	}
}

typedef void(__fastcall* FnCalcSpread)(int16_t, int, int, std::uint32_t, float, float, float, float*, float*);

Vector_t ragebot::calculate_spread_internal(int seed, float accuracy, float spread, float recoil_index, int item_def_idx)
{

	Vector_t result;
	if (!fnCalcSpread)
		fnCalcSpread = reinterpret_cast<FnCalcSpread>(MEM::FindPattern(CLIENT_DLL, "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA"));

	fnCalcSpread(
		item_def_idx,
		1,
		0,
		seed + 1,
		accuracy,
		spread,
		recoil_index,
		&result.x,
		&result.y
	);

	return result;
}


void ragebot::Hook()
{
	fnUpdateAimPunch_fn = reinterpret_cast<removed_aimfunction_t>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8D 4E ? 48 8D 54 24 ? E8 ? ? ? ? F2 0F 10 44 24")), 0x1));
}

float ScaleDamagese(C_CSPlayerPawn* target, C_CSPlayerPawn* pLocal, C_CSWeaponBase* weapon, Vector_t start, Vector_t aim_point, float& dmg, bool& canHit)
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

bool scan::calculate_best_point(EntityCache::cached_entity& entity, C_CSPlayerPawn* local_pawn, C_BaseEntity* local_ent_base,
	C_BaseEntity* target_ent_base, const lag_record_t& lag_record, target_info& best_info,
	bool has_heavy_armor, int team, int health) {

	const Vector_t eye_pos = SDK::EyePos;
	const float current_time = I::GlobalVars->flCurTime;
	float lethal_threshold = health + C_GET(int, Vars.iLethalModifer);
	bool is_adaptive = C_GET(int, Vars.iScanType) == 1;

	std::atomic<bool> lethal_found{ false };
	std::atomic<float> best_damage{ 0.0f };
	target_info lethal_info{};
	std::mutex lethal_mutex;

	std::vector<std::future<std::optional<target_info>>> hitbox_futures;
	hitbox_futures.reserve(pointscan_config.size());

	for (const auto& [hitbox_group, _] : pointscan_config) {
		hitbox_futures.push_back(std::async(std::launch::async, [&]() -> std::optional<target_info> {
			if (lethal_found.load(std::memory_order_relaxed)) {
				return std::nullopt;
			}

			const int bone = static_cast<int>(MapHitgroupToBone(static_cast<int>(hitbox_group)));
			const Vector_t& center = lag_record.m_bone_arr[bone].vecPosition;

			const hitboxes_data hitbox_data = entity.pawn->GetHitboxRadMinMax(static_cast<int>(hitbox_group));

			if (hitbox_data.rad <= 0.0f) {
				return std::nullopt;
			}

			auto points = GetMultipointPositions(lag_record, static_cast<int>(hitbox_group),
				hitbox_data.rad, hitbox_data.min, hitbox_data.max);

			if (points.empty()) {
				return std::nullopt;
			}

			target_info local_best{};
			local_best.damage = best_damage.load(std::memory_order_relaxed);

			for (const Vector_t& point : points) {
				if (lethal_found.load(std::memory_order_relaxed)) {
					return std::nullopt;
				}
				float damage = 0;
				bool as{};
				F::AUTOWALL::c_auto_wall::data_t data;
				F::AUTOWALL::g_auto_wall->pen(data, IPT::IsPressed(C_GET(int, Vars.iDuckPeekKey), C_GET(int, Vars.iDuckPeekKeyBind)) ? DUCKPEEKASSIST_EYEPOS : eye_pos, point, target_ent_base, local_ent_base, SDK::LocalPawn, entity.pawn, SDK::LocalPawn->GetWeaponActive()->GetWeaponVData(), damage, as);
				//const float damage = autowall::penetration_damage(
			/*DUCKPEEKASSIST ? DUCKPEEKASSIST_EYEPOS : eye_pos, point, local_ent_base, local_pawn,
					target_ent_base, entity.pawn, has_heavy_armor, team*/

				damage = data.m_dmg;
				if (is_adaptive /*&& !HOLDING_MIN*/ && damage > lethal_threshold) {
					target_info lethal_shot{
						damage, point, entity, lag_record,
						hitbox_data.rad, hitbox_data.min, hitbox_data.max,
						bone, hitbox_group,
						current_time - lag_record.m_simulation_time
					};

					std::lock_guard<std::mutex> lock(lethal_mutex);
					if (!lethal_found.load(std::memory_order_relaxed)) {
						lethal_info = lethal_shot;
						lethal_found.store(true, std::memory_order_release);
						return lethal_shot;
					}
					return std::nullopt;
				}


				if (damage > local_best.damage && damage >= C_GET(int, Vars.rage_minimum_damage)) {
					if (!IPT::IsPressed(C_GET(int, Vars.bKeyMinDamage), C_GET(int, Vars.bKeyBindMinDamage)))
					{
						damage > local_best.damage && damage >= 100;
					}
					else
					{
						damage > local_best.damage && damage >= C_GET(int, Vars.rage_minimum_damage);
					}

					local_best = {
						damage, point, entity, lag_record,
						hitbox_data.rad, hitbox_data.min, hitbox_data.max,
						bone, hitbox_group,
						current_time - lag_record.m_simulation_time
					};
					best_damage.store(damage, std::memory_order_relaxed);
				}
			}

			return local_best.damage > 0 ? std::optional<target_info>(local_best) : std::nullopt;
			}));
	}

	if (lethal_found.load(std::memory_order_acquire)) {
		best_info = lethal_info;
		for (auto& future : hitbox_futures) {
			if (future.valid()) {
				future.wait();
			}
		}
		return true;
	}

	bool improved = false;
	float current_best = 0.0f;

	for (auto& future : hitbox_futures) {
		if (auto result = future.get()) {
			if (result->damage > current_best) {
				best_info = *result;
				current_best = result->damage;
				improved = true;
				TIMER_START("RageBot");
			}
		}
	}
	//double s = TIMER_END("STARTSCAN RAGE");
	return improved;
}

void scan::process_player(EntityCache::cached_entity& entity, C_CSPlayerPawn* pLocalPawn, C_BaseEntity* local_ent_base, CUserCmd* pCmd, target_info& bestInfo)
{

	if (entity.controller) {
		entity.update_pawn();
	}

	if (!entity.valid()) {
		return;
	}

	auto log = g_player_records->player_records[entity.controller->GetRefEHandle().GetEntryIndex()];

	if (log.records.empty()) // non targets , such as teamates , pawns with immunitiy, or dead should have empty records
		return;

	auto target_ent_base = I::GameResourceService->pGameEntitySystem->Get(entity.pawn->GetRefEHandle());
	bool target_have_armor = entity.pawn->GetItemServices()->m_bHasHeavyArmor();
	int target_team = entity.pawn->GetTeam();
	int target_health = entity.pawn->GetHealth();

	lag_record_t newestRecord = log.get_record(true);
	bool current_record_good = false;
	bool predicted_record_good = false;
	if (!newestRecord.valid) {
		L_PRINT(LOG_ERROR) << ("NEWEST INVALID TYPE SHI");
		return;
	}

	float dist = SDK::EyePos.DistTo(newestRecord.m_bone_arr[6].vecPosition);

	//if (dist > SDK::WepRange) {
	//	return;
	//}

	newestRecord.execute(entity.pawn, false, [&]() {
		current_record_good = calculate_best_point(
			entity,
			pLocalPawn,
			local_ent_base,
			target_ent_base,
			newestRecord,
			bestInfo,
			target_have_armor,
			target_team,
			target_health
		);
		});

	if (!current_record_good) {

		if (!IPT::IsPressed(C_GET(int, Vars.bKeyPredict), C_GET(int, Vars.bKeyBindPredict)))
			return;

		if (C_GET(bool, Vars.bPredictLethal)) {


			newestRecord.execute(entity.pawn, true, [&]() {
				predicted_record_good = calculate_best_point(
					entity,
					pLocalPawn,
					local_ent_base,
					target_ent_base,
					newestRecord,
					bestInfo,
					target_have_armor,
					target_team,
					target_health
				);
				});


		}

		// check if use backtracking here .. 
		if (!predicted_record_good && true) {

			lag_record_t oldestRecord = log.get_record(false);

			if (!oldestRecord.is_in_bounds()) {

				return;
			}

			oldestRecord.execute(entity.pawn, false, [&]() {
				calculate_best_point(
					entity,
					pLocalPawn,
					local_ent_base,
					target_ent_base,
					oldestRecord,
					bestInfo,
					target_have_armor,
					target_team,
					target_health
				);
				});

		}
	}


}

std::array<Vector_t, 9> scan::GenerateMultipointPositions(const Vector_t& center, float rs, const Vector_t& forward)
{

	Vector_t up(0, 0, 1);
	Vector_t right = forward.cross(up).normalized() * rs;

	auto positions = std::array<Vector_t, 9>{
		center + right,                                  // Top-right
			center - right,                              // Top-left
			center + up * rs,                            // Top-center
			center - up * rs,                            // Bottom-center
			center,                                      // Center
			center + up * rs - right,                    // Top-left
			center + up * rs + right,                    // Top-right
			center - up * rs - right,                    // Bottom-left (new)
			center - up * rs + right                     // Bottom-right (new)
	};

	return positions;
}

void ragebot::debug_multipoints()
{
}

std::vector<Vector_t> scan::GetMultipointPositions(const lag_record_t& lagRecord, int group, float rad, const Vector_t& min, const Vector_t& max)
{

	const auto& multipointSets = pointscan_config[static_cast<EHitBoxes>(group)];
	if (multipointSets.empty()) {
		return {};
	}

	Vector_t center = lagRecord.m_bone_arr[static_cast<int>(MapHitgroupToBone(group))].vecPosition;

	Vector_t forward;
	g_Utils->AngleVectors_two(g_Utils->CalcAngles(center, SDK::EyePos), &forward);

	std::vector<Vector_t> multipointPositions;
	multipointPositions.reserve(multipointSets.size() * 7);

	for (const auto& set : multipointSets)
	{
		if (!set.top && !set.bottom && !set.left && !set.right && !C_GET(bool,Vars.bmiddle) && !C_GET(bool, Vars.btop_left)/*set.top_left*/ && !set.top_right && !set.bottom_left && !C_GET(bool, Vars.bbottom_right)/*set.bottom_right*/) {
			continue;
		}

		float rs = rad * (set.pointscale / 100.0f);

		auto positions = GenerateMultipointPositions(center, rs, forward);

		unsigned char mask = (set.right << 0) | (set.left << 1) | (set.top << 2) |
			(set.bottom << 3) | (C_GET(bool, Vars.bmiddle) << 4) | (C_GET(bool, Vars.btop_left) << 5) |
			(set.top_right << 6) | (set.bottom_left << 7) |
			(C_GET(bool, Vars.bbottom_right) << 8);

		for (int i = 0; i < 9; ++i)
		{
			if (mask & (1 << i)) {
				D::WorldToScreen(positions[i], &g_RageBot->v);
				multipointPositions.push_back(positions[i]);
			}
		}
	}

	return multipointPositions;
}

float ragebot::calculate_roll_compensation(float desired_yaw, float desired_pitch, float current_yaw, float current_pitch) {


	float d_yaw = desired_yaw * 0.017453292519943295f;
	float d_pitch = desired_pitch * 0.017453292519943295f;
	float c_yaw = current_yaw * 0.017453292519943295f;
	float c_pitch = current_pitch * 0.017453292519943295f;

	float dx = sinf(d_yaw) * cosf(d_pitch) - sinf(c_yaw) * cosf(c_pitch);
	float dy = -cosf(d_yaw) * cosf(d_pitch) + cosf(c_yaw) * cosf(c_pitch);
	float dz = sinf(d_pitch) - sinf(c_pitch);

	return atan2f(dx * dy, dx * dx - dy * dy) * 57.2957795131f;


}

void ragebot::autostop() {
	C_CSWeaponBase* weapon = SDK::ActiveWeapon;
	CBaseUserCmdPB* cmd = SDK::Cmd->csgoUserCmd.pBaseCmd;

	if (C_GET(int, Vars.iAutoStopMode) == 0) {
		cmd->flForwardMove = 1.f;
		cmd->flSideMove = 0.f;

		Vector_t* move = (Vector_t*)&cmd->flForwardMove;
		Vector_t move_backup = *move;

		Vector_t abs_origin = SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin();
		QAngle_t angle = g_Utils->CalcAngles(abs_origin, MATH::ExtrapolatePosition(abs_origin, SDK::LocalPawn->GetAbsVelocity() * -5.f, 1.f));

		const float delta = remainderf(angle.y - g_AntiAim->StoreAngels.y, 360.f);
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

		cmd->flForwardMove = move->x * mul;
		cmd->flSideMove = move->y * mul;
	}
	else {

		//weapon->update_accuracy();// dont do this.
		float inaccuracy = weapon->get_inaccuracyHitChance();
		float spread = weapon->get_spread();// g_PredictionSystem->GetSpread();// weapon->get_spread();

		float max_allowed_speed = [&]() {
			const auto weapon_type = SDK::ActiveWeaponVData->GetWeaponType();

			if (inaccuracy > 0.05f) {
				return 7.5f;
			}

			switch (weapon_type) {
			case WEAPONTYPE_SNIPER_RIFLE:
				return 10.f;
			case WEAPONTYPE_PISTOL:
				return 10.f;
			case WEAPONTYPE_SUBMACHINEGUN:
				return 10.f;
			default:
				return 10.f;
			}
			}();

		Vector_t velocity = SDK::LocalPawn->GetAbsVelocity();
		float current_speed = velocity.Length2D();

		if (current_speed > max_allowed_speed) {
			float speed_ratio = max_allowed_speed / (current_speed + 1.f);
			cmd->flForwardMove *= speed_ratio;
			cmd->flSideMove *= speed_ratio;
		}
	}

	cmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
	cmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
}

unsigned int get_hash_seeds(C_CSPlayerPawn* player, QAngle_t angle, int attack_tick)
{
	return MEM::GetHashSeed(player, angle, attack_tick);
}

bool ragebot::hitchance(Vector_t aim_point, QAngle_t angle, float hitbox_rad, C_CSWeaponBase* weapon) {


	Vector_t start = IPT::IsPressed(C_GET(int, Vars.iDuckPeekKey), C_GET(int, Vars.iDuckPeekKeyBind)) ? DUCKPEEKASSIST_EYEPOS : SDK::EyePos;
	float distance = start.DistTo(aim_point);
	weapon->update_accuracy();
	float inaccuracy = weapon->get_inaccuracyHitChance();
	float spread = weapon->get_spread();// g_PredictionSystem->GetSpread();// weapon->get_spread();
	auto recoil_index = weapon->m_iRecoilIndex();
	if (SDK::LocalPawn->IsScoped())
	    if (g_RageBot->weapon_is_at_max_accuracy(weapon->GetWeaponVData(), inaccuracy))
		     return 80;

	Vector_t fwd, right, up;
	Vector_t s = { angle.x,angle.y,angle.z };
	MATH::angle_vectors(s, &fwd, &right, &up);
	angle = { s.x,s.y,s.z };
	const int total_seeds = 256;

	int required_hits;

	if (!IPT::IsPressed(C_GET(int, Vars.bKeyHitchance), C_GET(int, Vars.bKeyBindHitchance)))
	{
		required_hits = 0;
	}
	else
	{
		required_hits = std::round((C_GET(int, Vars.rage_minimum_hitchance) / 100.0) * total_seeds);
	}

	int remaining_shots = total_seeds;
	int hits = 0;

	for (int i = 0; i < total_seeds; ++i)
	{

		Vector_t wep_spread = calculate_spread_internal(i,
			inaccuracy,
			spread,
			recoil_index,
			SDK::ItemDefIdx);
		Vector_t dir = fwd + (right * wep_spread.x) + (up * wep_spread.y);

		dir.normalize_inplace();

		Vector_t end = start + dir * distance;



		if (end.DistTo(aim_point) <= hitbox_rad)
		{

			if (++hits >= required_hits)
				return true;
		}

		if (--remaining_shots + hits < required_hits)
			return false;
	}
	return hits >= required_hits;
}

bool no_spread(CUserCmd* cmd, Vector_t& pos, C_CSPlayerPawn* player, int index)
{
	C_CSWeaponBase* weapon = SDK::LocalPawn->GetWeaponActive();

	auto weapon_data = weapon->GetWeaponVData();
	
	if (!weapon_data)
		return false;

	auto recoil_index = weapon->m_iRecoilIndex();

	weapon->update_accuracy();
	auto accuracy = weapon->get_inaccuracyHitChance();// g_PredictionSystem->GetAccuracy();;
	auto spread = weapon->get_spread();// g_PredictionSystem->GetSpread();
	if (g_RageBot->weapon_is_at_max_accuracy(weapon_data, accuracy))
		return true;
	Vector_t dir{}, end{}, fwd{}, right{}, up{};

	GameTrace_t trace{};

	TraceFilter_t filter(0x1C3003, SDK::LocalPawn, nullptr, 4);

	int hits{};
	float range = weapon_data->m_flRange();

	g_Utils->angle_vectors(pos, &fwd, &right, &up);
	auto spread_calc = ragebot::calculate_spread_internal(get_hash_seeds(SDK::LocalPawn, { pos.x,pos.y,pos.z},SDK::prediction_tick +1), accuracy, spread, recoil_index, SDK::ItemDefIdx);

	//auto spread_calc = calculate_spread(get_hash_seed(SDK::LocalPawn, best_point, SDK::prediction_tick + 1), weapon->get_weapon_data()->bullets(), acc, spread, weapon_index, recoil_index, weapon);

	dir = Vector_t(fwd + (right * spread_calc.x) + (up * spread_calc.y));

	end = SDK::EyePos + dir * range;

	Ray_t ray{};
	I::GameTraceManager->ClipRayToEntity(&ray, SDK::EyePos, end, player, &filter, &trace);
	if (trace.m_pHitEntity == player /*&& trace.m_pHitboxData->m_nHitboxId == index*/)
		return true;

	return false;
}

//int HitChanceNSa(QAngle_t& best_point, Vector_t start, C_CSPlayerPawn* player, C_CSWeaponBase* weapon, C_CSPlayerPawn* local)
//{
//	auto weapon_data = weapon->GetWeaponVData();
//	if (!weapon_data)
//		return false;
//
//	weapon = local->GetWeaponActive();
//
//	auto recoil_index = weapon->m_iRecoilIndex();
//
//	Vector_t dir{};
//	Vector_t end{};
//	Vector_t fwd{};
//	Vector_t right{};
//	Vector_t up{};
//
//	int hits{};
//	int aw_hits{};
//	float range = weapon_data->m_flRange();
//	weapon->update_accuracy();
//	auto accuracy = weapon->get_inaccuracyHitChance();
//	auto spread = weapon->get_spread();
//
//	auto EconItemView = weapon->GetEconItemView();
//	auto weapon_index = EconItemView->GetItemDefinitionIndex();
//	g_Utils->angle_vectors(best_point, &fwd, &right, &up);
//
//	if (g_RageBot->weapon_is_at_max_accuracy(weapon_data, accuracy))
//		return 100;
//
//	TraceFilter_t filter(0x1C3003, local, nullptr, 4);
//
//	Ray_t ray{};
//	game_trace_t trace{};
//
//	for (int i = 0; i < 256; i++)
//	{
//		//auto spread_calc = CalculateSpread(weapon, i, accuracy, spread);
//		auto spread_calc = ragebot::calculate_spread_internal(get_hash_seeds(local, best_point, SDK::prediction_tick),accuracy, spread, recoil_index, SDK::ItemDefIdx);
//
//		// get spread direction.
//		dir = Vector_t(fwd - (right * spread_calc.x) + (up * spread_calc.y));
//
//		end = local->GetEyePosition() + dir * range;
//
//		I::GameTraceManager->ClipRayToEntity(&ray, local->GetEyePosition(), end, player, &filter, &trace);
//
//		if (trace.HitEntity == player)
//			hits++;
//	}
//
//	return static_cast<int>((static_cast<float>(hits / 256)) * 100.f);
//}

void ragebot::run() {
	if (!SDK::ActiveWeapon->GetClip1() || !C_GET(bool, Vars.rage_enable))
		return;
	if (SDK::LocalPawn->GetWeaponActive()->IsNade() || SDK::LocalPawn->GetWeaponActive()->IsInReload())
		return;

	if (SDK::LocalPawn->GetWeaponActive()->IsKnife())
		return;
	if (!g_RageBot->can_shoot())
		return;

	DUCKPEEKASSIST_EYEPOS = SDK::EyePos;
	if (IPT::IsPressed(C_GET(int, Vars.iDuckPeekKey), C_GET(int, Vars.iDuckPeekKeyBind))) {
		DUCKPEEKASSIST_EYEPOS.z = SDK::EyePos.z + 18.f;
	}

	if (!SDK::ActiveWeaponVData->IsFullAuto()) {
		if (SDK::ActiveWeapon->GetNextPrimaryAttackTick() > I::GlobalVars->tickcount) {
			return;
		}
	}

	scan::target_info best_info{};
	auto local_ent_base = I::GameResourceService->pGameEntitySystem->Get(SDK::LocalPawn->GetRefEHandle());

	std::vector<std::future<void>> futures;

	auto players_snapshot = EntityCache::g_entity_cache->get_players_snapshot();
	futures.reserve(players_snapshot.size());

	for (const auto& player : players_snapshot) {
		futures.push_back(std::async(std::launch::async, [&]() {
			scan::process_player(const_cast<EntityCache::cached_entity&>(player), SDK::LocalPawn, local_ent_base, SDK::Cmd, best_info);
			}));
	}

	for (auto& future : futures) {
		future.get();
	}
	if (best_info.damage > 0.f) {
		if (C_GET(bool, Vars.bAutoStop))
			autostop();
		if (IPT::IsPressed(C_GET(int, Vars.iDuckPeekKey), C_GET(int, Vars.iDuckPeekKeyBind))) {
			auto* subtick = SDK::Cmd->CreateSubtick();
			subtick->flAnalogForwardDelta = 0.f;
			subtick->flAnalogLeftDelta = 0.f;
			subtick->nButton = IN_DUCK;
			subtick->bPressed = false;
			subtick->flWhen = 0.999f;
			subtick->nCachedBits |= 4;
			subtick->nCachedBits |= 1;
			subtick->nCachedBits |= 2;

			SDK::Cmd->nButtons.nValue &= ~IN_DUCK;
			SDK::Cmd->nButtons.nValueChanged &= ~IN_DUCK;
			SDK::Cmd->nButtons.nValueScroll &= ~IN_DUCK;
		}

		if (IPT::IsPressed(C_GET(int, Vars.iDuckPeekKey), C_GET(int, Vars.iDuckPeekKeyBind)) && SDK::LocalPawn->MovementServices()->IsDucked()) {
			L_PRINT(LOG_INFO) << (" ducking. returning ... ");
			return;
		}
		if (C_GET(bool, Vars.bAutoScope)) {

			bool can_scope = SDK::ActiveWeapon->GetZoomLevel() == 0 &&
				SDK::ActiveWeaponVData->GetWeaponType() == WEAPONTYPE_SNIPER_RIFLE;
			if (can_scope) {
				SDK::Cmd->nButtons.nValue |= IN_SECOND_ATTACK;
			}
		}

		//QAngle_t		  = SDK::Cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue;
		//Vector_t to_enemy = best_info.point - SDK::EyePos;

		//float yaw = std::remainder(atan2f(to_enemy.y, to_enemy.x) * 57.2957795131f - view_angle.y, 360.0f);
		//float pitch = std::remainder(-atan2f(to_enemy.z, std::hypot(to_enemy.x, to_enemy.y)) * 57.2957795131f - view_angle.x, 360.0f);

		QAngle_t angle = g_Utils->CalcAngles(SDK::EyePos, best_info.point);
		angle = angle - get_removed_aim_punch_angle(SDK::LocalPawn);
		//QAngle_t final_angle(pitch, yaw, 0.0f);
		//view_angle += final_angle;
		////view_angle -= get_removed_aim_punch_angle(SDK::LocalPawn);

		float delta_y = std::abs(SDK::Cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y - angle.y);
		float delta_x = std::abs(SDK::Cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.x - angle.x);
		bool is_uncompensated = false;

		//if (delta_y > 45.f || delta_x > 45.f) {
		//	is_uncompensated = true;
		//	SDK::Cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y =
		//		std::remainder(angle.y + 180.0f, 360.0f);
		//}
		D::WorldToScreen(best_info.point, &g_RageBot->svo);

		if (!IPT::IsPressed(C_GET(int, Vars.bKeyNoSpread), C_GET(int, Vars.bKeyBindNoSpread)))
		{
			if (!hitchance(best_info.point,
				g_Utils->CalcAngles(IPT::IsPressed(C_GET(int, Vars.iDuckPeekKey), C_GET(int, Vars.iDuckPeekKeyBind)) ? DUCKPEEKASSIST_EYEPOS : SDK::EyePos, best_info.point),
				best_info.hitbox_radius,
				SDK::ActiveWeapon))
			{
				return;
			}

		}
		else
		{

			/*if (!HitChanceNSa(angle, SDK::EyePos, best_info.target.pawn, SDK::ActiveWeapon, SDK::LocalPawn))
				return;	*/
			Vector_t NoSpreadAngel = { angle.x,angle.y,angle.z };
			if (!no_spread(SDK::Cmd, NoSpreadAngel, best_info.target.pawn, 0))
			{
				return;
			}
		} 
		{

			if (is_uncompensated) {
				SDK::Cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.x = 179.f;
			}

			if (!C_GET(bool, Vars.bSilent)) {
				SDK::Cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue = angle;
			}
			autostop();
			SDK::Cmd->adjust_input_history(
				SDK::Cmd->GetInputHistoryEntry(0),	
				angle,
				TIME_TO_TICKS(best_info.record.m_simulation_time),
				best_info.target.controller->GetRefEHandle().GetEntryIndex(),
				SDK::EyePos,
				SDK::LocalController->GetTickBase()
			);
			//SDK::Cmd->SetSubTickAngle(angle);
			if (!(SDK::Cmd->nButtons.nValue & IN_ATTACK)) {
				//SDK::Cmd->AddSubtick(true,IN_ATTACK);
				SDK::Cmd->nButtons.nValue |= IN_ATTACK;
				SDK::Cmd->nButtons.nValueScroll |= IN_ATTACK;
				SDK::Cmd->nButtons.nValueChanged |= IN_ATTACK;
				SDK::Cmd->csgoUserCmd.nAttack1StartHistoryIndex = 0;
				SDK::Cmd->csgoUserCmd.CheckAndSetBits(ECSGOUserCmdBits::CSGOUSERCMD_BITS_ATTACK1START);
				//SDK::Cmd->AddSubtick(false, IN_ATTACK);dw
			}
			const double Time = TIMER_END("RageBot");

			if (C_GET(bool, Vars.bHitLogs))
			{
				NOTIFY::Push({ N_TYPE_HIT, std::format("Target: {} | Damage: {:.1f} | Hitbox: {}",
					best_info.target.controller->GetPlayerName(),
					best_info.damage,
					scan::getHitgroupString(best_info.hitgroup)).c_str() });
			};
																									

		}
	}
}
