#pragma once

// used: mem_pad
#include "../../utilities/memory.h"

// used: cusercmd
#include "../datatypes/usercmd.h"
#include "../entity.h"
#define MULTIPLAYER_BACKUP 150

class CTinyMoveStepData
{
public:
	float flWhen; //0x0000
	MEM_PAD(0x4); //0x0004
	std::uint64_t nButton; //0x0008
	bool bPressed; //0x0010
	MEM_PAD(0x7); //0x0011
}; //Size: 0x0018

class CMoveStepButtons
{
public:
	std::uint64_t nKeyboardPressed; //0x0000
	std::uint64_t nMouseWheelheelPressed; //0x0008
	std::uint64_t nUnPressed; //0x0010
	std::uint64_t nKeyboardCopy; //0x0018
}; //Size: 0x0020

// @credits: www.unknowncheats.me/forum/members/2943409.html
class CExtendedMoveData : public CMoveStepButtons
{
public:
	float flForwardMove; //0x0020
	float flSideMove; //0x0024
	float flUpMove; //0x0028
	std::int32_t nMouseDeltaX; //0x002C
	std::int32_t nMouseDeltaY; //0x0030
	std::int32_t nAdditionalStepMovesCount; //0x0034
	CTinyMoveStepData tinyMoveStepData[12]; //0x0038
	Vector_t vecViewAngle; //0x0158
	std::int32_t nTargetHandle; //0x0164
}; //Size:0x0168

class CSubtickInput
{
	uint64_t nButton; // 0x18
	bool bPressed; // 0x20
	float flWhen; // 0x24
	float flAnalogForwardDelta; // 0x28
	float flAnalogLeftDelta; // 0x2C
};

struct CCSInputMessage
{
	int32_t m_frame_tick_count; //0x0000
	float m_frame_tick_fraction; //0x0004
	int32_t m_player_tick_count; //0x0008
	float m_player_tick_fraction; //0x000C
	QAngle_t m_view_angles; //0x0010
	QAngle_t m_shoot_position; //0x001C
	int32_t m_target_index; //0x0028
	QAngle_t m_target_head_position; //0x002C
	QAngle_t m_target_abs_origin; //0x0038
	QAngle_t m_target_angle; //0x0044
	int32_t m_sv_show_hit_registration; //0x0050
	int32_t m_entry_index_max; //0x0054
	int32_t m_index_idk; //0x0058
};

class CCSGOInput
{
public:
	char pad_0000[592]; //0x0000
	bool bBlockShot; //0x0250
	bool bInThirdPerson; //0x0251
	char pad_0252[6]; //0x0252
	QAngle_t angThirdPersonAngles; //0x0258
	char pad_0264[20]; //0x0264
	uint64_t nKeyboardPressed; //0x0278
	uint64_t nMouseWheelheelPressed; //0x0280
	uint64_t nUnPressed; //0x0288
	uint64_t nKeyboardCopy; //0x0290
	float flForwardMove; //0x0298
	float flSideMove; //0x029C
	float flUpMove; //0x02A0
	Vector2D_t nMousePos; //0x02A4
	int32_t SubticksCount; //0x02AC
	CSubtickInput Subticks[12]; //0x02B0
	Vector_t vecViewAngle; //0x03D0
	int32_t nTargetHandle; //0x03DC
	char pad_03E0[560]; //0x03E0
	int32_t nAttackStartHistoryIndex1; //0x0610
	int32_t nAttackStartHistoryIndex2; //0x0614
	int32_t nAttackStartHistoryIndex3; //0x0618
	char pad_061C[4]; //0x061C
	int32_t MessageSize; //0x0620
	char pad_0624[4]; //0x0624
	CCSInputMessage* Message; //0x0628

	int GetCommandIndex(void* pController) {
		static auto GetCommandIndex = reinterpret_cast<void* (__fastcall*)(void*, int*)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? 8B 8D ? ? ? ? 8D 51"), 1, 0));

		int nIndex = 0;
		GetCommandIndex(pController, &nIndex);

		return nIndex;
	}

	CUserCmd* GetUserCmd(void* pController, DWORD SequenceNumber) {
		static auto GetUserCmd = reinterpret_cast<CUserCmd * (__fastcall*)(void*, DWORD)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 E4 48 89 44 24"), 1, 0));

		return GetUserCmd(pController, SequenceNumber);
	}

	void* GetUserCmdEntry(void* pController, int nCommandIndex) {
		static auto GetUserCmdEntry = reinterpret_cast<void* (__fastcall*)(void*, int)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? 48 8B CF 4C 8B E8 44 8B B8"), 1, 0));

		return GetUserCmdEntry(pController, nCommandIndex);
	}

	CUserCmd* GetCmd() {
		static void* CmdBaseAddress = MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 4C 8B E8"), 0x3, 0x7);

		if (!CmdBaseAddress)
			return nullptr;

		auto pLocalController = CCSPlayerController::GetLocalPlayerController();
		if (!pLocalController)
			return nullptr;

		int nIndex = GetCommandIndex(pLocalController);
		if (!nIndex)
			return nullptr;

		int nCommandIndex = nIndex - 1;

		if (nCommandIndex == -1)
			nCommandIndex = 0xFFFFFFFF;

		void* pCmdEntry = GetUserCmdEntry(CmdBaseAddress, nCommandIndex);
		if (!pCmdEntry)
			return nullptr;

		DWORD SequenceNumber = *reinterpret_cast<DWORD*>(*(__int64*)pCmdEntry + 0x5C00);

		CUserCmd* pCmd = GetUserCmd(pLocalController, SequenceNumber);

		return pCmd;
	}

	template <typename T>
	void SetViewAngle(T& angView)
	{
		// @ida: this got called before GetMatricesForView
		using fnSetViewAngle = std::int64_t(CS_FASTCALL*)(void*, std::int32_t, T&);
		static auto oSetViewAngle = reinterpret_cast<fnSetViewAngle>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 75 3F 48")));

		#ifdef CS_PARANOID
		CS_ASSERT(oSetViewAngle != nullptr);
		#endif
		if (!this)
			return;

		oSetViewAngle(this, 0, std::ref(angView));
	}

	QAngle_t GetViewAngles()
	{
		using fnGetViewAngles = std::int64_t(CS_FASTCALL*)(CCSGOInput*, std::int32_t);
		static auto oGetViewAngles = reinterpret_cast<fnGetViewAngles>(MEM::FindPattern(CLIENT_DLL, CS_XOR("4C 8B C1 85 D2 74 ? 48 8D 05")));

		#ifdef CS_PARANOID
		CS_ASSERT(oGetViewAngles != nullptr);
		#endif

		return *reinterpret_cast<QAngle_t*>(oGetViewAngles(this, 0));
	}
};
