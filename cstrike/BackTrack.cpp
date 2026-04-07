#include "BackTrack.h"
#include "utilities/draw.h"
#include "sdk/EntityList/EntityList.h"
#include "AimBot.h"

inline QAngle_t CalcAngles(Vector_t viewPos, Vector_t aimPos)
{
	QAngle_t angle = { 0, 0, 0 };

	Vector_t delta = aimPos - viewPos;

	angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
	angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

	return angle;
}
inline float GetFov(const QAngle_t& viewAngle, const QAngle_t& aimAngle)
{
	QAngle_t delta = (aimAngle - viewAngle).Normalize();

	return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}

void LegitBacktrack::OnCreateMove(CUserCmd* cmd)
{
	auto entitass = g_EntityList->get(CS_XOR("CCSPlayerController"));
	std::vector<Vector_t> dataThisTick = {};
	C_CSPlayerPawn* player_pawn;
	for (auto entity : entitass)
	{
		CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(entity);
		if (entity == 0)
			continue;

		if (!controller)
			continue;

		player_pawn = reinterpret_cast<C_CSPlayerPawn*>(I::GameResourceService->pGameEntitySystem->Get(controller->GetPawnHandle().GetEntryIndex()));
		if (!player_pawn || !player_pawn->IsOtherEnemy(SDK::LocalPawn) || player_pawn->GetHealth() <= 0)
			continue;
		dataThisTick.push_back(player_pawn->GetGameSceneNode()->GetBonePosition(6));
	}
	oldPlayerRecords[cmd->csgoUserCmd.pBaseCmd->nClientTick] = dataThisTick;
	// Purge Old Player Position Data
	for (auto i = oldPlayerRecords.begin(); i != oldPlayerRecords.end();)
	{
		const int fromTick = i->first;
		if ((cmd->csgoUserCmd.pBaseCmd->nClientTick - fromTick) * 0.15625 /* g_GlobalVars->interval_per_tick*/ > 0.1 /*g_Options.misc_legitbacktrack_maxtime*/)
		{ // Default For Maxtime is 0.2f (or 200ms) - Attention c+p'ers: 200ms is the MAX unless you have fake lag.
			oldPlayerRecords.erase(i++->first);
		}
		else
		{
			++i;
		}
	}
	// Iterate Through Old Player Position Data If Firing
	if (cmd->nButtons.nValue & IN_ATTACK)
	{
		bool foundHittableTick = false;
		for (auto i = oldPlayerRecords.begin(); i != oldPlayerRecords.end(); i++)
		{
			const int recordTick = i->first;
			for (int record = 0; record < i->second.size(); record++)
			{
				QAngle_t goal = CalcAngles(SDK::LocalPawn->GetEyePosition(), i->second[record]) - SDK::LocalPawn->GetAimPuchAngle() * 2.0f;
				if (GetFov(cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue, goal) < 3.5f)
				{ // This is a simple FOV check to see if you can hit on the recorded tick. 3.5f is not a calculated value and could be inaccurate.
					L_PRINT(LOG_INFO) << "1 " << cmd->csgoUserCmd.pBaseCmd->nClientTick;
					cmd->csgoUserCmd.pBaseCmd->nClientTick -= 3;
					L_PRINT(LOG_INFO) << "2 " << cmd->csgoUserCmd.pBaseCmd->nClientTick;

					foundHittableTick = true;
					break;
				}
			}
			if (foundHittableTick)
			{
				break;
			}
		}
	}
}


//void LegitBacktrack::OnCreateMove(CUserCmd* cmd)
//{
//	if (!SDK::LocalPawn || !SDK::LocalPawn->IsAlive())
//		return;
//
//	// Add New Player Position Data
//	std::vector<Vector_t> dataThisTick = {};
//	for (int i = 1; i < I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex(); i++)
//	{
//		C_CSPlayerPawn* tempPlayer = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(i);
//		if (!tempPlayer /*|| tempPlayer->IsDormant()*/ /*|| !tempPlayer->IsAlive() || tempPlayer->GetRefEHandle().GetEntryIndex() == SDK::LocalPawn->GetRefEHandle().GetEntryIndex()*/)
//			continue;
//		if (/*g_Options.misc_legitbacktrack_teamcheck &&*/ tempPlayer->GetTeam() == SDK::LocalPawn->GetTeam())
//			continue;
//		// Record X Hitbox Positions - Add More At Will (Remember All Hitboxes Will Render A Dot)
//		dataThisTick.push_back(tempPlayer->GetGameSceneNode()->GetBonePosition(6));
//		// dataThisTick.push_back(tempPlayer->GetHitboxPos(HITBOX_UPPER_CHEST));
//	}
//	oldPlayerRecords[cmd->csgoUserCmd.pBaseCmd->nClientTick] = dataThisTick;
//	// Purge Old Player Position Data
//	for (auto i = oldPlayerRecords.begin(); i != oldPlayerRecords.end();)
//	{
//		const int fromTick = i->first;
//		if ((cmd->csgoUserCmd.pBaseCmd->nClientTick - fromTick) * 0.15625 /* g_GlobalVars->interval_per_tick*/ > 0.2f /*g_Options.misc_legitbacktrack_maxtime*/)
//		{ // Default For Maxtime is 0.2f (or 200ms) - Attention c+p'ers: 200ms is the MAX unless you have fake lag.
//			oldPlayerRecords.erase(i++->first);
//		}
//		else
//		{
//			++i;
//		}
//	}
//	// Iterate Through Old Player Position Data If Firing
//	if (cmd->nButtons.nValue & IN_ATTACK)
//	{
//		bool foundHittableTick = false;
//		for (auto i = oldPlayerRecords.begin(); i != oldPlayerRecords.end(); i++)
//		{
//			const int recordTick = i->first;
//			for (int record = 0; record < i->second.size(); record++)
//			{
//				QAngle_t goal = CalcAngles(SDK::LocalPawn->GetEyePosition(), i->second[record]) - SDK::LocalPawn->GetAimPuchAngle() * 2.0f;
//				if (GetFov(cmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue, goal) < 3.5f)
//				{ // This is a simple FOV check to see if you can hit on the recorded tick. 3.5f is not a calculated value and could be inaccurate.
//					L_PRINT(LOG_INFO) << "1 " << recordTick;
//					cmd->csgoUserCmd.pBaseCmd->nClientTick = recordTick;
//					L_PRINT(LOG_INFO) << "2 " << recordTick;
//
//					foundHittableTick = true;
//					break;
//				}
//			}
//			if (foundHittableTick)
//			{
//				break;
//			}
//		}
//	}
//}

void LegitBacktrack::RenderRecords()
{
	if (oldPlayerRecords.empty())
		return;
	// Render Old Position Data (Optional Visual)
	for (auto i = oldPlayerRecords.begin(); i != oldPlayerRecords.end(); i++)
	{
		for (int record = 0; record < i->second.size(); record++)
		{
			Vector_t posToRender = i->second[record];
			//if (g_Options.misc_legitbacktrack_viswallcheck && !g_LocalPlayer->CanSeePlayer(g_LocalPlayer, posToRender))
			//	continue;
			ImVec2 screenVector;	
			if (D::WorldToScreen(posToRender, &screenVector));
			{
				g_RageBot->v = screenVector;
				ImGui::GetBackgroundDrawList()->AddCircle(screenVector, 10, Color_t(255, 255, 255).GetU32(),10);
			}
		}
	}
}
