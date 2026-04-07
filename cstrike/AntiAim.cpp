#include "AntiAim.h"
#include <algorithm>
#include "core/variables.h"
#include "utilities\inputsystem.h"
#include "sdk/EntityList/EntityList.h"

float angle_normalize(float angle)
{
	angle = fmodf(angle, 360.0f);
	if (angle > 180)
	{
		angle -= 360;
	}
	if (angle < -180)
	{
		angle += 360;
	}
	return angle;
}

void CL_AntiAim::run(CUserCmd* cmd)
{
	if (!C_GET(bool, Vars.bAntiAim))
		return;

	if (SDK::LocalPawn->GetMoveType() == MOVETYPE_LADDER || SDK::LocalPawn->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	//if (bFreezTime)
	//	return;

	//if (I::GameRules->GamePhase == 5)
	//	return;

	if (cmd->nButtons.nValue & IN_USE)
		return;

	if (!cmd->csgoUserCmd.pBaseCmd || !cmd->csgoUserCmd.pBaseCmd->pViewAngles)
		return;

	if (SDK::LocalPawn->GetWeaponActive() && SDK::LocalPawn->GetWeaponActive()->IsGrenade())
	{
		if (SDK::LocalController->IsThrowingGrenade(SDK::LocalController->GetPlayerWeapon(SDK::LocalPawn)))
			return;
	}
	else
	{
		//	if (g_cs2->did_shoot && g_ragebot->can_shoot(cmd->csgo_user_cmd.input_history.m_pRep->m_tElements[cmd->csgo_user_cmd.input_history.m_nCurrentSize - 1]->player_tick_count) && !g_user->IsActive(xorstr_("ragebot_hideshots"), xorstr_("ragebot_hideshots"), 0))
			//	return;
	}

	pitch(cmd);

	yaw(cmd);
}

void CL_AntiAim::pitch(CUserCmd* cmd)
{
	if (!C_GET(bool, Vars.bPitch))
		return;

	float jitter_value = C_GET(float, Vars.flJitter);
	static int tick_3_way = 0;

	switch (int(C_GET(int, Vars.iPitchType)))
	{
	case 1:
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.x = 89.f;
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
		break;
	case 2:
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.x = -89.f;
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
		break;
	case 3:
		switch (tick_3_way)
		{
		case 0:
			cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.x -= jitter_value;
			cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
			break;
		case 2:
			cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.x += jitter_value;
			cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
			break;
		}
		++tick_3_way;
		tick_3_way %= 3;
		break;
	}
}

inline void angle_vectors(const QAngle_t& angles, Vector_t& forward)
{
	float sp, sy, cp, cy;

	MATH::sin_cos(MATH::deg2rad(angles[1]), &sy, &cy);
	MATH::sin_cos(MATH::deg2rad(angles[0]), &sp, &cp);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

float get_fov(const QAngle_t& view_angles, const Vector_t& start, const Vector_t& end)
{
	Vector_t dir, fw;

	// get direction and normalize.
	dir = (end - start).Normalized();

	// get the forward direction vector of the view angles.
	angle_vectors(view_angles, fw);

	// get the angle between the view angles forward directional vector and the target location.
	return std::max(MATH::rad2deg(std::acos(fw.dot(dir))), 0.f);
}

void at_target(CUserCmd* cmd)
{
	if (!C_GET(bool, Vars.bAtTarget))
		return;

	float best_dist{ std::numeric_limits<float>::max() };
	float dist;
	C_CSPlayerPawn* target, * best_target{ nullptr };

	auto entitass = g_EntityList->get(CS_XOR("CCSPlayerController"));

	for (auto entity : entitass) {
		CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(entity);
		if (entity == 0 && !controller)
			continue;

		C_CSPlayerPawn* player_pawn = reinterpret_cast<C_CSPlayerPawn*>(I::GameResourceService->pGameEntitySystem->Get(controller->GetPawnHandle().GetEntryIndex()));
		if (!player_pawn || !player_pawn->IsOtherEnemy(SDK::LocalPawn) || player_pawn->GetHealth() <= 0)
			continue;
		int team_num = player_pawn->GetTeam();
		if (SDK::LocalPawn->GetTeam() == team_num)
			continue;

		if (!player_pawn || !player_pawn->IsAlive() || !player_pawn->GetGameSceneNode())
			continue;

		C_CSPlayerPawn* pawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(controller->GetPawnHandle());
		if (!pawn)
			return;

		if (!SDK::LocalPawn->IsOtherEnemy(pawn))
			continue;

		if (pawn->GetHealth() <= 0)
			continue;

		// check if a player was closer to us.
		dist = get_fov(g_AntiAim->StoreAngels, SDK::LocalPawn->GetEyePosition(), pawn->GetGameSceneNode()->GetAbsOrigin());
		if (dist < best_dist)
		{
			best_dist = dist;
			best_target = pawn;
		}
	}

	if (best_target)
	{
		// todo - dex; calculate only the yaw needed for this (if we're not going to use the x component that is).
		Vector_t angle;
		MATH::vector_angles(best_target->GetGameSceneNode()->m_vecOrigin() - SDK::LocalPawn->GetGameSceneNode()->m_vecOrigin(), angle);
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y = angle.y;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
	}
}


void ManualAA(CUserCmd* cmd)
{
	if (IPT::IsPressed(C_GET(int, Vars.iLeftKey), C_GET(int, Vars.iLeftKeyBind)))
	{
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += 90.f;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
	}
	if (IPT::IsPressed(C_GET(int, Vars.iRightKey), C_GET(int, Vars.iRightKeyBind)))
	{
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y -= 90.f;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
	}
}

void CL_AntiAim::yaw(CUserCmd* cmd)
{
	at_target(cmd);
	ManualAA(cmd);
	//cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y = g_AntiAim->HuiAA.y;
	//cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);

	//cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y = angle_normalize(cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y);
	//cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);

	if (C_GET(int, Vars.iYaw) == 1)
	{
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += 180.f;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
	}

	//cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += C_GET(float, Vars.flYawOffset);

	int jitter_type = C_GET(int, Vars.iJitterType);
	float jitter_value = C_GET(float, Vars.flJitter);

	//auto nTickBase = SDK::LocalController->GetTickBase();

	static int tick_3_way = 0;
	static bool invert{};
	static int rotatedYaw{};

	switch (jitter_type)
	{
	case JITTER_TYPE_NONE:
		break;
	case JITTER_TYPE_CENTER:
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += jitter_value * JitterSide;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
		break;
	case JITTER_TYPE_3_WAY:
		switch (tick_3_way)
		{
		case 0:
			cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y -= jitter_value;
			cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
			break;
		case 2:
			cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += jitter_value;
			cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
			break;
		}
		++tick_3_way;
		tick_3_way %= 3;
		break;
	case JITTER_TYPE_RANDOM:
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += (rand() % (int)jitter_value) * JitterSide;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
		break;
	case JITTER_TYPE_SPIN:
		rotatedYaw += jitter_value;
		rotatedYaw = std::remainderf(rotatedYaw, 360.f);
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += rotatedYaw;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
		break;
	case JITTER_TYPE_DANCE:
	{
		float yaw = cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y;
		yaw += C_GET(int, Vars.iYaw);
		m_should_invert ^= 1;
		yaw += jitter_value * (m_should_invert ? 0.5f : -0.5f);
		cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y += yaw;
		cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
		break;
	}
	}
	//JitterSide = -JitterSide;

	//cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y = angle_normalize(cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue.y);
	//cmd->csgoUserCmd.pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);

	//}
	
}