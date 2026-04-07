#pragma once
#include <regex>
#include "sdk/entity.h"
#include "core/sdk.h"
#include "utilities/draw.h"
#include "sdk\interfaces\ccsgoinput.h"
#include "utilities/inputsystem.h"
#include <vector>
#include <unordered_map>
#include <string>

struct MovementPathRecordInputHistoryEntry_t {
	std::optional<QAngle_t> angViewAngles;
	int targetEntIndex;
};

struct MovementPathRecord_t {
	float flForwardMove;
	float flSideMove;
	float flUpMove;
	uint64_t nBaseButtons;
	uint64_t nButtons;
	QAngle_t angViewAngles;
	Vector_t vecPosition;
	int iFlags;

	int inpuHistoryField_CurrentSize;
	std::vector<MovementPathRecordInputHistoryEntry_t> inpuHistoryField_Elements;
};

struct MovementPath_t {
	std::string szName;
	float flInterval;
	Vector_t vecStartPos;
	std::vector<MovementPathRecord_t> arrRecords;
};

struct MovementRecorderState_t {
	MovementPath_t pRecording;
	int iCurrentRecordIndex;

	MovementPath_t mpPlaying;
	bool bPlaybackStarted;

	float flLastSnapshot;
	int iState = 0;
};

namespace MovementRecorder {
	static std::unordered_map<FNV1A_t, MovementPath_t> mpPaths;
	static MovementRecorderState_t* State = new MovementRecorderState_t{};

	void BeginPlayback(MovementPath_t path);
	void BeginRecording(std::string szName);
	MovementPath_t StopRecording();
	bool CreateMove(CCSGOInput* pInput, int nSlot, CUserCmd* pCmd);
	void Present();
}