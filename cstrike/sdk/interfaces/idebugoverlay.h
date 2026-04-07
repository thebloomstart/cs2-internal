#pragma once

// used: find pattern, call virtual function
#include "../../utilities/memory.h"

// used: vertor_t
#include "../datatypes/vector.h"
// used: color_t
#include "../datatypes/color.h"

class CDebugOverlay {
public:
	void AddBox(const Vector_t& end, const Vector_t& mins, const Vector_t& maxs, const Vector_t& a3, Color_t clr)
	{
		MEM::CallVFunc<void, 48U>(this, end, mins, maxs, a3, (int)clr.r, (int)clr.g, (int)clr.b, (int)clr.a, static_cast<double>(4.f));
	}

	//void AddLineOverlay(const Vector_t& vecOrigin, const Vector_t& vecDest, const ImU32& Color, const bool bNoDethTest, const double flDuration) {
	//	auto fn = MEM::CallVFunc<void,12>(this, void*, const Vector_t&, const Vector_t&, const ImU32&, bool, double);
	//	fn(this, vecOrigin, vecDest, Color, bNoDethTest, flDuration);
	//	//return CallClassFn<void, 12>(this, vecOrigin, vecDest, Color, bNoDethTest, static_cast<double>(flDuration));
	//}
};