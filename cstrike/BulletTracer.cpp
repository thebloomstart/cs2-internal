#include "BulletTracer.h"
#include "sdk/interfaces/cgametracemanager.h"
#include "sdk\interfaces\iengineclient.h"
#include "sdk\interfaces\igameresourceservice.h"
#include "sdk\interfaces\cgameentitysystem.h"
#include "sdk/EntityList/EntityList.h"
#include "HitMarker.h"
#include "utilities/draw.h"

void c_particle_snapshot::draw(int count, void* data)
{
	MEM::CallVFunc<void, 1U>(this, count, data);
}

void c_particle_manager::create_snapshot(CStrongHandle<c_particle_snapshot>* out_particle_snapshot)
{
	__int64 nUnknown = 0;
	MEM::CallVFunc<void, 42>(this, out_particle_snapshot, &nUnknown);
}

void c_particle_manager::draw(CStrongHandle<c_particle_snapshot>* particle_snapshot, int count, void* data)
{
	MEM::CallVFunc<void, 43>(this, particle_snapshot, count, data);
}

void c_game_particle_manager_system::create_effect_index(unsigned int* effect_index, particle_effect* effect_data)
{
	using fn_t = void(__fastcall*)(c_game_particle_manager_system*, unsigned int*, particle_effect*);
	static fn_t fn = reinterpret_cast<fn_t>(MEM::FindPattern(CLIENT_DLL, "40 57 48 83 EC 20 49 8B ?? 48 8B"));
	fn(this, effect_index, effect_data);
}
void c_game_particle_manager_system::create_effect2(unsigned int* pEffectIndex, const char* szName)
{
	using fn_t = void(__fastcall*)(void*, uint32_t*, const char*, int, int64_t, int64_t, int64_t, int);
	static fn_t fn = reinterpret_cast<fn_t>(MEM::FindPattern(CLIENT_DLL, "4C 8B ?? ?? 48 83 ?? ?? 48 8B 84 24 ?? ?? ?? ?? 48 8B DA"));
	fn(this, pEffectIndex, szName, 8, 0, 0, 0, 0);
}
void c_game_particle_manager_system::SetEffect(unsigned int effect_index, int unk, void* clr, int unk2)
{
	using fn_t = void(__fastcall*)(c_game_particle_manager_system*, unsigned int, int, void*, int);
	static fn_t fn = reinterpret_cast<fn_t>(MEM::FindPattern(CLIENT_DLL, "48 89 5C 24 ?? 48 89 74 24 10 57 48 83 EC ?? ?? ?? ?? ?? ?? ?? ?? ?? 41 8B F8 8B DA 4C"));
	fn(this, effect_index, unk, clr, unk2);
}

void c_game_particle_manager_system::fnInitEffect(int effect_index, unsigned int unk, const CStrongHandle<c_particle_snapshot>* particle_snapshot)
{
	using fn_t = bool(__fastcall*)(c_game_particle_manager_system*, int, unsigned int, const CStrongHandle<c_particle_snapshot>*);
	static fn_t fn = reinterpret_cast<fn_t>(MEM::FindPattern(CLIENT_DLL, "48 89 74 24 10 57 48 83 EC 30 4C 8B D9 49 8B F9 33 C9 41 8B F0 83 FA FF 0F"));
	fn(this, effect_index, unk, particle_snapshot);
}

std::vector<tracer_info> bullets{};

void cbullet_tracer::add_bullet_trace(Vector_t start, Vector_t end, Color_t clr_)
{
	if (!I::GameParticleManagerSystem) {
		try {
			using fn_t = c_game_particle_manager_system*(__fastcall*)();
			static fn_t fn = reinterpret_cast<fn_t>(MEM::FindPattern(CLIENT_DLL, "48 8b 05 ? ? ? ? c3 cc cc cc cc cc cc cc cc 48 89 5c 24 ? 57")); //Pattern create CheatEngine 
			I::GameParticleManagerSystem = fn();
		}
		catch (...)
		{
			L_PRINT(LOG_WARNING) << "NOT FOUND GameParticleManagerSystem";;
		}
	}

	//if (!I::ParticleManager)
	//{
	//	try {
	//		I::ParticleManager = reinterpret_cast<c_particle_manager*>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, "48 8B 05 ?? ?? ?? ?? 48 8B 08 48 8B 59 68"), 0x3, 0x7));
	//		//I::ParticleManager = *(c_particle_manager**)GetSigPtr(FNV1A::HashConst("PARTICLE_MANAGER"));
	//	}
	//	catch (...)
	//	{
	//		L_PRINT(LOG_WARNING) << "NOT FOUND ParticleManager";;

	//	}
	//}
	try {
		auto& bullet = bullets.emplace_back();

		particle_effect particle_effect_{};
		particle_effect_.name = "particles/entity/spectator_utility_trail.vpcf";// "particles/entity/spectator_utility_trail.vpcf";
		particle_effect_.pad[0] = 8;
		I::GameParticleManagerSystem->create_effect_index(&bullet.effect_index, &particle_effect_);
		particle_color clr = { float(clr_.r), float(clr_.g), float(clr_.b) };
		I::GameParticleManagerSystem->SetEffect(bullet.effect_index, 16, &clr, 0);

		bullet.particle_data_ = {};

		auto dir = (end - start);
		auto stage_2 = start + (dir * 0.3f);
		auto stage_3 = start + (dir * 0.5f);

		Vector_t positions_[] = { start,stage_2, stage_3,end };

		for (int i{}; i < sizeof(positions_) / sizeof(Vector_t); i++) {

			particle_information particle_info{};
			particle_info.time = 4.f;
			particle_info.width = 2.f;
			particle_info.unk2 = 1.f;
			I::GameParticleManagerSystem->SetEffect(bullet.effect_index, 3, &particle_info, 0);

			bullet.positions = new Vector_t[i + 1];
			bullet.times = new float[i + 1];

			for (int j{}; j < i + 1; j++) {
				bullet.positions[j] = positions_[j];
				bullet.times[j] = TICK_INTERVAL * float(j);
			}

			bullet.particle_data_.positions = bullet.positions;
			bullet.particle_data_.times2 = bullet.times;

			I::ParticleManager->create_snapshot(&bullet.handle_snapshot_particle);

			I::GameParticleManagerSystem->fnInitEffect(bullet.effect_index, 0, &bullet.handle_snapshot_particle);
			I::ParticleManager->draw(&bullet.handle_snapshot_particle, i + 1, &bullet.particle_data_);

			delete[] bullet.positions;	
			delete[] bullet.times;
		}
	}
	catch (...)
	{
		L_PRINT(LOG_WARNING) << "HUI TEBE";
	}

}

void cbullet_tracer::AddHit(Vector_t position,Vector_t EyePos)
{
	MHitMutex.lock();
	logs.push_back(HitLog(position, EyePos, I::GlobalVars->flCurTime + 1.f));
	MHitMutex.unlock();
}

void cbullet_tracer::render()
{
	if (!C_GET(bool, Vars.bBulletTrace))
		return;
	if (!I::GlobalVars)
		return;
	//add_bullet_trace(g_BulletTrace->logs[i].pos, g_BulletTrace->logs[i].EyePos, Color_t(255, 255, 255));

	//for (size_t i = 0; i < logs.size(); i++)
	//{
	//	//if (!g_hitmarker->logs[i].is_world_screen)
	//	//	continue;

	//	//if (!g_hitmarker->logs[i].is_world_screen_eye)
	//	//	continue;
	//	
	//	float fadeDuration = 1.0f;
	//	float elapsedTime = I::GlobalVars->flCurTime - g_BulletTrace->logs[i].time;

	//	int alpha = static_cast<int>(std::fmax(0.0f, 255 * (1.0f - elapsedTime / fadeDuration)));
	//	ImColor color = ImColor(255, 255, 255, alpha);

	//	if (elapsedTime >= fadeDuration)
	//	{
	//		logs.erase(logs.begin() + i);
	//		--i;
	//		continue;
	//	}
	//	I::Client->GetSceneDebugOverlay()->AddBox(g_BulletTrace->logs[i].pos, Vector_t(-2, -2, -2), Vector_t(2, 2, 2), Vector_t(0, 0, 0), Color_t(255, 255, 255));
	//		ImVec2 EyePos;
	//	ImVec2 nigerPos;
	//	if (D::WorldToScreen(g_BulletTrace->logs[i].pos, &nigerPos))
	//	{	
	//		if (D::WorldToScreen(g_BulletTrace->logs[i].EyePos, &EyePos))
	//		   ImGui::GetBackgroundDrawList()->AddLine(EyePos, nigerPos, color);
	//	}
	//}
}
