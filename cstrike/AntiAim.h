#pragma once
#include "sdk/entity.h"
#include "core/sdk.h"
#include "CSGameRules.h"
#include "sdk\interfaces\igameresourceservice.h"
#include "sdk\interfaces\cgameentitysystem.h"
#include "sdk\interfaces\ccsgoinput.h"
#include "sdk/datatypes/qangle.h"

class CL_AntiAim
{
	enum Manual
	{
		none,
		left,
		backward,
		right
	};

	enum
	{
		SIDE_NONE = -1,
		SIDE_BACK,
		SIDE_LEFT,
		SIDE_RIGHT
	};

	enum JitterType
	{
		JITTER_TYPE_NONE = 0,
		JITTER_TYPE_CENTER,
		JITTER_TYPE_3_WAY,
		JITTER_TYPE_RANDOM,
		JITTER_TYPE_SPIN,
		JITTER_TYPE_DANCE,
	};

public:
	void run(CUserCmd* cmd);
	QAngle_t StoreAngels;
	QAngle_t HuiAA;
	bool bFreezTime{};
	bool m_should_invert{};
private:
	void pitch(CUserCmd* cmd);
	void yaw(CUserCmd* cmd);
public:
	float SinnerAmount = 0.f;
	int Manual = 0;
	Vector_t AntiAimAngels{};
	int JitterSide = -1;
	int FinalManualSide = -1;
};

inline std::unique_ptr<CL_AntiAim> g_AntiAim = std::make_unique<CL_AntiAim>();
