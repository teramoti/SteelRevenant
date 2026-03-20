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

	// 既定値でウェーブディレクタを構築する。
	SurvivalDirector::SurvivalDirector()
		: m_dangerLevel(1)
		, m_peakDangerLevel(1)
		, m_targetAliveCount(0)
		, m_spawnBatch(1)
		, m_spawnSerial(0)
		, m_spawnCooldownSec(0.0f)
		, m_spawnIntervalSec(1.0f)
		, m_waveBreakTimerSec(0.0f)
		, m_waveBreakDurationSec(0.0f)
		, m_currentWave(1)
		, m_totalWaveCount(1)
		, m_waveEnemyQuota(0)
		, m_waveSpawnedCount(0)
		, m_waveBreak(false)
		, m_completed(false)
	{
	}

	// ステージ開始時のウェーブ進行へ戻す。
	void SurvivalDirector::Reset()
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		m_dangerLevel = 1;
		m_peakDangerLevel = 1;
		m_spawnSerial = 0;
		m_spawnCooldownSec = 0.0f;
		m_spawnIntervalSec = ruleBook.GetBaseSpawnIntervalSec();
		m_waveBreakTimerSec = 0.0f;
		m_waveBreakDurationSec = ruleBook.GetWaveBreakSec();
		m_totalWaveCount = std::max(1, ruleBook.GetTotalWaveCount());
		m_currentWave = 1;
		m_waveBreak = false;
		m_completed = false;
		BeginWave(1);
	}

	// 時間経過、撃破数、残敵数からウェーブ状態を進める。
	void SurvivalDirector::Update(float dt, int killCount, float survivalTimeSec, int livingEnemyCount)
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		const int killStep = std::max(1, ruleBook.GetKillRampStep());
		const float timeStepSec = std::max(1.0f, ruleBook.GetTimeRampStepSec());
		const int killTier = std::max(0, killCount / killStep);
		const int timeTier = std::max(0, static_cast<int>(std::floor(survivalTimeSec / timeStepSec)));
		m_dangerLevel = Utility::MathEx::Clamp(1 + killTier + timeTier + (m_currentWave - 1), 1, kDangerLevelMax);
		m_peakDangerLevel = std::max(m_peakDangerLevel, m_dangerLevel);

		m_spawnIntervalSec = std::max(
			ruleBook.GetMinSpawnIntervalSec(),
			ruleBook.GetBaseSpawnIntervalSec() - 0.08f * static_cast<float>(m_currentWave - 1));

		if (m_completed)
		{
			m_spawnCooldownSec = 0.0f;
			return;
		}

		if (m_waveBreak)
		{
			m_waveBreakTimerSec = std::max(0.0f, m_waveBreakTimerSec - dt);
			if (m_waveBreakTimerSec <= 0.0f)
			{
				if (m_currentWave >= m_totalWaveCount)
				{
					m_completed = true;
				}
				else
				{
					BeginWave(m_currentWave + 1);
				}
			}
			return;
		}

		if (livingEnemyCount < m_targetAliveCount && m_waveSpawnedCount < m_waveEnemyQuota)
		{
			m_spawnCooldownSec = std::max(0.0f, m_spawnCooldownSec - dt);
		}
		else
		{
			m_spawnCooldownSec = std::min(m_spawnCooldownSec, m_spawnIntervalSec);
		}

		if (m_waveSpawnedCount >= m_waveEnemyQuota && livingEnemyCount <= 0)
		{
			m_waveBreak = true;
			m_waveBreakTimerSec = m_waveBreakDurationSec;
			m_spawnCooldownSec = 0.0f;
		}
	}

	// 初回配置分を生成する。
	std::vector<EnemyState> SurvivalDirector::BuildInitialSpawn(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints)
	{
		const int openingCount = std::min(BuildWaveAliveCap(m_currentWave), m_waveEnemyQuota);
		m_waveSpawnedCount = 0;
		m_spawnCooldownSec = m_spawnIntervalSec;
		return BuildEnemies(spawnPoints, openingCount);
	}

	// 現在ウェーブの不足分だけ増援を生成する。
	std::vector<EnemyState> SurvivalDirector::BuildSpawnBatch(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints, int livingEnemyCount)
	{
		if (!CanSpawn(livingEnemyCount))
		{
			return {};
		}

		const int remainingToSpawn = std::max(0, m_waveEnemyQuota - m_waveSpawnedCount);
		const int deficit = std::max(0, m_targetAliveCount - livingEnemyCount);
		const int spawnCount = std::min({ remainingToSpawn, deficit, m_spawnBatch });
		m_spawnCooldownSec = m_spawnIntervalSec;
		return BuildEnemies(spawnPoints, spawnCount);
	}

	int SurvivalDirector::GetDangerLevel() const { return m_dangerLevel; }
	int SurvivalDirector::GetPeakDangerLevel() const { return m_peakDangerLevel; }

	// 現在の増援クールダウン進行率を返す。
	float SurvivalDirector::GetSpawnCooldownRatio() const
	{
		if (m_spawnIntervalSec <= 0.0f)
		{
			return 1.0f;
		}
		return Utility::MathEx::Clamp(1.0f - (m_spawnCooldownSec / m_spawnIntervalSec), 0.0f, 1.0f);
	}

	int SurvivalDirector::GetTargetAliveCount() const { return m_targetAliveCount; }
	int SurvivalDirector::GetCurrentWave() const { return m_currentWave; }
	int SurvivalDirector::GetTotalWaveCount() const { return m_totalWaveCount; }
	bool SurvivalDirector::IsWaveBreak() const { return m_waveBreak; }

	// ウェーブ休止時間の進行率を返す。
	float SurvivalDirector::GetWaveBreakRatio() const
	{
		if (!m_waveBreak || m_waveBreakDurationSec <= 0.0f)
		{
			return 0.0f;
		}
		return Utility::MathEx::Clamp(1.0f - (m_waveBreakTimerSec / m_waveBreakDurationSec), 0.0f, 1.0f);
	}

	// 現ウェーブでまだ倒れていない敵数を返す。
	int SurvivalDirector::GetRemainingEnemiesInWave(int livingEnemyCount) const
	{
		const int remainingToSpawn = std::max(0, m_waveEnemyQuota - m_waveSpawnedCount);
		return std::max(0, livingEnemyCount + remainingToSpawn);
	}

	bool SurvivalDirector::IsCompleted() const { return m_completed; }

	// 生成要求に応じて敵リストを作る。
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
		m_waveSpawnedCount += spawnCount;
		return enemies;
	}

	// ウェーブ番号から総敵数を組み立てる。
	int SurvivalDirector::BuildWaveEnemyCount(int waveIndex) const
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		return std::max(
			ruleBook.GetOpeningAliveCount(),
			ruleBook.GetBaseWaveEnemyCount() + (std::max(1, waveIndex) - 1) * ruleBook.GetWaveEnemyIncreasePerWave());
	}

	// ウェーブ番号から同時出現上限を組み立てる。
	int SurvivalDirector::BuildWaveAliveCap(int waveIndex) const
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		return std::max(
			1,
			ruleBook.GetBaseWaveAliveCap() + (std::max(1, waveIndex) - 1) * ruleBook.GetWaveAliveCapIncreasePerWave());
	}

	// 指定ウェーブの目標値へ更新する。
	void SurvivalDirector::BeginWave(int waveIndex)
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		m_currentWave = Utility::MathEx::Clamp(waveIndex, 1, m_totalWaveCount);
		m_waveEnemyQuota = BuildWaveEnemyCount(m_currentWave);
		m_waveSpawnedCount = 0;
		m_waveBreak = false;
		m_waveBreakTimerSec = 0.0f;
		m_targetAliveCount = BuildWaveAliveCap(m_currentWave);
		m_spawnBatch = Utility::MathEx::Clamp(
			ruleBook.GetBaseSpawnBatch() + (m_currentWave - 1),
			ruleBook.GetBaseSpawnBatch(),
			ruleBook.GetMaxSpawnBatch());
		m_spawnCooldownSec = 0.0f;
	}

	// 現在ウェーブで増援可能か判定する。
	bool SurvivalDirector::CanSpawn(int livingEnemyCount) const
	{
		if (m_completed || m_waveBreak)
		{
			return false;
		}

		return
			livingEnemyCount < m_targetAliveCount &&
			m_waveSpawnedCount < m_waveEnemyQuota &&
			m_spawnCooldownSec <= 0.0f;
	}
}
