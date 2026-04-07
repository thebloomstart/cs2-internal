#include "sdk/datatypes/color.h"
#include "World.h"
#include "core/variables.h"
#include "sdk\interfaces\iengineclient.h"
#include "sdk\interfaces\igameresourceservice.h"
#include "sdk\interfaces\cgameentitysystem.h"
#include "sdk/EntityList/EntityList.h"
#include "utilities/draw.h"

void CL_World::skybox()
{
	static const auto update_sky_box = reinterpret_cast<void(__fastcall*)(void*)>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 57 48 83 EC 30 48 8B F9 E8 ?? ?? ?? ?? 48 8B 47")));

    bool DisableFogRender = C_GET(unsigned int, Vars.bRemovalsNames) & FOG;

    Color_t skybox_clr = C_GET(Color_t, Vars.colSky); // g_cfg->world.m_sky;

	static bool prev_disable_fog_rendering = !DisableFogRender;

	static Color_t prev_skybox_clr(-1, -1, -1);

	if (prev_skybox_clr == skybox_clr && prev_disable_fog_rendering == DisableFogRender)
		return;

	prev_disable_fog_rendering = DisableFogRender;

	prev_skybox_clr = skybox_clr;

	const auto& sky = g_EntityList->get("C_EnvSky");
	for (CEntityInstance* entity : sky)
	{
		C_EnvSky* env_sky = reinterpret_cast<C_EnvSky*>(entity);

		skybox(env_sky);

		update_sky_box(env_sky);
	}
}

void CL_World::skybox(C_EnvSky* env_sky)
{
	bool DisableFogRender = C_GET(unsigned int, Vars.bRemovalsNames) & FOG;

	Color_t skybox_clr = C_GET(Color_t, Vars.colSky); // g_cfg->world.m_sky;

	ByteColor color = (skybox_clr * 255).ToByte();

	env_sky->m_vTintColor() = color;

	env_sky->m_flBrightnessScale() = DisableFogRender ? 0.f : 1.f;
}


void CL_World::lighting(C_SceneLightObject* light_object)
{
	Color_t lighintg_color = C_GET(Color_t, Vars.colLightning); /*g_cfg->world.m_lighting;*/

	light_object->r = lighintg_color.r / 255.0f * C_GET(float, Vars.flLightingIntensity);
	light_object->g = lighintg_color.g / 255.0f * C_GET(float, Vars.flLightingIntensity);
	light_object->b = lighintg_color.b / 255.0f * C_GET(float, Vars.flLightingIntensity);
}



void CL_World::exposure(C_CSPlayerPawn* pawn)
{
	if (!I::Engine->IsInGame())
		return;

    static const auto update_exposure = reinterpret_cast<void(__fastcall*)(CPlayer_CameraServices*, int)>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 E8 ?? ?? ?? ?? 84 C0 0F 84")));

	static float prev_exposure = -1;

    float exposure = static_cast<float>(C_GET(float, Vars.flExposure) /*g_cfg->world.m_exposure*/);

	if (prev_exposure == exposure)
		return;

	if (!pawn || !pawn->GetCameraServices())
		return;

	CPlayer_CameraServices* camera_service = pawn->GetCameraServices();
	if (!camera_service)
		return;

    C_PostProcessingVolume* post_processing_volume = reinterpret_cast<C_PostProcessingVolume*>(I::GameResourceService->pGameEntitySystem->Get(camera_service->m_PostProcessingVolumes().GetEntryIndex()));
	if (!post_processing_volume)
		return;

	prev_exposure = exposure;

	this->exposure(post_processing_volume);
	update_exposure(camera_service, 0);
}

void CL_World::exposure(C_PostProcessingVolume* post_processing)
{
	float exposure = C_GET(float, Vars.flExposure) * 0.01f;

	if (!SDK::Alive)
		return;

	if (!post_processing)
		return;
	try {
		post_processing->ExposureControl() = true;

		post_processing->FadeSpeedDown() = post_processing->FadeSpeedUp() = 0;

		post_processing->MinExposure() = post_processing->MaxExposure() = exposure;
	}
	catch (...)
	{
		L_PRINT(LOG_INFO) << "ExposureControl";
	}
}

void CL_World::DrawScopeOverlay()
{
	if (!I::Engine->IsInGame())
		return;

	if (!SDK::Alive)
		return;

	C_CSPlayerPawn* local_player = SDK::LocalPawn;

	if (!local_player)
		return;

	float crosshairAnimationValue = D::Animate("crosshair", "", local_player->IsScoped(), 1.0f, 10.0f, DYNAMIC);

	if (!local_player->IsScoped())
		return;

	ImDrawList* draw = ImGui::GetBackgroundDrawList();
	ImVec2 screen = ImGui::GetIO().DisplaySize;
	ImVec2 center = ImVec2(screen.x * 0.5f, screen.y * 0.5f);

	Color_t color = Color_t(0, 0, 0);
	draw->AddLine(ImVec2(0, screen.y / 2), ImVec2(screen.x, screen.y / 2), color.GetU32());
	draw->AddLine(ImVec2(screen.x / 2, 0), ImVec2(screen.x / 2, screen.y), color.GetU32());	
	//ImU32 color_transparent = IM_COL32(0, 0, 0, 0);

	//int gap = C_GET(int,Vars.iCrossGap);
	//int thickness = C_GET(int, Vars.iCrossthickness);
	//int line_length = C_GET(int, Vars.iCrosslength);

	//int animated_line_length = static_cast<int>(crosshairAnimationValue * line_length);

	//// Top line
	//draw->AddRectFilledMultiColor(
	//	ImVec2(center.x - thickness * 0.5f, center.y - gap * 0.5f - animated_line_length),
	//	ImVec2(center.x + thickness * 0.5f, center.y - gap * 0.5f),
	//	color_transparent, color_transparent, color.GetU32(), color.GetU32()
	//);

	//// Bottom line
	//draw->AddRectFilledMultiColor(
	//	ImVec2(center.x - thickness * 0.5f, center.y + gap * 0.5f),
	//	ImVec2(center.x + thickness * 0.5f, center.y + gap * 0.5f + animated_line_length),
	//	color.GetU32(), color.GetU32(), color_transparent, color_transparent
	//);

	//// Left line
	//draw->AddRectFilledMultiColor(
	//	ImVec2(center.x - gap * 0.5f - animated_line_length, center.y - thickness * 0.5f),
	//	ImVec2(center.x - gap * 0.5f, center.y + thickness * 0.5f),
	//	color_transparent, color.GetU32(), color.GetU32(), color_transparent
	//);

	//// Right line
	//draw->AddRectFilledMultiColor(
	//	ImVec2(center.x + gap * 0.5f, center.y - thickness * 0.5f),
	//	ImVec2(center.x + gap * 0.5f + animated_line_length, center.y + thickness * 0.5f),
	//	color.GetU32(), color_transparent, color_transparent, color.GetU32()
	//);
}
