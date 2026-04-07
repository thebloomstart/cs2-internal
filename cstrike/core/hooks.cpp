#include "hooks.h"

// used: variables
#include "variables.h"

// used: game's sdk
#include "../sdk/interfaces/iswapchaindx11.h"
#include "../sdk/interfaces/iviewrender.h"
#include "../sdk/interfaces/cgameentitysystem.h"
#include "../sdk/interfaces/ccsgoinput.h"
#include "../sdk/interfaces/iinputsystem.h"
#include "../sdk/interfaces/iengineclient.h"
#include "../sdk/interfaces/inetworkclientservice.h"
#include "../sdk/interfaces/iglobalvars.h"
#include "../sdk/interfaces/imaterialsystem.h"
#include "../sdk/interfaces/ipvs.h"

// used: viewsetup
#include "../sdk/datatypes/viewsetup.h"

// used: entity
#include "../sdk/entity.h"

// used: get virtual function, find pattern, ...
#include "../utilities/memory.h"
// used: inputsystem
#include "../utilities/inputsystem.h"
// used: draw
#include "../utilities/draw.h"

// used: features callbacks
#include "../features.h"
// used: CRC rebuild
#include "../features/CRC.h"

// used: game's interfaces
#include "interfaces.h"
#include "sdk.h"

// used: menu
#include "menu.h"
#include "../InvecntoryChanger.h"
#include "../AntiAim.h"
#include "../sdk\interfaces\cgametracemanager.h"
#include "../sdk/EntityList/EntityList.h"
#include "../World.h"
#include "../features/misc/movement.h"
#include "../features/visuals/overlay.h"
#include "../sdk\EntityList\Events.h"
#include "../AimBot.h"
#include "../MovementRecorder.h"
#include "../Spectator.h"
#include <DirectXMath.h>
#include "../BackTrack.h"
#include "../BulletTracer.h"
#include "../Lagcomp.h"
#include "../Utils.h"
#include "../pred.h"
#include "../GrenadeVisuals.h"
#include "../lagcompensation.h"

bool H::Setup()
{
	if (MH_Initialize() != MH_OK)
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to initialize minhook");

		return false;
	}
	L_PRINT(LOG_INFO) << CS_XOR("minhook initialization completed");

	if (!hkPresent.Create(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::PRESENT), reinterpret_cast<void*>(&Present)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"Present\" hook has been created");

	if (!hkResizeBuffers.Create(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::RESIZEBUFFERS), reinterpret_cast<void*>(&ResizeBuffers)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"ResizeBuffers\" hook has been created");

	// creat swap chain hook
	IDXGIDevice* pDXGIDevice = NULL;
	I::Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));

	IDXGIAdapter* pDXGIAdapter = NULL;
	pDXGIDevice->GetAdapter(&pDXGIAdapter);

	IDXGIFactory* pIDXGIFactory = NULL;
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));

	if (!hkCreateSwapChain.Create(MEM::GetVFunc(pIDXGIFactory, VTABLE::DXGI::CREATESWAPCHAIN), reinterpret_cast<void*>(&CreateSwapChain)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"CreateSwapChain\" hook has been created");

	pDXGIDevice->Release();
	pDXGIDevice = nullptr;
	pDXGIAdapter->Release();
	pDXGIAdapter = nullptr;
	pIDXGIFactory->Release();
	pIDXGIFactory = nullptr;

	// @ida: class CViewRender->OnRenderStart call GetMatricesForView
	if (!hkGetMatrixForView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 81 EC ? ? ? ? 49 8B C1")), reinterpret_cast<void*>(&GetMatrixForView)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"GetMatrixForView\" hook has been created");

	if (!hkCreateMoveUserCmd.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 4C 89 40 ? 48 89 48 ? 55 53 57")), reinterpret_cast<void*>(&CreateMoveUserCmd)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"CreateMove2\" hook has been created");

	if (!hkMouseInputEnabled.Create(MEM::GetVFunc(I::Input, VTABLE::CLIENT::MOUSEINPUTENABLED), reinterpret_cast<void*>(&MouseInputEnabled)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"MouseInputEnabled\" hook has been created");

	if (!hkFrameStageNotify.Create(MEM::GetVFunc(I::Client, VTABLE::CLIENT::FRAMESTAGENOTIFY), reinterpret_cast<void*>(&FrameStageNotify)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"FrameStageNotify\" hook has been created");

	// @ida: ClientModeShared -> #STR: "mapname", "transition", "game_newmap"
	if (!hkLevelInit.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 56 48 83 EC ? 48 8B 0D ? ? ? ? 48 8B F2")), reinterpret_cast<void*>(&LevelInit)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"LevelInit\" hook has been created");

	// @ida: ClientModeShared -> #STR: "map_shutdown"
	if (!hkLevelShutdown.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 4C 8B 01 41 FF 50 ? 48 83 C4")), reinterpret_cast<void*>(&LevelShutdown)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"LevelShutdown\" hook has been created");

	// @note: seems to do nothing for now...
	// @ida: ClientModeCSNormal->OverrideView idx 15
	//v21 = flSomeWidthSize * 0.5;
	//v22 = *flSomeHeightSize * 0.5;
	//*(float*)(pSetup + 0x49C) = v21; // m_OrthoRight
	//*(float*)(pSetup + 0x494) = -v21; // m_OrthoLeft
	//*(float*)(pSetup + 0x498) = -v22; // m_OrthoTop
	//*(float*)(pSetup + 0x4A0) = v22; // m_OrthoBottom

	if (!hkOverrideView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8")), reinterpret_cast<void*>(&OverrideView)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"OverrideView\" hook has been created");

	if (!hkDrawObject.Create(MEM::FindPattern(SCENESYSTEM_DLL, CS_XOR("48 83 EC 48 48 8B 84 24 ? ? ? ? 48 8D 0D ? ? ? ?")), reinterpret_cast<void*>(&DrawObject)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"DrawObject\" hook has been created");

	if (!hkIsRelativeMouseMode.Create(MEM::GetVFunc(I::InputSystem, VTABLE::INPUTSYSTEM::ISRELATIVEMOUSEMODE), reinterpret_cast<void*>(&IsRelativeMouseMode)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"IsRelativeMouseMode\" hook has been created");

	if (!hkSetModel.Create(MEM::FindPattern(CLIENT_DLL, ("40 53 48 83 EC 20 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24")), reinterpret_cast<void*>(&SetModel)))
		return false;
	L_PRINT(LOG_INFO) << ("\"hkSetModel Loaded\" hook has been created");

	if (!hkAddEntity.Create(MEM::GetVFunc(I::GameResourceService->pGameEntitySystem, 15), reinterpret_cast<void*>(&AddEntity)))
		return false;
	L_PRINT(LOG_INFO) << ("\"hkAddEntity\" hook has been created");

	if (!hkRemoveEntity.Create(MEM::GetVFunc(I::GameResourceService->pGameEntitySystem, 16), reinterpret_cast<void*>(&RemoveEntity)))
		return false;
	L_PRINT(LOG_INFO) << ("\"hkRemoveEntity\" hook has been created");

	if (!hkDrawLightScene.Create(MEM::FindPattern(SCENESYSTEM_DLL, ("48 89 54 24 ? 53 41 56 41 57")), reinterpret_cast<void*>(&DrawLightScene)))
		return false;
	L_PRINT(LOG_INFO) << ("\"DrawLightScene Loaded\" hook has been created");

	if (!hkUpdatePostProccesing.Create(MEM::FindPattern(CLIENT_DLL, ("48 89 5C 24 08 57 48 83 EC 60 80")), reinterpret_cast<float*>(&UpdatePostProccesing)))
		return false;
	L_PRINT(LOG_INFO) << ("\"UpdatePostProccesing Loaded\" hook has been created");

	if (!hkUpdateSkyBox.Create(MEM::FindPattern(CLIENT_DLL, ("48 8B C4 48 89 58 18 48 89 70 20 55 57 41 54 41 55")), reinterpret_cast<void*>(&UpdateSkyBox)))
		return false;
	L_PRINT(LOG_INFO) << ("\"hkUpdateSkyBox Loaded\" hook has been created");

	if (!hkVerify.Create(MEM::FindPattern(CLIENT_DLL, ("40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? C6 83 ? ? ? ? ? 33 C0")), reinterpret_cast<void*>(&ValidQAngel)))
		return false;
	L_PRINT(LOG_INFO) << ("\"hkVerify Loaded\" hook has been created");

	//if (!hkGameEvents.Create(MEM::FindPattern(CLIENT_DLL, ("40 55 53 56 57 41 55 41 57 48 8D 6C 24 ??")), reinterpret_cast<void*>(&HandleGameEvents)))
	//	return false;
	//L_PRINT(LOG_INFO) << ("\"GameEvents\" hook has been created");

	if (!hkDrawScope.Create(MEM::FindPattern(CLIENT_DLL, ("4C 8B DC 53 56 57 48 83 EC")), reinterpret_cast<void*>(&DrawScope)))
		return false;
	L_PRINT(LOG_INFO) << ("\"hkDrawScope Loaded\" hook has been created");

	if (!hkFov.Create(MEM::FindPattern(CLIENT_DLL, ("40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4")), reinterpret_cast<float*>(&GetRenderFov)))
		return false;
	L_PRINT(LOG_INFO) << ("\"Fov Loaded\" hook has been created");

	//if (!hkViewModelFov.Create(MEM::FindPattern(CLIENT_DLL, ("48 89 5C 24 ? 55 56 57 41 56 41 57 48 83 EC ? 49 8B E8")), reinterpret_cast<float*>(&ViewModelFov)))
	//	return false;
	//L_PRINT(LOG_INFO) << ("\"ViewModelFov Loaded\" hook has been created");

	//if (!hkAutoAccept.Create(MEM::FindPattern(CLIENT_DLL, ("E8 ? ? ? ? E8 ? ? ? ? 33 D2 48 8B C8 E8 ? ? ? ? 41 B8")), reinterpret_cast<void*>(&AutoAccept)))
	//	return false;
	//L_PRINT(LOG_INFO) << ("\"AutoAccept Loaded\" hook has been created");

	//if (!hkDrawSceneObject.Create(MEM::FindPattern(SCENESYSTEM_DLL, ("48 89 54 24 ? 55 57 41 55 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 49 63 F9")), reinterpret_cast<void*>(&DrawSceneObject)))
	//	return false;
	//L_PRINT(LOG_INFO) << ("\"DrawSceneObject Loaded\" hook has been created");

	if (!hkUpdateSceneObject.Create(MEM::FindPattern(SCENESYSTEM_DLL, ("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 4C 8B F9")), reinterpret_cast<void*>(&UpdateSceneObject)))
		return false;
	L_PRINT(LOG_INFO) << ("\"UpdateSceneObject Loaded\" hook has been created");

	if (!hkHookInput.Create(MEM::FindPattern(CLIENT_DLL, ("4c 89 4c 24 ?? 55 53 57 41 56 48 8d 6c 24")), reinterpret_cast<void*>(&HookInput)))
		return false;
	L_PRINT(LOG_INFO) << ("\"HookInput Loaded\" hook has been created");

	if (!hkViewModelChanger.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24? 48 89 74 24? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC ? 4D 8B E0")), reinterpret_cast<void*>(&ViewModelChanger)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"ViewModelChanger\" hook has been created");

	if (!hkDrawCrosshair.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 57 48 83 EC 20 48 8B D9 E8 ?? ?? ?? ?? 48 85")), reinterpret_cast<void*>(&DrawCrosshair)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"DrawCrosshair\" hook has been created");

	if (!hkUnlockInv.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 74 24 18 57 48 83 EC 20 48 8B 0D")), reinterpret_cast<void*>(&UnlockInventory)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"UnlockInventory Loaded\" hook has been created");

	if (!hkNoSmoke.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 40 48 8B 9C 24 88 00 00 00 4D")), reinterpret_cast<void*>(&Smoke)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"Smoke\" hook has been created");

	if (!hkInterPlayer.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 0F 84 ? ? ? ? 55 57 48 83 EC")), reinterpret_cast<void*>(&InterPlayer)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"InterPlayer\" hook has been created");

	if (!hkDrawLegs.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 85 C0 0F 85")), reinterpret_cast<void*>(&DrawLegs)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"DrawLegs\" hook has been created");

	if (!hkDrawTeamIntro.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 83 EC ? 45 0F B6 08")), reinterpret_cast<void*>(&DrawTeamIntro)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"DrawTeamIntro\" hook has been created");

	//if (!hkSky3DParams.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 70 8B 05 AB D8 08 01")), reinterpret_cast<void*>(&Sky3DParams)))
	//	return false;
	//L_PRINT(LOG_INFO) << CS_XOR("\"Sky3DParams\" hook has been created");

	if (!hkApplyViewPunch.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 55 56 41 56 48 81 EC ? ? ? ? 48 8B D9")), reinterpret_cast<void*>(&ApplyViewPunch)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"ApplyViewPunch\" hook has been created");

	if (!hkNoFlash.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 0F 88 ? ? ? ? 55 56 41 55 ")), reinterpret_cast<void*>(&NoFlash)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"hkNoFlash\" hook has been created");

	I::GameTraceManager->TraceHook();
	g_RageBot->Hook();
	fuck_this_shit::find_gethitboxset_pttr();
	fuck_this_shit::find_hitboxtoworldtransforms_pptr();
	ragebot::Hook();
	//if (!hkDrawRadarEntities.Create(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 49 8D 4E D8 E8 ? ? ? ? 49 8B 9E")), 0x1), reinterpret_cast<void*>(&DrawRadarEntities)));
	//	return false;
	//L_PRINT(LOG_INFO) << CS_XOR("\"hkDrawRadarEntities\" hook has been created");

	SDK::fnConColorMsg(Color_t(0, 0, 0), "UNDERAGER Loaded");
	return true;
}

void H::Destroy()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	MH_Uninitialize();
}

float H::ViewModelFov()
{
	return static_cast<float>(C_GET(float, Vars.flViewModelFov));
	//return static_cast<float>(150.f/*g_menu->viewmodel_fov*/);
}

HRESULT __stdcall H::Present(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags)
{
	const auto oPresent = hkPresent.GetOriginal();

	// recreate it if it's not valid
	if (I::RenderTargetView == nullptr)
		I::CreateRenderTarget();

	// set our render target
	if (I::RenderTargetView != nullptr)
		I::DeviceContext->OMSetRenderTargets(1, &I::RenderTargetView, nullptr);

	g_Esp->store_players();
	F::OnPresent();
	return oPresent(I::SwapChain->pDXGISwapChain, uSyncInterval, uFlags);
}

void* H::UpdateSkyBox(C_EnvSky* sky)
{
	static auto original = hkUpdateSkyBox.GetOriginal();

	g_world->skybox(sky);

	return original(sky);
}

void* H::DrawLightScene(void* a1, C_SceneLightObject* a2, __int64 a3)
{
	static auto original = hkDrawLightScene.GetOriginal();

	g_world->lighting(a2);

	return original(a1, a2, a3);
}

void* H::UpdatePostProccesing(C_PostProcessingVolume* a1, int a2)
{
	static auto original = hkUpdatePostProccesing.GetOriginal();

	original(a1, a2);

	g_world->exposure(a1);

	return original(a1, a2);
}

HRESULT CS_FASTCALL H::ResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags)
{
	const auto oResizeBuffer = hkResizeBuffers.GetOriginal();

	auto hResult = oResizeBuffer(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);
	if (SUCCEEDED(hResult))
		I::CreateRenderTarget();

	return hResult;
}

HRESULT __stdcall H::CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	const auto oCreateSwapChain = hkCreateSwapChain.GetOriginal();

	I::DestroyRenderTarget();
	L_PRINT(LOG_INFO) << CS_XOR("render target view has been destroyed");

	return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
}

long H::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (D::OnWndProc(hWnd, uMsg, wParam, lParam))
		return 1L;

	return ::CallWindowProcW(IPT::pOldWndProc, hWnd, uMsg, wParam, lParam);
}

ViewMatrix_t* CS_FASTCALL H::GetMatrixForView(CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels)
{
	const auto oGetMatrixForView = hkGetMatrixForView.GetOriginal();
	ViewMatrix_t* matResult = oGetMatrixForView(pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels);

	// get view matrix
	SDK::ViewMatrix = *pOutWorldToProjection;
	// get camera position
	// @note: ida @GetMatrixForView(global_pointer, pRenderGameSystem + 16, ...)
	SDK::CameraPosition = pViewRender->vecOrigin;

	return matResult;
}
__int64  CS_FASTCALL H::AutoAccept(void* unk, const char* event_name, void* unk1, float unk2)
{
	const auto original = hkAutoAccept.GetOriginal();

	if (FNV1A::Hash(event_name) == FNV1A::Hash("popup_accept_match_found"))
	{
		L_PRINT(LOG_INFO) << event_name;
		using SetPlayerReady = bool(__fastcall*)(void*, const char*);
		// string ref: "deferred"
		static const SetPlayerReady fn = reinterpret_cast<SetPlayerReady>(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 83 EC ? 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 ? 8D 50 ? 33 C9")));
				fn(nullptr, "");
		L_PRINT(LOG_INFO) << "FOUND MATCH";
	}
	return original(unk, event_name, unk1, unk2);
}

// xref: "winpanel-basic-round-result-visible"
void CS_FASTCALL H::HandleGameEvents(void* rcx, CGameEvent* const ev)
{
	const auto original = hkGameEvents.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame() || !SDK::LocalPawn)
		return original(rcx, ev);

	if (FNV1A::Hash(ev->GetName()) == FNV1A::HashConst("team_intro_start"))
		return;

	original(rcx, ev);
}

void* CS_FASTCALL H::ApplyViewPunch(CPlayer_CameraServices* pCameraServices, float* a2, float* a3, float* a4) {
	if (C_GET(unsigned int, Vars.bRemovalsNames) & RECOIL)
		pCameraServices->ViewPunchAngle() = Vector_t(0,0,0);

	return hkApplyViewPunch.GetOriginal()(pCameraServices, a2, a3, a4);
}

void* H::AddEntity(void* rcx, C_BaseEntity* inst, CBaseHandle handle)
{
	static const auto og = hkAddEntity.GetOriginal();
	if (!inst || !handle.IsValid())
		return hkAddEntity.GetOriginal()(rcx, inst, handle);
	g_EntityList->add(inst, handle);

	return og(rcx, inst, handle);
}

void* H::RemoveEntity(void* rcx, C_BaseEntity* inst, CBaseHandle handle)
{
	static const auto og = hkRemoveEntity.GetOriginal();
	if (!inst || !handle.IsValid())
		return hkRemoveEntity.GetOriginal()(rcx, inst, handle);
	g_EntityList->remove(inst, handle);
	return og(rcx, inst, handle);
}

void CS_FASTCALL H::SetModel(void* rcx, const char* model)
{
	const auto oSetModel = hkSetModel.GetOriginal();
	skin_changer::OnSetModel((C_BaseModelEntity*)rcx, model);
	return oSetModel(rcx, model);
}

bool CS_FASTCALL H::DrawLegs(C_CSPlayerPawn* Entity) {
	const auto oDrawLegs = hkDrawLegs.GetOriginal();

	if (!I::Engine->IsInGame() || !I::Engine->IsConnected())
		return oDrawLegs(Entity);

	if ((C_GET(unsigned int, Vars.bRemovalsNames) & LEGS) && (Entity == SDK::LocalPawn) && !IPT::IsPressed(C_GET(int, Vars.iThirdKey), C_GET(int, Vars.iThirdKeyBind)))
		return false;

	return oDrawLegs(Entity);
}

// XREF: FlashbangOverlay & XREF: CsgoForward & XREF: cs_flash_frame_render_target_split_%d
void CS_FASTCALL H::NoFlash(__int64 a1, int a2, __int64* a3, __int64 a4, __m128* a5)
{
	const auto orig = hkNoFlash.GetOriginal();

	if (C_GET(unsigned int, Vars.bRemovalsNames) & FLASH)
		return;

	orig(a1, a2, a3, a4, a5);
}

void CS_FASTCALL H::Smoke(__int64 a1, __int64* a2, int a3, int a4, __int64 a5, __int64 a6)
{
	const auto orig = hkNoSmoke.GetOriginal();

	if (C_GET(unsigned int, Vars.bRemovalsNames) & SMOKE)
		return;

	orig(a1, a2, a3, a4, a5, a6);
}

void CS_FASTCALL H::DrawTeamIntro(std::uintptr_t a1, std::uintptr_t a2, char* a3) {
	const auto orig = hkDrawTeamIntro.GetOriginal();
	if (C_GET(unsigned int, Vars.bRemovalsNames) & TEAM_INTRO)
		return;
	orig(a1, a2, a3);
}

bool CS_FASTCALL H::InterPlayer(C_BaseEntity* ent, int some_int)
{
	const auto orig = hkInterPlayer.GetOriginal();

	if (!SDK::LocalPawn || !ent)
		return orig(ent, some_int);

	if (ent && static_cast<C_CSPlayerPawn*>(ent)->IsOtherEnemy(SDK::LocalPawn) && some_int == 0)
		return true; // do not perform interpolation on non-teammates (enemies), this also skips localplayer

	return orig(ent, some_int);
}

//void* CS_FASTCALL H::Sky3DParams(void* a1) {
//	const auto orig = hkSky3DParams.GetOriginal();
//
//	if (C_GET(unsigned int, Vars.bRemovalsNames) & SKY)
//		return nullptr;
//
//	return orig;
//}

void CS_FASTCALL H::ViewModelChanger(float* a1, Vector_t a2, float* R)
{
	const auto orig = hkViewModelChanger.GetOriginal();
	if (!SDK::Alive)
		return orig(a1, a2, R);
	//if (C_GET(bool, Vars.bViewMode))
	{
		a2.x += C_GET(float, Vars.flViX); // x
		a2.y += C_GET(float, Vars.flViY); // y
		a2.z += C_GET(float, Vars.flViZ); // z
		*R += C_GET(float, Vars.flViewModelFov);
	}
	return orig(a1, a2, R);
}

void __fastcall H::ValidQAngel(CCSGOInput* input, int a2)
{
	const auto oVerify = hkVerify.GetOriginal();
	if (C_GET(bool, Vars.bLegitbot))
		oVerify(input, a2);
	else
	{
		auto angle = input->GetViewAngles();
		input->SetViewAngle(g_AntiAim->StoreAngels);
		oVerify(input, a2);
		input->SetViewAngle(angle);
	}
}

bool CS_FASTCALL H::UnlockInventory(void* a1)
{
	const auto Unlock = hkUnlockInv.GetOriginal();

	if (C_GET(bool, Vars.bUnlockINVENTORY))
		return true;

	Unlock(a1);
}

int get_player_tick(std::uintptr_t* container)
{
	using function_t = int* (__fastcall*)(std::uintptr_t*, std::uintptr_t*, char);
	static const function_t fn = reinterpret_cast<function_t>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 57 48 83 EC 20 33 C9 41")));
	CS_ASSERT(fn != nullptr);
	return *fn(nullptr, container, 0);
}

bool CS_FASTCALL H::CreateMoveUserCmd(CCSGOInput* pInput, int nSlot, CUserCmd* pCmd)
{
	const auto oCreateMove = hkCreateMoveUserCmd.GetOriginal();
	const bool bResult = oCreateMove(pInput, nSlot, pCmd);

	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return oCreateMove;

	SDK::Cmd = pCmd;
	SDK::ScreenSize = ImGui::GetIO().DisplaySize;

	CBaseUserCmdPB* pBaseCmd = SDK::Cmd->csgoUserCmd.pBaseCmd;
	if (pBaseCmd == nullptr)
		return bResult;

	auto Ent = g_EntityList->get("CCSPlayerController");
	if (I::Engine->IsInGame() && Ent.empty())
		g_EntityList->ForceUpdateEntityList();

	SDK::LocalController = CCSPlayerController::GetLocalPlayerController();
	if (SDK::LocalController == nullptr)
		return bResult;

	SDK::LocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(SDK::LocalController->GetPawnHandle());
	if (SDK::LocalPawn == nullptr)
		return bResult;

	SDK::Alive = SDK::LocalPawn->GetHealth() > 0 && SDK::LocalPawn->GetLifeState() != ELifeState::LIFE_DEAD;
	SDK::WeaponServices = SDK::LocalPawn->GetWeaponServices();

	if (!SDK::WeaponServices)
		return bResult;

	SDK::ActiveWeapon = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBaseGun>(SDK::WeaponServices->GetActiveWeapon());
	if (!SDK::ActiveWeapon)
		return bResult;

	if (SDK::Alive)
		if (pBaseCmd->pViewAngles != NULL)
			g_AntiAim->StoreAngels = g_Utils->normalize(pBaseCmd->pViewAngles->angValue);

	//for (int i = 0; i < pCmd->csgoUserCmd.inputHistoryField.pRep->nAllocatedSize; i++)
	//{
	//	CCSGOInputHistoryEntryPB* pImputEntry = pCmd->GetInputHistoryEntry(i);
	//	if (!pImputEntry)
	//		continue;
	//	if (pImputEntry)
	//	{
	//	//	pCmd->nButtons.nValueScroll |= IN_ATTACK;
	//		//if (pCmd->nButtons.nValue & IN_ATTACK)
	//		//{
	//		//	pCmd->AddSubtick(true, IN_ATTACK);
	//		//	pCmd->AddSubtick(true, IN_ATTACK);
	//		//	pCmd->AddSubtick(false, IN_ATTACK);
	//		//}
	//		pImputEntry->setPlayerTickCount(SDK::prediction_tick/*pCmd->csgoUserCmd.pBaseCmd->nClientTick*/ - 25);
	//		pImputEntry->setRenderTickCount(SDK::prediction_tick/*pCmd->csgoUserCmd.pBaseCmd->nClientTick*/ - 25);
	//		pCmd->csgoUserCmd.nAttack1StartHistoryIndex = -1;
	//		//if (pCmd->nButtons.nValue & IN_ATTACK)
	//		//{
	//		//	pCmd->AddSubtick(true, IN_ATTACK);
	//		//	pCmd->AddSubtick(true, IN_ATTACK);
	//		//	pCmd->AddSubtick(false, IN_ATTACK);
	//		//}



	//	//	pCmd->nButtons.nValueScroll |= IN_ATTACK;

	//	}û
	//}	
	F::OnCreateMove(SDK::Cmd, SDK::Cmd->csgoUserCmd.pBaseCmd, SDK::LocalController);
	//GrenadeVisuals::_createmove(pInput);
	g_AntiAim->run(SDK::Cmd);
	if (I::Engine->IsConnected() || I::Engine->IsInGame() && SDK::LocalPawn && SDK::LocalPawn->GetHealth() > 0)
	{
		//g_PredictionSystem->Begin(pInput, SDK::Cmd);
		{
			SDK::ActiveWeaponVData = SDK::ActiveWeapon->GetWeaponVData();
			SDK::Latency = I::Engine->GetNetChannelInfo()->get_network_latency();
			SDK::EngLatency = I::Engine->GetNetChannelInfo()->get_engine_latency();
			SDK::ClientLerp = I::NetworkClientService->GetNetworkGameClient()->maybe();
			if (SDK::ActiveWeaponVData) {
				SDK::WepRange = SDK::ActiveWeaponVData->m_flRange();
				SDK::WepRangeMod = SDK::ActiveWeaponVData->m_flRangeModifier();
				SDK::WepDamage = SDK::ActiveWeaponVData->m_nDamage();
				SDK::WepPen = SDK::ActiveWeaponVData->m_flPenetration();
				SDK::WepArmorRatio = SDK::ActiveWeaponVData->m_flArmorRatio();
				SDK::WepHeadShotMulti = SDK::ActiveWeaponVData->m_flHeadshotMultiplier();

				C_AttributeContainer* pAttributeContainer = SDK::ActiveWeapon->GetAttributeManager();
				C_EconItemView* pWeaponItemView = &pAttributeContainer->m_Item();
				SDK::ItemDefIdx = pWeaponItemView->GetItemDefinitionIndex();

			}
			else
				return bResult;
			SDK::EyePos = SDK::LocalPawn->GetEyePosition();
			std::uintptr_t v200;
			auto player_tick = get_player_tick(&v200) - 1;
			SDK::prediction_tick = player_tick;

			try
			{
				SDK::WepAccuracy = SDK::ActiveWeapon->get_inaccuracyHitChance();
				ragebot::run();
				//g_RageBot->RageBotAim(SDK::Cmd,pInput);
			}
			catch (...)
			{
				L_PRINT(LOG_INFO) << "RAGE ERROR";
			}
			//g_RageBot->runRagebot(pInput);
		}
		//g_PredictionSystem->End(pInput, SDK::Cmd);
	}
	F::MISC::MOVEMENT::movment_fix(SDK::Cmd, g_AntiAim->StoreAngels);

	//if (I::Cvar->Find(FNV1A::Hash("ragdoll_gravity_scale"))->value.fl != -1.5f)
	//	I::Cvar->Find(FNV1A::Hash("ragdoll_gravity_scale"))->value.fl = -1.5f;

	//if (pCmd->nButtons.nValue & IN_ATTACK)
	//	I::Engine->client_cmd_unrestricted("say Sosi Pd");


	//CRC::Save(pBaseCmd, SDK::Cmd);
	//if (CRC::CalculateCRC(pBaseCmd) == true)
	//	CRC::Apply(SDK::Cmd);

	return bResult;
}

bool __fastcall H::DrawCrosshair(C_CSWeaponBaseGun* pWeaponBaseGun)
{
	const auto oDrawCrosshair = hkDrawCrosshair.GetOriginal();

	////	if (!C_GET(bool, Vars.bForceCrosshair))
			//return oDrawCrosshair(pWeaponBaseGun);
	if (!SDK::LocalPawn)
		return oDrawCrosshair(pWeaponBaseGun);

	if (!SDK::Alive)
		return oDrawCrosshair(pWeaponBaseGun);

	return !SDK::LocalPawn->IsScoped();
}

int get_model_type(const std::string_view& name) {
	if (name.find("sun") != std::string::npos
		|| name.find("clouds") != std::string::npos)
		return MODEL_SUN;

	if (name.find("effects") != std::string::npos)
		return MODEL_EFFECTS;

	return MODEL_OTHER;
}

bool s = false;

void CS_FASTCALL H::DrawSceneObject(void* ptr, void* a2, CBaseSceneData* scene_data, int count, int a5, void* a6, void* a7, void* a8)
{
	const auto orig = hkDrawSceneObject.GetOriginal();

	if (!scene_data->material)
		orig(ptr, a2, scene_data, count, a5, a6, a7, a8);

	int type = get_model_type(scene_data->material->GetName());
	ByteColor color{};

	switch (type) {
	case MODEL_SUN:
	{
		color = (C_GET(Color_t, Vars.colClouds) * 255).ToByte();
		break;
	}
	}

	for (int i = 0; i < count; ++i)
	{
		auto a1 = scene_data->sceneObject;
		a1->r = color.r;
		a1->g = color.g;
		a1->b = color.b;
	}
	orig(ptr, a2, scene_data, count, a5, a6, a7, a8);
}


void* __fastcall H::UpdateSceneObject(C_AggregateSceneObject* object, void* unk)
{
	const auto original = hkUpdateSceneObject.GetOriginal();
	auto result = original(object, unk);



	if (C_GET(bool, Vars.bWorldModulation))
	{
		Color_t colors = C_GET(Color_t, Vars.colWorld);

		for (int i = 0; i < object->m_nCount; i++)
		{
			object->m_pData[i].r = colors.r;
			object->m_pData[i].g = colors.g;
			object->m_pData[i].b = colors.b;
		}
	}

	return result;
}

bool CS_FASTCALL H::MouseInputEnabled(void* pThisptr)
{
	const auto oMouseInputEnabled = hkMouseInputEnabled.GetOriginal();
	return MENU::bMainWindowOpened ? false : oMouseInputEnabled(pThisptr);
}

void CS_FASTCALL H::FrameStageNotify(void* rcx, int nFrameStage)
{
	const auto oFrameStageNotify = hkFrameStageNotify.GetOriginal();

	switch (nFrameStage)
	{
	case FRAME_RENDER_START:
		skin_changer::OnFrameStageNotify(nFrameStage);
		break;
	case FRAME_RENDER_END:
		skin_changer::OnFrameStageNotify(nFrameStage);
		break;
	case FRAME_NET_UPDATE_END:
		//Lagcomp::g_LagCompensation->Run();
		g_player_records->run();
		//g_anim_sync->on_frame_stage();
		break;
	case FRAME_SIMULATE_END:
		g_Esp->store_players();
		break;
	default:
		break;
	}
	return oFrameStageNotify(rcx, nFrameStage);
}

__int64* CS_FASTCALL H::LevelInit(void* pClientModeShared, const char* szNewMap)
{
	const auto oLevelInit = hkLevelInit.GetOriginal();
	// if global variables are not captured during I::Setup or we join a new game, recapture it
	if (I::GlobalVars == nullptr)
		I::GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 15 ? ? ? ? 48 89 42")), 0x3, 0x7));
	g_EntityList->on_level_init();

	if (!I::GameParticleManagerSystem) {
		try {
			using fn_t = c_game_particle_manager_system * (__fastcall*)();
			static fn_t fn = reinterpret_cast<fn_t>(MEM::FindPattern(CLIENT_DLL, "48 8b 05 ? ? ? ? c3 cc cc cc cc cc cc cc cc 48 89 5c 24 ? 57"));
			I::GameParticleManagerSystem = fn();
		}
		catch (...)
		{
			L_PRINT(LOG_WARNING) << "NOT FOUND GameParticleManagerSystem";;
		}
	}

	static const auto pParticleRegister = I::GetRegisterList(PARTICLES_DLL);
	if (pParticleRegister == nullptr)
		return oLevelInit(pClientModeShared, szNewMap);
	I::ParticleManager = I::Capture<c_particle_manager>(pParticleRegister, PARTICLE_MGR);

	L_PRINT(LOG_INFO) << szNewMap;
	g_BulletTrace->logs.clear();
	// disable model occlusion
	I::PVS->Set(false);
	g_AntiAim->bFreezTime = true;

	return oLevelInit(pClientModeShared, szNewMap);
}

__int64 CS_FASTCALL H::LevelShutdown(void* pClientModeShared)
{
	const auto oLevelShutdown = hkLevelShutdown.GetOriginal();
	// reset global variables since it got discarded by the game
	I::GlobalVars = nullptr;
	g_EntityList->level_shutdown();
	g_BulletTrace->logs.clear();

	I::ParticleManager = nullptr;
	I::GameParticleManagerSystem = nullptr;

	return oLevelShutdown(pClientModeShared);
}

void CS_FASTCALL H::OverrideView(void* a1, CViewSetup* pSetup, __int64 a3, __int64 a4, unsigned long a5)
{
	const auto oOverrideView = hkOverrideView.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return hkOverrideView.GetOriginal()(a1, pSetup, a3, a4, a5);
	if (!SDK::Alive)
		return hkOverrideView.GetOriginal()(a1, pSetup, a3, a4, a5);

	if (C_GET(float, Vars.flAspectRatio) == 1)
	{
		pSetup->nSomeFlags &= ~2;
	}
	else
	{
		pSetup->flAspectRatio = C_GET(float, Vars.flAspectRatio);
		pSetup->nSomeFlags |= 2;
	}

	bool in_third_person = false;
	if (IPT::IsPressed(C_GET(int, Vars.iThirdKey), C_GET(int, Vars.iThirdKeyBind)))
		in_third_person = true;
	//CPlayer_ObserverServices* pObserverServices = SDK::LocalPawn->GetObserverServices();
	//if (!pObserverServices)
	//	return;

	if (in_third_person /*|| pObserverServices->GetObserverMode() == ObserverMode_t::OBS_MODE_IN_EYE*/)
	{
		QAngle_t adjusted_cam_view_angle = g_AntiAim->StoreAngels;
		adjusted_cam_view_angle.x *= -1;

		pSetup->vecOrigin = g_Utils->CalculateCameraPosition(SDK::LocalPawn->GetEyePosition(), -C_GET(int, Vars.iDistanceThird), adjusted_cam_view_angle);

		Ray_t ray{};
		GameTrace_t trace{};
		TraceFilter_t filter{ 0x1C3003, SDK::LocalPawn, nullptr, 4 };

		if (I::GameTraceManager->TraceShape2(&ray, SDK::LocalPawn->GetEyePosition(), pSetup->vecOrigin, &filter, &trace))
		{
			if (trace.m_pHitEntity != nullptr)
				pSetup->vecOrigin = trace.m_vecPosition;
		}

		pSetup->angView = g_Utils->normalize(g_Utils->CalcAngles(pSetup->vecOrigin, SDK::LocalPawn->GetEyePosition()));
	}



	return oOverrideView(a1, pSetup, a3, a4, a5);
}

void CS_FASTCALL H::DrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	const auto oDrawObject = hkDrawObject.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	if (SDK::LocalController == nullptr || SDK::LocalPawn == nullptr)
		return oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	if (!F::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2))
		oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}

void* H::IsRelativeMouseMode(void* pThisptr, bool bActive)
{
	const auto oIsRelativeMouseMode = hkIsRelativeMouseMode.GetOriginal();

	MENU::bMainActive = bActive;

	if (MENU::bMainWindowOpened)
		return oIsRelativeMouseMode(pThisptr, false);

	return oIsRelativeMouseMode(pThisptr, bActive);
}

float CS_FASTCALL H::GetRenderFov(uintptr_t rcx)
{
	const auto oFOV = hkFov.GetOriginal();

	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
	{
		oFOV(rcx);
	}
	else
	{
		if (C_GET(bool, Vars.bFov) && C_GET(float, Vars.flFov) != 0.f /*&& !g_cross->scope*/ /* && !SDK::LocalPawn->IsScoped()*/)
		{
			return C_GET(float, Vars.flFov);
		}
		else
		{
			oFOV(rcx);
		}
	}
}

void H::DrawScope(void* a1, void* a2)
{
	const auto original = hkDrawScope.GetOriginal();

	if (C_GET(bool, Vars.bNoScope))
		return;

	original(a1, a2);
}

void* CS_FASTCALL H::HookInput(CCSInputMessage* a1, CCSGOInputHistoryEntryPB* a2, bool mouseuse, bool idk, void* unk, C_CSPlayerPawn* a6) {
	auto res = hkHookInput.GetOriginal()(a1, a2, mouseuse, idk, unk, a6);
	//L_PRINT(LOG_INFO) << a1 << " | " << a2 << " | " << mouseuse << " | " << idk << " | " << unk << " | " << a6;
	//a2->setPlayerTickCount(0);
	//a1->m_player_tick_count = 0;
	//a2->setRenderTickCount(0);	
	//SDK::Cmd->AddSubtick(true, IN_ATTACK);

	return res;
}