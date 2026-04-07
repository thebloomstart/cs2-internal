#pragma once
#include "utilities/memory.h"
#include "sdk/datatypes/vector.h"

class CViewVectors
{
public:
	Vector_t vec_view; //0x0000
	Vector_t hull_min; //0x000C
	Vector_t hull_max; //0x0018
	Vector_t duck_hull_min; //0x0024
	Vector_t duck_hull_max; //0x0030
	Vector_t duck_view; //0x003C
	Vector_t obs_hull_min; //0x0048
	Vector_t obs_hull_max; //0x0054
	Vector_t dead_view; //0x0060
}; //Size: 0x006C

class CGameRules
{
public:
	char pad[0x30];
	bool FreezePause;
	char pad2[0xB];
	int Pause;
	char pad3[0x38];
	int GamePhase;

public:
	CViewVectors* GetViewVectors()
	{
		return MEM::CallVFunc<CViewVectors*, 38>(this);
	}
};
