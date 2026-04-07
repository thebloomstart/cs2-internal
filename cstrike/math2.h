#pragma once

#include "core/sdk.h"
#include "sdk/entity.h"
#include <DirectXMath.h>

namespace MATH
{
    [[nodiscard]] inline void vec_angles(Vector_t forward, Vector_t* angles)
    {
        float tmp, yaw, pitch;

        if (forward.y == 0.f && forward.x == 0.f) {
            yaw = 0;
            if (forward.z > 0) {
                pitch = 270;
            }
            else {
                pitch = 90.f;
            }
        }
        else {
            yaw = (float)(atan2(forward.y, forward.x) * 180.f / 3.14159265358979323846f);
            if (yaw < 0) {
                yaw += 360.f;
            }
            tmp = (float)sqrt(forward.x * forward.x + forward.y * forward.y);
            pitch = (float)(atan2(-forward.z, tmp) * 180.f / 3.14159265358979323846f);
            if (pitch < 0) {
                pitch += 360.f;
            }
        }
        angles->x = pitch;
        angles->y = yaw;
        angles->z = 0.f;
    }

    static void AngleQangles(const QAngle_t& angles, QAngle_t* forward, QAngle_t* right, QAngle_t* up)
    {
        float angle;
        float sr, sp, sy, cr, cp, cy;

        // Convert angles from degrees to radians
        angle = angles.y * (MATH::_PI / 180.0);
        sy = sin(angle);
        cy = cos(angle);
        angle = angles.x * (MATH::_PI / 180.0);
        sp = sin(angle);
        cp = cos(angle);
        angle = angles.z * (MATH::_PI / 180.0);
        sr = sin(angle);
        cr = cos(angle);

        if (forward)
        {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }

        if (right)
        {
            right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
            right->y = (-1 * sr * sp * sy + -1 * cr * cy);
            right->z = -1 * sr * cp;
        }

        if (up)
        {
            up->x = (cr * sp * cy + -sr * -sy);
            up->y = (cr * sp * sy + -sr * cy);
            up->z = cr * cp;
        }
    }

    static void AngleVectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
    {
        float angle;
        static float sr, sp, sy, cr, cp, cy;

        angle = angles.y * (3.14159265358979323846264338327950288f * 2 / 360);
        sy = sin(angle);
        cy = cos(angle);

        angle = angles.x * (3.14159265358979323846264338327950288f * 2 / 360);
        sp = sin(angle);
        cp = cos(angle);

        angle = angles.z * (3.14159265358979323846264338327950288f * 2 / 360);
        sr = sin(angle);
        cr = cos(angle);

        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;

        right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right->y = (-1 * sr * sp * sy + -1 * cr * cy);
        right->z = -1 * sr * cp;

        up->x = (cr * sp * cy + -sr * -sy);
        up->y = (cr * sp * sy + -sr * cy);
        up->z = cr * cp;
    }

    static void AngleVectors(Vector_t angles, Vector_t& forward, Vector_t& right, Vector_t& up)
    {
        float angle;
        static float sr, sp, sy, cr, cp, cy;

        angle = angles.y * (MATH::_2PI / 360);
        sy = sin(angle);
        cy = cos(angle);

        angle = angles.x * (MATH::_2PI / 360);
        sp = sin(angle);
        cp = cos(angle);

        angle = angles.z * (MATH::_2PI / 360);
        sr = sin(angle);
        cr = cos(angle);

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;

        right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right.y = (-1 * sr * sp * sy + -1 * cr * cy);
        right.z = -1 * sr * cp;

        up.x = (cr * sp * cy + -sr * -sy);
        up.y = (cr * sp * sy + -sr * cy);
        up.z = cr * cp;
    }

    static void AngleVectors(const Vector_t& angles, Vector_t* forward)
    {
        float sp, sy, cp, cy;

        sy = sin(M_DEG2RAD(angles[1]));
        cy = cos(M_DEG2RAD(angles[1]));

        sp = sin(M_DEG2RAD(angles[0]));
        cp = cos(M_DEG2RAD(angles[0]));

        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    static void AngleVectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right = nullptr, Vector_t* up = nullptr)
    {
        float cp = M_COS(M_DEG2RAD(angles.x)), sp = M_SIN(M_DEG2RAD(angles.x));
        float cy = M_COS(M_DEG2RAD(angles.y)), sy = M_SIN(M_DEG2RAD(angles.y));
        float cr = M_COS(M_DEG2RAD(angles.z)), sr = M_SIN(M_DEG2RAD(angles.z));

        if (forward) {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }

        if (right) {
            right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
            right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
            right->z = -1.f * sr * cp;
        }

        if (up) {
            up->x = cr * sp * cy + -sr * -sy;
            up->y = cr * sp * sy + -sr * cy;
            up->z = cr * cp;
        }
    }

    inline void VectorAngles(const Vector_t& vecForward, Vector_t& vecAngles) {
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

    // angles math

    inline QAngle_t CalcAngles(Vector_t viewPos, Vector_t aimPos)
    {
        QAngle_t angle = { 0, 0, 0 };

        Vector_t delta = aimPos - viewPos;

        angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
        angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

        return angle;
    }

    static Vector_t CalculateCameraPosition(Vector_t anchorPos, float distance, QAngle_t viewAngles)
    {
        float yaw = DirectX::XMConvertToRadians(viewAngles.y);
        float pitch = DirectX::XMConvertToRadians(viewAngles.x);

        float x = anchorPos.x + distance * cosf(yaw) * cosf(pitch);
        float y = anchorPos.y + distance * sinf(yaw) * cosf(pitch);
        float z = anchorPos.z + distance * sinf(pitch);

        return Vector_t{ x, y, z };
    }

    static QAngle_t NormalizeAngles(QAngle_t angles)
    {
        while (angles.x > 89.0f)
            angles.x -= 180.0f;
        while (angles.x < -89.0f)
            angles.x += 180.0f;
        while (angles.y > 180.0f)
            angles.y -= 360.0f;
        while (angles.y < -180.0f)
            angles.y += 360.0f;
        angles.z = 0.0f;
        return angles;
    }

    static float GetFOV(const QAngle_t& view_angles, const Vector_t& start, const Vector_t& end) {
        Vector_t dir, * fw = nullptr;

        dir = (end - start).Normalized();

        AngleVectors(view_angles, fw);

        return std::max(M_RAD2DEG(std::acos(fw->DotProduct(dir))), 0.f);
    }

    // matrix

    static void VectorTransform(Vector_t& in, Matrix3x4_t& matrix, Vector_t& out) {
        out = {
            in.DotProduct({ matrix[0][0], matrix[0][1], matrix[0][2] }) + matrix[0][3],
            in.DotProduct({ matrix[1][0], matrix[1][1], matrix[1][2] }) + matrix[1][3],
            in.DotProduct({ matrix[2][0], matrix[2][1], matrix[2][2] }) + matrix[2][3]
        };
    }

    //static Matrix3x4_t TransformToMatrix(CBoneData& in) {
    //    Matrix3x4_t  matrix;

    //    Vector4D_t rot = in.rot;
    //    Vector_t pos = in.pos;

    //    matrix[0][0] = 1.0f - 2.0f * rot.y * rot.y - 2.0f * rot.z * rot.z;
    //    matrix[1][0] = 2.0f * rot.x * rot.y + 2.0f * rot.w * rot.z;
    //    matrix[2][0] = 2.0f * rot.x * rot.z - 2.0f * rot.w * rot.y;

    //    matrix[0][1] = 2.0f * rot.x * rot.y - 2.0f * rot.w * rot.z;
    //    matrix[1][1] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.z * rot.z;
    //    matrix[2][1] = 2.0f * rot.y * rot.z + 2.0f * rot.w * rot.x;

    //    matrix[0][2] = 2.0f * rot.x * rot.z + 2.0f * rot.w * rot.y;
    //    matrix[1][2] = 2.0f * rot.y * rot.z - 2.0f * rot.w * rot.x;
    //    matrix[2][2] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.y * rot.y;

    //    matrix[0][3] = pos.x;
    //    matrix[1][3] = pos.y;
    //    matrix[2][3] = pos.z;

    //    return matrix;
    //}
}
