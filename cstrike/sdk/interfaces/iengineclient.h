#pragma once

// used: callvfunc
#include "../../utilities/memory.h"
#include "../../sdk/datatypes/vector.h"

enum EClientFrameStage : int
{
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_FULL_FRAME_UPDATE_ON_REMOVE,
	FRAME_RENDER_START,
	FRAME_RENDER_END,
	FRAME_NET_UPDATE_END,
	FRAME_NET_CREATION,
	FRAME_SIMULATE_END
};

	class CEconItemSchema;

inline constexpr uint64_t Helper_GetAlternateIconKeyForWeaponPaintWearItem(
uint16_t nDefIdx, uint32_t nPaintId, uint32_t nWear)
{
	return (nDefIdx << 16) + (nPaintId << 2) + nWear;
}

class CEconItemSystem
{
public:
	auto GetEconItemSchema()
	{
		return *reinterpret_cast<CEconItemSchema**>((uintptr_t)(this) + 0x8);
	}
};

class CDebugOverlay;
class ISource2Client
{
public:
	auto GetEconItemSystem()
	{
		static auto fnGetEconItemSystem = *reinterpret_cast<CEconItemSystem**>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 05 ?? ?? ?? ?? 48 85 C0 75 6B")), 0x3));

		return fnGetEconItemSystem;
	}
	CDebugOverlay* GetSceneDebugOverlay() {
		return MEM::CallVFunc<CDebugOverlay*, 163U>(this);
	}
};


class c_networked_client_info
{
	std::byte pad_001[0x4];

public:
	int m_render_tick;
	float m_render_tick_fraction;
	int m_player_tick_count;
	float m_player_tick_fraction;

private:
	std::byte pad_002[0x4];

public:
	struct
	{
	private:
		std::byte pad_022[0xC];

	public:
		Vector_t m_eye_pos;
	}* m_local_data;

private:
	std::byte pad_003[0x8];
};
class INetChannelInfo
{
public:

	float get_network_latency()
	{
		return MEM::CallVFunc<float, 10U>(this);
	}

	float get_engine_latency()
	{
		return MEM::CallVFunc<float, 11U>(this);
	}
};

class IEngineClient
{
public:
	int GetMaxClients()
	{
		return MEM::CallVFunc<int, 34U>(this);
	}

	bool IsInGame()
	{
		return MEM::CallVFunc<bool, 35U>(this);
	}

	bool IsConnected()
	{
		return MEM::CallVFunc<bool, 36U>(this);
	}
	[[nodiscard]] INetChannelInfo* GetNetChannelInfo(int nSplitScreenSlot = 0)
	{
		return MEM::CallVFunc<INetChannelInfo*, 37U>(this, nSplitScreenSlot);
	}
	// return CBaseHandle index
	int GetLocalPlayer()
	{
		int nIndex = -1;

		MEM::CallVFunc<void, 49U>(this, std::ref(nIndex), 0);

		return nIndex + 1;
	}
	void client_cmd_unrestricted(const char* cmd)
	{
		//xref: buy vesthelm
		return MEM::CallVFunc<void, 45U>(this, 0, cmd, 0x7FFEF001);
	}
	[[nodiscard]] const char* GetLevelName()
	{
		return MEM::CallVFunc<const char*, 56U>(this);
	}

	[[nodiscard]] const char* GetLevelNameShort()
	{
		return MEM::CallVFunc<const char*, 57U>(this);
	}

	c_networked_client_info* get_networked_client_info()
	{
		c_networked_client_info client_info;

		MEM::CallVFunc<void*, 178>(this, &client_info);
		return &client_info;
	}
	[[nodiscard]] const char* GetProductVersionString()
	{
		return MEM::CallVFunc<const char*, 82U>(this);
	}
};
