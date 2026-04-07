#pragma once
#include <mutex>
#include "sdk/EntityList/Events.h"

struct hitmarker_log
{
	hitmarker_log(Vector_t position, Vector_t EyePosition, float time) :
		position(position), Eyeposition(EyePosition), is_world_screen{}, world_position{}, is_world_screen_eye{}, time(time) /*animation()*/ { }

	Vector_t position;
	Vector_t Eyeposition;
	bool is_world_screen;
	bool is_world_screen_eye;
	ImVec2 world_position;
	ImVec2 world_position_eye;

	float time;
	//c_animation animation;
};

class CDamageMarker
{
public:
	CDamageMarker(std::string damage, Vector_t position, float time) :
		damage(damage), position(position), is_world_screen{}, world_position{}, time(time)/*, animation() */{ }

	bool is_world_screen;
	Vector_t position;
	ImVec2 world_position;
	std::string damage;
	float time;
	//c_animation animation;
	//c_animation animation2;
};



class c_hitmarker
{
public:
	void run();
	void UpdateDamage();
	void AddDamage(std::string damage, Vector_t position);
	void RenderDamage();
	void HitMarker(Vector_t EyePos, Vector_t Pos, int damage);
	void add(Vector_t position, Vector_t EyePosition);
	void update_world_screen();
	std::vector<hitmarker_log> logs;
	float CurrentTime;
private:
	std::vector<CDamageMarker> MDamageMarker;
	std::mutex MDamageMutex;
	int Damage = 0;
	std::mutex mutex;
};

inline auto g_hitmarker = std::make_unique<c_hitmarker>();
