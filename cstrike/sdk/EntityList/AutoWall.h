#pragma once
#include "../../SDK/Interfaces/CGameTraceManager.h"
#include "../../SDK\Interfaces\ienginecvar.h"
#include "../../SDK/Entity.h"
#include "../../core/sdk.h" 

struct penetration_data_t
{
	float m_damage{};
	uint8_t m_hitgroup{};

	int m_hitbox{};

	bool m_penetrated{ true };
};

struct handle_bulllet_penetration_data_t
{
	handle_bulllet_penetration_data_t(const float damage, const float pen, const float range_mod, const float range, const int pen_count, const bool failed) :
		m_damage(damage),
		m_penetration(pen),
		m_range_modifier(range_mod),
		m_range(range),
		m_pen_count(pen_count),
		m_failed(failed)
	{ }

	float m_damage{};
	float m_penetration{};
	float m_range_modifier{};
	float m_range{};
	int m_pen_count{};
	bool m_failed{};
};

enum e_hitgroups
{
	hitgroup_gear,
	hitgroup_head,
	hitgroup_chest,
	hitgroup_stomach,
	hitgroup_left_hand,
	hitgroup_right_hand,
	hitgroup_left_leg,
	hitgroup_right_leg,
	hitgroup_neck
};

class autowall {
public:
    enum class HitGroup : std::uint32_t {
        INVALID = -1,
        GENERIC = 0,
        HEAD = 1,
        CHEST = 2,
        STOMACH = 3,
        LEFTARM = 4,
        RIGHTARM = 5,
        LEFTLEG = 6,
        RIGHTLEG = 7,
        NECK = 8,
        UNUSED = 9,
        GEAR = 10,
        SPECIAL = 11,
        COUNT = 12
    };

    struct BulletData {
        float damage;
        float penetration;
        float range_modifier;
        float range;
        int penetration_count;
        bool failed;

        BulletData(float dmg, float pen, float range_mod, float rng, int pen_count) noexcept
            : damage(dmg)
            , penetration(pen)
            , range_modifier(range_mod)
            , range(rng)
            , penetration_count(pen_count)
            , failed(false) {}
    };

    static float penetration_damage(
        const Vector_t& local_pos,
        const Vector_t& target_pos,
        C_BaseEntity* local,
        C_CSPlayerPawn* local_pawn,
        C_BaseEntity* target,
        C_CSPlayerPawn* target_pawn,
        bool has_heavy_armor,
        int target_team)
    {
        constexpr float MAX_TRACE_LENGTH = 3000.f;
        constexpr float DAMAGE_SCALE = 500.f;
        constexpr uint32_t TARGET_CONTENTS = 268828672;

        trace_data_t trace_data{};
        trace_data.m_arr_pointer = &trace_data.m_arr;

        __m128 start = _mm_set_ps(0, local_pos.z, local_pos.y, local_pos.x);
        __m128 end = _mm_set_ps(0, target_pos.z, target_pos.y, target_pos.x);
        __m128 dir = _mm_sub_ps(end, start);

        __m128 length_sq = _mm_dp_ps(dir, dir, 0x7F);
        __m128 inv_length = _mm_rsqrt_ps(length_sq);
        dir = _mm_mul_ps(dir, inv_length);

        __m128 scaled_dir = _mm_mul_ps(dir, _mm_set1_ps(SDK::WepRange));
        VectorAligned_t end_pos = {};
        _mm_store_ps((float*)&end_pos, scaled_dir);

        TraceFilter_t filter(0x1C300B, local_pawn, nullptr, 3);

        I::GameTraceManager->CreateTrace(&trace_data, local_pos, end_pos, filter, 4);

        BulletData bullet_data(
            static_cast<float>(SDK::WepDamage),
            SDK::WepPen,
            SDK::WepRangeMod,
            SDK::WepRange,
            4
        );

        float current_damage = static_cast<float>(SDK::WepDamage);
        float trace_length = 0.f;
        float max_range = SDK::WepRange;

        __m128 damage_vec = _mm_set1_ps(current_damage);
        __m128 range_mod_vec = _mm_set1_ps(SDK::WepRangeMod);
        __m128 trace_scale = _mm_set1_ps(1.0f / DAMAGE_SCALE);

        if (trace_data.m_num_update > 0) {
            for (int i = 0; i < trace_data.m_num_update; ++i) {
                auto* value = reinterpret_cast<UpdateValueT*>(
                    reinterpret_cast<std::uintptr_t>(trace_data.m_pointer_update_value) +
                    i * sizeof(UpdateValueT)
                    );

                game_trace_t game_trace{};
                I::GameTraceManager->InitializeTraceInfo(&game_trace);
                I::GameTraceManager->get_trace_info(
                    &trace_data,
                    &game_trace,
                    0.0f,
                    reinterpret_cast<void*>(
                        reinterpret_cast<std::uintptr_t>(trace_data.m_arr.data()) +
                        sizeof(trace_arr_element_t) * (value->handleIdx & 0x7fffu)
                        )
                );

                max_range -= trace_length;
                if (game_trace.Fraction == 1.0f) break;

                trace_length += game_trace.Fraction * max_range;

                __m128 length_factor = _mm_mul_ps(_mm_set1_ps(trace_length), trace_scale);
                __m128 power = _mm_pow_ps(range_mod_vec, length_factor);
                damage_vec = _mm_mul_ps(damage_vec, power);
                _mm_store_ss(&current_damage, damage_vec);

                if (trace_length > MAX_TRACE_LENGTH) break;

                if (game_trace.HitEntity &&
                    game_trace.HitEntity->GetRefEHandle().GetEntryIndex() == target_pawn->GetRefEHandle().GetEntryIndex() &&
                    game_trace.Contents == TARGET_CONTENTS)
                {
                    scale_damage(
                        static_cast<int>(game_trace.HitboxData->m_nHitGroup),
                        target_pawn,
                        SDK::WepArmorRatio,
                        SDK::WepHeadShotMulti,
                        &current_damage,
                        has_heavy_armor,
                        target_team
                    );
                    return current_damage;
                }

                if (I::GameTraceManager->handle_bullet_penetration(
                    &trace_data,
                    reinterpret_cast<void*>(&bullet_data),
                    value,
                    false
                )) return 0.0f;

                current_damage = bullet_data.damage;
                damage_vec = _mm_set1_ps(current_damage);
            }
        }

        return 0.0f;
    }

private:
    static void scale_damage(const int hitGroup, C_CSPlayerPawn* player,
        const float armorRatio, const float headshotMultiplier,
        float* damage, bool hasHeavyArmor, int team)
    {
        // Cache these values
        static const auto get_damage_scales = []() {
            static CConVar* mp_damage_scale_ct_head = I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_ct_head"));
            static CConVar* mp_damage_scale_t_head = I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_t_head"));
            static CConVar* mp_damage_scale_ct_body = I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_ct_body"));
            static CConVar* mp_damage_scale_t_body = I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_t_body"));
            return std::make_tuple(mp_damage_scale_ct_head, mp_damage_scale_t_head,
                mp_damage_scale_ct_body, mp_damage_scale_t_body);
            };

        static const auto [ct_head, t_head, ct_body, t_body] = get_damage_scales();

        const float headScale = (team == 3 ? ct_head->value.fl : t_head->value.fl) * (hasHeavyArmor ? 0.5f : 1.0f);
        const float bodyScale = (team == 2 ? ct_body->value.fl : t_body->value.fl);

        // Use SIMD for multiplications
        __m128 damage_vec = _mm_set1_ps(*damage);
        __m128 scale_vec;

        switch (hitGroup) {
        case static_cast<int>(HitGroup::HEAD):
            scale_vec = _mm_set1_ps(headshotMultiplier * headScale);
            break;
        case static_cast<int>(HitGroup::CHEST):
        case static_cast<int>(HitGroup::LEFTARM):
        case static_cast<int>(HitGroup::RIGHTARM):
        case static_cast<int>(HitGroup::NECK):
            scale_vec = _mm_set1_ps(bodyScale);
            break;
        case static_cast<int>(HitGroup::STOMACH):
            scale_vec = _mm_set1_ps(1.25f * bodyScale);
            break;
        case static_cast<int>(HitGroup::LEFTLEG):
        case static_cast<int>(HitGroup::RIGHTLEG):
            scale_vec = _mm_set1_ps(0.75f * bodyScale);
            break;
        default:
            return;
        }

        damage_vec = _mm_mul_ps(damage_vec, scale_vec);
        _mm_store_ss(damage, damage_vec);

        if (player->hasArmour(hitGroup)) {
            const int armorValue = player->GetArmorValue();
            const float heavyArmorBonus = hasHeavyArmor ? 0.25f : 1.0f;
            const float armorBonus = hasHeavyArmor ? 0.33f : 0.5f;
            const float adjustedArmorRatio = armorRatio * (hasHeavyArmor ? 0.2f : 0.5f);

            const float healthDamage = *damage * adjustedArmorRatio;
            const float armorDamage = (*damage - healthDamage) * heavyArmorBonus * armorBonus;

            *damage = armorDamage > static_cast<float>(armorValue)
                ? *damage - static_cast<float>(armorValue) / armorBonus
                : healthDamage;
        }
    }
};

class c_auto_wall
{
public:
	void scale_damage(int hitgroup, C_CSPlayerPawn* entity, CCSWeaponBaseVData* weapon_data, float& damage);
	bool fire_bullet(Vector_t start, QAngle_t end, C_CSPlayerPawn* target, CCSWeaponBaseVData* weapon_data, penetration_data_t& pen_data, bool is_taser = false);
};

inline std::unique_ptr<c_auto_wall> g_auto_wall = std::make_unique<c_auto_wall>();
