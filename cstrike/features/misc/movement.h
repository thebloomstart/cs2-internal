#pragma once
#include "../../sdk/datatypes/vector.h"
class CUserCmd;
class CBaseUserCmdPB;
class CCSGOInputHistoryEntryPB;

class CCSPlayerController;
class C_CSPlayerPawn;

struct QAngle_t;

struct qucik_peek_data_t
{
	void reset(Vector_t vecOrigin = Vector_t(0.f, 0.f, 0.f))
	{
		m_old_origin = vecOrigin;
		m_should_retrack = false;
		m_autopeeking = false;
		m_fired = false;
		m_retracking_time = 0;
	}

	bool m_autopeeking = false;
	bool m_fired = false;
	bool m_should_retrack = false;
	int m_retracking_time = 0;
	Vector_t m_old_origin = Vector_t(0, 0, 0);
};

namespace F::MISC::MOVEMENT
{
	void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);

	void BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn);
	void JumpBug(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn);
	//void AutoStrafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn);
	void MovementCorrection(CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputHistory, const QAngle_t& angDesiredViewPoint);

	void stop_movement(CUserCmd* user_cmd, C_CSPlayerPawn* local_player);

	void quick_stop(CUserCmd* user_cmd);

	void quick_peek_assistant(CUserCmd* user_cmd, CBaseUserCmdPB* base_cmd);

	void draw_autopeek();

	void AutoStrafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn);

	// will call MovementCorrection && validate user's angView to avoid untrusted ban
	void ValidateUserCommand(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputHistory);
	void movment_fix(CUserCmd* pCmd, QAngle_t angle);
	inline bool m_in_bhop;
	inline qucik_peek_data_t m_quick_peek_data{};
}
