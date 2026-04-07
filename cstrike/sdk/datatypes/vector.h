#pragma once
// used: [stl] numeric_limits
#include <limits>
// used: [crt] isfinite, fmodf, sqrtf
#include <cmath>
#include <xmmintrin.h>
#include <intrin.h>	
// forward declarations
struct QAngle_t;
struct Matrix3x4_t;

// @source: master/public/mathlib/vector.h
template <typename T>
class CUtlVectorCS2
{
public:
	auto begin() const
	{
		return m_Data;
	}

	auto end() const
	{
		return m_Data + m_Size;
	}

	auto At(int i) const
	{
		return m_Data[i];
	}

	auto AtPtr(int i) const
	{
		return m_Data + i;
	}

	T get_element(int i)
	{
		return m_Data[i];
	}

	int get_size()
	{
		return m_Size;
	}

	int m_Size;
	char pad0[0x4];
	T* m_Data;
	char pad1[0x8];
};

struct Vector2D_t
{
	constexpr Vector2D_t(const float x = 0.0f, const float y = 0.0f) :
		x(x), y(y) { }

	[[nodiscard]] bool IsZero() const
	{
		// @note: to make this implementation right, we should use fpclassify here, but game aren't doing same, probably it's better to keep this same, just ensure that it will be compiled same
		return (this->x == 0.0f && this->y == 0.0f);
	}
	constexpr float Length2DSqr(Vector2D_t _this)
	{
		return (_this.x * _this.x + _this.y * _this.y);
	}

	Vector2D_t operator+(const Vector2D_t& vecAdd) const
	{
		return { this->x + vecAdd.x, this->y + vecAdd.y };
	}

	Vector2D_t operator-(const Vector2D_t& vecSubtract) const
	{
		return { this->x - vecSubtract.x, this->y - vecSubtract.y };
	}

	Vector2D_t operator*(const Vector2D_t& vecMultiply) const
	{
		return { this->x * vecMultiply.x, this->y * vecMultiply.y };
	}

	Vector2D_t operator/(const Vector2D_t& vecDivide) const
	{
		return { this->x / vecDivide.x, this->y / vecDivide.y };
	}

	Vector2D_t operator+(const float flAdd) const
	{
		return { this->x + flAdd, this->y + flAdd };
	}

	Vector2D_t operator-(const float flSubtract) const
	{
		return { this->x - flSubtract, this->y - flSubtract };
	}

	Vector2D_t operator*(const float flMultiply) const
	{
		return { this->x * flMultiply, this->y * flMultiply };
	}

	Vector2D_t operator/(const float flDivide) const
	{
		return { this->x / flDivide, this->y / flDivide };
	}

	float Length2D(Vector2D_t _this)
	{
		return std::sqrtf(Length2DSqr(_this));
	}
	float dist(Vector2D_t a, Vector2D_t b)
	{
		return Length2D(a - b);
	}
	float x = 0.0f, y = 0.0f;
};

struct Vector_t
{
	constexpr Vector_t(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f) :
		x(x), y(y), z(z) { }

	constexpr Vector_t(const float* arrVector) :
		x(arrVector[0]), y(arrVector[1]), z(arrVector[2]) { }

	constexpr Vector_t(const Vector2D_t& vecBase2D) :
		x(vecBase2D.x), y(vecBase2D.y) { }

#pragma region vector_array_operators

	[[nodiscard]] float& operator[](const int nIndex)
	{
		return reinterpret_cast<float*>(this)[nIndex];
	}

	[[nodiscard]] const float& operator[](const int nIndex) const
	{
		return reinterpret_cast<const float*>(this)[nIndex];
	}

#pragma endregion

#pragma region vector_relational_operators

	bool operator==(const Vector_t& vecBase) const
	{
		return this->IsEqual(vecBase);
	}

	bool operator!=(const Vector_t& vecBase) const
	{
		return !this->IsEqual(vecBase);
	}

	float dot(const Vector_t& vOther) const
	{
		return (x * vOther.x + y * vOther.y + z * vOther.z);
	}

#pragma endregion

#pragma region vector_assignment_operators

	constexpr Vector_t& operator=(const Vector_t& vecBase)
	{
		this->x = vecBase.x;
		this->y = vecBase.y;
		this->z = vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator=(const Vector2D_t& vecBase2D)
	{
		this->x = vecBase2D.x;
		this->y = vecBase2D.y;
		this->z = 0.0f;
		return *this;
	}

#pragma endregion

#pragma region vector_arithmetic_assignment_operators

	constexpr Vector_t& operator+=(const Vector_t& vecBase)
	{
		this->x += vecBase.x;
		this->y += vecBase.y;
		this->z += vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator-=(const Vector_t& vecBase)
	{
		this->x -= vecBase.x;
		this->y -= vecBase.y;
		this->z -= vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator*=(const Vector_t& vecBase)
	{
		this->x *= vecBase.x;
		this->y *= vecBase.y;
		this->z *= vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator/=(const Vector_t& vecBase)
	{
		this->x /= vecBase.x;
		this->y /= vecBase.y;
		this->z /= vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator+=(const float flAdd)
	{
		this->x += flAdd;
		this->y += flAdd;
		this->z += flAdd;
		return *this;
	}

	constexpr Vector_t& operator-=(const float flSubtract)
	{
		this->x -= flSubtract;
		this->y -= flSubtract;
		this->z -= flSubtract;
		return *this;
	}

	constexpr Vector_t& operator*=(const float flMultiply)
	{
		this->x *= flMultiply;
		this->y *= flMultiply;
		this->z *= flMultiply;
		return *this;
	}

	constexpr Vector_t& operator/=(const float flDivide)
	{
		this->x /= flDivide;
		this->y /= flDivide;
		this->z /= flDivide;
		return *this;
	}

#pragma endregion

#pragma region vector_arithmetic_unary_operators

	constexpr Vector_t& operator-()
	{
		this->x = -this->x;
		this->y = -this->y;
		this->z = -this->z;
		return *this;
	}

	constexpr Vector_t operator-() const
	{
		return { -this->x, -this->y, -this->z };
	}

#pragma endregion

#pragma region vector_arithmetic_ternary_operators

	__forceinline float dot_absolute(const Vector_t& other) const {
		return std::fabs(x * other.x) + std::fabs(y * other.y) + std::fabs(z * other.z);
	}

	Vector_t operator+(const Vector_t& vecAdd) const
	{
		return { this->x + vecAdd.x, this->y + vecAdd.y, this->z + vecAdd.z };
	}

	Vector_t operator-(const Vector_t& vecSubtract) const
	{
		return { this->x - vecSubtract.x, this->y - vecSubtract.y, this->z - vecSubtract.z };
	}

	Vector_t operator*(const Vector_t& vecMultiply) const
	{
		return { this->x * vecMultiply.x, this->y * vecMultiply.y, this->z * vecMultiply.z };
	}

	Vector_t operator/(const Vector_t& vecDivide) const
	{
		return { this->x / vecDivide.x, this->y / vecDivide.y, this->z / vecDivide.z };
	}

	Vector_t operator+(const float flAdd) const
	{
		return { this->x + flAdd, this->y + flAdd, this->z + flAdd };
	}

	Vector_t operator-(const float flSubtract) const
	{
		return { this->x - flSubtract, this->y - flSubtract, this->z - flSubtract };
	}

	Vector_t operator*(const float flMultiply) const
	{
		return { this->x * flMultiply, this->y * flMultiply, this->z * flMultiply };
	}

	Vector_t operator/(const float flDivide) const
	{
		return { this->x / flDivide, this->y / flDivide, this->z / flDivide };
	}

#pragma endregion

	/// @returns: true if each component of the vector is finite, false otherwise
	[[nodiscard]] bool IsValid() const
	{
		return std::isfinite(this->x) && std::isfinite(this->y) && std::isfinite(this->z);
	}

	constexpr void Invalidate()
	{
		this->x = this->y = this->z = std::numeric_limits<float>::infinity();
	}

	/// @returns: true if each component of the vector equals to another, false otherwise
	[[nodiscard]] bool IsEqual(const Vector_t& vecEqual, const float flErrorMargin = std::numeric_limits<float>::epsilon()) const
	{
		return (std::fabsf(this->x - vecEqual.x) < flErrorMargin && std::fabsf(this->y - vecEqual.y) < flErrorMargin && std::fabsf(this->z - vecEqual.z) < flErrorMargin);
	}

	/// @returns: true if each component of the vector equals to zero, false otherwise
	[[nodiscard]] bool IsZero() const
	{
		// @note: to make this implementation right, we should use fpclassify here, but game aren't doing same, probably it's better to keep this same, just ensure that it will be compiled same
		return (this->x == 0.0f && this->y == 0.0f && this->z == 0.0f);
	}

	[[nodiscard]] float Length() const
	{
		return std::sqrtf(this->LengthSqr());
	}

	[[nodiscard]] constexpr float LengthSqr() const
	{
		return DotProduct(*this);
	}

	[[nodiscard]] float Length2D() const
	{
		return std::sqrtf(this->Length2DSqr());
	}

	[[nodiscard]] constexpr float Length2DSqr() const
	{
		return (this->x * this->x + this->y * this->y);
	}

	[[nodiscard]] float DistTo(const Vector_t& vecEnd) const
	{
		return (*this - vecEnd).Length();
	}

	[[nodiscard]] constexpr float DistToSqr(const Vector_t& vecEnd) const
	{
		return (*this - vecEnd).LengthSqr();
	}
	__forceinline float normalize_inplace() {
		__m128 v = _mm_set_ps(0, z, y, x);
		__m128 sq = _mm_mul_ps(v, v);
		__m128 sum = _mm_hadd_ps(sq, sq);
		sum = _mm_hadd_ps(sum, sum);
		__m128 length = _mm_sqrt_ps(sum);
		__m128 inv_length = _mm_div_ps(_mm_set_ps1(1.0f), _mm_add_ps(length, _mm_set_ps1(std::numeric_limits<float>::epsilon())));
		__m128 normalized = _mm_mul_ps(v, inv_length);

		x = _mm_cvtss_f32(normalized);
		y = _mm_cvtss_f32(_mm_shuffle_ps(normalized, normalized, _MM_SHUFFLE(1, 1, 1, 1)));
		z = _mm_cvtss_f32(_mm_shuffle_ps(normalized, normalized, _MM_SHUFFLE(2, 2, 2, 2)));

		return _mm_cvtss_f32(length);
	}
	/// normalize magnitude of each component of the vector
	/// @returns: length of the vector
	float NormalizeInPlace()
	{
		const float flLength = this->Length();
		const float flRadius = 1.0f / (flLength + std::numeric_limits<float>::epsilon());

		this->x *= flRadius;
		this->y *= flRadius;
		this->z *= flRadius;

		return flLength;
	}

	Vector_t normalized() const
	{
		Vector_t res = *this;
		float l = res.Length();

		if (l) //-V550
			res /= l;
		else
			res.x = res.y = res.z = 0.0f;

		return res;
	}
	/// normalize magnitude of each component of the vector
	/// @returns: copy of the vector with normalized components
	[[nodiscard]] Vector_t Normalized() const
	{
		Vector_t vecOut = *this;
		vecOut.NormalizeInPlace();
		return vecOut;
	}

	Vector_t normalize_angle()
	{
		while (x < -180.0f)
			x += 360.0f;
		while (x > 180.0f)
			x -= 360.0f;

		while (y < -180.0f)
			y += 360.0f;
		while (y > 180.0f)
			y -= 360.0f;

		while (z < -180.0f)
			z += 360.0f;
		while (z > 180.0f)
			z -= 360.0f;

		return *this;
	}
	float normalize() const
	{
		Vector_t res = *this;
		float l = res.Length();

		if (l) //-V550
			res /= l;
		else
			res.x = res.y = res.z = 0.0f;

		return l;
	}

	[[nodiscard]] constexpr float DotProduct(const Vector_t& vecDot) const
	{
		return (this->x * vecDot.x + this->y * vecDot.y + this->z * vecDot.z);
	}

	[[nodiscard]] constexpr Vector_t CrossProduct(const Vector_t& vecCross) const
	{
		return { this->y * vecCross.z - this->z * vecCross.y, this->z * vecCross.x - this->x * vecCross.z, this->x * vecCross.y - this->y * vecCross.x };
	}

	__forceinline Vector_t cross(const Vector_t& other) const {
		__m128 a = _mm_set_ps(0, z, y, x);
		__m128 b = _mm_set_ps(0, other.z, other.y, other.x);
		__m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
		__m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
		__m128 c = _mm_sub_ps(
			_mm_mul_ps(a, b_yzx),
			_mm_mul_ps(a_yzx, b)
		);
		return Vector_t(_mm_cvtss_f32(_mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1))),
			_mm_cvtss_f32(_mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 1, 0, 2))),
			_mm_cvtss_f32(_mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 2, 1, 0))));
	}

	/// @returns: transformed vector by given transformation matrix
	[[nodiscard]] Vector_t Transform(const Matrix3x4_t& matTransform) const;

	[[nodiscard]] Vector2D_t ToVector2D() const
	{
		return { this->x, this->y };
	}

	/// convert forward direction vector to other direction vectors
	/// @param[out] pvecRight [optional] output for converted right vector
	/// @param[out] pvecUp [optional] output for converted up vector
	void ToDirections(Vector_t* pvecRight, Vector_t* pvecUp) const
	{
		if (std::fabsf(this->x) < 1e-6f && std::fabsf(this->y) < 1e-6f)
		{
			// pitch 90 degrees up/down from identity
			if (pvecRight != nullptr)
			{
				pvecRight->x = 0.0f;
				pvecRight->y = -1.0f;
				pvecRight->z = 0.0f;
			}

			if (pvecUp != nullptr)
			{
				pvecUp->x = -this->z;
				pvecUp->y = 0.0f;
				pvecUp->z = 0.0f;
			}
		}
		else
		{
			if (pvecRight != nullptr)
			{
				pvecRight->x = this->y;
				pvecRight->y = -this->x;
				pvecRight->z = 0.0f;
				pvecRight->NormalizeInPlace();
			}

			if (pvecUp != nullptr)
			{
				pvecUp->x = (-this->x) * this->z;
				pvecUp->y = -(this->y * this->z);
				pvecUp->z = this->y * this->y - (-this->x) * this->x;
				pvecUp->NormalizeInPlace();
			}
		}
	}

	/// @returns: 2D angles converted from direction vector
	[[nodiscard]] QAngle_t ToAngles() const;

	/// @returns: matrix converted from forward direction vector
	[[nodiscard]] Matrix3x4_t ToMatrix() const;

	float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct Vector4D_t
{
	constexpr Vector4D_t(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f, const float w = 0.0f) :
		x(x), y(y), z(z), w(w) { }

	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
};

struct alignas(16) VectorAligned_t : Vector_t
{
	VectorAligned_t() = default;

	explicit VectorAligned_t(const Vector_t& vecBase)
	{
		this->x = vecBase.x;
		this->y = vecBase.y;
		this->z = vecBase.z;
		this->w = 0.0f;
	}

	constexpr VectorAligned_t& operator=(const Vector_t& vecBase)
	{
		this->x = vecBase.x;
		this->y = vecBase.y;
		this->z = vecBase.z;
		this->w = 0.0f;
		return *this;
	}

	float w = 0.0f;
};

static_assert(alignof(VectorAligned_t) == 16);
