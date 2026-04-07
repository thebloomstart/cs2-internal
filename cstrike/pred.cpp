#include "pred.h"
#include "sdk/entity.h"
#include "features/misc.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/interfaces/igameresourceservice.h"
#include "sdk/interfaces/iengineclient.h"

class C_CSWeaponBaseGun;
class IGameResourceService;
class CGameEntitySystem;

void CPredictionSystem::RunPrediction(CNetworkGameClient* pNetworkGameClient, int nPredictionReason)
{
	using func_t = void(__fastcall*)(CNetworkGameClient*, int);
	static func_t fn = (func_t)MEM::FindPattern(ENGINE2_DLL, CS_XOR("40 55 41 56 48 83 EC 28 80 B9"));
	fn(pNetworkGameClient, nPredictionReason);
}

void CPredictionSystem::Begin(CCSGOInput* pInput, CUserCmd* pUserCmd)
{
	if (!I::GlobalVars || !SDK::LocalPawn || !SDK::LocalController->IsPawnAlive())
		return;

	//if (this->nLastSequence - I::Input->nSequenceNumber == 0)
	//	return;

	CPlayer_MovementServices* pMovementServices = SDK::LocalPawn->MovementServices();
	if (!pMovementServices)
		return;

	CNetworkGameClient* pNetworkGameClient = I::NetworkClientService->GetNetworkGameClient();
	if (!pNetworkGameClient)
		return;

	// construct prediction data
	this->predictionData = { };

	// save pre prediction flags
	predictionData.nFlags = SDK::LocalPawn->GetFlags();

	// store current global variables
	predictionData.flIntervalPerSubTick = I::GlobalVars->flFrameTime2;

	predictionData.flCurrentTime = I::GlobalVars->flCurTime;
	predictionData.flCurrentTime2 = I::GlobalVars->flCurTime2;

	predictionData.nTickCount = I::GlobalVars->tickcount;

	predictionData.flFrameTime = I::GlobalVars->flFrametime;
	predictionData.flFrameTime2 = I::GlobalVars->flFrameTime2;

	// store current tickbase
	predictionData.nTickBase = SDK::LocalController->GetTickBase();
	if (auto active_weapon = SDK::LocalPawn->GetWeaponActive()) {
		active_weapon->update_accuracy();

		predictionData.flSpread = active_weapon->get_spread();
		predictionData.flAccuracy = active_weapon->get_inaccuracyHitChance();
	}

	// store prediction state
	predictionData.bInPrediction = I::Prediction->InPrediction;
	predictionData.bFirstPrediction = I::Prediction->bFirstPrediction;
	predictionData.bHasBeenPredicted = pUserCmd->bHasBeenPredicted;
	predictionData.bShouldPredict = pNetworkGameClient->bShouldPredict;

	// set prediction state
	pUserCmd->bHasBeenPredicted = false;
	pNetworkGameClient->bShouldPredict = true;
	I::Prediction->bFirstPrediction = false;
	I::Prediction->InPrediction = true;
	c_networked_client_info* clientinfo = I::Engine->get_networked_client_info();
	if (clientinfo->m_local_data)
	    predictionData.EyePos = clientinfo->m_local_data->m_eye_pos;
	pMovementServices->SetPredictionCommand(pUserCmd);
	SDK::LocalController->GetCurrentCommand() = pUserCmd;

	// update prediction
	if (pNetworkGameClient->bShouldPredict && pNetworkGameClient->GetDeltaTick() > 0 && pNetworkGameClient->nSomePredictionTick > 0)
		RunPrediction(pNetworkGameClient, client_command_tick);

	// restore tickbase
	SDK::LocalController->GetTickBase() = predictionData.nTickBase;
}

void CPredictionSystem::End(CCSGOInput* pInput, CUserCmd* pUserCmd)
{
	if (!I::GlobalVars || !SDK::LocalPawn || !SDK::LocalController->IsPawnAlive())
		return;

	// run movement features
	//F::MISC::MOVEMENT::PostPrediction(pUserCmd);

	//if (this->nLastSequence - I::Input->nSequenceNumber == 0)
	//	return;

	CPlayer_MovementServices* pMovementServices = SDK::LocalPawn->MovementServices();
	if (!pMovementServices)
		return;

	CNetworkGameClient* pNetworkGameClient = I::NetworkClientService->GetNetworkGameClient();
	if (!pNetworkGameClient)
		return;

	pMovementServices->ResetPredictionCommand();
	SDK::LocalController->GetCurrentCommand() = nullptr;

	// restore global variables
	I::GlobalVars->flFrameTime2 = predictionData.flIntervalPerSubTick;

	I::GlobalVars->flCurTime = predictionData.flCurrentTime;
	I::GlobalVars->flCurTime2 = predictionData.flCurrentTime2;

	I::GlobalVars->tickcount = predictionData.nTickCount;

	I::GlobalVars->flFrametime = predictionData.flFrameTime;
	I::GlobalVars->flFrameTime2 = predictionData.flFrameTime2;

	// restore our tickbase
	SDK::LocalController->GetTickBase() = predictionData.nTickBase;

	// restore prediction state
	I::Prediction->bFirstPrediction = predictionData.bFirstPrediction;
	I::Prediction->InPrediction = predictionData.bInPrediction;
	pUserCmd->bHasBeenPredicted = predictionData.bHasBeenPredicted;
	pNetworkGameClient->bShouldPredict = predictionData.bShouldPredict;

	// save for next call
	//this->nLastSequence = I::Input->nSequenceNumber;
}