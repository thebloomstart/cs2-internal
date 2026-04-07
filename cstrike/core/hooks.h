#pragma once

// used: [d3d] api
#include <d3d11.h>
#include <dxgi1_2.h>

// used: chookobject
#include "../utilities/detourhook.h"

// used: viewmatrix_t
#include "../sdk/datatypes/matrix.h"
#include "../sdk/interfaces/ccsgoinput.h"

namespace VTABLE
{
	namespace D3D
	{
		enum
		{
			PRESENT = 8U,
			RESIZEBUFFERS = 13U,
			RESIZEBUFFERS_CSTYLE = 39U,
		};
	}

	namespace DXGI
	{
		enum
		{
			CREATESWAPCHAIN = 10U,
		};
	}

	namespace CLIENT
	{
		enum
		{
			CREATEMOVE = 5U,
			CREATEMOVEUSERCMD = 19U,
			MOUSEINPUTENABLED = 17U,
			FRAMESTAGENOTIFY = 36U,
		};
	}

	namespace INPUTSYSTEM
	{
		enum
		{
			ISRELATIVEMOUSEMODE = 76U,
		};
	}
}

class CRenderGameSystem;
class IViewRender;
class CCSGOInput;
class CViewSetup;
class CMeshData;
class CBaseHandle;
class CEntityInstance;
class C_SceneLightObject;
class C_PostProcessingVolume;
class C_EnvSky;
class CGameEvent;
class C_BaseEntity;

namespace H
{
	bool Setup();
	void Destroy();

	float ViewModelFov();

	/* @section: handlers */
	// d3d11 & wndproc
	HRESULT WINAPI Present(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags);
	void* UpdateSkyBox(C_EnvSky* sky);
	void* DrawLightScene(void* a1, C_SceneLightObject* a2, __int64 a3);
	void* UpdatePostProccesing(C_PostProcessingVolume* a1, int a2);
	HRESULT CS_FASTCALL ResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags);
	HRESULT WINAPI CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
	long CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// game's functions
	ViewMatrix_t* CS_FASTCALL GetMatrixForView(CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels);
	__int64  CS_FASTCALL AutoAccept(void* unk, const char* event_name, void* unk1, float unk2);
	void CS_FASTCALL HandleGameEvents(void* rcx, CGameEvent* const ev);
	void* CS_FASTCALL ApplyViewPunch(CPlayer_CameraServices* pCameraServices, float* a2, float* a3, float* a4);
	void* AddEntity(void* rcx, C_BaseEntity* inst, CBaseHandle handle);
	void* RemoveEntity(void* rcx, C_BaseEntity* inst, CBaseHandle handle);
	void CS_FASTCALL SetModel(void* rcx, const char* model);
	void CS_FASTCALL NoFlash(__int64 a1, int a2, __int64* a3, __int64 a4, __m128* a5);
	void CS_FASTCALL Smoke(__int64 a1, __int64* a2, int a3, int a4, __int64 a5, __int64 a6);
	void CS_FASTCALL DrawTeamIntro(std::uintptr_t a1, std::uintptr_t a2, char* a3);
	bool CS_FASTCALL InterPlayer(C_BaseEntity* ent, int some_int);
	void* CS_FASTCALL Sky3DParams(void* a1);
	void CS_FASTCALL ViewModelChanger(float* a1, Vector_t a2, float* R);
	void __fastcall ValidQAngel(CCSGOInput* input, int a2);
	bool CS_FASTCALL UnlockInventory(void* a1);
	bool CS_FASTCALL CreateMoveUserCmd(CCSGOInput* pInput, int nSlot, CUserCmd* pCmd);
	bool __fastcall DrawCrosshair(C_CSWeaponBaseGun* pWeaponBaseGun);
	void* __fastcall UpdateSceneObject(C_AggregateSceneObject* object, void* unk);
	void CS_FASTCALL DrawSceneObject(void* ptr, void* a2, CBaseSceneData* scene_data, int count, int a5, void* a6, void* a7, void* a8);
	bool CS_FASTCALL MouseInputEnabled(void* pThisptr);
	void CS_FASTCALL FrameStageNotify(void* rcx, int nFrameStage);
	__int64* CS_FASTCALL LevelInit(void* pClientModeShared, const char* szNewMap);
	__int64 CS_FASTCALL LevelShutdown(void* pClientModeShared);
	void CS_FASTCALL OverrideView(void* a1, CViewSetup* pSetup, __int64 a3, __int64 a4, unsigned long a5);
	void CS_FASTCALL DrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);
	void* IsRelativeMouseMode(void* pThisptr, bool bActive);
	float CS_FASTCALL GetRenderFov(uintptr_t rcx);
	void DrawScope(void* a1, void* a2);
	void* CS_FASTCALL HookInput(CCSInputMessage* a1, CCSGOInputHistoryEntryPB* a2, bool mouseuse, bool idk, void* unk, C_CSPlayerPawn* a6);
	bool CS_FASTCALL DrawLegs(C_CSPlayerPawn* Entity);

	/* @section: managers */
	inline CBaseHookObject<decltype(&Present)> hkPresent = {};
	inline CBaseHookObject<decltype(&ResizeBuffers)> hkResizeBuffers = {};
	inline CBaseHookObject<decltype(&CreateSwapChain)> hkCreateSwapChain = {};
	inline CBaseHookObject<decltype(&WndProc)> hkWndProc = {};
	inline CBaseHookObject<decltype(&GetMatrixForView)> hkGetMatrixForView = {};
	inline CBaseHookObject<decltype(&CreateMoveUserCmd)> hkCreateMoveUserCmd = {};
	inline CBaseHookObject<decltype(&MouseInputEnabled)> hkMouseInputEnabled = {};
	inline CBaseHookObject<decltype(&IsRelativeMouseMode)> hkIsRelativeMouseMode = {};
	inline CBaseHookObject<decltype(&FrameStageNotify)> hkFrameStageNotify = {};
	inline CBaseHookObject<decltype(&LevelInit)> hkLevelInit = {};
	inline CBaseHookObject<decltype(&LevelShutdown)> hkLevelShutdown = {};
	inline CBaseHookObject<decltype(&OverrideView)> hkOverrideView = {};
	inline CBaseHookObject<decltype(&DrawObject)> hkDrawObject = {};
	inline CBaseHookObject<decltype(&SetModel)> hkSetModel = {};
	inline CBaseHookObject<decltype(&AddEntity)> hkAddEntity = {};
	inline CBaseHookObject<decltype(&RemoveEntity)> hkRemoveEntity = {};
	inline CBaseHookObject<decltype(&UpdateSkyBox)> hkUpdateSkyBox = {};
	inline CBaseHookObject<decltype(&DrawLightScene)> hkDrawLightScene = {};
	inline CBaseHookObject<decltype(&UpdatePostProccesing)> hkUpdatePostProccesing = {};
	inline CBaseHookObject<decltype(&ValidQAngel)> hkVerify = {};
	inline CBaseHookObject<decltype(&HandleGameEvents)> hkGameEvents = {};
	inline CBaseHookObject<decltype(&DrawScope)> hkDrawScope = {};
	inline CBaseHookObject<decltype(&GetRenderFov)> hkFov = {};
	inline CBaseHookObject<decltype(&ViewModelFov)> hkViewModelFov = {};
	inline CBaseHookObject<decltype(&AutoAccept)> hkAutoAccept = {};
	inline CBaseHookObject<decltype(&DrawSceneObject)> hkDrawSceneObject = {};
	inline CBaseHookObject<decltype(&UpdateSceneObject)> hkUpdateSceneObject = {};
	inline CBaseHookObject<decltype(&ViewModelChanger)> hkViewModelChanger = {};
	inline CBaseHookObject<decltype(&DrawCrosshair)> hkDrawCrosshair = {};
	inline CBaseHookObject<decltype(&UnlockInventory)> hkUnlockInv = {};
	inline CBaseHookObject<decltype(&Smoke)> hkNoSmoke = {};
	inline CBaseHookObject<decltype(&InterPlayer)> hkInterPlayer = {};
	inline CBaseHookObject<decltype(&DrawLegs)> hkDrawLegs = {};
	inline CBaseHookObject<decltype(&DrawTeamIntro)> hkDrawTeamIntro = {};
	inline CBaseHookObject<decltype(&HookInput)> hkHookInput = {};
	inline CBaseHookObject<decltype(&Sky3DParams)> hkSky3DParams = {};
	inline CBaseHookObject<decltype(&ApplyViewPunch)> hkApplyViewPunch = {};
	inline CBaseHookObject<decltype(&NoFlash)> hkNoFlash = {};

}
