#include "KillEffects.h"
#include "utilities/memory.h"
#include "sdk/entity.h"
#include "core/variables.h"

void Effect::ShokedEffect(C_CSPlayerPawn* pLocal)
{
	if (!C_GET(bool, Vars.bHiteffect))
		return;
	/*static auto EffectDataVftable = (MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8D 05 F9 CB 06 01 48 8D 4D DF 48 89 45 D7 41")));*/

	CEffectData effect = {};
	//effect.vftable = EffectDataVftable;
	effect.handle_entity = pLocal->GetHandleEntity();
	effect.handle_other_entity = static_cast<uint32_t>(-1);
	MEM::fnDispatchEffect("csshocked", &effect); //csshocked
}
