#pragma once
#include "../../utilities/memory.h"

enum Flow : int {
	FLOW_OUTGOING = 0,
	FLOW_INCOMING = 1,
};

class CNetChannelInfo {
public:
	float GetLatency(Flow flow) {
		return MEM::CallVFunc<float, 10U>(this, flow);
	}
};

class CNetworkGameClient
{
public:
	MEM_PAD(0xE8); //0x0000
	CNetChannelInfo* pNetChannelInfo; // 0x00E8
	MEM_PAD(0x8); // 0x00F0
	bool bShouldPredict; // 0x00F8
	MEM_PAD(0x7B); // 0x00F9
	int nSomePredictionTick; // 0x0174
	MEM_PAD(0x104); // 0x0178
	int nDeltaTick; // 0x027C

	void ClientPredict() {
		using prediction_fn = void(__fastcall*)(CNetworkGameClient*, unsigned int);
		static prediction_fn func = reinterpret_cast<prediction_fn>(MEM::FindPattern(ENGINE2_DLL, "40 55 41 56 48 83 EC 28 80 B9"));
		return func(this, 1);
	}
	bool IsConnected()
	{
		return MEM::CallVFunc<bool, 12U>(this);
	}
	float maybe() {
		return MEM::CallVFunc<float, 62u>(this);
	}
	// force game to clear cache and reset delta tick
	void FullUpdate()
	{
		// @ida: #STR: "Requesting full game update (%s)...\n"
		MEM::CallVFunc<void, 28U>(this, CS_XOR("unk"));
	}

	int GetDeltaTick()
	{
		// @ida: offset in FullUpdate();
		// (nDeltaTick = -1) == FullUpdate() called
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x25C);
	}
};

class INetworkClientService
{
public:
	[[nodiscard]] CNetworkGameClient* GetNetworkGameClient()
	{
		return MEM::CallVFunc<CNetworkGameClient*, 23U>(this);
	}
};
