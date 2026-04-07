// used: sdk entity
#include "sdk/entity.h"
// used: cusercmd
#include "sdk/datatypes/usercmd.h"
#include "sdk\interfaces\igameresourceservice.h"
#include "core/interfaces.h"
#include "hitmarker.h"
#include "utilities/draw.h"
#include "core/sdk.h"

void c_hitmarker::add(Vector_t position, Vector_t EyePosition)
{
	mutex.lock();
	logs.push_back(hitmarker_log(position, EyePosition, I::GlobalVars->flCurTime + 1.f));
	mutex.unlock();
}

void c_hitmarker::update_world_screen()
{
	mutex.lock();

	for (int i = 0; i < logs.size(); i++)
	{
		logs[i].is_world_screen = D::WorldToScreen(logs[i].position, &logs[i].world_position);
		logs[i].is_world_screen_eye = D::WorldToScreen(logs[i].Eyeposition, &logs[i].world_position_eye);
	}

	mutex.unlock();
}

void c_hitmarker::run()
{
	if (!SDK::Alive)
		return;

	if (!I::GlobalVars)
		return;

	if (!C_GET(bool, Vars.bHitmarker))
		return;

	std::lock_guard<std::mutex> lock(mutex);

	CurrentTime = I::GlobalVars->flCurTime;
	for (int i = 0; i < logs.size(); ++i)
	{
		float fadeDuration = 1.0f;

		if (!logs[i].is_world_screen)
			continue;

		float elapsedTime = CurrentTime - logs[i].time;

		if (elapsedTime >= fadeDuration)
		{
			logs.erase(logs.begin() + i);
			--i;
			continue;
		}

		int alpha = static_cast<int>(std::fmax(0.0f, 255 * (1.0f - elapsedTime / fadeDuration)));
		ImColor color = ImColor(255, 255, 255, alpha);

		ImVec2 pos = ImVec2(logs[i].world_position.x, logs[i].world_position.y);

		constexpr float gap = 2.0f;
		constexpr float size = 4.0f;

		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pos.x - gap, pos.y - gap), ImVec2(pos.x - gap - size, pos.y - gap - size), color, 1.0f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pos.x - gap, pos.y + gap), ImVec2(pos.x - gap - size, pos.y + gap + size), color, 1.0f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pos.x + gap, pos.y - gap), ImVec2(pos.x + gap + size, pos.y - gap - size), color, 1.0f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pos.x + gap, pos.y + gap), ImVec2(pos.x + gap + size, pos.y + gap + size), color, 1.0f);
	}
}


void c_hitmarker::UpdateDamage()
{
	MDamageMutex.lock();

	for (int i = 0; i < MDamageMarker.size(); i++)
	{
		MDamageMarker[i].is_world_screen = D::WorldToScreen(Vector_t(MDamageMarker[i].position.x,MDamageMarker[i].position.y), &MDamageMarker[i].world_position);
	}

	MDamageMutex.unlock();
}

void c_hitmarker::AddDamage(std::string damage, Vector_t position)
{
	MDamageMutex.lock();
	MDamageMarker.push_back(CDamageMarker(damage, position, I::GlobalVars->flCurTime + 1.f));
	MDamageMutex.unlock();
}

void c_hitmarker::RenderDamage()
{
	if (!SDK::Alive)
		return;

	if (!I::GlobalVars)
		return;

	std::lock_guard<std::mutex> lock(MDamageMutex); 

	float currentTime = I::GlobalVars->flCurTime; 

	for (int i = 0; i < MDamageMarker.size(); ++i)
	{
		if (!MDamageMarker[i].is_world_screen)
			continue;

		
		float elapsedTime = currentTime - MDamageMarker[i].time;
		float fadeDuration = 1.0f; 

		if (elapsedTime >= fadeDuration)
		{
			MDamageMarker.erase(MDamageMarker.begin() + i);
			--i;
			continue;
		}

		float alpha = std::fmax(0.0f, 1.0f - (elapsedTime / fadeDuration));

		ImU32 color = ImColor(255, 255, 255, static_cast<int>(alpha * 255)); 

		ImGui::GetBackgroundDrawList()->AddText(
		ImVec2(MDamageMarker[i].world_position.x + 50, MDamageMarker[i].world_position.y),
		color,
		MDamageMarker[i].damage.c_str());
	}
}

void c_hitmarker::HitMarker(Vector_t EyePos,Vector_t Pos ,int damage)
{
   g_hitmarker->AddDamage(std::to_string(damage), Pos);
   g_hitmarker->add(Pos, EyePos);
}
