//#include <vector>
//#include <string>
//#include <imgui/imgui.h>
//#include "utilities/draw.h"
//#include "GrenadeVisuals.h"
//#include "core/sdk.h"
//#include "sdk/interfaces/cgametracemanager.h"
//#include "sdk/const.h"
//#include "sdk/entity.h"
//#include "sdk/interfaces/ccsgoinput.h"
//#include "core/variables.h"
//#include "sdk/interfaces/iengineclient.h"
//#include "sdk/interfaces/ienginecvar.h"
//
//namespace GrenadePrediction {
//	std::vector<std::pair<ImVec2, ImVec2>> screenPoints;
//	std::vector<std::pair<ImVec2, ImVec2>> endPoints;
//	std::vector<std::pair<ImVec2, std::string>> dmgPoints;
//
//	int grenadeAct{ 1 };
//
//	void traceHull(Vector_t& src, Vector_t& end, GameTrace_t* tr)
//	{
//		Ray_t ray{};
//		TraceFilter_t filter{ 0x1C3003, SDK::LocalPawn, NULL, 4 };
//
//		I::GameTraceManager->TraceShape2(&ray, src, end, &filter, tr);
//	}
//
//	void setup(Vector_t& vecSrc, Vector_t& vecThrow, QAngle_t viewangles) noexcept
//	{
//		QAngle_t angThrow = viewangles;
//		float pitch = angThrow.x;
//
//		if (pitch <= 90.0f)
//		{
//			if (pitch < -90.0f)
//			{
//				pitch += 360.0f;
//			}
//		}
//		else
//		{
//			pitch -= 360.0f;
//		}
//
//		float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
//		angThrow.x = a;
//
//		float flVel = 750.0f * 0.9f;
//
//		static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
//		float b = power[grenadeAct];
//		b = b * 0.7f;
//		b = b + 0.3f;
//		flVel *= b;
//
//		Vector_t vForward, vRight, vUp;
//		angThrow.ToDirections(&vForward, &vRight, &vUp);
//
//		vecSrc = SDK::LocalPawn->GetEyePosition();
//		float off = (power[grenadeAct] * 12.0f) - 12.0f;
//		vecSrc.z += off;
//
//		GameTrace_t tr;
//		Vector_t vecDest = vecSrc;
//		vecDest += vForward * 22.0f;
//
//		traceHull(vecSrc, vecDest, &tr);
//
//		Vector_t vecBack = vForward; vecBack *= 6.0f;
//		vecSrc = tr.m_vecEndPos;
//		vecSrc -= vecBack;
//
//		vecThrow = SDK::LocalPawn->GetAbsVelocity(); vecThrow *= 1.25f;
//		vecThrow += vForward * flVel;
//	}
//
//	int physicsClipVelocity(const Vector_t& in, const Vector_t& normal, Vector_t& out, float overbounce) noexcept
//	{
//		static const float STOP_EPSILON = 0.1f;
//
//		float    backoff;
//		float    change;
//		float    angle;
//		int        i, blocked;
//
//		blocked = 0;
//
//		angle = normal[2];
//
//		if (angle > 0)
//		{
//			blocked |= 1;        // floor
//		}
//		if (!angle)
//		{
//			blocked |= 2;        // step
//		}
//
//		backoff = in.DotProduct(normal) * overbounce;
//
//		for (i = 0; i < 3; i++)
//		{
//			change = normal[i] * backoff;
//			out[i] = in[i] - change;
//			if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
//			{
//				out[i] = 0;
//			}
//		}
//
//		return blocked;
//	}
//
//	void pushEntity(Vector_t& src, const Vector_t& move, GameTrace_t* tr) noexcept
//	{
//		Vector_t vecAbsEnd = src;
//		vecAbsEnd += move;
//		traceHull(src, vecAbsEnd, tr);
//	}
//
//	void resolveFlyCollisionCustom(GameTrace_t* tr, Vector_t& vecVelocity, float interval) noexcept
//	{
//		// Calculate elasticity
//		const float surfaceElasticity = 1.0;
//		const float grenadeElasticity = 0.45f;
//		float totalElasticity = grenadeElasticity * surfaceElasticity;
//		if (totalElasticity > 0.9f) totalElasticity = 0.9f;
//		if (totalElasticity < 0.0f) totalElasticity = 0.0f;
//
//		// Calculate bounce
//		Vector_t vecAbsVelocity;
//		physicsClipVelocity(vecVelocity, tr->m_vecNormal, vecAbsVelocity, 2.0f);
//		vecAbsVelocity *= totalElasticity;
//
//		float speedSqr = vecAbsVelocity.LengthSqr();
//		static const float minSpeedSqr = 20.0f * 20.0f;
//
//		if (speedSqr < minSpeedSqr)
//		{
//			vecAbsVelocity.x = 0.0f;
//			vecAbsVelocity.y = 0.0f;
//			vecAbsVelocity.z = 0.0f;
//		}
//
//		if (tr->m_vecNormal.z > 0.7f)
//		{
//			vecVelocity = vecAbsVelocity;
//			vecAbsVelocity *= ((1.0f - tr->m_flFraction) * interval);
//			pushEntity(tr->m_vecEndPos, vecAbsVelocity, tr);
//		}
//		else
//		{
//			vecVelocity = vecAbsVelocity;
//		}
//	}
//
//	void addGravityMove(Vector_t& move, Vector_t& vel, float frametime, bool onground) noexcept
//	{
//		Vector_t basevel{ 0.0f, 0.0f, 0.0f };
//
//		move.x = (vel.x + basevel.x) * frametime;
//		move.y = (vel.y + basevel.y) * frametime;
//
//		if (onground)
//		{
//			move.z = (vel.z + basevel.z) * frametime;
//		}
//		else
//		{
//			float gravity = 800.0f * 0.4f;
//			float newZ = vel.z - (gravity * frametime);
//			move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;
//			vel.z = newZ;
//		}
//	}
//
//	enum ACT
//	{
//		ACT_NONE,
//		ACT_THROW,
//		ACT_LOB,
//		ACT_DROP,
//	};
//
//	void tick(int buttons) noexcept
//	{
//		bool in_attack = buttons & IN_ATTACK;
//		bool in_attack2 = buttons & IN_SECOND_ATTACK;
//
//		grenadeAct = (in_attack && in_attack2) ? ACT_LOB :
//			(in_attack2) ? ACT_DROP :
//			(in_attack) ? ACT_THROW :
//			ACT_NONE;
//	}
//
//	bool checkDetonate(const Vector_t& vecThrow, const GameTrace_t* tr, int tick, float interval, C_CSWeaponBase* activeWeapon) noexcept
//	{
//		int di = activeWeapon->GetAttributeManager()->GetItem()->GetItemDefinitionIndex();
//		static float tplen = 0.1f;
//		switch (di)
//		{
//		case WEAPON_SMOKE_GRENADE:
//		case WEAPON_DECOY_GRENADE:
//			if (vecThrow.Length2D() < tplen)
//			{
//				int det_tick_mod = (int)(0.2f / interval);
//				return !(tick % det_tick_mod);
//			}
//			return false;
//		case WEAPON_MOLOTOV:
//		case WEAPON_INCENDIARY_GRENADE:
//			if (tr->m_flFraction != 1.0f && tr->m_vecNormal.z > 0.7f)
//				return true;
//		case WEAPON_FLASHBANG:
//		case WEAPON_HIGH_EXPLOSIVE_GRENADE:
//			return (float)tick * interval > 1.5f && !(tick % (int)(0.2f / interval));
//		default:
//			return false;
//		}
//	}
//
//	float calculateArmor(float damage, int armorValue) noexcept
//	{
//		if (armorValue > 0) {
//			float newDamage = damage * 0.5f;
//			float armor = (damage - newDamage) * 0.5f;
//
//			if (armor > static_cast<float>(armorValue)) {
//				armor = static_cast<float>(armorValue) * (1.f / 0.5f);
//				newDamage = damage - armor;
//			}
//
//			damage = newDamage;
//		}
//		return damage;
//	}
//
//	void run(CUserCmd* cmd, CCSGOInput* input) noexcept
//	{
//		screenPoints.clear();
//		endPoints.clear();
//		dmgPoints.clear();
//
//		//if (!C_GET(bool, Vars.bGrenadeVisuals))
//		//	return;
//
//		if (SDK::LocalPawn->nActualMoveType() == MOVETYPE_NOCLIP)
//			return;
//
//		tick(cmd->csgoUserCmd.pBaseCmd->pInButtonState->nValue);
//
//		auto activeWeapon = SDK::LocalPawn->GetActiveWeapon();
//		if (!activeWeapon || !activeWeapon->IsNade())
//			return;
//
//		const auto itemDefinition = activeWeapon->GetAttributeManager()->GetItem()->GetItemDefinitionIndex();
//		if (itemDefinition != WEAPON_SMOKE_GRENADE
//			&& itemDefinition != WEAPON_DECOY_GRENADE
//			&& itemDefinition != WEAPON_MOLOTOV
//			&& itemDefinition != WEAPON_INCENDIARY_GRENADE
//			&& itemDefinition != WEAPON_FLASHBANG
//			&& itemDefinition != WEAPON_HIGH_EXPLOSIVE_GRENADE)
//			return;
//
//		Vector_t vecSrc, vecThrow;
//		setup(vecSrc, vecThrow, { input->GetViewAngles().x,input->GetViewAngles().y,input->GetViewAngles().z });
//
//		float interval = TICK_INTERVAL;
//		int logstep = static_cast<int>(0.05f / interval);
//		int logtimer = 0;
//
//		std::vector<Vector_t> path;
//
//		for (unsigned int i = 0; i < 256; ++i)
//		{
//			if (!logtimer)
//				path.emplace_back(vecSrc);
//
//			Vector_t move;
//			addGravityMove(move, vecThrow, interval, false);
//
//			// Push entity
//			GameTrace_t tr;
//			pushEntity(vecSrc, move, &tr);
//
//			int result = 0;
//			if (checkDetonate(vecThrow, &tr, i, interval, activeWeapon))
//				result |= 1;
//
//			if (tr.m_flFraction != 1.0f)
//			{
//				result |= 2; // Collision!
//				resolveFlyCollisionCustom(&tr, vecThrow, interval);
//			}
//
//			vecSrc = tr.m_vecEndPos;
//
//			if (result & 1)
//				break;
//
//			if ((result & 2) || logtimer >= logstep)
//				logtimer = 0;
//			else
//				++logtimer;
//		}
//
//		//path.emplace_back(vecSrc);
//
//		Vector_t prev = path[0];
//		ImVec2 nadeStart, nadeEnd;
//		Vector_t lastPos{ };
//		for (auto& nade : path)
//		{
//			if (D::WorldToScreen(prev, &nadeStart) && D::WorldToScreen(nade, &nadeEnd))
//			{
//				screenPoints.emplace_back(std::pair<ImVec2, ImVec2>{ nadeStart, nadeEnd });
//				prev = nade;
//				lastPos = nade;
//			}
//		}
//	}
//
//	void draw() noexcept
//	{
//		if (screenPoints.empty())
//			return;
//
//		auto drawList = ImGui::GetBackgroundDrawList();
//		//	draw nade path
//		for (auto& point : screenPoints)
//			drawList->AddLine(ImVec2(point.first.x, point.first.y), ImVec2(point.second.x, point.second.y), ImColor(255, 255, 255)/*config.misc.grenade_visuals.grenadePredictionColor.GetU32()*/, 1.5f);
//
//	}
//}
//
//void GrenadeVisuals::present() {
//
//
//	if (!SDK::LocalPawn || !SDK::LocalController || !SDK::LocalController->IsPawnAlive() || !SDK::LocalPawn->GetActiveWeapon())
//		return;
//
//	GrenadePrediction::draw();
//}
//
//bool GrenadeVisuals::_createmove(CCSGOInput* pInput) {
//	if (!SDK::LocalPawn || !SDK::LocalController || !SDK::LocalController->IsPawnAlive() || !SDK::LocalPawn->GetActiveWeapon())
//		return false;
//
//	GrenadePrediction::run(SDK::Cmd, pInput);
//
//	return false;
//}