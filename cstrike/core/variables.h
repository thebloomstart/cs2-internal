#pragma once

#include "config.h"
#include "../Rage.h"

#pragma region variables_combo_entries
using VisualOverlayBox_t = int;

enum EVisualOverlayBox : VisualOverlayBox_t
{
	VISUAL_OVERLAY_BOX_NONE = 0,
	VISUAL_OVERLAY_BOX_FULL,
	VISUAL_OVERLAY_BOX_CORNERS,
	VISUAL_OVERLAY_BOX_MAX
};

using VisualChamMaterial_t = int;

enum EVisualsChamMaterials : VisualChamMaterial_t
{
	VISUAL_MATERIAL_PRIMARY_WHITE = 0,
	VISUAL_MATERIAL_ILLUMINATE,
	VISUAL_MATERIAL_LATEX,
	VISUAL_MATERIAL_GLOW,
	VISUAL_MATERIAL_GLOW2,
	VISUAL_MATERIAL_METALIC,
	VISUAL_MATERIAL_MAX
};

enum EChamsList : int
{
	NONE_CHAMS,
	ENEMY_CHAMS,
	LOCAL_CHAMS,
	ALLIES_CHAMS,
	CHAMSLIST = NONE_CHAMS
};

enum EChamsListEntryLocal : int
{
	PLAYER_CHAMS_LOCAL,
	WEAPON_CHAMS_LOCAL,
	ATTACHMENT_CHAMS_LOCAL,
	HANDS_CHAMS_LOCAL,
	CHAMSLISTENTRYLOCAL = PLAYER_CHAMS_LOCAL
};

enum EChamsListEntry : int
{
	PLAYER_CHAMS,
	RAGDOLLS_CHAMS,
	WEAPON_CHAMS,
	CHAMSLISTENTRY = PLAYER_CHAMS
};

using Type = int;

enum mindamage_type : Type {
	strict,
	adaptive_scan
};

enum stop_mode : Type {
	full_stop,
	dynamic_stop
};



enum JitterType : Type
{
	JITTER_TYPE_NONE = 0,
	JITTER_TYPE_CENTER,
	JITTER_TYPE_3_WAY,
	JITTER_TYPE_RANDOM,
	JITTER_TYPE_SPIN,
};

enum BoxType : Type
{
	DefaultBox,
	FilledBox,
	CornerBox
};

using MiscDpiScale_t = int;

enum EMiscDpiScale : MiscDpiScale_t
{
	MISC_DPISCALE_DEFAULT = 0,
	MISC_DPISCALE_125,
	MISC_DPISCALE_150,
	MISC_DPISCALE_175,
	MISC_DPISCALE_200,
	MISC_DPISCALE_MAX
};

#pragma endregion

#pragma region variables_multicombo_entries
using MenuAddition_t = unsigned int;

enum EMenuAddition : MenuAddition_t
{
	MENU_ADDITION_NONE = 0U,
	MENU_ADDITION_DIM_BACKGROUND = 1 << 0,
	MENU_ADDITION_BACKGROUND_PARTICLE = 1 << 1,
	MENU_ADDITION_GLOW = 1 << 2,
	MENU_ADDITION_ALL = MENU_ADDITION_DIM_BACKGROUND | MENU_ADDITION_BACKGROUND_PARTICLE | MENU_ADDITION_GLOW
};

enum ESelectedRemovals : unsigned int
{
	NONE = 0,
	SMOKE = 1,
	LEGS = 2,
	TEAM_INTRO = 4,
	FOG = 8,
	/*SKY = 16,*/
	RECOIL = 16,
	FLASH = 32,
	REMOVALS = NONE
};

enum EESPFlags : unsigned int
{
	FLAGS_NONE = 0U,
	FLAGS_KIT = 1 << 0,
	FLAGS_ARMOR = 1 << 1,
	FLAGS_DEFUSER = 1 << 2,
	FLAGS_SCOPED = 1 << 3
};

#pragma endregion

struct Variables_t
{
#pragma region variables_visuals
	C_ADD_VARIABLE(bool, bVisualOverlay, false);

	C_ADD_VARIABLE(FrameOverlayVar_t, overlayBox, FrameOverlayVar_t(false));
	C_ADD_VARIABLE(bool, bWeapon, false);
	C_ADD_VARIABLE(Color_t, colName, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colBox, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colSkeleton, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colWeaponName, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colWeaponIcon, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colHealthBar, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colAmmobar, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colLowHealthBar, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colFullHealthBar, Color_t(255, 255, 255));
	C_ADD_VARIABLE(bool, bColorEditor, false);
	C_ADD_VARIABLE(bool, bHealthBar, false);
	C_ADD_VARIABLE(bool, bNameEsp, false);
	C_ADD_VARIABLE(bool, bAmmoBar, false);
	C_ADD_VARIABLE(bool, bWeaponName, false);
	C_ADD_VARIABLE(bool, bWeaponIcon, false);
	C_ADD_VARIABLE(unsigned int, pEspFlags, FLAGS_NONE);
	C_ADD_VARIABLE(bool, bTeamCheck, false);
	C_ADD_VARIABLE(int, BoxType, DefaultBox);
	C_ADD_VARIABLE(bool, bSkeleton, false);
	C_ADD_VARIABLE(bool, bDrawCube, false);
	C_ADD_VARIABLE(bool, bGrenadeVisuals, false);
	C_ADD_VARIABLE(bool, bBulletTrace, false);
	C_ADD_VARIABLE(bool, bOof, false);

	C_ADD_VARIABLE(bool, bVisualChams, false);
	C_ADD_VARIABLE(bool, bWeaponChams, false);
	C_ADD_VARIABLE(bool, bEnemyChams, false);
	C_ADD_VARIABLE(bool, bRagdollChams, false);
	C_ADD_VARIABLE(bool, bViewModelChams, false);
	C_ADD_VARIABLE(bool, bOtherVisuals, false);
	C_ADD_VARIABLE(bool, bLocalChams, false);
	C_ADD_VARIABLE(bool, bHitLogs, false);
	C_ADD_VARIABLE(bool, bHitmarker, false);
	C_ADD_VARIABLE(bool, bHiteffect, false);

	C_ADD_VARIABLE(Color_t, colSunClouds, Color_t(255, 255, 255))
	C_ADD_VARIABLE(Color_t, colClouds, Color_t(255, 255, 255))

	C_ADD_VARIABLE(bool, bWorldModulation, false);
	C_ADD_VARIABLE(Color_t, colWorld, Color_t(255, 255, 255));
	C_ADD_VARIABLE(float, flAspectRatio, 1.f);

	C_ADD_VARIABLE(bool, bChams, false);

	C_ADD_VARIABLE(bool, bGlow, false);
	C_ADD_VARIABLE(Color_t, colGlowColor, Color_t(0, 0, 0, 1));

	C_ADD_VARIABLE(bool, bGlowWeapon, false);
	C_ADD_VARIABLE(Color_t, colGlowColorWeapon, Color_t(0, 0, 0, 1));

	C_ADD_VARIABLE(Color_t, colSky, Color_t(255, 255, 255));
	C_ADD_VARIABLE(Color_t, colLightning, Color_t(255, 255, 255));
	C_ADD_VARIABLE(float, flLightingIntensity, 1.f);
	C_ADD_VARIABLE(float, flExposure, 90.f);

	C_ADD_VARIABLE(int, nVisualChamMaterial, VISUAL_MATERIAL_PRIMARY_WHITE);
	C_ADD_VARIABLE(int, nWeaponChamMaterial, VISUAL_MATERIAL_PRIMARY_WHITE);
	C_ADD_VARIABLE(int, nViewModelChamMaterial, VISUAL_MATERIAL_PRIMARY_WHITE);
	C_ADD_VARIABLE(int, nLocalChamMaterial, VISUAL_MATERIAL_PRIMARY_WHITE);

	C_ADD_VARIABLE(bool, bVisualChamsIgnoreZ, false); // invisible chams
	C_ADD_VARIABLE(Color_t, colVisualChams, Color_t(0, 255, 0));
	C_ADD_VARIABLE(Color_t, colVisualChamsIgnoreZ, Color_t(255, 0, 0));
	C_ADD_VARIABLE(Color_t, colWeapon, Color_t(0, 255, 0));
	C_ADD_VARIABLE(Color_t, colViewModel, Color_t(0, 255, 0));
	C_ADD_VARIABLE(Color_t, colBulletTracer, Color_t(0, 255, 0));
	C_ADD_VARIABLE(Color_t, colLocalChams, Color_t(0, 255, 0));
	C_ADD_VARIABLE(Color_t, color, Color_t(112, 110, 215, 255));

#pragma endregion
#pragma region variables_AntiAim
	C_ADD_VARIABLE(bool, bAntiAim, false);
	C_ADD_VARIABLE(int, iJitterType, JITTER_TYPE_NONE);
	C_ADD_VARIABLE(float, flJitter, 1);
	C_ADD_VARIABLE(float, flYawOffset, 1);
	C_ADD_VARIABLE(bool, bPitch, false);
	C_ADD_VARIABLE(bool, bAtTarget, false);
	C_ADD_VARIABLE(int, iPitchType, 0);
	C_ADD_VARIABLE(int, iYaw, 0);
	C_ADD_VARIABLE(int, iLeftKey, 0);
	C_ADD_VARIABLE(int, iLeftKeyBind, 0);
	C_ADD_VARIABLE(int, iRightKey, 0);
	C_ADD_VARIABLE(int, iRightKeyBind, 0);
#pragma endregion

#pragma region variables_misc
	C_ADD_VARIABLE(bool, bHotkeys, false);
	C_ADD_VARIABLE(bool, bAntiUntrusted, true);
	C_ADD_VARIABLE(bool, bHealthBoost, false);
	C_ADD_VARIABLE(bool, bHitSound, false);
	C_ADD_VARIABLE(float, flVolumeHit, 1);
	C_ADD_VARIABLE(bool, bJumpBug, false);
	C_ADD_VARIABLE(bool, bWatermark1, false);

	C_ADD_VARIABLE(bool, bAutoBHop, false);
	C_ADD_VARIABLE(int, iDistanceThird, 1);
	C_ADD_VARIABLE(int, iThirdKey, 0);
	C_ADD_VARIABLE(int, iThirdKeyBind, 0);
	C_ADD_VARIABLE(KeyBind_t, iThirdPersonKey, 0);
	C_ADD_VARIABLE(int, iThirdPersonKeyBind, 0);
	C_ADD_VARIABLE(float, flStrafeSmoothing, 0);
	C_ADD_VARIABLE(unsigned int, bRemovalsNames, REMOVALS);

	C_ADD_VARIABLE(int, nAutoBHopChance, 100);
	C_ADD_VARIABLE(bool, bNoScope, false);
	C_ADD_VARIABLE(bool, bSpectatorList, false);
	C_ADD_VARIABLE(bool, bKeybindsList, false);
	C_ADD_VARIABLE(bool, bBombTimer, false);
	C_ADD_VARIABLE(bool, bFov, false);
	C_ADD_VARIABLE(float, flFov, 90);
	C_ADD_VARIABLE(float, flViewModelFov, 90);
	C_ADD_VARIABLE(float, flViX, 0);
	C_ADD_VARIABLE(float, flViY, 0);
	C_ADD_VARIABLE(float, flViZ, 0);

	C_ADD_VARIABLE(bool, bSubtickStrafe, false);
	C_ADD_VARIABLE(bool, bAutoStrafe, false);
#pragma endregion
#pragma region Settings
	C_ADD_VARIABLE(int, iCrossGap, 5);
	C_ADD_VARIABLE(int, iCrossthickness, 1);
	C_ADD_VARIABLE(int, iCrosslength, 10);
#pragma endregion

#pragma region ragebot
	C_ADD_VARIABLE(int, rage_minimum_damage, 0);
	C_ADD_VARIABLE(int, rage_minimum_hitchance, 0);
	C_ADD_VARIABLE(int, rage_weapon_selection, 0);
	C_ADD_VARIABLE(int, iDuckPeekKey, 0);
	C_ADD_VARIABLE(int, iDuckPeekKeyBind, 0);
	C_ADD_VARIABLE(bool, rage_enable, false);
	C_ADD_VARIABLE(bool, bSilent, false);
	C_ADD_VARIABLE(bool, bNoSpread, false);
	C_ADD_VARIABLE(int, bKeyNoSpread, 0);
	C_ADD_VARIABLE(int, bKeyBindNoSpread, 0);
	C_ADD_VARIABLE(int, bKeyPredict, 0);
	C_ADD_VARIABLE(int, bKeyBindPredict, 0);
	C_ADD_VARIABLE(int, iMaxBackTrackTicks, 0);
	C_ADD_VARIABLE(int, iMinBackTrackTicks, 0);
	C_ADD_VARIABLE(int, iSelectTarget, 0);
	C_ADD_VARIABLE(bool, bPerfectSilent, false);
	C_ADD_VARIABLE(bool, bAutoScope, false);
	C_ADD_VARIABLE(bool, bAutoStop, false);
	C_ADD_VARIABLE(int, iHitbox, 0);
	C_ADD_VARIABLE(int, iScanType, strict);
	C_ADD_VARIABLE(int, iAutoStopMode, dynamic_stop);
	C_ADD_VARIABLE(bool, bPredictLethal, 0);
	C_ADD_VARIABLE(int, bKeyMinDamage, 0);
	C_ADD_VARIABLE(int, bKeyBindMinDamage, 0);
	C_ADD_VARIABLE(int, bKeyHitchance, 0);
	C_ADD_VARIABLE(int, bKeyBindHitchance, 0);
	C_ADD_VARIABLE(int, iLethalModifer, 1);
	C_ADD_VARIABLE(bool, btop_left, false);
	C_ADD_VARIABLE(bool, bbottom_right, false);
	C_ADD_VARIABLE(bool, bmiddle, false);

	C_ADD_VARIABLE(int, iHitChance, 0);
	C_ADD_VARIABLE(bool, bAutoFire, false);
	;
	C_ADD_VARIABLE(unsigned int, AutoPeek, VK_HOME);

#pragma endregion

#pragma region variables_menu
	C_ADD_VARIABLE(unsigned int, nMenuKeys, VK_INSERT);
	C_ADD_VARIABLE(unsigned int, nPanicKey, VK_END);
	C_ADD_VARIABLE(int, nDpiScale, 0);

	/*
	 * color navigation:
	 * [definition N][purpose]
	 * 1. primitive:
	 * - primtv 0 (text)
	 * - primtv 1 (background)
	 * - primtv 2 (disabled)
	 * - primtv 3 (control bg)
	 * - primtv 4 (border)
	 * - primtv 5 (hover)
	 *
	 * 2. accents:
	 * - accent 0 (main)
	 * - accent 1 (dark)
	 * - accent 2 (darker)
	 */
	C_ADD_VARIABLE(unsigned int, bMenuAdditional, MENU_ADDITION_ALL);
	C_ADD_VARIABLE(float, flAnimationSpeed, 1.f);

	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv0, ColorPickerVar_t(255, 255, 255)); // (text)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv1, ColorPickerVar_t(50, 55, 70)); // (background)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv2, ColorPickerVar_t(190, 190, 190)); // (disabled)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv3, ColorPickerVar_t(20, 20, 30)); // (control bg)
	C_ADD_VARIABLE(ColorPickerVar_t, colPrimtv4, ColorPickerVar_t(0, 0, 0)); // (border)

	C_ADD_VARIABLE(ColorPickerVar_t, colAccent0, ColorPickerVar_t(85, 90, 160)); // (main)
	C_ADD_VARIABLE(ColorPickerVar_t, colAccent1, ColorPickerVar_t(100, 105, 175)); // (dark)
	C_ADD_VARIABLE(ColorPickerVar_t, colAccent2, ColorPickerVar_t(115, 120, 190)); // (darker)
#pragma endregion

#pragma region variables_InventoryChanger
	C_ADD_VARIABLE(bool, bUnlockINVENTORY, false);
#pragma endregion

#pragma region variables_legitbot
	C_ADD_VARIABLE(bool, bLegitbot, false);
	C_ADD_VARIABLE(float, flAimRange, 10.0f);
	C_ADD_VARIABLE(float, flSmoothing, 10.0f);
	C_ADD_VARIABLE(bool, bLegitbotAlwaysOn, false);
	C_ADD_VARIABLE(int, nLegitbotActivationKey, VK_HOME);
	C_ADD_VARIABLE(int, nLegitbotActivationKeysoso, VK_HOME);
	C_ADD_VARIABLE(bool, allskins, false);
	C_ADD_VARIABLE(bool, AimVof, false);
	C_ADD_VARIABLE(Color_t, colVof, Color_t(255, 255, 255));
#pragma endregion
};

inline Variables_t Vars = {};
