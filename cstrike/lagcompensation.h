#pragma once
#include "sdk/entity.h"
#include <deque>
#include "sdk/interfaces/iengineclient.h"
#include "sdk/interfaces/inetworkclientservice.h"
#include "sdk/interfaces/ienginecvar.h"
#include "EntityCache.h"
#include "sdk/entity.h"

__forceinline Vector_t latency_adjust(const Vector_t& pos, const Vector_t& vel, float latency) {
    __m128 position = _mm_set_ps(0.0f, pos.z, pos.y, pos.x);
    __m128 velocity = _mm_set_ps(0.0f, vel.z, vel.y, vel.x);
    __m128 latencyScalar = _mm_set_ps1(latency);

    __m128 adjusted = _mm_add_ps(
        position,
        _mm_mul_ps(velocity, latencyScalar)
    );

    Vector_t result;
    _mm_store_ps((float*)&result, adjusted);
    return result;
}

class lag_record_t {
public:
    bool valid = true;
    BoneData_t m_bone_arr[128]{};
    float m_simulation_time = 0.f;
    C_BaseEntity* m_target = nullptr;
    Vector_t m_origin{}, m_abs_origin{};
    QAngle_t m_rotation{}, m_abs_rotation{};

    lag_record_t() = default;

    explicit lag_record_t(C_CSPlayerPawn* target) noexcept {
        if (!target) return;

        m_simulation_time = target->GetSimulationTime();
        m_target = target;

        auto scene_node = target->GetGameSceneNode();
        if (!scene_node) return;

        auto skeleton = scene_node->GetSkeletonInstance();
        if (!skeleton) return;

        skeleton->calc_world_space_bones(FLAG_ALL_BONE_FLAGS);

        m_origin = scene_node->m_vecOrigin();
        m_abs_origin = scene_node->GetAbsOrigin();
        m_rotation = scene_node->GetAngleRotation();
        m_abs_rotation = scene_node->GetAbsAngleRotation();

        if (auto bone_data = skeleton->m_modelState().m_pBones) {
            memcpy(&m_bone_arr, bone_data, sizeof(bone_data) * 128);
        }
    }

    template <typename fn_t>
    void execute(C_CSPlayerPawn* target, bool use_prediction, fn_t func) noexcept {
        auto scenenode = target->GetGameSceneNode();
        if (!scenenode) return;

        auto skeleton = scenenode->GetSkeletonInstance();
        if (!skeleton) return;

        struct backup_data_t {
            Vector_t origin;
            Vector_t abs_origin;
            QAngle_t rotation;
            QAngle_t abs_rotation;
            alignas(16) BoneData_t bone_arr[128];
        } backup{
            scenenode->m_vecOrigin(),
            scenenode->GetAbsOrigin(),
            scenenode->GetAngleRotation(),
            scenenode->GetAbsAngleRotation()
        };

        memcpy(&backup.bone_arr, skeleton->m_modelState().m_pBones, sizeof(BoneData_t) * 128);

        if (use_prediction) {
            const float latency = I::Engine->GetNetChannelInfo()->get_network_latency();
            const Vector_t& velocity = target->GetAbsVelocity();

            __m128 velocity_simd = _mm_set_ps(0, velocity.z, velocity.y, velocity.x);
            __m128 latency_simd = _mm_set_ps1(latency);
            __m128 abs_origin_simd = _mm_set_ps(0, m_abs_origin.z, m_abs_origin.y, m_abs_origin.x);

            __m128 predicted_pos = _mm_add_ps(abs_origin_simd, _mm_mul_ps(velocity_simd, latency_simd));

            VectorAligned_t test_predict_good{};
            _mm_store_ps((float*)&test_predict_good, predicted_pos);

            if (backup.abs_origin.DistTo(test_predict_good) < 3.f) {
                scenenode->m_vecOrigin() = latency_adjust(m_origin, velocity, latency);
                scenenode->GetAbsOrigin() = test_predict_good;

                for (int i = 0; i < 128; i += 4) {
                    __m128 bone_x = _mm_set_ps(m_bone_arr[i + 3].vecPosition.x, m_bone_arr[i + 2].vecPosition.x, m_bone_arr[i + 1].vecPosition.x, m_bone_arr[i].vecPosition.x);
                    __m128 bone_y = _mm_set_ps(m_bone_arr[i + 3].vecPosition.y, m_bone_arr[i + 2].vecPosition.y, m_bone_arr[i + 1].vecPosition.y, m_bone_arr[i].vecPosition.y);
                    __m128 bone_z = _mm_set_ps(m_bone_arr[i + 3].vecPosition.z, m_bone_arr[i + 2].vecPosition.z, m_bone_arr[i + 1].vecPosition.z, m_bone_arr[i].vecPosition.z);

                    bone_x = _mm_add_ps(bone_x, _mm_mul_ps(_mm_set_ps1(velocity.x), latency_simd));
                    bone_y = _mm_add_ps(bone_y, _mm_mul_ps(_mm_set_ps1(velocity.y), latency_simd));
                    bone_z = _mm_add_ps(bone_z, _mm_mul_ps(_mm_set_ps1(velocity.z), latency_simd));

                    for (int j = 0; j < 4 && (i + j) < 128; ++j) {
                        m_bone_arr[i + j].vecPosition.x = ((float*)&bone_x)[j];
                        m_bone_arr[i + j].vecPosition.y = ((float*)&bone_y)[j];
                        m_bone_arr[i + j].vecPosition.z = ((float*)&bone_z)[j];
                    }
                }
            }
        }
        else {
            scenenode->m_vecOrigin() = m_origin;
            scenenode->GetAbsOrigin() = m_abs_origin;
            scenenode->GetAngleRotation() = m_rotation;
            scenenode->GetAbsAngleRotation() = m_abs_rotation;
        }

        memcpy(skeleton->m_modelState().m_pBones, &m_bone_arr, sizeof(BoneData_t) * 128);

        func();

        scenenode->m_vecOrigin() = backup.origin;
        scenenode->GetAbsOrigin() = backup.abs_origin;
        scenenode->GetAngleRotation() = backup.rotation;
        scenenode->GetAbsAngleRotation() = backup.abs_rotation;
        memcpy(skeleton->m_modelState().m_pBones, &backup.bone_arr, sizeof(BoneData_t) * 128);
    }

    __forceinline bool is_in_bounds() const {

        static CConVar* sv_maxunlag = I::Cvar->Find(FNV1A::HashConst("sv_maxunlag"));
        float max_unlag = sv_maxunlag ? sv_maxunlag->value.fl : 0.2f;

        auto engine = I::Engine;
        auto netclient = I::NetworkClientService;
   
        float lag_compensation_factor = (engine->GetNetChannelInfo()->get_network_latency() * 0.5f) + engine->GetNetChannelInfo()->get_engine_latency();

        float lastValidTime = I::GlobalVars->flCurTime - max_unlag + lag_compensation_factor;

        lastValidTime += TICK_INTERVAL;

        return lastValidTime < m_simulation_time;
    }

};

class player_records
{
public:
    player_records() {};
    EntityCache::cached_entity player;
    std::deque<lag_record_t> records;

    lag_record_t newest;
    lag_record_t oldest;

    lag_record_t get_record(bool newest_record) {
        lag_record_t bad_return;
        bad_return.valid = false;

        if (newest_record && records.size() && records.back().valid) {
            return records.back();
        }
        else {
            while (records.size() > 32) {

                records.pop_front();
            }
            while (records.size() > 1 && !records.front().is_in_bounds()) {

                records.pop_front();
            }
            if (records.size() && records.front().valid) {
                return records.front();
            }
        }
        return bad_return;
    }
};

class PlayerLog
{
public:
    PlayerLog() {};
    void run();

    player_records player_records[65];
};
inline std::unique_ptr<PlayerLog> g_player_records = std::make_unique<PlayerLog>();
