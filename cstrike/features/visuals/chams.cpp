#include "chams.h"

// used: game's interfaces
#include "../../core/interfaces.h"
#include "../../sdk/interfaces/imaterialsystem.h"
#include "../../sdk/interfaces/igameresourceservice.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/iresourcesystem.h"
#include "../../core/sdk.h"
#include "../../sdk/entity.h"

// used: original call in hooked function
#include "../../core/hooks.h"

// used: cheat variables
#include "../../core/variables.h"

CStrongHandle<CMaterial2> F::VISUALS::CHAMS::CreateMaterial(const char* szMaterialName, const char szVmatBuffer[])
{
	CKeyValues3* pKeyValues3 = CKeyValues3::CreateMaterialResource();
	pKeyValues3->LoadFromBuffer(szVmatBuffer);

	CStrongHandle<CMaterial2> pCustomMaterial = {};
	MEM::fnCreateMaterial(nullptr, &pCustomMaterial, szMaterialName, pKeyValues3, 0, 1);

	return pCustomMaterial;
}

struct CustomMaterial_t
{
	CStrongHandle<CMaterial2> pMaterial;
	CStrongHandle<CMaterial2> pMaterialInvisible;
};

static CustomMaterial_t arrMaterials[VISUAL_MATERIAL_MAX];

bool F::VISUALS::CHAMS::Initialize()
{
	// check if we already initialized materials
	if (bInitialized)
		return bInitialized;

	arrMaterials[VISUAL_MATERIAL_PRIMARY_WHITE] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferWhiteVisible),
		.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferWhiteInvisible)
	};

	arrMaterials[VISUAL_MATERIAL_ILLUMINATE] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferIlluminateVisible),
		.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferIlluminateInvisible)
	};
	arrMaterials[VISUAL_MATERIAL_LATEX] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/glowproperty.vmat"), szVMatBufferIlatex),
		.pMaterialInvisible = CreateMaterial(("materials/dev/glowproperty.vmat"), szVMatBufferIlatexInvisible)
	};
	arrMaterials[VISUAL_MATERIAL_GLOW] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferGlowVisible),
		.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferGlowInvisible)
	};
	arrMaterials[VISUAL_MATERIAL_GLOW2] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/glowproperty.vmat"), szVMatBufferGlow2Visible),
		.pMaterialInvisible = CreateMaterial(("materials/dev/glowproperty.vmat"), szVMatBufferGlow2Visible)
	};
	arrMaterials[VISUAL_MATERIAL_METALIC] = CustomMaterial_t{
		.pMaterial = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferMetalic),
		.pMaterialInvisible = CreateMaterial(("materials/dev/primary_white.vmat"), szVMatBufferMetalicInvisible)
	};

	bInitialized = true;
	for (auto& [pMaterial, pMaterialInvisible] : arrMaterials)
	{
		if (pMaterial == nullptr || pMaterialInvisible == nullptr)
			bInitialized = false;
	}

	return bInitialized;
}

void F::VISUALS::CHAMS::Destroy()
{
	for (auto& [pVisible, pInvisible] : arrMaterials)
	{
		if (pVisible)
			I::ResourceHandleUtils->DeleteResource(pVisible.pBinding);

		if (pInvisible)
			I::ResourceHandleUtils->DeleteResource(pInvisible.pBinding);
	}
}

bool F::VISUALS::CHAMS::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	if (!bInitialized)
		return false;

	if (!C_GET(bool, Vars.bVisualChams))
		return false;

	if (arrMeshDraw == nullptr)
		return false;

	if (arrMeshDraw->pSceneAnimatableObject == nullptr)
		return false;

	CBaseHandle hOwner = arrMeshDraw->pSceneAnimatableObject->hOwner;

	auto pEntity = I::GameResourceService->pGameEntitySystem->Get<C_BaseEntity>(hOwner);
	if (pEntity == nullptr)
		return false;

	auto pClassInfo = pEntity->GetSchemaNoCrash();
	if (pClassInfo == nullptr)
		return false;

	auto HashName = pClassInfo->szName;

	if (CRT::StringCompare(HashName, CS_XOR("C_CSPlayerPawn")) == 0)
	{
		if (C_GET(bool, Vars.bEnemyChams))
		{
			auto pPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(hOwner);
			if (pPlayerPawn == nullptr)
				return false;

			bool bIsLocalPlayer = (pPlayerPawn == SDK::LocalPawn);


			// Include enemies in the chams rendering
			if (!bIsLocalPlayer && !pPlayerPawn->IsOtherEnemy(SDK::LocalPawn))
				return false;

			/*		if (C_GET(bool, Vars.bTeamChams))
				{

				}*/
			if (!pPlayerPawn->IsOtherEnemy(SDK::LocalPawn))
				return false;
			// Check if the entity is alive or if we should render chams on ragdolls
			if (pPlayerPawn->GetHealth() <= 0 && !C_GET(bool, Vars.bRagdollChams))
				return false;

			return OverrideMaterial(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
		}
	}
	if (CRT::StringCompare(HashName, CS_XOR("C_CSPlayerPawn")) == 0)
	{
		if (C_GET(bool, Vars.bLocalChams))
		{
			auto pPlayerPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(hOwner);
			if (pPlayerPawn == nullptr)
				return false;

			bool bIsLocalPlayer = (pPlayerPawn == SDK::LocalPawn);

			// Include enemies in the chams renderin
			//if (!bIsLocalPlayer)
			//	return false;

			//if (!pPlayerPawn->IsOtherEnemy(SDK::LocalPawn))
			//	return false;

			// Check if the entity is alive or if we should render chams on ragdolls
			if (pPlayerPawn->GetHealth() <= 0)
				return false;

			return OverrideMaterial(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
		}
	}

	if (CRT::StringCompare(HashName, CS_XOR("C_CSGOViewModel")) == 0)
	{
		if (C_GET(bool, Vars.bWeaponChams))
		{
			return OverrideMaterial(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
		}
	}
	if (CRT::StringCompare(HashName, CS_XOR("C_ViewmodelAttachmentModel")) == 0)
	{
		if (C_GET(bool, Vars.bViewModelChams))
		{
			return OverrideMaterial(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
		}
	}


	return false;
}

bool F::VISUALS::CHAMS::OverrideMaterial(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	const auto oDrawObject = H::hkDrawObject.GetOriginal();
	const CustomMaterial_t customMaterial = arrMaterials[C_GET(int, Vars.nVisualChamMaterial)];
	const CustomMaterial_t customLocalMaterial = arrMaterials[C_GET(int, Vars.nLocalChamMaterial)];
	const CustomMaterial_t customWeaponMaterial = arrMaterials[C_GET(int, Vars.nWeaponChamMaterial)];
	const CustomMaterial_t customViewModelMaterial = arrMaterials[C_GET(int, Vars.nViewModelChamMaterial)];

	auto pEntity = I::GameResourceService->pGameEntitySystem->Get<C_BaseEntity>(arrMeshDraw->pSceneAnimatableObject->hOwner);
	if (pEntity == nullptr)
		return false;

	auto pClassInfo = pEntity->GetSchemaNoCrash();
	if (pClassInfo == nullptr)
		return false;

	auto HashName = pClassInfo->szName;

	if (C_GET(bool, Vars.bEnemyChams))
	{
		if (CRT::StringCompare(HashName, CS_XOR("C_CSPlayerPawn")) == 0)
		{
			if (C_GET(bool, Vars.bVisualChamsIgnoreZ))
			{
				arrMeshDraw->pMaterial = customMaterial.pMaterialInvisible;
				arrMeshDraw->colValue = C_GET(Color_t, Vars.colVisualChamsIgnoreZ);
				oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
			}

			arrMeshDraw->pMaterial = customMaterial.pMaterial;
			arrMeshDraw->colValue = C_GET(Color_t, Vars.colVisualChams);
		}
	}
	if (C_GET(bool, Vars.bLocalChams))
	{
		if (CRT::StringCompare(HashName, CS_XOR("C_CSPlayerPawn")) == 0)
		{
			bool bIsLocalPlayer = (pEntity == SDK::LocalPawn);

			// Include enemies in the chams renderin
			//if (!bIsLocalPlayer)
			//	return false;

			//if (!pPlayerPawn->IsOtherEnemy(SDK::LocalPawn))
			//	return false;

			// Check if the entity is alive or if we should render chams on ragdolls
			if (pEntity->GetHealth() <= 0)
				return false;

			arrMeshDraw->pMaterial = customLocalMaterial.pMaterial;
			arrMeshDraw->colValue = C_GET(Color_t, Vars.colLocalChams);
		}
	}
	if (C_GET(bool, Vars.bWeaponChams))
	{
		if (CRT::StringCompare(HashName, CS_XOR("C_CSGOViewModel")) == 0)
		{
			arrMeshDraw->pMaterial = customWeaponMaterial.pMaterial;
			arrMeshDraw->colValue = C_GET(Color_t, Vars.colWeapon);
		}
	}

	if (C_GET(bool, Vars.bViewModelChams))
	{
		if (CRT::StringCompare(HashName, CS_XOR("C_ViewmodelAttachmentModel")) == 0)
		{
			arrMeshDraw->pMaterial = customViewModelMaterial.pMaterial;
			arrMeshDraw->colValue = C_GET(Color_t, Vars.colViewModel);
		}
	}

	oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	return true;
}
