#pragma once
#include "core/sdk.h"
#include "sdk/EntityList/Events.h"
#include "sdk\datatypes\stronghandle.h"
#include <mutex>

class particle_effect
{
public:
	const char* name{};
	char pad[0x30]{};
};

class particle_information
{
public:
	float time{};
	float width{};
	float unk2{};
};

class particle_data
{
public:
	Vector_t* positions{};
	char pad[0x74]{};
	float* times{};
	void* unk_ptr{};
	char pad2[0x28];
	float* times2{};
	char pad3[0x98];
	void* unk_ptr2{};
};

struct particle_color
{
	float r;
	float g;
	float b;
};

class c_particle_snapshot {
public:
	void draw(int count, void* data);
};

class c_game_particle_manager_system
{
public:
	void create_effect_index(unsigned int* effect_index, particle_effect* effect_data);
	void create_effect2(unsigned int* pEffectIndex, const char* szName);
	void SetEffect(unsigned int effect_index, int unk, void* clr, int unk2);
	void fnInitEffect(int effect_index, unsigned int unk, const CStrongHandle<c_particle_snapshot>* particle_snapshot);
};

class c_particle_manager
{
public:
	void create_snapshot(CStrongHandle<c_particle_snapshot>* out_particle_snapshot);
	void draw(CStrongHandle<c_particle_snapshot>* particle_snapshot, int count, void* data);
};
class tracer_info
{
public:
	unsigned int effect_index = -1;
	Vector_t* positions = nullptr;
	float* times = nullptr;
	CStrongHandle<c_particle_snapshot> handle_snapshot_particle{};
	particle_data particle_data_;
};

struct HitLog
{
	HitLog(Vector_t pos, Vector_t EyePos,float time) :
		pos(pos), time(time), EyePos(EyePos) { }

	Vector_t pos;
	Vector_t EyePos;
	float time;
};

class cbullet_tracer
{
public:
	void AddHit(Vector_t position, Vector_t EyePos);
	void render();
	std::vector<HitLog> logs;
	void add_bullet_trace(Vector_t start, Vector_t end, Color_t clr_);
private:
	std::mutex MHitMutex;
	class cbullet_tracer_info
	{
	public:
		cbullet_tracer_info(Vector_t src, Vector_t dst, float time, Color_t color)
		{
			this->src = src;
			this->dst = dst;
			this->time = time;
			this->color = color;
		}

		Vector_t src, dst;
		float time;
		Color_t color;
	};
};
inline std::unique_ptr<cbullet_tracer> g_BulletTrace = std::make_unique<cbullet_tracer>();

