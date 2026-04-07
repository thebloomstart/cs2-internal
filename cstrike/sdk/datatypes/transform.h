#pragma once

// used: matResult
#include "matrix.h"
// used: quaternion
#include "quaternion.h"

class CTransform
{
public:
	VectorAligned_t vecPosition;
	QuaternionAligned_t quatOrientation;
	Matrix3x4_t ToMatrix() const
	{
		Matrix3x4_t matrix{};

		const auto& rot = quatOrientation;
		const auto& pos = vecPosition;

		matrix[0][0] = 1.0f - 2.0f * rot.y * rot.y - 2.0f * rot.z * rot.z;
		matrix[1][0] = 2.0f * rot.x * rot.y + 2.0f * rot.w * rot.z;
		matrix[2][0] = 2.0f * rot.x * rot.z - 2.0f * rot.w * rot.y;

		matrix[0][1] = 2.0f * rot.x * rot.y - 2.0f * rot.w * rot.z;
		matrix[1][1] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.z * rot.z;
		matrix[2][1] = 2.0f * rot.y * rot.z + 2.0f * rot.w * rot.x;

		matrix[0][2] = 2.0f * rot.x * rot.z + 2.0f * rot.w * rot.y;
		matrix[1][2] = 2.0f * rot.y * rot.z - 2.0f * rot.w * rot.x;
		matrix[2][2] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.y * rot.y;

		matrix[0][3] = pos.x;
		matrix[1][3] = pos.y;
		matrix[2][3] = pos.z;

		return matrix;
	};
};

static_assert(alignof(CTransform) == 16);
