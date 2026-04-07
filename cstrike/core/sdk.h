#pragma once

// used: viewmatrix_t
#include "../sdk/datatypes/matrix.h"
// used: color_t
#include "../sdk/datatypes/color.h"
// used: cmd
#include "../sdk/datatypes/usercmd.h"
#include "../sdk/interfaces/ccsgoinput.h"
#include "../dependencies/imgui/imgui.h"
#include "../utilities/math.h"
#include <unordered_map>

#pragma region sdk_definitions
// @source: master/public/worldsize.h
// world coordinate bounds
#define MAX_COORD_FLOAT 16'384.f
#define MIN_COORD_FLOAT (-MAX_COORD_FLOAT)

// @source: master/public/vphysics_interface.h
// coordinates are in HL units. 1 unit == 1 inch
#define METERS_PER_INCH 0.0254f
#pragma endregion



class CCSPlayerController;
class C_CSPlayerPawn;

namespace SDK
{
	typedef __declspec(align(16)) union
	{
		float f[4];
		uint32_t u[4];
		__m128 v;
	} m128;

	__forceinline float sqrt_ps(const float squared)
	{
		m128 tmp;
		tmp.f[0] = squared;
		tmp.v = _mm_sqrt_ps(tmp.v);
		return tmp.f[0];
	};

	class vector
	{
	public:
		float x{}, y{}, z{};

		__forceinline vector() :
			vector(0.f, 0.f, 0.f) {
		}

		__forceinline vector(const float x, const float y, const float z) :
			x(x), y(y), z(z) {
		}

		__forceinline vector operator+(const vector& v) const { return vector(x + v.x, y + v.y, z + v.z); }

		__forceinline vector operator-(const vector& v) const { return vector(x - v.x, y - v.y, z - v.z); }

		__forceinline vector operator*(const vector& v) const { return vector(x * v.x, y * v.y, z * v.z); }

		__forceinline vector operator/(const vector& v) const { return vector(x / v.x, y / v.y, z / v.z); }

		__forceinline vector operator*(const float v) const { return vector(x * v, y * v, z * v); }

		__forceinline vector operator/(const float v) const { return vector(x / v, y / v, z / v); }

		__forceinline vector operator+=(const vector& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		__forceinline vector operator-=(const vector& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		__forceinline vector operator*=(const vector& other)
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}

		__forceinline vector operator/=(const vector& other)
		{
			x /= other.x;
			y /= other.y;
			z /= other.z;
			return *this;
		}

		__forceinline vector operator*=(const float other)
		{
			x *= other;
			y *= other;
			z *= other;
			return *this;
		}

		__forceinline vector operator/=(const float other)
		{
			x /= other;
			y /= other;
			z /= other;
			return *this;
		}

		__forceinline bool operator==(const vector& other) const { return x == other.x && y == other.y && z == other.z; }

		__forceinline bool operator!=(const vector& other) const { return x != other.x || y != other.y || z != other.z; }

		__forceinline float operator[](int i) const { return ((float*)this)[i]; }

		__forceinline float& operator[](int i) { return ((float*)this)[i]; }

		__forceinline bool is_zero(float tolerance = 0.01f) const
		{
			return (x > -tolerance && x < tolerance && y > -tolerance && y < tolerance && z > -tolerance && z < tolerance);
		}

		__forceinline float dot(const vector& other) const { return x * other.x + y * other.y + z * other.z; }

		__forceinline float dot(const float* other) const { return x * other[0] + y * other[1] + z * other[2]; }

		__forceinline float dist(const vector& other) const { return (other - *this).length(); }

		__forceinline float dist_sqr(const vector& other) const { return (other - *this).length_sqr(); }

		__forceinline vector cross(const vector& other) const { return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x }; }

		__forceinline bool is_valid() const { return std::isfinite(this->x) && std::isfinite(this->y) && std::isfinite(this->z); }

		__forceinline float length() const { return sqrt_ps(x * x + y * y + z * z); }
		__forceinline float length_sqr() const { return this->x * this->x + this->y * this->y + this->z * this->z; }
		__forceinline float length_2d() const { return sqrt_ps(x * x + y * y); }
		__forceinline float length_2d_sqr() const { return this->x * this->x + this->y * this->y; }

		__forceinline static float vector_normalize(vector& vec)
		{
			const auto sqrlen = vec.length_sqr() + 1.0e-10f;
			float invlen;
			const auto xx = _mm_load_ss(&sqrlen);
			auto xr = _mm_rsqrt_ss(xx);
			auto xt = _mm_mul_ss(xr, xr);
			xt = _mm_mul_ss(xt, xx);
			xt = _mm_sub_ss(_mm_set_ss(3.f), xt);
			xt = _mm_mul_ss(xt, _mm_set_ss(0.5f));
			xr = _mm_mul_ss(xr, xt);
			_mm_store_ss(&invlen, xr);
			vec.x *= invlen;
			vec.y *= invlen;
			vec.z *= invlen;
			return sqrlen * invlen;
		}

		__forceinline bool IsValid() const
		{
			return (std::isfinite(this->x) && std::isfinite(this->y) && std::isfinite(this->z));
		}

		__forceinline constexpr vector& Clamp()
		{
			this->x = std::clamp(this->x, -89.f, 89.f);
			this->y = std::clamp(std::remainder(this->y, 360.0f), -180.f, 180.f);
			this->z = 0.f;
			return *this;
		}

		__forceinline float normalize_in_place() { return vector_normalize(*this); }

		__forceinline vector normalized() const
		{
			auto norm = *this;
			vector_normalize(norm);
			return norm;
		}
	};


	// capture game's exported functions
	bool Setup();

	inline ViewMatrix_t ViewMatrix = ViewMatrix_t();
	inline Vector_t CameraPosition = Vector_t();
	inline CCSPlayerController* LocalController = nullptr;
	inline C_CSPlayerPawn* LocalPawn = nullptr;
	inline CUserCmd* Cmd = nullptr;
	inline bool Alive = 0;
	inline CCSPlayer_WeaponServices* WeaponServices = nullptr;
	inline C_CSWeaponBaseGun* ActiveWeapon = nullptr;
	inline float WepAccuracy = 0;
	inline CCSWeaponBaseVData* ActiveWeaponVData = nullptr;
	inline float Latency = 0.f;
	inline float EngLatency = 0.f;
	inline float ClientLerp = 0.f;
	inline int prediction_tick = 0;
	static inline float WepRange;
	static inline float WepRangeMod;
	static inline float WepDamage;
	static inline float WepPen;
	static inline float WepArmorRatio;
	static inline float WepHeadShotMulti;
	inline Vector_t EyePos = Vector_t();
	inline int ItemDefIdx;

	inline ImVec2 ScreenSize{};
	inline void(CS_CDECL* fnConColorMsg)(const Color_t&, const char*, ...) = nullptr;
}

class Cheat
{
public:
	bool alive = false;
	bool canShoot = false;
	bool canScope = false;
	bool NoSpread = false;
	bool InThirdperson = false;
	bool mShouldClearNotice = false;
	QAngle_t StoreAngle{};
	Vector_t EyePos;

	float OldForwardMove = 0.f;
	float OldSideMove = 0.f;

	bool MapInit = false;
	bool MapInitSky = false;
	bool ModulationInit = true;
	bool ModulationInit1 = true;

	bool MapInitAspectRatio = false;
	bool Init = false;

	Color_t StoreSkyColor{};
	Color_t StoreWorldColor{};
	Color_t StorePropsColor{};
};

inline Cheat* cheat = new Cheat();
