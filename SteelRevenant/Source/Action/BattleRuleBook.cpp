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
				L"外周デッキと中央デッキ",
				L"近接敵が途切れず迫る大量撃破の導入",
				L"正面から押し寄せる群れを止めずに切り崩せ",
				L"90秒の制限時間で撃破を積み続けろ",
				90.0f, // stageStartTimeSec
				2.0f,  // resultDelaySec
				6,     // killRampStep
				20.0f, // timeRampStepSec
				12,    // openingAliveCount
				12,    // baseAliveCount
				16,    // maxAliveCount
				5,     // baseSpawnBatch
				0.18f, // baseSpawnIntervalSec
				0.08f, // minSpawnIntervalSec
				6,     // maxSpawnBatch
				4,     // totalWaveCount
				12,    // baseWaveEnemyCount
				2,     // waveEnemyIncreasePerWave
				0.0f,  // waveBreakSec
				10,    // baseWaveAliveCap
				2,     // waveAliveCapIncreasePerWave
				0,     // laserEnemyCount
				0,     // dashEnemyCount
				0,     // heavyEnemyCount
				0.0f   // cameraHeightOverride
			},
			StageRuleDefinition{
				L"防衛回廊",
				L"直線回廊と側面障害",
				L"近接敵に加えて射撃敵と高速敵が混ざる",
				L"接近群を捌きながら射線と突進を優先処理しろ",
				L"120秒の制限時間で高圧混成を処理し続けろ",
				120.0f,
				2.1f,
				5,
				30.0f,
				18,
				18,
				24,
				6,
				0.14f,
				0.06f,
				8,
				5,
				14,   // baseWaveEnemyCount
				3,    // waveEnemyIncreasePerWave
				0.0f, // waveBreakSec
				14,   // baseWaveAliveCap
				2,    // waveAliveCapIncreasePerWave
				4,    // laserEnemyCount
				3,    // dashEnemyCount
				0,    // heavyEnemyCount
				14.0f // cameraHeightOverride
			},
			StageRuleDefinition{
				L"中枢区画",
				L"中央コアと四隅障害",
				L"近接敵と射撃敵に耐久敵が加わる最終混成",
				L"射線を切りながら耐久敵と群れの優先順位を見極めろ",
				L"150秒の制限時間で混成圧を切り崩し続けろ",
				150.0f,
				2.3f,
				4,
				24.0f,
				24,
				24,
				30,
				7,
				0.10f,
				0.05f,
				9,
				6,
				16,   // baseWaveEnemyCount
				4,    // waveEnemyIncreasePerWave
				0.0f, // waveBreakSec
				20,   // baseWaveAliveCap
				2,    // waveAliveCapIncreasePerWave
				5,    // laserEnemyCount
				0,    // dashEnemyCount
				4,    // heavyEnemyCount
				16.0f // cameraHeightOverride
			}
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
