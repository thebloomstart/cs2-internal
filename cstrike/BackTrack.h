#pragma once
#include "sdk/datatypes/vector.h"
#include <vector>
#include "sdk/entity.h"
#include "core/sdk.h"
#include <map>
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/interfaces/igameresourceservice.h"
class CUserCmd;

namespace LegitBacktrack
{
	static std::map<int, std::vector<Vector_t>> oldPlayerRecords = {};
	void OnCreateMove(CUserCmd* cmd);
	void RenderRecords();
}
