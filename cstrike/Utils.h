#pragma once
#include "sdk/datatypes/qangle.h"
#include <vector>
#include "sdk/entity.h"
#include "core/sdk.h"
#include "AimBot.h"
#include <DirectXMath.h>

class Utils
{
public:
	QAngle_t normalize(QAngle_t angs);
	float GetFov(const QAngle_t& viewAngle, const QAngle_t& aimAngle);
	void VectorAngles(const Vector_t& vecForward, QAngle_t& vecAngles);
	void AngleVectors_two(const QAngle_t& angles, Vector_t* forward);
	std::vector<C_CSPlayerPawn*> GetPlayers(bool aliveOnly, bool includeTeammates, bool allowPerformanceSorting);
	void angle_vectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up);
	Vector_t CalculateCameraPosition(Vector_t anchorPos, float distance, QAngle_t viewAngles);
	void TransformAABB(const Matrix3x4a_t& transform, const Vector_t& minsIn, const Vector_t& maxsIn, Vector_t& minsOut, Vector_t& maxsOut);
	QAngle_t CalcAngles(Vector_t viewPos, Vector_t aimPos);
	void angle_vectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up);

};
inline std::unique_ptr<Utils> g_Utils = std::make_unique<Utils>();
