	#include "movement.h"

// used: sdk entity
#include "../../sdk/entity.h"
// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: convars
#include "../../core/convars.h"
#include "../../sdk/interfaces/ienginecvar.h"

// used: cheat variables
#include "../../core/variables.h"
#include <algorithm>
#include "../../core/sdk.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../utilities/draw.h"
#include "../../sdk/interfaces/cgametracemanager.h"
#include "../../AntiAim.h"

using namespace MATH;
#define INTERVAL_PER_TICK 0, 015625
// movement correction angles
static QAngle_t angCorrectionView = {};

void F::MISC::MOVEMENT::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!pLocalController->IsPawnAlive())
		return;

	if (!SDK::Alive)
		return;

	// check if player is in noclip or on ladder or in water
	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	BunnyHop(pCmd, pBaseCmd, pLocalPawn);
	AutoStrafe(pCmd, pBaseCmd,pLocalPawn);
	if (pCmd->csgoUserCmd.inputHistoryField.pRep == nullptr)
		return;
	// loop through all tick commands
	for (int nSubTick = 0; nSubTick < pCmd->csgoUserCmd.inputHistoryField.pRep->nAllocatedSize; nSubTick++)
	{
		CCSGOInputHistoryEntryPB* pInputEntry = pCmd->GetInputHistoryEntry(nSubTick);
		if (pInputEntry == nullptr)
			continue;

		// save view angles for movement correction
		angCorrectionView = pInputEntry->pViewAngles->angValue;

		// movement correction & anti-untrusted
		ValidateUserCommand(pCmd, pBaseCmd, pInputEntry);
	}
}

void F::MISC::MOVEMENT::BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bAutoBHop) || CONVAR::sv_autobunnyhopping->value.i1)
		return;
	// 1794 - noclip, 2313 - ladder
	if (!SDK::LocalPawn || !SDK::LocalPawn->IsAlive() || SDK::LocalPawn->GetMoveType() == 1794 || SDK::LocalPawn->GetMoveType() == 2313)
		return;

	auto base_cmd = pUserCmd;
	if (!base_cmd)
		return;
	if (SDK::LocalPawn->GetFlags() & FL_ONGROUND) {
		pCmd->nButtons.nValue &= ~IN_JUMP;


	}

	//const auto move_type = pLocalPawn->GetMoveType();
	//const auto in_water = pLocalPawn->GetWaterLevel() >= 2.0f;

	//// Check if the player can jump (not in water or special move types)
	//const bool can_jump = !in_water && move_type != 10/*MoveType_t::MOVETYPE_OBSERVER*/;

	//// If we can't jump or the jump key isn't pressed, skip
	//if (!can_jump || !(pCmd->nButtons.nValue & IN_JUMP || pCmd->nButtons.nValueScroll & IN_JUMP)) {
	//	return;
	//}

	//pCmd->nButtons.nValue &= ~IN_JUMP;
	//pCmd->nButtons.nValueScroll &= ~IN_JUMP;
	//pCmd->nButtons.nValueChanged &= ~IN_JUMP;

	//// Only process if the player is on the ground
	//if (!(pLocalPawn->GetFlags() & FL_ONGROUND)) {
	//	return;
	//}

	//pCmd->nButtons.nValue |= IN_JUMP;
	//pCmd->nButtons.nValueScroll |= IN_JUMP;

	//// Define how many subticks to split the jump into
	//const int max_ticks = 12; //experiment

	//// Distribute the subticks across the frame
	//for (int i = 0; i < max_ticks; ++i)
	//{
	//	auto subtick = pCmd->CreateSubtick();

	//	// Set the timing for the subtick move (even distribution)
	//	subtick->flWhen = static_cast<float>(i) / static_cast<float>(max_ticks);

	//	if (i == 0) // First subtick: Press the jump button
	//	{
	//		subtick->nButton = IN_JUMP;
	//		subtick->bPressed = true;
	//	}
	//	else if (i == max_ticks - 1) // Last subtick: Release the jump button
	//	{
	//		subtick->nButton = IN_JUMP;
	//		subtick->bPressed = false;
	//	}
	//}
}

void F::MISC::MOVEMENT::JumpBug(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bJumpBug))
		return;

	if (pCmd->nButtons.nValue & IN_JUMP && SDK::LocalPawn->GetFlags() & FL_ONGROUND)
	{
		pCmd->nButtons.nValue |= IN_DUCK;
		pCmd->nButtons.nValue &= ~IN_JUMP;
	}
}

void F::MISC::MOVEMENT::AutoStrafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return;

	if (!SDK::LocalPawn || !SDK::LocalController->IsPawnAlive())
		return;

	if (pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	if (pLocalPawn->GetFlags() & FL_ONGROUND)
		return;
	// 1794 - noclip, 2313 - ladder
	if (!SDK::LocalPawn || !SDK::LocalPawn->IsAlive() || SDK::LocalPawn->GetMoveType() == 1794 || SDK::LocalPawn->GetMoveType() == 2313)
		return;

	static uint64_t last_pressed = 0;
	static uint64_t last_buttons = 0;

	if (!C_GET(bool, Vars.bAutoStrafe))
		return;

	const auto current_buttons = pCmd->nButtons.nValue;
	float yaw{};
	if (pUserCmd->pViewAngles)
	    yaw = MATH::normalize_yaw(pUserCmd->pViewAngles->angValue.y)/*pUserCmd->pViewAngles->angValue.y)*/;

	const auto check_button = [&](const uint64_t button)
	{
		if (current_buttons & button && (!(last_buttons & button) || button & IN_MOVELEFT && !(last_pressed & IN_MOVERIGHT) || button & IN_MOVERIGHT && !(last_pressed & IN_MOVELEFT) || button & IN_FORWARD && !(last_pressed & IN_BACK) || button & IN_BACK && !(last_pressed & IN_FORWARD)))
		{
			if (button & IN_MOVELEFT)
				last_pressed &= ~IN_MOVERIGHT;
			else if (button & IN_MOVERIGHT)
				last_pressed &= ~IN_MOVELEFT;
			else if (button & IN_FORWARD)
				last_pressed &= ~IN_BACK;
			else if (button & IN_BACK)
				last_pressed &= ~IN_FORWARD;

			last_pressed |= button;
		}
		else if (!(current_buttons & button))
			last_pressed &= ~button;
	};

	check_button(IN_MOVELEFT);
	check_button(IN_MOVERIGHT);
	check_button(IN_FORWARD);
	check_button(IN_BACK);

	last_buttons = current_buttons;

	const auto velocity = pLocalPawn->GetAbsVelocity();

	auto offset = 0.f;
	if (last_pressed & IN_MOVELEFT)
		offset += 90.f;
	if (last_pressed & IN_MOVERIGHT)
		offset -= 90.f;
	if (last_pressed & IN_FORWARD)
		offset *= 0.5f;
	else if (last_pressed & IN_BACK)
		offset = -offset * 0.5f + 180.f;

	yaw += offset;

	auto velocity_angle = M_RAD2DEG(std::atan2f(velocity.y, velocity.x));
	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	velocity_angle -= floorf(velocity_angle / 360.0f + 0.5f) * 360.0f;

	const auto speed = velocity.Length2D();
	auto ideal = 0.f;

	if (speed > 0.f)
		ideal = MATH::Clamp(M_RAD2DEG(std::atan2(15.f, speed)), 0.0f, 90.0f);

	auto correct = (100.f - C_GET(float, Vars.flStrafeSmoothing)) * 0.02f * (ideal + ideal);

	pUserCmd->flForwardMove = 0.f;
	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
	const auto velocity_delta = MATH::normalize_yaw(yaw - velocity_angle);

	auto rotate_movement = [](CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, float target_yaw)
	{
      const float rot = M_DEG2RAD(pUserCmd->pViewAngles->angValue.y - target_yaw);

      const float new_forward = std::cos(rot) * pUserCmd->flForwardMove - std::sin(rot) * pUserCmd->flSideMove;
      const float new_side = std::sin(rot) * pUserCmd->flForwardMove + std::cos(rot) * pUserCmd->flSideMove;

      pCmd->nButtons.nValue &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);
      pCmd->nButtons.nValueChanged &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

      pUserCmd->flForwardMove = MATH::Clamp(new_forward, -1.f, 1.f);
      pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
      pUserCmd->flSideMove = MATH::Clamp(new_side * -1.f, -1.f, 1.f);
      pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);

      if (pUserCmd->flForwardMove > 0.0f)
      {
        //Interfaces::csgo_input->add_button(IN_FORWARD);
        pUserCmd->pInButtonState->nValue |= IN_FORWARD;
        pUserCmd->pInButtonState->SetBits(IN_FORWARD);
      }
      else if (pUserCmd->flForwardMove < 0.0f)
      {
        //Interfaces::csgo_input->add_button(IN_BACK);
        pUserCmd->pInButtonState->nValue |= IN_BACK;
        pUserCmd->pInButtonState->SetBits(IN_BACK);
      }
		if (pUserCmd->flSideMove > 0.0f)
		{
			//Interfaces::csgo_input->add_button(IN_MOVELEFT);
			pUserCmd->pInButtonState->nValue |= IN_MOVELEFT;
			pUserCmd->pInButtonState->SetBits(IN_MOVELEFT);
		}
		else if (pUserCmd->flSideMove < 0.0f)
		{
			//Interfaces::csgo_input->add_button(IN_MOVERIGHT);
			pUserCmd->pInButtonState->nValue |= IN_MOVERIGHT;
			pUserCmd->pInButtonState->SetBits(IN_MOVERIGHT);
		}
	};

	if (fabsf(velocity_delta) > 170.0f && speed > 80.0f || velocity_delta > correct && speed > 80.0f)
	{
		yaw = correct + velocity_angle;
		pUserCmd->flSideMove = -1.0f;
		pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
		rotate_movement(pCmd, pUserCmd, MATH::normalize_yaw(yaw));
		return;
	}
	static bool m_switch_value;
	m_switch_value = !m_switch_value;

	if (-correct <= velocity_delta || speed <= 80.f)
	{
		if (m_switch_value)
		{
			yaw = yaw - ideal;
			pUserCmd->flSideMove = -1.0f;
			pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
		}
		else
		{
			yaw = ideal + yaw;
			pUserCmd->flSideMove = 1.0f;
			pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
		}
	}
	else
	{
		yaw = velocity_angle - correct;
		pUserCmd->flSideMove = 1.0f;
		pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
	}

	rotate_movement(pCmd, pUserCmd, MATH::normalize_yaw(yaw));
	if (!C_GET(bool, Vars.bSubtickStrafe))
		return;

	I::Input->flForwardMove = pUserCmd->flForwardMove;
	I::Input->flSideMove = pUserCmd->flSideMove;
	if (!pUserCmd->subtickMovesField.pRep)
		return;
	for (auto i = 0; i < pUserCmd->subtickMovesField.pRep->nAllocatedSize; i++)
	{
		auto Subtick = pUserCmd->subtickMovesField.pRep->tElements[i];
		
		Subtick->flAnalogForwardDelta = pUserCmd->flForwardMove;
		Subtick->flAnalogLeftDelta = pUserCmd->flSideMove;

		Subtick->nButton |= IN_SPRINT;
		Subtick->SetBits(IN_SPRINT);

		Subtick->flWhen = (1.f / 12.f) * (i);
	}

	CSubtickMoveStep* Subtick = pCmd->CreateSubtick();

	Subtick->flAnalogForwardDelta = pUserCmd->flForwardMove;
	Subtick->flAnalogLeftDelta = pUserCmd->flSideMove;

	Subtick->nButton = pUserCmd->pInButtonState->nValue;
	Subtick->SetBits(BUTTON_STATE_PB_BITS_BUTTONSTATE1);
	Subtick->flWhen = (1.f / 12.f) * pUserCmd->subtickMovesField.pRep->nAllocatedSize;
	
}

void F::MISC::MOVEMENT::ValidateUserCommand(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry)
{
	if (pUserCmd == nullptr)
		return;

	// clamp angle to avoid untrusted angle
	if (C_GET(bool, Vars.bAntiUntrusted))
	{
		pInputEntry->SetBits(EInputHistoryBits::INPUT_HISTORY_BITS_VIEWANGLES);
		if (pInputEntry->pViewAngles->angValue.IsValid())
		{
			pInputEntry->pViewAngles->angValue.Clamp();
			pInputEntry->pViewAngles->angValue.z = 0.f;
		}
		else
		{
			pInputEntry->pViewAngles->angValue = {};
			L_PRINT(LOG_WARNING) << CS_XOR("view angles have a NaN component, the value is reset");
		}
	}

	//MovementCorrection(pUserCmd, pInputEntry, angCorrectionView);

	// correct movement buttons while player move have different to buttons values
	// clear all of the move buttons states
	pUserCmd->pInButtonState->SetBits(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE1);
	pUserCmd->pInButtonState->nValue &= (~IN_FORWARD | ~IN_BACK | ~IN_LEFT | ~IN_RIGHT);

	// re-store buttons by active forward/side moves
	if (pUserCmd->flForwardMove > 0.0f)
		pUserCmd->pInButtonState->nValue |= IN_FORWARD;
	else if (pUserCmd->flForwardMove < 0.0f)
		pUserCmd->pInButtonState->nValue |= IN_BACK;

	if (pUserCmd->flSideMove > 0.0f)
		pUserCmd->pInButtonState->nValue |= IN_RIGHT;
	else if (pUserCmd->flSideMove < 0.0f)
		pUserCmd->pInButtonState->nValue |= IN_LEFT;

	if (!pInputEntry->pViewAngles->angValue.IsZero())
	{
		const float flDeltaX = std::remainderf(pInputEntry->pViewAngles->angValue.x - angCorrectionView.x, 360.f);
		const float flDeltaY = std::remainderf(pInputEntry->pViewAngles->angValue.y - angCorrectionView.y, 360.f);

		float flPitch = CONVAR::m_pitch->value.fl;
		float flYaw = CONVAR::m_yaw->value.fl;

		float flSensitivity = CONVAR::sensitivity->value.fl;
		if (flSensitivity == 0.0f)
			flSensitivity = 1.0f;

		pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_MOUSEDX);
		pUserCmd->nMousedX = static_cast<short>(flDeltaX / (flSensitivity * flPitch));

		pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_MOUSEDY);
		pUserCmd->nMousedY = static_cast<short>(-flDeltaY / (flSensitivity * flYaw));
	}
}

void AngleQangles(const QAngle_t& angles, QAngle_t* forward, QAngle_t* right, QAngle_t* up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;

	// Convert angles from degrees to radians
	angle = angles.y * (MATH::_PI / 180.0);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles.x * (MATH::_PI / 180.0);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles.z * (MATH::_PI / 180.0);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}

void F::MISC::MOVEMENT::movment_fix(CUserCmd* pCmd, QAngle_t angle)
{
	Vector2D_t vecMove = Vector2D_t(pCmd->csgoUserCmd.pBaseCmd->flForwardMove, pCmd->csgoUserCmd.pBaseCmd->flSideMove);
	Vector2D_t vecMoveBackup = vecMove;

	if (!pCmd->csgoUserCmd.pBaseCmd->pViewAngles)
		return;

	const QAngle_t& angCurrentAngle = pCmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue;
	const float flDelta = remainderf(angle.y - angCurrentAngle.y, 360.f);
	const float flYaw = M_DEG2RAD(flDelta);

	vecMove.x = vecMoveBackup.x * std::cos(flYaw) - vecMoveBackup.y * std::sin(flYaw);
	vecMove.y = vecMoveBackup.x * std::sin(flYaw) + vecMoveBackup.y * std::cos(flYaw);

	float flMultiply = 1.f;
	if (std::fabsf(vecMove.x) > 1.f)
		flMultiply = 1.f / std::fabsf(vecMove.x);
	else if (std::fabsf(vecMove.y) > 1.f)
		flMultiply = 1.f / std::fabsf(vecMove.y);

	vecMove.x *= flMultiply;
	vecMove.y *= flMultiply;

	I::Input->flForwardMove = vecMove.x;
	I::Input->flSideMove = -vecMove.y;

	pCmd->csgoUserCmd.pBaseCmd->SetBits(0x20 | 0x40);
	pCmd->csgoUserCmd.pBaseCmd->flForwardMove = vecMove.x;
	pCmd->csgoUserCmd.pBaseCmd->flSideMove = vecMove.y;
}
//	if (!SDK::Alive)
//		return;
//
//	QAngle_t wish_angle;
//	wish_angle = pCmd->csgoUserCmd.pBaseCmd->pViewAngles->angValue;
//
//	int revers = wish_angle.x > 89.f ? -1 : 1;
//	wish_angle.Clamp();
//
//	QAngle_t view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
//	auto viewangles = angle;
//
//	AngleQangles(wish_angle, &view_fwd, &view_right, &view_up);
//	AngleQangles(viewangles, &cmd_fwd, &cmd_right, &cmd_up);
//
//	const float v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
//	const float v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
//	const float v12 = sqrtf(view_up.z * view_up.z);
//
//	const Vector_t norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
//	const Vector_t norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
//	const Vector_t norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);
//
//	const float v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
//	const float v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
//	const float v18 = sqrtf(cmd_up.z * cmd_up.z);
//
//	const Vector_t norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
//	const Vector_t norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
//	const Vector_t norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);
//
//	const float v22 = norm_view_fwd.x * pCmd->csgoUserCmd.pBaseCmd->flForwardMove;
//	const float v26 = norm_view_fwd.y * pCmd->csgoUserCmd.pBaseCmd->flForwardMove;
//	const float v28 = norm_view_fwd.z * pCmd->csgoUserCmd.pBaseCmd->flForwardMove;
//	const float v24 = norm_view_right.x * pCmd->csgoUserCmd.pBaseCmd->flSideMove;
//	const float v23 = norm_view_right.y * pCmd->csgoUserCmd.pBaseCmd->flSideMove;
//	const float v25 = norm_view_right.z * pCmd->csgoUserCmd.pBaseCmd->flSideMove;
//	const float v30 = norm_view_up.x * pCmd->csgoUserCmd.pBaseCmd->flUpMove;
//	const float v27 = norm_view_up.z * pCmd->csgoUserCmd.pBaseCmd->flUpMove;
//	const float v29 = norm_view_up.y * pCmd->csgoUserCmd.pBaseCmd->flUpMove;
//
//	pCmd->csgoUserCmd.pBaseCmd->flForwardMove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25)) + (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28))) + (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
//	pCmd->csgoUserCmd.pBaseCmd->flSideMove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25)) + (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28))) + (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
//	pCmd->csgoUserCmd.pBaseCmd->flUpMove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25)) + (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28))) + (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));
//
//	pCmd->csgoUserCmd.pBaseCmd->flForwardMove = revers * ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25)) + (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)));
//
//	pCmd->csgoUserCmd.pBaseCmd->flSideMove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25)) + (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)));
//
//	pCmd->csgoUserCmd.pBaseCmd->flForwardMove = std::clamp(pCmd->csgoUserCmd.pBaseCmd->flForwardMove, -1.f, 1.f);
//
//	pCmd->csgoUserCmd.pBaseCmd->flSideMove = std::clamp(pCmd->csgoUserCmd.pBaseCmd->flSideMove, -1.f, 1.f);
//}}*/



void F::MISC::MOVEMENT::MovementCorrection(CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry, const QAngle_t& angDesiredViewPoint)
{
	if (pUserCmd == nullptr)
		return;

	Vector_t vecForward = {}, vecRight = {}, vecUp = {};
	angDesiredViewPoint.ToDirections(&vecForward, &vecRight, &vecUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecForward.z = vecRight.z = vecUp.x = vecUp.y = 0.0f;

	vecForward.NormalizeInPlace();
	vecRight.NormalizeInPlace();
	vecUp.NormalizeInPlace();

	Vector_t vecOldForward = {}, vecOldRight = {}, vecOldUp = {};
	pInputEntry->pViewAngles->angValue.ToDirections(&vecOldForward, &vecOldRight, &vecOldUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecOldForward.z = vecOldRight.z = vecOldUp.x = vecOldUp.y = 0.0f;

	vecOldForward.NormalizeInPlace();
	vecOldRight.NormalizeInPlace();
	vecOldUp.NormalizeInPlace();

	const float flPitchForward = vecForward.x * pUserCmd->flForwardMove;
	const float flYawForward = vecForward.y * pUserCmd->flForwardMove;
	const float flPitchSide = vecRight.x * pUserCmd->flSideMove;
	const float flYawSide = vecRight.y * pUserCmd->flSideMove;
	const float flRollUp = vecUp.z * pUserCmd->flUpMove;

	// solve corrected movement speed
	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
	pUserCmd->flForwardMove = vecOldForward.x * flPitchSide + vecOldForward.y * flYawSide + vecOldForward.x * flPitchForward + vecOldForward.y * flYawForward + vecOldForward.z * flRollUp;

	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
	pUserCmd->flSideMove = vecOldRight.x * flPitchSide + vecOldRight.y * flYawSide + vecOldRight.x * flPitchForward + vecOldRight.y * flYawForward + vecOldRight.z * flRollUp;

	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_UPMOVE);
	pUserCmd->flUpMove = vecOldUp.x * flYawSide + vecOldUp.y * flPitchSide + vecOldUp.x * flYawForward + vecOldUp.y * flPitchForward + vecOldUp.z * flRollUp;
}

float get_max_player_speed()
{
	if (!SDK::LocalPawn->GetWeaponActive() || !SDK::LocalPawn->GetWeaponActive()->GetWeaponVData())
		return 260.f;

	return SDK::LocalPawn->IsScoped() ? SDK::LocalPawn->GetWeaponActive()->GetWeaponVData()->m_flMaxSpeed() : SDK::LocalPawn->GetWeaponActive()->GetWeaponVData()->m_flMaxSpeed();
}

void F::MISC::MOVEMENT::stop_movement(CUserCmd* user_cmd, C_CSPlayerPawn* local_player)
{
	CPlayer_MovementServices* movement_services = local_player->MovementServices();

	if (!movement_services)
		return;

	Vector_t velocity = SDK::LocalPawn->m_vecVelocity();
	float speed = velocity.Length2D();
	if (speed < 1.f)
	{
		user_cmd->csgoUserCmd.pBaseCmd->flForwardMove = 0.f;
		user_cmd->csgoUserCmd.pBaseCmd->flSideMove = 0.f;
		return;
	}

	const float max_accelspeed = (I::Cvar->Find(FNV1A::Hash("sv_accelerate"))->value.fl * I::Cvar->Find(FNV1A::Hash("sv_maxspeed"))->value.fl * movement_services->m_flSurfaceFriction() * INTERVAL_PER_TICK);

	auto get_maximum_accelerate_speed = [&]
	{
		const auto speed_ratio = speed / (I::Cvar->Find(FNV1A::Hash("sv_accelerate"))->value.fl * INTERVAL_PER_TICK);
		return speed - max_accelspeed <= -1.f ? max_accelspeed / speed_ratio : max_accelspeed;
	};

	QAngle_t view_angles{};
	view_angles = g_AntiAim->StoreAngels;

	Vector_t velocity_angle{};
	MATH::vector_angles(velocity * -1.f, velocity_angle);
	velocity_angle.y = view_angles.y - velocity_angle.y;

	Vector_t forward{};
	MATH::vector_angles(velocity_angle, forward);

	const auto total_speed = get_maximum_accelerate_speed();
	const auto max_weapon_speed = get_max_player_speed();

	user_cmd->csgoUserCmd.pBaseCmd->flForwardMove = (forward.x * total_speed) / max_weapon_speed;
	user_cmd->csgoUserCmd.pBaseCmd->flSideMove = (forward.y * -total_speed) / max_weapon_speed;
}

void F::MISC::MOVEMENT::quick_stop(CUserCmd* user_cmd)
{
	//if (!g_cfg->misc.m_quick_stop)
	//	return;

	C_CSPlayerPawn* local_player = SDK::LocalPawn;

	if (!local_player || !local_player->IsAlive() || !(local_player->GetFlags() & FL_ONGROUND) /*|| m_in_bhop*/)
		return;

	if (local_player->m_vecVelocity().Length2D() <= 0.f)
		return;

	//if (m_quick_peek_data.m_should_retrack)
	//	return;

	bool processing_movement_keys = (user_cmd->nButtons.nValue & (IN_LEFT) || user_cmd->nButtons.nValue & (IN_FORWARD) || user_cmd->nButtons.nValue & (IN_BACK) ||
	user_cmd->nButtons.nValue & (IN_RIGHT) || user_cmd->nButtons.nValue & (IN_MOVELEFT) || user_cmd->nButtons.nValue & (IN_MOVERIGHT) ||
	user_cmd->nButtons.nValue & (IN_JUMP));

	if (!processing_movement_keys)
		stop_movement(user_cmd, local_player);
}

void F::MISC::MOVEMENT::quick_peek_assistant(CUserCmd* user_cmd, CBaseUserCmdPB* base_cmd)
{
	static Vector_t vec_origin = Vector_t(0.0f, 0.0f, 0.0f);

	bool processing_movement_keys = (user_cmd->nButtons.nValue & (IN_LEFT) || user_cmd->nButtons.nValue & (IN_FORWARD) || user_cmd->nButtons.nValue & (IN_BACK) ||
	user_cmd->nButtons.nValue & (IN_RIGHT) || user_cmd->nButtons.nValue & (IN_MOVELEFT) || user_cmd->nButtons.nValue & (IN_MOVERIGHT) ||
	user_cmd->nButtons.nValue & (IN_JUMP));

	//if (!g_key_handler->is_pressed(g_cfg->misc.m_override_quick_peek_assistant, g_cfg->misc.m_override_quick_peek_assistant_style))
	//{
	//	m_quick_peek_data.reset();
	//	return;
	//}

	if (m_quick_peek_data.m_old_origin.IsZero())
	{
		vec_origin = SDK::LocalPawn->GetGameSceneNode()->m_vecOrigin();

		game_trace_t trace;
		TraceFilter_t filter(0x3001, nullptr, nullptr, 3);
		filter.m_arrCollisions.at(1) = 0x10000;

		Ray_t ray = Ray_t();
		I::GameTraceManager->TraceShape(&ray, SDK::LocalPawn->GetEyePosition(), SDK::LocalPawn->GetEyePosition() - Vector_t(0.0f, 0.0f, 1000.f), &filter, &trace);

		if (trace.Fraction < 1.0f)
			vec_origin = trace.m_end_pos + Vector_t(0.0f, 0.0f, 2.0f);

		m_quick_peek_data.reset(vec_origin);
	}
	else
	{
		if (SDK::LocalPawn->GetGameSceneNode()->m_vecOrigin().DistTo(m_quick_peek_data.m_old_origin) > 5.0f)
		{
			if (m_quick_peek_data.m_should_retrack)
			{
				++m_quick_peek_data.m_retracking_time;

				Vector_t ang_angles_to_pos = Vector_t(0, 0, 0);
				MATH::vector_angles(m_quick_peek_data.m_old_origin - SDK::LocalPawn->GetGameSceneNode()->m_vecOrigin(), ang_angles_to_pos);

				base_cmd->flForwardMove = (std::fmaxf(std::fminf(1.0f, static_cast<float>(std::cos(deg2rad((base_cmd->pViewAngles->angValue.y - ang_angles_to_pos.y))) * static_cast<float>(std::cos(ang_angles_to_pos.x * 0.017453292)))), -1.0f));
				base_cmd->flSideMove = (-std::fmaxf(std::fminf(1.0f, static_cast<float>(std::sin(deg2rad((base_cmd->pViewAngles->angValue.y - ang_angles_to_pos.y))) * static_cast<float>(std::cos(ang_angles_to_pos.x * 0.017453292)))), -1.0f));
			}
		}
		else
		{
			m_quick_peek_data.m_should_retrack = false;
			m_quick_peek_data.m_fired = false;
			m_quick_peek_data.m_retracking_time = 0;
		}
	}

	//if (user_cmd->nButtons.nValue & IN_ATTACK || !processing_movement_keys && g_cfg->misc.m_quick_peek_assistant_type == 0)
	{
		m_quick_peek_data.m_should_retrack = true;
		m_quick_peek_data.m_autopeeking = true;

		if (user_cmd->nButtons.nValue & IN_ATTACK)
			m_quick_peek_data.m_fired = true;
	}
	//else if (!user_cmd->nButtons.nValue & IN_ATTACK && (processing_movement_keys && g_cfg->misc.m_quick_peek_assistant_type == 1) && m_quick_peek_data.m_should_retrack && !m_quick_peek_data.m_fired)
		m_quick_peek_data.m_should_retrack = false;
}

void F::MISC::MOVEMENT::draw_autopeek()
{
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame() || !SDK::LocalPawn)
		return;

	Vector_t position = Vector_t();

	if (!m_quick_peek_data.m_old_origin.IsZero())
		position = m_quick_peek_data.m_old_origin;

	if (position.IsZero())
		return;

	//if (g_key_handler->is_pressed(g_cfg->misc.m_override_quick_peek_assistant, g_cfg->misc.m_override_quick_peek_assistant_style))
	D::RadialGradient3D(m_quick_peek_data.m_old_origin, 20.f, Color_t(255, 255, 255), Color_t(40, 20, 20),true);
}
