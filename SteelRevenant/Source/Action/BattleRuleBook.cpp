#include "BattleRuleBook.h"

#include "../Utility/SimpleMathEx.h"

namespace Action
{
	// 既定の 3 ステージ定義を構築する。
	BattleRuleBook::BattleRuleBook()
		: m_stageRules
		{
			StageRuleDefinition{
				L"外縁区画",
				L"外周デッキ + 中央デッキ",
				L"近接主体 / 時間圧: 低",
				L"短期ウェーブ戦で前圧と側面圧を裁け",
				L"Wave 4 まで制圧しつつ Relay 1 を確保せよ",
				90.0f,
				2.0f,
				1,
				4.0f,
				2.0f,
				6,
				20.0f,
				12,
				12,
				16,
				2,
				0.85f,
				0.35f,
				2,
				4,
				12,
				4,
				2.8f,
				4,
				2 },
			StageRuleDefinition{
				L"防衛回廊",
				L"直線回廊 + 側面障害",
				L"近接主体 / 時間圧: 中",
				L"中盤ウェーブで正面維持とフランク処理を見せろ",
				L"Wave 5 の制圧戦  Relay 2 / Beacon +7 sec",
				180.0f,
				2.1f,
				2,
				11.0f,
				7.0f,
				5,
				30.0f,
				14,
				14,
				20,
				3,
				0.72f,
				0.30f,
				3,
				5,
				7,
				3,
				3.2f,
				3,
				1 },
			StageRuleDefinition{
				L"中央区中枢",
				L"中央コア + 四隅障害",
				L"近接主体 / 時間圧: 高",
				L"高圧ウェーブで包囲と危険地帯の両立を成立させろ",
				L"Wave 6 耐久戦  Relay 3 / Beacon +12 sec",
				300.0f,
				2.3f,
				3,
				18.0f,
				12.0f,
				4,
				24.0f,
				16,
				16,
				24,
				4,
				0.58f,
				0.24f,
				4,
				6,
				9,
				4,
				3.6f,
				4,
				1 }
		}
		, m_activeStageIndex(1)
	{
	}

	// 解放専用。追加処理は持たない。
	BattleRuleBook::~BattleRuleBook() = default;

	// 操作中のステージ定義を切り替える。
	void BattleRuleBook::SetActiveStage(int stageIndex)
	{
		m_activeStageIndex = Utility::MathEx::Clamp(stageIndex, 1, static_cast<int>(m_stageRules.size()));
	}

	// 現在選択中のステージ番号を返す。
	int BattleRuleBook::GetActiveStageIndex() const
	{
		return m_activeStageIndex;
	}

	// 現在有効なステージ定義を返す。
	const StageRuleDefinition& BattleRuleBook::GetActiveRule() const
	{
		return m_stageRules[static_cast<size_t>(Utility::MathEx::Clamp(m_activeStageIndex, 1, static_cast<int>(m_stageRules.size())) - 1)];
	}

	// 任意ステージ番号の定義を返す。
	const StageRuleDefinition& BattleRuleBook::GetStageRule(int stageIndex) const
	{
		const int index = Utility::MathEx::Clamp(stageIndex, 1, static_cast<int>(m_stageRules.size())) - 1;
		return m_stageRules[static_cast<size_t>(index)];
	}

	float BattleRuleBook::GetStageStartTimeSec() const { return GetActiveRule().stageStartTimeSec; }
	float BattleRuleBook::GetResultDelaySec() const { return GetActiveRule().resultDelaySec; }
	int BattleRuleBook::GetRequiredRelayCount() const { return GetActiveRule().requiredRelayCount; }
	float BattleRuleBook::GetRelayTimeBonusSec() const { return GetActiveRule().relayTimeBonusSec; }
	float BattleRuleBook::GetBeaconTimeBonusSec() const { return GetActiveRule().beaconTimeBonusSec; }
	int BattleRuleBook::GetKillRampStep() const { return GetActiveRule().killRampStep; }
	float BattleRuleBook::GetTimeRampStepSec() const { return GetActiveRule().timeRampStepSec; }
	int BattleRuleBook::GetOpeningAliveCount() const { return GetActiveRule().openingAliveCount; }
	int BattleRuleBook::GetBaseAliveCount() const { return GetActiveRule().baseAliveCount; }
	int BattleRuleBook::GetMaxAliveCount() const { return GetActiveRule().maxAliveCount; }
	int BattleRuleBook::GetBaseSpawnBatch() const { return GetActiveRule().baseSpawnBatch; }
	float BattleRuleBook::GetBaseSpawnIntervalSec() const { return GetActiveRule().baseSpawnIntervalSec; }
	float BattleRuleBook::GetMinSpawnIntervalSec() const { return GetActiveRule().minSpawnIntervalSec; }
	int BattleRuleBook::GetMaxSpawnBatch() const { return GetActiveRule().maxSpawnBatch; }
	int BattleRuleBook::GetTotalWaveCount() const { return GetActiveRule().totalWaveCount; }
	int BattleRuleBook::GetBaseWaveEnemyCount() const { return GetActiveRule().baseWaveEnemyCount; }
	int BattleRuleBook::GetWaveEnemyIncreasePerWave() const { return GetActiveRule().waveEnemyIncreasePerWave; }
	float BattleRuleBook::GetWaveBreakSec() const { return GetActiveRule().waveBreakSec; }
	int BattleRuleBook::GetBaseWaveAliveCap() const { return GetActiveRule().baseWaveAliveCap; }
	int BattleRuleBook::GetWaveAliveCapIncreasePerWave() const { return GetActiveRule().waveAliveCapIncreasePerWave; }
}
