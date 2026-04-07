#include "Utils.h"
#include "pred.h"

QAngle_t Utils::normalize(QAngle_t angs)
{
	while (angs.y < -180.0f)
		angs.y += 360.0f;
	while (angs.y > 180.0f)
		angs.y -= 360.0f;
	if (angs.x > 89.0f)
		angs.x = 89.0f;
	if (angs.x < -89.0f)
		angs.x = -89.0f;
	return angs;
}

float Utils::GetFov(const QAngle_t& viewAngle, const QAngle_t& aimAngle)
{
	QAngle_t delta = (aimAngle - viewAngle).Normalize();

	return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}

void Utils::VectorAngles(const Vector_t& vecForward, QAngle_t& vecAngles) {
	Vector_t vecView;
	if (vecForward.y == 0 && vecForward.x == 0)
	{
		vecView.x = 0.f;
		vecView.y = 0.f;
	}
	else
	{
		vecView.y = atan2(vecForward.y, vecForward.x) * 180.f / 3.14f;

		if (vecView.y < 0.f)
			vecView.y += 360.f;

		auto tmp = vecForward.Length2D();

		vecView.x = atan2(-vecForward.z, tmp) * 180.f / 3.14f;

		if (vecView.x < 0.f)
			vecView.x += 360.f;
	}

	vecAngles.x = vecView.x;
	vecAngles.y = vecView.y;
	vecAngles.z = 0.f;
}

void Utils::AngleVectors_two(const QAngle_t& angles, Vector_t* forward) {
	float pitchRad = angles.x * 57.2957795131f;
	float yawRad = angles.y * 57.2957795131f;

	forward->x = cos(pitchRad) * cos(yawRad);
	forward->y = cos(pitchRad) * sin(yawRad);
	forward->z = -sin(pitchRad);
}

std::vector<C_CSPlayerPawn*> Utils::GetPlayers(bool aliveOnly, bool includeTeammates, bool allowPerformanceSorting) {
	std::vector<C_CSPlayerPawn*> tmp{};

	for (size_t i = 0; i < g_RageBot->pawns.size(); i++)
	{
		if (!g_RageBot->pawns[i])
			continue;

		if (aliveOnly && g_RageBot->pawns[i]->GetHealth() < 1.f)
			continue;

		if (!g_RageBot->pawns[i]->IsOtherEnemy(SDK::LocalPawn))
			continue;
		auto activeWeapon = SDK::LocalPawn->GetWeaponActive();
		auto pLocalPawn = SDK::LocalPawn;
		if (allowPerformanceSorting) {
			for (int hitbox : g_RageBot->Hitbox)
			{
				Vector_t m = g_RageBot->pawns[i]->GetGameSceneNode()->GetSkeletonInstance()->GetBonePosition(hitbox);

				if (m.DistTo(pLocalPawn->GetEyePosition()) > activeWeapon->GetWeaponVData()->m_flRange() / 3.f)
					continue;

				float dmg;
				bool _ = 0;
				Vector_t eye_pos = (SDK::LocalPawn->GetGameSceneNode()->GetAbsOrigin() - SDK::LocalPawn->GetAbsVelocity() * TICK_INTERVAL) + SDK::LocalPawn->GetViewOffset();

				Vector_t n = eye_pos;

				auto start = n;
				auto direction = (m - start).Normalized();
				direction *= n.DistTo(m) + 1.f;
				dmg = g_RageBot->ScaleDamage(g_RageBot->pawns[i], SDK::LocalPawn, (C_CSWeaponBaseGun*)SDK::LocalPawn->GetWeaponActive(), SDK::EyePos, direction, dmg, _);

				//dmg = ScaleDamage(g_RageBot->pawns[i], pLocalPawn, activeWeapon, pLocalPawn->GetEyePosition(), direction, dmg, _);
				if (dmg <= 0.f)
					continue;
			}
		}

		tmp.push_back(g_RageBot->pawns[i]);
	}

	return tmp;
}
void Utils::angle_vectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
{
	float sp, sy, sr, cp, cy, cr;

	MATH::sin_cos(angles.x * 0.017453292f, &sp, &cp);
	MATH::sin_cos(angles.y * 0.017453292f, &sy, &cy);
	MATH::sin_cos(angles.z * 0.017453292f, &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = sr * sp * cy - cr * sy;
		right->y = cr * cy + sr * sp * sy;
		right->z = sr * cp;
	}

	if (up)
	{
		up->x = sr * sy + cr * sp * cy;
		up->y = cr * sp * sy - sr * cy;
		up->z = cr * cp;
	}
}

void Utils::angle_vectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
{
	float sp, sy, sr, cp, cy, cr;

	MATH::sin_cos(angles.x * 0.017453292f, &sp, &cp);
	MATH::sin_cos(angles.y * 0.017453292f, &sy, &cy);
	MATH::sin_cos(angles.z * 0.017453292f, &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = sr * sp * cy - cr * sy;
		right->y = cr * cy + sr * sp * sy;
		right->z = sr * cp;
	}

	if (up)
	{
		up->x = sr * sy + cr * sp * cy;
		up->y = cr * sp * sy - sr * cy;
		up->z = cr * cp;
	}
}

Vector_t Utils::CalculateCameraPosition(Vector_t anchorPos, float distance, QAngle_t viewAngles)
{
	float yaw = DirectX::XMConvertToRadians(viewAngles.y);
	float pitch = DirectX::XMConvertToRadians(viewAngles.x);

	float x = anchorPos.x + distance * cosf(yaw) * cosf(pitch);
	float y = anchorPos.y + distance * sinf(yaw) * cosf(pitch);
	float z = anchorPos.z + distance * sinf(pitch);

	return Vector_t{ x, y, z };
}
void Utils::TransformAABB(const Matrix3x4a_t& transform, const Vector_t& minsIn, const Vector_t& maxsIn, Vector_t& minsOut, Vector_t& maxsOut)
{
	const Vector_t localCenter = (minsIn + maxsIn) * 0.5f;
	const Vector_t localExtent = maxsIn - localCenter;

	const auto& mat = transform.arrData;
	const Vector_t worldAxisX{ mat[0][0], mat[0][1], mat[0][2] };
	const Vector_t worldAxisY{ mat[1][0], mat[1][1], mat[1][2] };
	const Vector_t worldAxisZ{ mat[2][0], mat[2][1], mat[2][2] };

	const Vector_t worldCenter = localCenter.Transform(transform);
	const Vector_t worldExtent{
		localExtent.dot_absolute(worldAxisX),
		localExtent.dot_absolute(worldAxisY),
		localExtent.dot_absolute(worldAxisZ),
	};

	minsOut = worldCenter - worldExtent;
	maxsOut = worldCenter + worldExtent;
}

QAngle_t Utils::CalcAngles(Vector_t viewPos, Vector_t aimPos)
{
	__m128 view = _mm_set_ps(0.0f, viewPos.z, viewPos.y, viewPos.x);
	__m128 aim = _mm_set_ps(0.0f, aimPos.z, aimPos.y, aimPos.x);
	__m128 delta = _mm_sub_ps(aim, view);

	__m128 squared = _mm_mul_ps(delta, delta);
	__m128 sum = _mm_hadd_ps(squared, squared);
	sum = _mm_hadd_ps(sum, sum);
	__m128 length = _mm_sqrt_ps(sum);

	float deltaZ = _mm_cvtss_f32(_mm_shuffle_ps(delta, delta, _MM_SHUFFLE(2, 2, 2, 2)));
	float deltaY = _mm_cvtss_f32(_mm_shuffle_ps(delta, delta, _MM_SHUFFLE(1, 1, 1, 1)));
	float deltaX = _mm_cvtss_f32(delta);

	QAngle_t angle;
	angle.x = -asinf(deltaZ / _mm_cvtss_f32(length)) * 57.2957795131f;
	angle.y = atan2f(deltaY, deltaX) * 57.2957795131f;
	angle.z = 0.0f;

	return angle;
}