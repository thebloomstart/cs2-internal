#pragma once

// used: mem_pad
#include "../../utilities/memory.h"

class IGlobalVars
{
public:
	float flRealTime; //0x0000
	int32_t iFrameCount; //0x0004
	float flFrametime; //0x0008
	float flAbsFrametime; //0x000C
	int32_t MaxClient; //0x0010
	char pad_0014[28]; //0x0014
	float flFrameTime2; //0x0030
	float flCurTime; //0x0034
	float flCurTime2; //0x0038
	char pad_003C[20]; //0x003C
	int32_t tickcount; //0x0050
	char pad_0054[292]; //0x0054
	uint64_t current_map; //0x0178
	uint64_t current_mapname; //0x0180
}; //Size: 0x0188
