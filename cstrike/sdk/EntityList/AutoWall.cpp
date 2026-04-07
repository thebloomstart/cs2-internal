#include "AutoWall.h"
#include "../../core/sdk.h"
#include "../../sdk\interfaces\cgametracemanager.h"
#include "EntityList.h"

float get_hitgroup_damage_multiplier(C_CSPlayerPawn* player, float head_shot_mult, int hit_group)
{
	float mult = 1.F;
	auto head_scale = player->GetTeam() == 3 ? I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_ct_head"))->value.fl : I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_t_head"))->value.fl;
	auto body_scale = player->GetTeam() == 3 ? I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_ct_body"))->value.fl : I::Cvar->Find(FNV1A::HashConst("mp_damage_scale_t_body"))->value.fl;

	switch (hit_group)
	{
	case 1:
		mult = head_shot_mult * head_scale;
		break;
	case 3:
		mult = 1.25F * body_scale;
		break;
	case 6:
	case 7:
		mult = 0.75F * body_scale;
		break;
	default:
		mult *= body_scale;
	}
	return mult;
}

void c_auto_wall::scale_damage(int hitgroup, C_CSPlayerPawn* entity, CCSWeaponBaseVData* weapon_data, float& damage)
{
	if (!SDK::Alive)
		return;

	damage *= get_hitgroup_damage_multiplier(entity, weapon_data ? weapon_data->m_flHeadshotMultiplier() : 4.f, hitgroup);

	if (entity->GetArmorValue() > 0 && weapon_data)
	{
		if (hitgroup == 1)
		{
			if (entity->GetItemServices() && entity->GetItemServices()->m_bHasHelmet())
			{
				damage *= (weapon_data->m_flArmorRatio() * 0.5f);
			}
		}
		else
		{
			damage *= (weapon_data->m_flArmorRatio() * 0.5f);
		}
	}
}

bool c_auto_wall::fire_bullet(Vector_t start, QAngle_t end, C_CSPlayerPawn* target, CCSWeaponBaseVData* weapon_data, penetration_data_t& pen_data, bool is_taser)
{
	C_CSPlayerPawn* local_player = SDK::LocalPawn;

	if (!local_player)
		return false;

	trace_data_t trace_data = trace_data_t();
	trace_data.m_arr_pointer = &trace_data.m_arr;
	Vector_t EndVec = { end.x, end.y, end.z };
	Vector_t direction = EndVec - start;
	Vector_t end_pos = direction * weapon_data->m_flRange();

	const TraceFilter_t filter(0x1C300B, local_player, nullptr, 3);
	I::GameTraceManager->CreateTrace(&trace_data, start, end_pos, filter, 4);

	handle_bulllet_penetration_data_t handle_bullet_data = handle_bulllet_penetration_data_t(
	static_cast<float>(weapon_data->m_nDamage()),
	weapon_data->m_flPenetration(),
	weapon_data->m_flRange(),
	weapon_data->m_flRangeModifier(),
	4,
	false);

	float max_range = weapon_data->m_flRange();
	float trace_length = 0.0f;
	pen_data.m_damage = static_cast<float>(weapon_data->m_nDamage());

	for (int i = 0; i < trace_data.m_num_update; i++)
	{
		UpdateValueT* modulate_values = reinterpret_cast<UpdateValueT* const>(reinterpret_cast<std::uintptr_t>(trace_data.m_pointer_update_value) + i * sizeof(UpdateValueT));

		game_trace_t game_trace = {};
		I::GameTraceManager->init_trace_info(&game_trace);
		I::GameTraceManager->get_trace_info(&trace_data, &game_trace, 0.0f, reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(trace_data.m_arr.data()) + sizeof(trace_arr_element_t) * (modulate_values->handleIdx & ENT_ENTRY_MASK)));

		max_range -= trace_length;

		if (game_trace.Fraction == 1.0f)
			break;

		trace_length += game_trace.Fraction * max_range;
		pen_data.m_damage *= std::powf(weapon_data->m_flRangeModifier(), trace_length / 500.f);

		if (trace_length > 3000.f)
			break;

		if (game_trace.HitEntity && game_trace.HitEntity->IsPlayer())
		{
			if (pen_data.m_damage < 1.0f)
				continue;

			scale_damage(game_trace.HitboxData->m_nHitGroup, game_trace.HitEntity, weapon_data, pen_data.m_damage);

			pen_data.m_hitbox = game_trace.HitboxData->m_nHitboxId;
			pen_data.m_penetrated = i == 0;

			if (is_taser && pen_data.m_penetrated)
				break;

			return true;
		}

		if (I::GameTraceManager->handle_bullet_penetration(&trace_data, &handle_bullet_data, modulate_values, false))
			return false;

		pen_data.m_damage = handle_bullet_data.m_damage;
	}

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////
