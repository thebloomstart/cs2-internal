#pragma once
#include "core/sdk.h"

class Effect
{
public:
	void ShokedEffect(C_CSPlayerPawn* pLocal);
};
inline std::unique_ptr<Effect> g_Effect = std::make_unique<Effect>();

