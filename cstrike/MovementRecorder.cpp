#include "MovementRecorder.h"
#include <DirectXMath.h>
#include "utilities/notify.h"
#include "utilities/draw.h"
#include "Utils.h"

MovementPathRecord_t CreateRecord(CUserCmd* pCmd) {
	MovementPathRecord_t mpRecord{};

	CBaseUserCmdPB* pBaseCmd = pCmd->csgoUserCmd.pBaseCmd;

	mpRecord.nButtons = pCmd->nButtons.nValue;
	mpRecord.nBaseButtons = pBaseCmd->pInButtonState->nValue;
	mpRecord.flUpMove = pBaseCmd->flUpMove;
	mpRecord.flSideMove = pBaseCmd->flSideMove;
	mpRecord.flForwardMove = pBaseCmd->flForwardMove;
	mpRecord.angViewAngles = pBaseCmd->pViewAngles->angValue;
	mpRecord.inpuHistoryField_CurrentSize = pCmd->csgoUserCmd.inputHistoryField.nCurrentSize;
	mpRecord.iFlags = SDK::LocalPawn->GetFlags();
	mpRecord.vecPosition = SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin();

	for (size_t i = 0; i < pCmd->csgoUserCmd.inputHistoryField.nCurrentSize; i++)
	{
		MovementPathRecordInputHistoryEntry_t ihe{};
		ihe.angViewAngles = pCmd->csgoUserCmd.inputHistoryField.pRep->tElements[i]->pViewAngles ? std::make_optional(pCmd->csgoUserCmd.inputHistoryField.pRep->tElements[i]->pViewAngles->angValue) : std::nullopt;
		ihe.targetEntIndex = pCmd->csgoUserCmd.inputHistoryField.pRep->tElements[i]->nTargetEntIndex;

		mpRecord.inpuHistoryField_Elements.push_back(ihe);
	}

	return mpRecord;
}

void ApplyRecord(CUserCmd* pCmd, MovementPathRecord_t mpRecord) {
	CBaseUserCmdPB* pBaseCmd = pCmd->csgoUserCmd.pBaseCmd;

	pCmd->nButtons.nValue = mpRecord.nButtons;
	pBaseCmd->pInButtonState->nValue = mpRecord.nBaseButtons;
	pBaseCmd->flUpMove = mpRecord.flUpMove;
	pBaseCmd->flSideMove = mpRecord.flSideMove;
	pBaseCmd->flForwardMove = mpRecord.flForwardMove;
	pBaseCmd->pViewAngles->angValue = mpRecord.angViewAngles;

	for (size_t i = 0; i < pCmd->csgoUserCmd.inputHistoryField.nCurrentSize; i++)
	{
		size_t index = i;
		while (index >= mpRecord.inpuHistoryField_CurrentSize)
			index = i - mpRecord.inpuHistoryField_CurrentSize;

		MovementPathRecordInputHistoryEntry_t ihe = mpRecord.inpuHistoryField_Elements[index];
		if (ihe.angViewAngles.has_value()) {
			if (!pCmd->GetInputHistoryEntry(i)->pViewAngles)
				pCmd->GetInputHistoryEntry(i)->pViewAngles = pCmd->GetInputHistoryEntry(i)->pViewAngles;
		//	pCmd->GetInputHistoryEntry(i)->SetBits(ihe.angViewAngles.value());
		}
		pCmd->GetInputHistoryEntry(i)->nTargetEntIndex = ihe.targetEntIndex;
		pCmd->GetInputHistoryEntry(i)->SetBits(0x4000U);
	}
}

bool MovementRecorder::CreateMove(CCSGOInput* pInput, int nSlot, CUserCmd* pCmd) {
	if (!I::GlobalVars)
		return false;

	if (State->iState == 0) { // idling

		State->flLastSnapshot = 0.f;
		State->bPlaybackStarted = false;
		State->iCurrentRecordIndex = 0;

	}
	else if (State->iState == 1) { // recording

		static float flRecordInterval = 1.f / 64.f;

		float flCurrentTime = I::GlobalVars->flCurTime;

		State->pRecording.flInterval = flRecordInterval;

		if (State->flLastSnapshot + flRecordInterval <= flCurrentTime) {
			State->pRecording.arrRecords.push_back(CreateRecord(SDK::Cmd));
			State->flLastSnapshot = flCurrentTime;
		}
		return true;

	}
	else if (State->iState == 2) { // playing

		float flCurrentTime = I::GlobalVars->flCurTime;

		if (!State->bPlaybackStarted) {
			// Start playback by going to the start position

			float dist = SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin().DistTo(State->mpPlaying.vecStartPos);
			if (dist <= 0.75f) {
				// Close enough, start playback
				State->bPlaybackStarted = true;
				//NOTIFY_SUCCESS("Movement Recorder", "Reached start point, beginning playback");
				return true;
			}

			CBaseUserCmdPB* pCmd = SDK::Cmd->csgoUserCmd.pBaseCmd;
			pCmd->flForwardMove = 1.f;
			pCmd->flSideMove = 0.f;
			Vector_t* move = (Vector_t*)&pCmd->flForwardMove;
			Vector_t move_backup = *move;
			const QAngle_t& current_angles = pCmd->pViewAngles->angValue;

			QAngle_t angle = g_Utils->CalcAngles(SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin(), State->mpPlaying.vecStartPos);

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

			float mul2 = 1.f;
			if (dist < 8.f)
				mul2 = dist / 8.f;

			pCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
			pCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
			pCmd->flForwardMove = move->x * mul2;
			pCmd->flSideMove = move->y * mul2;

			return true;
		}

		ApplyRecord(SDK::Cmd, State->mpPlaying.arrRecords[State->iCurrentRecordIndex]);

		if (State->flLastSnapshot + State->mpPlaying.flInterval <= flCurrentTime) {
			State->iCurrentRecordIndex++;
			State->flLastSnapshot = flCurrentTime;

			if (State->iCurrentRecordIndex == State->mpPlaying.arrRecords.size()) {
				NOTIFY::Push({ N_TYPE_SUCCESS,CS_XOR("Movement Recorder", "Playback finished") });
				State->iState = 0;
			}
		}
		return true;
	}
	return false;
}

void MovementRecorder::BeginPlayback(MovementPath_t path) {
	if (!SDK::LocalPawn || !SDK::LocalController) {
		NOTIFY::Push({ N_TYPE_WARNING,CS_XOR("Movement Recorder", "Movement Recorder is only available in-game.") });
		return;
	}

	if (State->iState != 0)
		return;

	State->mpPlaying = path;
	State->iState = 2;
}

void MovementRecorder::BeginRecording(std::string szName) {
	if (!SDK::LocalPawn || !SDK::LocalController) {
		NOTIFY::Push({ N_TYPE_WARNING,CS_XOR("Movement Recorder", "Movement Recorder is only available in-game.") });
		return;
	}

	if (State->iState != 0)
		return;

	State->pRecording = MovementPath_t{};
	State->pRecording.szName = szName;
	State->pRecording.vecStartPos = SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin();
	State->iState = 1;
}

MovementPath_t MovementRecorder::StopRecording() {
	if (State->iState == 0)
		return {};

	State->iState = 0;
	return State->pRecording;
}

void MovementRecorder::Present() {

	ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2{ 20.f, 20.f }, ImVec2{ 60.f, 20.f + ImGui::GetFontSize() + 8.f }, ImColor(0, 0, 0, 150), 6.f);
	if (State->iState == 0)
		ImGui::GetBackgroundDrawList()->AddText(ImVec2{ 30.f, 20.f + (ImGui::GetFontSize() / 2.f) + 4.f }, ImColor(255, 255, 255), "Idling...");
	else if (State->iState == 1)
		ImGui::GetBackgroundDrawList()->AddText(ImVec2{ 30.f, 20.f + (ImGui::GetFontSize() / 2.f) + 4.f }, ImColor(255, 255, 255), "Recording...");
	else if (State->iState == 2) {
		ImGui::GetBackgroundDrawList()->AddText(ImVec2{ 30.f, 20.f + (ImGui::GetFontSize() / 2.f) + 4.f }, ImColor(255, 255, 255), "Playing...");

		Vector_t previous = {};
		for (size_t i = 0; i < State->mpPlaying.arrRecords.size(); i++)
		{
			if (i > 0) {
				ImVec2 a, b;
				if (D::WorldToScreen(previous, &a) && D::WorldToScreen(State->mpPlaying.arrRecords[i].vecPosition, &b))
					ImGui::GetBackgroundDrawList()->AddLine(a, b, ImColor(255, 255, 255), 1.f);

			}

			previous = State->mpPlaying.arrRecords[i].vecPosition;
		}
	}
}
