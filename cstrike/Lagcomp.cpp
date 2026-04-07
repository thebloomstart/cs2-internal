#include "Lagcomp.h"
#include "features/visuals/chams.h"
#include "utilities/draw.h"
#include <algorithm>
#include "AimBot.h"
#include "sdk/interfaces/iengineclient.h"
#include "sdk/interfaces/inetworkclientservice.h"

namespace Lagcomp {
	//bool LagRecord::IsValidRecord() {
	//	const auto NetworkChannel = interfaces::Engine->GetNetChannelInfo(0);
	//	if (!NetworkChannel || !interfaces::NetworkClientService->GetNetworkGameClient())
	//		return false;

	//	const float flMaxUnlag = std::min(I::Cvar->Find(FNV1A::HashConst("sv_maxunlag"))->value.fl, 0.2f);
	//	const float flLatency = NetworkChannel->GetLatency(FLOW_OUTGOING) + NetworkChannel->GetLatency(FLOW_INCOMING);
	//	const float flCorrectedValue = std::clamp(flLatency + I::NetworkClientService->GetNetworkGameClient()->GetClientInterpAmount(), 0.0f, flMaxUnlag);
	//	float flMaxDelta = std::min(flMaxUnlag - flCorrectedValue, 0.2f);
	//	const float flDelta = flMaxDelta - I::GlobalVars->flCurTime;

	//	return GetSimulationTime > fabsf(flDelta);
	//}

	void CLagCompensationSystem::Run() {
		std::vector<LagEntity> valid{};
		for (size_t i = 0; i < m_arrEntities.size(); i++)
		{
			if (!m_arrEntities[i].m_pEntity)
				continue;

			m_arrEntities[i].DeleteIfDead();
			m_arrEntities[i].CreateRecord();
			valid.push_back(m_arrEntities[i]);
		}
		m_arrEntities = valid;
	}
	//void setup() {
	//	L_PRINT(LOG_INFO) << "[LAGCOMP]: Initializing...";
	//	g_LagCompensation = std::make_unique<CLagCompensationSystem>();
	//	L_PRINT(LOG_INFO) << "[LAGCOMP]: Ready.";
	//}
	bool LagRecord::IsValidRecord() {

		static CConVar* sv_maxunlag = I::Cvar->Find(FNV1A::HashConst("sv_maxunlag"));
		float max_unlag = sv_maxunlag ? sv_maxunlag->value.fl : 0.2f;

		auto engine = I::Engine;
		auto netclient = I::NetworkClientService;

		float lag_compensation_factor = (engine->GetNetChannelInfo()->get_network_latency() * 0.5f) + engine->GetNetChannelInfo()->get_engine_latency();

		float lastValidTime = I::GlobalVars->flCurTime - max_unlag + lag_compensation_factor;

		lastValidTime += TICK_INTERVAL;

		return lastValidTime < GetSimulationTime;
	}
}