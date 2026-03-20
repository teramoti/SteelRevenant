//------------------------//------------------------
// Contents(処理内容) サバイバル進行と敵補充制御を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 18
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
#include "SurvivalDirector.h"

#include <algorithm>
#include <cmath>

#include "BattleRuleBook.h"
#include "EnemyFactory.h"
#include "../Utility/SimpleMathEx.h"

namespace Action
{
	namespace
	{
		constexpr int kDangerLevelMax = 10;
	}

	// サバイバル制御の内部状態を既定値で初期化する。
	SurvivalDirector::SurvivalDirector()
		: m_dangerLevel(1)
		, m_peakDangerLevel(1)
		, m_targetAliveCount(0)
		, m_spawnBatch(1)
		, m_spawnSerial(0)
		, m_spawnCooldownSec(0.0f)
		, m_spawnIntervalSec(1.0f)
	{
	}

	// ステージ開始時の進行状態へ戻す。
	void SurvivalDirector::Reset()
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		m_dangerLevel = 1;
		m_peakDangerLevel = 1;
		m_targetAliveCount = ruleBook.GetBaseAliveCount();
		m_spawnBatch = ruleBook.GetBaseSpawnBatch();
		m_spawnSerial = 0;
		m_spawnCooldownSec = 0.0f;
		m_spawnIntervalSec = ruleBook.GetBaseSpawnIntervalSec();
	}

	// 撃破数、経過時間、生存敵数から危険度と補充量を更新する。
	void SurvivalDirector::Update(float dt, int killCount, float survivalTimeSec, int livingEnemyCount)
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		const int killStep = std::max(1, ruleBook.GetKillRampStep());
		const float timeStepSec = std::max(1.0f, ruleBook.GetTimeRampStepSec());
		const int killTier = std::max(0, killCount / killStep);
		const int timeTier = std::max(0, static_cast<int>(std::floor(survivalTimeSec / timeStepSec)));
		m_dangerLevel = Utility::MathEx::Clamp(1 + killTier + timeTier, 1, kDangerLevelMax);
		m_peakDangerLevel = std::max(m_peakDangerLevel, m_dangerLevel);

		const float dangerRatio = static_cast<float>(m_dangerLevel - 1) / static_cast<float>(kDangerLevelMax - 1);
		const float aliveValue =
			static_cast<float>(ruleBook.GetBaseAliveCount()) +
			(static_cast<float>(ruleBook.GetMaxAliveCount() - ruleBook.GetBaseAliveCount()) * dangerRatio);
		const float batchValue =
			static_cast<float>(ruleBook.GetBaseSpawnBatch()) +
			(static_cast<float>(ruleBook.GetMaxSpawnBatch() - ruleBook.GetBaseSpawnBatch()) * dangerRatio);

		m_targetAliveCount = Utility::MathEx::Clamp(
			static_cast<int>(std::round(aliveValue)),
			ruleBook.GetBaseAliveCount(),
			ruleBook.GetMaxAliveCount());
		m_spawnBatch = Utility::MathEx::Clamp(
			static_cast<int>(std::round(batchValue)),
			ruleBook.GetBaseSpawnBatch(),
			ruleBook.GetMaxSpawnBatch());
		m_spawnIntervalSec = GetCurrentSpawnIntervalSec();

		if (livingEnemyCount < m_targetAliveCount)
		{
			m_spawnCooldownSec = std::max(0.0f, m_spawnCooldownSec - dt);
		}
		else
		{
			m_spawnCooldownSec = std::min(m_spawnCooldownSec, m_spawnIntervalSec);
		}
	}

	// ステージ開始直後に配置する敵群を構築する。
	std::vector<EnemyState> SurvivalDirector::BuildInitialSpawn(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints)
	{
		const int spawnCount = BattleRuleBook::GetInstance().GetOpeningAliveCount();
		m_spawnCooldownSec = GetCurrentSpawnIntervalSec();
		return BuildEnemies(spawnPoints, spawnCount);
	}

	// 現在不足している敵を補充する。
	std::vector<EnemyState> SurvivalDirector::BuildSpawnBatch(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints, int livingEnemyCount)
	{
		if (!CanSpawn(livingEnemyCount))
		{
			return {};
		}

		const int deficit = std::max(0, m_targetAliveCount - livingEnemyCount);
		const int spawnCount = Utility::MathEx::Clamp(deficit, 1, m_spawnBatch);
		m_spawnCooldownSec = GetCurrentSpawnIntervalSec();
		return BuildEnemies(spawnPoints, spawnCount);
	}

	// 現在の危険度を返す。
	int SurvivalDirector::GetDangerLevel() const
	{
		return m_dangerLevel;
	}

	// これまでに到達した最大危険度を返す。
	int SurvivalDirector::GetPeakDangerLevel() const
	{
		return m_peakDangerLevel;
	}

	// 補充クールダウンの進行率を返す。
	float SurvivalDirector::GetSpawnCooldownRatio() const
	{
		if (m_spawnIntervalSec <= 0.0f)
		{
			return 1.0f;
		}
		return Utility::MathEx::Clamp(1.0f - (m_spawnCooldownSec / m_spawnIntervalSec), 0.0f, 1.0f);
	}

	// 現在の目標生存敵数を返す。
	int SurvivalDirector::GetTargetAliveCount() const
	{
		return m_targetAliveCount;
	}

	// 指定数の敵を構築し、内部スポーン通番を進める。
	std::vector<EnemyState> SurvivalDirector::BuildEnemies(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints, int spawnCount)
	{
		std::vector<EnemyState> enemies;
		if (spawnPoints.empty() || spawnCount <= 0)
		{
			return enemies;
		}

		enemies.reserve(static_cast<size_t>(spawnCount));
		for (int spawnIndex = 0; spawnIndex < spawnCount; ++spawnIndex)
		{
			const int pointIndex = (m_spawnSerial + spawnIndex) % static_cast<int>(spawnPoints.size());
			enemies.push_back(EnemyFactory::CreateEnemy(
				spawnPoints[static_cast<size_t>(pointIndex)],
				m_dangerLevel,
				m_spawnSerial + spawnIndex));
		}

		m_spawnSerial += spawnCount;
		return enemies;
	}

	// 現在危険度に応じた補充間隔を返す。
	float SurvivalDirector::GetCurrentSpawnIntervalSec() const
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		return std::max(
			ruleBook.GetMinSpawnIntervalSec(),
			ruleBook.GetBaseSpawnIntervalSec() - 0.12f * static_cast<float>(m_dangerLevel - 1));
	}

	// 補充可能状態に入っているか返す。
	bool SurvivalDirector::CanSpawn(int livingEnemyCount) const
	{
		return livingEnemyCount < m_targetAliveCount && m_spawnCooldownSec <= 0.0f;
	}
}
