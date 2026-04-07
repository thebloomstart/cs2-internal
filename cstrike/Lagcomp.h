#pragma once
#include "sdk/datatypes/vector.h"
#include "sdk/entity.h"
#include "core/sdk.h"
#include <deque>
#include <optional>	
#include <unordered_map>	

namespace Lagcomp {
	struct LagRecordBackup {
	public:
		// Did player die this frame
		int						m_fFlags;

		// Player position, orientation, bbox and bone data
		Vector_t					m_vecOrigin;
		QAngle_t					m_vecAngles;
		Vector_t					m_vecMins;
		Vector_t					m_vecMaxs;
		BoneData_t    		        m_boneDataCache[128];
		BoneData_t					m_boneData[128];
		float					    GetSimulationTime;
	};

	struct LagRecord
	{
	public:
		// Official valve code
		LagRecord()
		{
			Clear();
		}

		LagRecord(const LagRecord& src)
		{
			m_fFlags = src.m_fFlags;
			m_vecOrigin = src.m_vecOrigin;
			m_vecAngles = src.m_vecAngles;
			m_vecMins = src.m_vecMins;
			m_vecMaxs = src.m_vecMaxs;
			GetSimulationTime = src.GetSimulationTime;
			memcpy(m_boneData, src.m_boneData, sizeof(BoneData_t) * 128);
		}

		void Clear()
		{
			m_fFlags = 0;
			m_vecOrigin = Vector_t{};
			m_vecAngles = QAngle_t{};
			m_vecMins = Vector_t{};
			m_vecMaxs = Vector_t{};
			GetSimulationTime = -1;
			memset(m_boneData, 0, sizeof(BoneData_t) * 128);
		}

		// Not so official valve code
		void Setup(C_CSPlayerPawn* pawn)
		{
			m_fFlags = pawn->GetFlags();
			m_vecOrigin = pawn->GetGameSceneNode()->GetAbsOrigin();
			m_vecAngles = pawn->GetGameSceneNode()->GetAngleRotation();
			m_vecMins = pawn->GetCollision()->GetMins();
			m_vecMaxs = pawn->GetCollision()->GetMaxs();
			GetSimulationTime = pawn->GetSimulationTime();
			memcpy(m_boneData, pawn->GetGameSceneNode()->GetSkeletonInstance()->m_modelState().m_pBones, sizeof(BoneData_t) * 128);
		}

		LagRecordBackup Apply(C_CSPlayerPawn* pawn) {
			// Create backup of current pawn's data
			LagRecordBackup backup{};
			backup.m_fFlags = pawn->GetFlags();
			backup.m_vecOrigin = pawn->GetGameSceneNode()->GetAbsOrigin();
			backup.m_vecAngles = pawn->GetGameSceneNode()->GetAngleRotation();
			backup.m_vecMins = pawn->GetCollision()->GetMins();
			backup.m_vecMaxs = pawn->GetCollision()->GetMaxs();
			backup.GetSimulationTime = pawn->GetSimulationTime();
			memcpy(backup.m_boneData, pawn->GetGameSceneNode()->GetSkeletonInstance()->m_modelState().m_pBones, sizeof(BoneData_t) * 128);

			// Cool, now we set the pawn's data to our lagcomp data
			pawn->GetFlags() = m_fFlags;
			pawn->GetGameSceneNode()->GetAbsOrigin() = m_vecOrigin;
			pawn->GetGameSceneNode()->GetAngleRotation() = m_vecAngles;
			pawn->GetCollision()->GetMins() = m_vecMins;
			pawn->GetCollision()->GetMaxs() = m_vecMaxs;
			pawn->GetSimulationTime() = GetSimulationTime;
			memcpy(pawn->GetGameSceneNode()->GetSkeletonInstance()->m_modelState().m_pBones, m_boneData, sizeof(BoneData_t) * 128);

			// Return our backup so that we can restore the pawn's original data
			return backup;
		}

		void Restore(C_CSPlayerPawn* pawn, LagRecordBackup backup) {
			// Restore the pawn
			pawn->GetFlags() = backup.m_fFlags;
			pawn->GetGameSceneNode()->GetAbsOrigin() = backup.m_vecOrigin;
			pawn->GetGameSceneNode()->GetAngleRotation() = backup.m_vecAngles;
			pawn->GetCollision()->GetMins() = backup.m_vecMins;
			pawn->GetCollision()->GetMaxs() = backup.m_vecMaxs;
			pawn->GetSimulationTime() = backup.GetSimulationTime;
			memcpy(pawn->GetGameSceneNode()->GetSkeletonInstance()->pBoneCache, backup.m_boneDataCache, sizeof(BoneData_t) * 128);
			memcpy(pawn->GetGameSceneNode()->GetSkeletonInstance()->m_modelState().m_pBones, backup.m_boneData, sizeof(BoneData_t) * 128);
		}

		bool IsValidRecord();

		// Did player die this frame
		int						m_fFlags;

		// Player position, orientation, bbox and bone data
		Vector_t					m_vecOrigin;
		QAngle_t					m_vecAngles;
		Vector_t					m_vecMins;
		Vector_t					m_vecMaxs;
		BoneData_t					m_boneData[128];

		float					GetSimulationTime;
	};

	struct LagEntity {
	public:
		LagEntity() {
			m_pEntity = nullptr;
			m_arrLagRecords = {};
		}

		LagEntity(C_CSPlayerPawn* entity) {
			m_pEntity = entity;
			m_arrLagRecords = {};
		}

		void DeleteIfDead() {
			if (!SDK::LocalPawn) {
				m_arrLagRecords.clear();
				return;
			}

			if (m_pEntity->GetHealth() < 1 || m_pEntity->GetTeam() == SDK::LocalPawn->GetTeam())
				m_arrLagRecords.clear();
		}

		void CreateRecord() {
			if (!SDK::LocalPawn)
				return;

			// Only run lagcomp for alive entities
			if (m_pEntity->GetHealth() < 1 || m_pEntity->GetTeam() == SDK::LocalPawn->GetTeam())
				return;

			// We cant backtrack more than 12 ticks
			if (m_arrLagRecords.size() > 12)
				m_arrLagRecords.pop_back();

			m_pEntity->GetGameSceneNode()->GetSkeletonInstance()->calc_world_space_bones(0xfffffu);

			// Create a new record and set it up
			LagRecord& record = m_arrLagRecords.emplace_front();
			record.Setup(m_pEntity);
		}

		// Entity and lag records
		C_CSPlayerPawn* m_pEntity;
		std::deque<LagRecord> m_arrLagRecords;
	};

	class CLagCompensationSystem {
	public:
		std::vector<LagEntity> m_arrEntities{};

		CLagCompensationSystem() {
			m_arrEntities = {};
		}

		CLagCompensationSystem(CLagCompensationSystem& src) {
			m_arrEntities = src.m_arrEntities;
		}

		void Run();

		void AddEntity(C_CSPlayerPawn* entity) {
			if (GetLagEntity(entity).has_value())
				return;

			m_arrEntities.push_back(LagEntity(entity));
			L_PRINT(LOG_INFO) << "[LAGCOMP]: Added entity.";
		}

		void RemoveEntity(C_CSPlayerPawn* entity) {
			if (!GetLagEntity(entity).has_value())
				return;
			// Remove the entity from m_arrEntities
			try
			{
				size_t index = MAXSIZE_T;
				for (size_t i = 0; i < m_arrEntities.size(); i++)
				{
					if (m_arrEntities[i].m_pEntity->GetRefEHandle().GetEntryIndex() == entity->GetRefEHandle().GetEntryIndex()) {
						index = i;
						break;
					}
				}
				if (index == MAXSIZE_T)
					return;

				m_arrEntities.erase(m_arrEntities.begin() + index);
				L_PRINT(LOG_INFO) << "[LAGCOMP]: Removed entity.";

			}
			catch (...)
			{
				L_PRINT(LOG_INFO) << "[LAGCOMP]: CRASH";
			}
		}

		std::optional<LagEntity> GetLagEntity(C_CSPlayerPawn* entity) {
			if (!entity)
				return std::nullopt;
			if (&m_arrEntities == nullptr)
				return std::nullopt;

			size_t index = MAXSIZE_T;
			for (size_t i = 0; i < m_arrEntities.size(); i++)
			{
				if (m_arrEntities[i].m_pEntity->GetRefEHandle().GetEntryIndex() == entity->GetRefEHandle().GetEntryIndex()) {
					index = i;
					break;
				}
			}
			if (index == MAXSIZE_T)
				return std::nullopt;

			return m_arrEntities[index];
		}
	};
	inline std::unique_ptr<CLagCompensationSystem> g_LagCompensation = std::make_unique<CLagCompensationSystem>();

	void setup();
}