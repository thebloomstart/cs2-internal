#pragma once

// used: [stl] vector
#include <vector>

#include "../common.h"

// used: [ext] imgui, draw, animation
#include "../utilities/draw.h"

#define MENU_MAX_BACKGROUND_PARTICLES 100

enum wep_type : int {
	PISTOL = 1,
	HEAVY_PISTOL = 2,
	ASSULT = 3,
	SNIPERS = 4,
	SCOUT = 5,
	AWP = 6,
};

class CTab
{
public:
	const char* szName;
	void (*pRenderFunction)();
};

inline const char* hitbox_names[] = {
	"Head",
	"Neck",
	"Upper Chest",
	"Chest",
	"Stomach",
	"Legs",
	"Feet"
};

inline const char* Bones[] = {
	"Head", // head = 6
	"Neck", // neck_0 = 5
	"Spine 1", // spine_1 = 4
	"Spine 2", // spine_2 = 2
	"Pelvis", // pelvis = 0
	"Left Upper Arm", // arm_upper_L = 8
	"Left Lower Arm", // arm_lower_L = 9
	"Left Hand", // hand_L = 10
	"Right Upper Arm", // arm_upper_R = 13
	"Right Lower Arm", // arm_lower_R = 14
	"Right Hand", // hand_R = 15
	"Left Upper Leg", // leg_upper_L = 22
	"Left Lower Leg", // leg_lower_L = 23
	"Left Ankle", // ankle_L = 24
	"Right Upper Leg", // leg_upper_R = 25
	"Right Lower Leg", // leg_lower_R = 26
	"Right Ankle" // ankle_R = 27
};




namespace MENU
{
	void menu();
	void Hotkeys();
	void RenderWatermark();

	void UpdateStyle(ImGuiStyle* pStyle = nullptr);

	/* @section: particles */
	struct ParticleData_t
	{
		ParticleData_t(const ImVec2& vecPosition, const ImVec2& vecVelocity) :
			vecPosition(vecPosition), vecVelocity(vecVelocity) {
		}

		// current particle position
		ImVec2 vecPosition = {};
		// current particle velocity
		ImVec2 vecVelocity = {};
	};

	struct ParticleContext_t
	{
		ParticleContext_t(const int nMaxParticles = 100)
		{
			// allocate memory for particles
			this->vecParticles.reserve(nMaxParticles);
			// create particles if needed
		}

		~ParticleContext_t()
		{
			// since no memory allocated, just clear vector
			this->vecParticles.clear();
		}

		void Render(ImDrawList* pDrawList, const ImVec2& vecScreenSize, const float flAlpha);

		// create particle with random velocity/position
		void AddParticle(const ImVec2& vecScreenSize);
		// current size of particles
		const size_t Count() const { return this->vecParticles.size(); }
	private:
		// draw particle (circle)
		void DrawParticle(ImDrawList* pDrawList, ParticleData_t& particle, const Color_t& colPrimary);

		// find & draw connection as a line between particles
		void FindConnections(ImDrawList* pDrawList, ParticleData_t& particle, const Color_t& colPrimary, float flMaxDistance);
		void DrawConnection(ImDrawList* pDrawList, ParticleData_t& particle, ParticleData_t& otherParticle, float flAlpha, const Color_t& colPrimary) const;

		// update particle position/velocity
		// reversed direction when particle is out of screen
		void UpdatePosition(ParticleData_t& particle, const ImVec2& vecScreenSize) const;
		void ResolveScreenCollision(ParticleData_t& particle, const ImVec2& vecScreenSize) const;

		// all our particles data
		std::vector<ParticleData_t> vecParticles;
	};

	inline bool bMainWindowOpened = false;
	inline int nCurrentMainTab = 0;
	inline bool bMainActive = false;
	inline int g_cur_tab_idx = 0;
	inline char szConfigFile[256U] = {};
	inline unsigned long long nSelectedConfig = ~1U;
	inline ParticleContext_t menuParticle = ParticleContext_t(MENU_MAX_BACKGROUND_PARTICLES);
	inline AnimationHandler_t animMenuDimBackground;
	inline float flDpiScale = 1.f;
}

namespace T
{
	/* @section: main */
	void Render(const char* szTabBar, const CTab* arrTabs, const unsigned long long nTabsCount, int* nCurrentTab, ImGuiTabBarFlags flags = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip);

	/* @section: tabs */
	void RageBot();
	void LegitBot();
	void Visuals();
	void Miscellaneous();
	void SkinsChanger();

	/* @section: values */
	// user-defined configuration filename in miscellaneous tab
	inline char szConfigFile[256U] = {};
	// current selected configuration in miscellaneous tab
	inline unsigned long long nSelectedConfig = ~1U;
	// current sub tab overlay in visuals tab
	inline int nCurrentOverlaySubtab = 0;
}
