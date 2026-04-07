#pragma once
// used: pad and findpattern
#include "../../utilities/memory.h"
// used: vector
#include "../../sdk/datatypes/vector.h"
// used: array
#include <array>
#define CLIP_TRACE_TO_PLAYERS "48 8B C4 55 56 48 8D A8 58 FF FF FF 48 81 EC 98 01 00 00 48"

struct Ray_t
{
public:
	Vector_t m_vecStart;
	Vector_t m_vecEnd;
	Vector_t m_vecMins;
	Vector_t m_vecMaxs;
	MEM_PAD(0x4);
	std::uint8_t UnkType;
};
static_assert(sizeof(Ray_t) == 0x38);

struct SurfaceData_t
{
public:
	MEM_PAD(0x8)
	float m_flPenetrationModifier;
	float m_flDamageModifier;
	MEM_PAD(0x4)
	int m_iMaterial;
};

static_assert(sizeof(SurfaceData_t) == 0x18);

struct TraceHitboxData_t
{
public:
	MEM_PAD(0x38);
	int m_nHitGroup;
	MEM_PAD(0x4);
	int m_nHitboxId;
};
static_assert(sizeof(TraceHitboxData_t) == 0x44);

class C_CSPlayerPawn;
struct GameTrace_t
{
public:
	GameTrace_t() = default;

	SurfaceData_t* GetSurfaceData();
	int GetHitboxId();
	int GetHitgroup();
	bool DidHitWorld() const;
	bool IsVisible() const;

	void* m_pSurface;
	C_CSPlayerPawn* m_pHitEntity;
	TraceHitboxData_t* m_pHitboxData;
	MEM_PAD(0x38);
	std::uint32_t m_uContents;
	MEM_PAD(0x24);
	Vector_t m_vecStartPos;
	Vector_t m_vecEndPos;
	Vector_t m_vecNormal;
	Vector_t m_vecPosition;
	MEM_PAD(0x4);
	float m_flFraction;
	MEM_PAD(0x6);
	bool m_bAllSolid;
	MEM_PAD(0x4D)
}; // Size: 0x108

static_assert(sizeof(GameTrace_t) == 0x108);

struct TraceFilter_t
{
public:
	MEM_PAD(0x8);
	std::int64_t m_uTraceMask;//0x8
	std::array<std::int64_t, 2> m_v1;//
	std::array<std::int32_t, 4> m_arrSkipHandles;//
	std::array<std::int16_t, 2> m_arrCollisions;// 0x20
	std::int16_t m_v2;//0x24
	std::uint8_t m_v3;//
	std::uint8_t m_v4;//
	std::uint8_t collision1; //0x30
	std::uint16_t collision2; // 0x32

	TraceFilter_t() = default;
	TraceFilter_t(std::uint64_t uMask, C_CSPlayerPawn* pSkip1, C_CSPlayerPawn* pSkip2, int nLayer);
};
static_assert(sizeof(TraceFilter_t) == 0x40);

struct trace_arr_element_t
{
	MEM_PAD(0x30)
};

struct trace_data_t
{
	std::int32_t m_uk1{};
	float m_uk2{ 52.0f };
	void* m_arr_pointer{};
	std::int32_t m_uk3{ 128 };
	std::int32_t m_uk4{ static_cast<std::int32_t>(0x80000000) };
	std::array<trace_arr_element_t, 0x80> m_arr = {};
	MEM_PAD(0x8)
	std::int64_t m_num_update{};
	void* m_pointer_update_value{};
	MEM_PAD(0xC8)
	Vector_t m_start{}, m_end{};
	MEM_PAD(0x50)
};

struct UpdateValueT
{
	float previousLenght{};
	float currentLenght{};
	MEM_PAD(0x8)
	std::int16_t handleIdx{};
	MEM_PAD(0x6)
};

class game_trace_t
{
public:
	void* Surface;
	C_CSPlayerPawn* HitEntity;
	TraceHitboxData_t* HitboxData;
	MEM_PAD(0x38)
	std::uint32_t Contents;
	MEM_PAD(0x24)
	Vector_t m_start_pos, m_end_pos, m_normal, m_pos;
	MEM_PAD(0x4);
	float Fraction;
	MEM_PAD(0x6);
	bool m_all_solid;
	MEM_PAD(0x4D)
		bool DidHitWorld() const;
};

class CGameTraceManager
{
private:
	using CreateTraces = void(__fastcall*)(trace_data_t*, Vector_t, Vector_t,TraceFilter_t, int);
	using fn_init_trace_info = void(__fastcall*)(game_trace_t*);
	using gettraceinfo = void(__fastcall*)(trace_data_t*, game_trace_t*, float, void*);
	using Inits = TraceFilter_t * (__fastcall*)(TraceFilter_t&, void*, uint64_t, uint8_t, uint16_t);
	using handle_bullet_penetrations = bool(__fastcall*)(trace_data_t*, void*, UpdateValueT*, void*, void*, void*, void*, void*, bool);
	using TraceShapes = bool(__fastcall*)(CGameTraceManager*, Ray_t*, Vector_t*, Vector_t*, TraceFilter_t*, GameTrace_t*);
	using fnTraceShape2 = bool(__fastcall*)(CGameTraceManager*, Ray_t*, Vector_t*, Vector_t*, TraceFilter_t*, game_trace_t*);
	using funClipTraceToPlayers = void(__fastcall*)(Vector_t&, Vector_t&, TraceFilter_t*, GameTrace_t*, float, int, float);
	using funClipTraceToPlayers2 = void(__fastcall*)(Vector_t&, Vector_t&, TraceFilter_t*, game_trace_t*, float, int, float);
	using fClipRayToEntity = bool(__fastcall*)(CGameTraceManager*, Ray_t*, Vector_t*, Vector_t*, C_CSPlayerPawn*, TraceFilter_t*, GameTrace_t*);
	using fClipRayToEntity2 = bool(__fastcall*)(CGameTraceManager*, Ray_t*, Vector_t*, Vector_t*, C_CSPlayerPawn*, TraceFilter_t*, game_trace_t*);

	inline static TraceShapes fnTraceShape;
	inline static handle_bullet_penetrations fnhandle_bullet_penetration;
	inline static Inits fnInit;
	inline static gettraceinfo fnget_trace_info;
	inline static fn_init_trace_info fninittraceinfo;
	inline static CreateTraces fnCreateTrace;
	inline static fnTraceShape2 fnTraceShape22;
	inline static funClipTraceToPlayers fnClipTraceToPlayers;
	inline static funClipTraceToPlayers2 fnClipTraceToPlayers22;
	inline static fClipRayToEntity fnClipRayToEntity;
	inline static fClipRayToEntity2 fnClipRayToEntity22;
public:


	bool TraceHook()
	{
		fnCreateTrace = reinterpret_cast<CreateTraces>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 40 F2")));
		fninittraceinfo = reinterpret_cast<fn_init_trace_info>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 57 48 83 EC 20 48 8B D9 33 FF 48 8B 0D")));
		fnget_trace_info = reinterpret_cast<gettraceinfo>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 48 8B E9 0F")));
		fnInit = reinterpret_cast<Inits>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 0F B6 41 37 33")));
		fnhandle_bullet_penetration = reinterpret_cast<handle_bullet_penetrations>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 44 89 48 20 48 89 50 10 48")));
		fnTraceShape = reinterpret_cast<TraceShapes>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 20 48 89 4C 24 08 55 56 41")));
		fnTraceShape22 = reinterpret_cast<fnTraceShape2>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 20 48 89 4C 24 08 55 56 41")));
		fnClipTraceToPlayers = reinterpret_cast<funClipTraceToPlayers>(MEM::FindPattern(CLIENT_DLL, "48 8B C4 55 56 48 8D A8 48 FF FF FF 48 81 EC A8 01 00 00 48 89 58 08 49"));
		fnClipTraceToPlayers22 = reinterpret_cast<funClipTraceToPlayers2>(MEM::FindPattern(CLIENT_DLL, "48 8B C4 55 56 48 8D A8 48 FF FF FF 48 81 EC A8 01 00 00 48 89 58 08 49"));
		fnClipRayToEntity = reinterpret_cast<fClipRayToEntity>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC C0 00 00 00 48 8B 9C")));
		fnClipRayToEntity22 = reinterpret_cast<fClipRayToEntity2>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC C0 00 00 00 48 8B 9C")));
		
		return true;
	}

	static void CreateTrace(trace_data_t* const trace, const Vector_t start,
	const Vector_t end, const TraceFilter_t& filler,
	const int penetration_count)
	{
		CS_ASSERT(fnCreateTrace != nullptr);
		return fnCreateTrace(trace, start, end, filler, penetration_count);
	}

	void init_trace_info(game_trace_t* const hit)
	{
		fninittraceinfo(hit);
	}

	static void get_trace_info(trace_data_t* trace, game_trace_t* hit,
	const float unknown_float, void* unknown)
	{
		return fnget_trace_info(trace, hit, unknown_float, unknown);
	}

	void InitializeTraceInfo(game_trace_t* const hit)
	{
		fninittraceinfo(hit);
	}

	void Init(TraceFilter_t& filter, C_CSPlayerPawn* skip, uint64_t mask, uint8_t layer, uint16_t idk)
	{
		//initfilter_19A770((__int64)filter, a2, 536577i64, 4, 7);
		CS_ASSERT(fnInit != nullptr);
		fnInit(filter, skip, mask, layer, idk);
	}

	static bool handle_bullet_penetration(trace_data_t* const trace, void* stats,
	UpdateValueT* const mod_value,
	const bool draw_showimpacts = false)
	{
		return fnhandle_bullet_penetration(trace, stats, mod_value, nullptr, nullptr, nullptr, nullptr, nullptr, draw_showimpacts);
	}

	bool TraceShape2(Ray_t* pRay, Vector_t vecStart, Vector_t vecEnd, TraceFilter_t* pFilter, GameTrace_t* trace)
	{
		CS_ASSERT(fnTraceShape != nullptr);
		return fnTraceShape(this, pRay, &vecStart, &vecEnd, pFilter, trace);
	}

	bool TraceShape(Ray_t* pRay, Vector_t vecStart, Vector_t vecEnd, TraceFilter_t* pFilter, game_trace_t* pGameTrace)
	{
		return fnTraceShape22(this, pRay, &vecStart, &vecEnd, pFilter, pGameTrace);
	}

	void ClipTraceToPlayers(Vector_t& start, Vector_t& end, TraceFilter_t* filter, GameTrace_t* trace, float min, int length, float max)
	{ // cHoca

		CS_ASSERT(fnClipTraceToPlayers != nullptr);
		fnClipTraceToPlayers(start, end, filter, trace, min, max, length);
	}
	void ClipTraceToPlayers(Vector_t& start, Vector_t& end, TraceFilter_t* filter, game_trace_t* trace, float min, int length, float max)
	{ // cHoca

		CS_ASSERT(fnClipTraceToPlayers22 != nullptr);
		fnClipTraceToPlayers22(start, end, filter, trace, min, max, length);
	}
	bool ClipRayToEntity(Ray_t* pRay, Vector_t vecStart, Vector_t vecEnd, C_CSPlayerPawn* pPawn, TraceFilter_t* pFilter, GameTrace_t* pGameTrace)
	{
		if (!fnClipRayToEntity)
			return 0;
		return fnClipRayToEntity(this, pRay, &vecStart, &vecEnd, pPawn, pFilter, pGameTrace);
	}
	bool ClipRayToEntity(Ray_t* pRay, Vector_t vecStart, Vector_t vecEnd, C_CSPlayerPawn* pPawn, TraceFilter_t* pFilter, game_trace_t* pGameTrace)
	{
		if (fnClipRayToEntity22 == nullptr)
			return false;
		return fnClipRayToEntity22(this, pRay, &vecStart, &vecEnd, pPawn, pFilter, pGameTrace);
	}
};
