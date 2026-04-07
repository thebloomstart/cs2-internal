#pragma once
#include "memory.h"
#include "utilities/memory.h"

class CUIPanel;

struct CPanel2D {
	void* vmt;
	CUIPanel* uiPanel;
};

class CUIEngine {
public:
	void RunScript(CUIPanel* panel, const char* script) {
		if (!panel || !script)
			return;

		MEM::CallVFunc<void, 81>(this, panel, script, (const char*)nullptr, 0);
	}
};

class CPanoramaUIEngine {
public:
	CUIEngine* AccessUIEngine() {
		return MEM::CallVFunc<CUIEngine*, 13>(this);
	}

	//CPanel2D* GetMainMenuPanel() {
	//	return *reinterpret_cast<CPanel2D**>(GetSigPtr(FNV1A::HashConst("MAIN_MENU_PANEL")));
	//}

	//void RunScriptOnMainMenuPanel(const char* script) {
	//	CUIEngine* engine = AccessUIEngine();
	//	CPanel2D* panel = GetMainMenuPanel();
	//	if (!engine || !panel || !script)
	//		return;

	//	engine->RunScript(panel->uiPanel, script);
	//}
};