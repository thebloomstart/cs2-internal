#pragma once
#include "core/sdk.h"
#include "cprediction.h"
#include "sdk/interfaces/inetworkclientservice.h"
#include "core/interfaces.h"

class INetworkGameService;
class INetworkClientService;

struct PredictionData_t
{
	bool bInPrediction = false;
	bool bFirstPrediction = false;
	bool bHasBeenPredicted = false;
	bool bShouldPredict = false;

	int nTickBase = 0;
	int nTickCount = 0;
	Vector_t EyePos = { 0,0,0 };
	float flIntervalPerSubTick = 0.0f;

	float flCurrentTime = 0.0f;
	float flCurrentTime2 = 0.0f;

	float flFrameTime = 0.0f;
	float flFrameTime2 = 0.0f;

	float flSpread = 0.0f;
	float flAccuracy = 0.0f;

	std::uint32_t nFlags;
};

class CUserCmd;

class CPredictionSystem
{
public:
	void Begin(CCSGOInput* pInput, CUserCmd* pUserCmd);
	void End(CCSGOInput* pInput, CUserCmd* pUserCmd);
public:
	float GetSpread() { return predictionData.flSpread; };
	float GetAccuracy() { return predictionData.flAccuracy; };
	int GetOldFlags() { return predictionData.nFlags; };
	Vector_t GetShootPos() { return predictionData.EyePos; };
private:
	void RunPrediction(CNetworkGameClient* pNetworkGameClient, int nPredictionReason);
public:
	PredictionData_t predictionData;
	int nLastSequence = 0;
};
inline std::unique_ptr<CPredictionSystem> g_PredictionSystem = std::make_unique<CPredictionSystem>();

