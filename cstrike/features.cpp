#include "features.h"

// used: draw callbacks
#include "utilities/draw.h"
// used: notify
#include "utilities/notify.h"

// used: cheat variables
#include "core/variables.h"
// used: menu
#include "core/menu.h"

// used: features callbacks
#include "features/visuals.h"
#include "features/misc.h"
#include "features/legitbot.h"
#include "features/legitbot/aim.h"

// used: interfaces
#include "core/interfaces.h"
#include "sdk/interfaces/iengineclient.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/datatypes/usercmd.h"
#include "sdk/entity.h"
#include "features/visuals/overlay.h"
#include "World.h"
#include "HitMarker.h"
#include "BulletTracer.h"
#include "features/misc/movement.h"
#include "GrenadeVisuals.h"

bool F::Setup()
{
	if (!VISUALS::Setup())
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to setup visuals");
		return false;
	}

	return true;
}

void F::Destroy()
{
	VISUALS::OnDestroy();
}

void F::OnPresent()
{
	if (!D::bInitialized)
		return;

	D::NewFrame();
	{
		// render watermark
		//MENU::RenderWatermark();
		// @note: here you can draw your stuff
		MENU::menu();
		MENU::Hotkeys();
		MENU::RenderWatermark();
		F::LEGITBOT::AIM::draw();
		//MENU::BombTimer();
		try
		{
			g_Esp->handle_players();

		}
		catch (...)
		{
			L_PRINT(LOG_ERROR) << "RENDER ERROR";
		}
		g_hitmarker->update_world_screen();
		g_hitmarker->run();
		g_world->DrawScopeOverlay();
		g_world->exposure(SDK::LocalPawn);
		g_world->skybox();
		//GrenadeVisuals::present();
		F::MISC::MOVEMENT::draw_autopeek();
		g_BulletTrace->render();
		NOTIFY::Render();
	}
	D::Render();
}

void F::OnCreateMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController)
{
	C_CSPlayerPawn* pLocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalController->GetPawnHandle());
	if (pLocalPawn == nullptr)
		return;
	F::MISC::OnMove(SDK::Cmd, SDK::Cmd->csgoUserCmd.pBaseCmd, SDK::LocalController, pLocalPawn);
	F::LEGITBOT::OnMove(pCmd, pBaseCmd, pLocalController, pLocalPawn);
}

bool F::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	return VISUALS::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}
