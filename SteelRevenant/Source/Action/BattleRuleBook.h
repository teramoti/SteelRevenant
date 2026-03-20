#pragma once

#include <array>

#include "../Utility/SingletonBase.h"

namespace Action
{
	// ステージごとの進行ルール定義。
	struct StageRuleDefinition
	{
		const wchar_t* missionName;
		const wchar_t* terrainLabel;
		const wchar_t* enemyTrend;
		const wchar_t* tacticalMemo;
		const wchar_t* missionSummary;
		float stageStartTimeSec;
		float resultDelaySec;
		int requiredRelayCount;
		float relayTimeBonusSec;
		float beaconTimeBonusSec;
		int killRampStep;
		float timeRampStepSec;
		int openingAliveCount;
		int baseAliveCount;
		int maxAliveCount;
		int baseSpawnBatch;
		float baseSpawnIntervalSec;
		float minSpawnIntervalSec;
		int maxSpawnBatch;
		int totalWaveCount;
		int baseWaveEnemyCount;
		int waveEnemyIncreasePerWave;
		float waveBreakSec;
		int baseWaveAliveCap;
		int waveAliveCapIncreasePerWave;
	};

	// ステージ定義の参照 API を提供するルールブック。
	class BattleRuleBook : public Utility::SingletonBase<BattleRuleBook>
	{
	public:
		friend class Utility::SingletonBase<BattleRuleBook>;

	private:
		BattleRuleBook();
		~BattleRuleBook();

	public:
		void SetActiveStage(int stageIndex);
		int GetActiveStageIndex() const;
		const StageRuleDefinition& GetActiveRule() const;
		const StageRuleDefinition& GetStageRule(int stageIndex) const;

		float GetStageStartTimeSec() const;
		float GetResultDelaySec() const;
		int GetRequiredRelayCount() const;
		float GetRelayTimeBonusSec() const;
		float GetBeaconTimeBonusSec() const;
		int GetKillRampStep() const;
		float GetTimeRampStepSec() const;
		int GetOpeningAliveCount() const;
		int GetBaseAliveCount() const;
		int GetMaxAliveCount() const;
		int GetBaseSpawnBatch() const;
		float GetBaseSpawnIntervalSec() const;
		float GetMinSpawnIntervalSec() const;
		int GetMaxSpawnBatch() const;
		int GetTotalWaveCount() const;
		int GetBaseWaveEnemyCount() const;
		int GetWaveEnemyIncreasePerWave() const;
		float GetWaveBreakSec() const;
		int GetBaseWaveAliveCap() const;
		int GetWaveAliveCapIncreasePerWave() const;

	private:
		std::array<StageRuleDefinition, 3> m_stageRules;
		int m_activeStageIndex;
	};
}
