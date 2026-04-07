#pragma once

#include "../common.h"
// used: std::is_integral_v
#include <type_traits>
// used: sin, cos, pow, abs, sqrt
#include <corecrt_math.h>// used: MATH::Sin, cos, MATH::Pow, abs, sqrt
// used: rand, srand
#include <cstdlib>
// used: time
#include <ctime>

// convert angle in degrees to radians
#define M_DEG2RAD(DEGREES) ((DEGREES) * (MATH::_PI / 180.f))
// convert angle in radians to degrees
#define M_RAD2DEG(RADIANS) ((RADIANS) * (180.f / MATH::_PI))
/// linearly interpolate the value between @a'X0' and @a'X1' by @a'FACTOR'
#define M_LERP(X0, X1, FACTOR) ((X0) + ((X1) - (X0)) * (FACTOR))
/// trigonometry
#define M_COS(ANGLE) cos(ANGLE)
#define M_SIN(ANGLE) sin(ANGLE)
#define M_TAN(ANGLE) tan(ANGLE)
/// power
#define M_POW(BASE, EXPONENT) pow(BASE, EXPONENT)
/// absolute value
#define M_ABS(VALUE) abs(VALUE)
/// square root
#define M_SQRT(VALUE) sqrt(VALUE)
/// floor
#define M_FLOOR(VALUE) floor(VALUE)
#include "../sdk/datatypes/vector.h"

/*
 * MATHEMATICS
 * - basic trigonometry, algebraic mathematical functions and constants
 */
namespace MATH
{
	/* @section: constants */
	// pi value
	inline constexpr float _PI = 3.14159265358f;
	// double of pi
	inline constexpr float _2PI = 6.283185307f;
	// half of pi
	inline constexpr float _HPI = 1.570796327f;
	// quarter of pi
	inline constexpr float _QPI = 0.785398163f;
	// reciprocal of double of pi
	inline constexpr float _1DIV2PI = 0.159154943f;
	// golden ratio
	inline constexpr float _PHI = 1.618033988f;

	// capture game's exports
	bool Setup();

	/* @section: algorithm */
	/// alternative of 'std::min'
	/// @returns : minimal value of the given comparable values
	template <typename T>
	[[nodiscard]] CS_INLINE constexpr const T& Min(const T& left, const T& right) noexcept
	{
		return (right < left) ? right : left;
	}

	/// alternative of 'std::max'
	/// @returns : maximal value of the given comparable values
	template <typename T>
	[[nodiscard]] CS_INLINE constexpr const T& Max(const T& left, const T& right) noexcept
	{
		return (right > left) ? right : left;
	}

	/// alternative of 'std::clamp'
	/// @returns : value clamped in range ['minimal' .. 'maximal']
	template <typename T>
	[[nodiscard]] CS_INLINE constexpr const T& Clamp(const T& value, const T& minimal, const T& maximal) noexcept
	{
		return (value < minimal) ? minimal : (value > maximal) ? maximal :
																 value;
	}


	CS_INLINE float rad2deg(float rad)
	{
		float result = rad * (180.0f / _PI);
		return result;
	}

	inline void vector_angles(const Vector_t& vecForward, Vector_t& vecView)
	{
		if (vecForward.y == 0 && vecForward.x == 0)
		{
			vecView.y = 0.f;
			vecView.x = vecForward.z > 0.f ? 270.f : 90.f;
		}
		else
		{
			vecView.y = rad2deg(std::atan2(vecForward.y, vecForward.x));
			if (vecView.y < 0.f)
				vecView.y += 360.f;

			vecView.x = rad2deg(std::atan2(-vecForward.z, vecForward.Length2D()));
			if (vecView.x < 0.f)
				vecView.x += 360.f;
		}

		vecView.x = std::remainder(vecView.x, 360.f);
		vecView.y = std::remainder(vecView.y, 360.f);
		vecView.z = std::remainder(vecView.z, 360.f);
	}
	inline Vector_t normalize(Vector_t angs)
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
	[[nodiscard]] CS_INLINE float normalize_yaw(float yaw) noexcept
	{
		while (yaw > 180.0f)
			yaw -= 360.0f;

		while (yaw < -180.0f)
			yaw += 360.0f;

		return yaw;
	}
	CS_INLINE float deg2rad(float deg)
	{
		float result = deg * (_PI / 180.0f);
		return result;
	}
	inline void sin_cos(float rad, float* sine, float* cosine)
	{
		*sine = std::sinf(rad);
		*cosine = std::cosf(rad);
	}
	/* @section: exponential */
	/// @returns: true if given number is power of two, false otherwise
	template <typename T> requires (std::is_integral_v<T>)
	[[nodiscard]] CS_INLINE constexpr bool IsPowerOfTwo(const T value) noexcept
	{
		return value != 0 && (value & (value - 1)) == 0;
	}
	__forceinline Vector_t ExtrapolatePosition(const Vector_t& pos, const Vector_t& vel, float ticks) {
		__m128 position = _mm_set_ps(0.0f, pos.z, pos.y, pos.x);
		__m128 velocity = _mm_set_ps(0.0f, vel.z, vel.y, vel.x);
		__m128 tickScalar = _mm_set_ps1(0.015625f * ticks);

		__m128 extrapolated = _mm_add_ps(
			position,
			_mm_add_ps(
				_mm_mul_ps(velocity, tickScalar),
				velocity
			)
		);

		Vector_t result;
		_mm_store_ps((float*)&result, extrapolated);
		return result;
	}
	inline void angle_vectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
	{
		float sp, sy, sr, cp, cy, cr;

		sin_cos(angles.x * 0.017453292f, &sp, &cp);
		sin_cos(angles.y * 0.017453292f, &sy, &cy);
		sin_cos(angles.z * 0.017453292f, &sr, &cr);

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
	/* @section: random using game's exports */
	inline int(CS_CDECL* fnRandomSeed)(int iSeed) = nullptr;
	inline float(CS_CDECL* fnRandomFloat)(float flMinValue, float flMaxValue) = nullptr;
	inline float(CS_CDECL* fnRandomFloatExp)(float flMinValue, float flMaxValue, float flExponent) = nullptr;
	inline int(CS_CDECL* fnRandomInt)(int iMinValue, int iMaxValue) = nullptr;
	inline float(CS_CDECL* fnRandomGaussianFloat)(float flMean, float flStdDev) = nullptr;
}
