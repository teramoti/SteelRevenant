#include "SurvivalDirector.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "BattleRuleBook.h"
#include "EnemyFactory.h"
#include "../Utility/SimpleMathEx.h"

// For debug output to Visual Studio Output window
#include <Windows.h>

namespace Action
{
	namespace
	{
		constexpr int kDangerLevelMax = 10;
		inline void DebugPrint(const char* /*msg*/)
		{
			// No-op to avoid runtime debug output overhead.
		}
	}

	SurvivalDirector::SurvivalDirector()
		: m_dangerLevel(1)
		, m_peakDangerLevel(1)
		, m_targetAliveCount(0)
		, m_maxAliveCount(0)
		, m_spawnBatch(1)
		, m_spawnSerial(0)
		, m_spawnCooldownSec(0.0f)
		, m_spawnIntervalSec(1.0f)
		, m_waveDurationSec(0.0f)
		, m_currentWave(1)
		, m_totalWaveCount(1)
		, m_waveBreak(false)
		, m_completed(false)
	{
	}

	void SurvivalDirector::Reset()
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		m_dangerLevel = 1;
		m_peakDangerLevel = 1;
		m_spawnSerial = 0;
		m_spawnCooldownSec = 0.0f;
		m_spawnIntervalSec = ruleBook.GetBaseSpawnIntervalSec();
		m_totalWaveCount = std::max(1, ruleBook.GetTotalWaveCount());
		m_waveDurationSec = ruleBook.GetStageStartTimeSec() / static_cast<float>(m_totalWaveCount);
		m_currentWave = 1;
		m_waveBreak = false;
		m_completed = false;
		BeginWave(1);

		// Debug: print rulebook relevant values
		{
			char buf[256];
			sprintf_s(buf, "SurvivalDirector::Reset - Stage=%d opening=%d base=%d max=%d\n",
				ruleBook.GetActiveStageIndex(),
				ruleBook.GetOpeningAliveCount(),
				ruleBook.GetBaseAliveCount(),
				ruleBook.GetMaxAliveCount());
			DebugPrint(buf);
		}
	}

	void SurvivalDirector::Update(float dt, int killCount, float survivalTimeSec, int livingEnemyCount)
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		const int killStep = std::max(1, ruleBook.GetKillRampStep());
		const float timeStepSec = std::max(1.0f, ruleBook.GetTimeRampStepSec());
		const int killTier = std::max(0, killCount / killStep);
		const int timeTier = std::max(0, static_cast<int>(std::floor(survivalTimeSec / timeStepSec)));
		const int desiredWave = Utility::MathEx::Clamp(
			(m_waveDurationSec > 0.0f)
				? (static_cast<int>(std::floor(survivalTimeSec / m_waveDurationSec)) + 1)
				: 1,
			1,
			m_totalWaveCount);

		if (desiredWave != m_currentWave)
		{
			BeginWave(desiredWave);
		}

		m_dangerLevel = Utility::MathEx::Clamp(1 + killTier + timeTier + (m_currentWave - 1), 1, kDangerLevelMax);
		m_peakDangerLevel = std::max(m_peakDangerLevel, m_dangerLevel);

		const float wavePressure = static_cast<float>(std::max(0, m_currentWave - 1));
		const float dangerPressure = static_cast<float>(std::max(0, m_dangerLevel - 1));
		m_spawnIntervalSec = std::max(
			ruleBook.GetMinSpawnIntervalSec(),
			ruleBook.GetBaseSpawnIntervalSec() - wavePressure * 0.015f - dangerPressure * 0.004f);

		// Respect ruleBook minimum; avoid additional aggressive scaling here to reduce spawn burstiness
		m_spawnIntervalSec = std::max(ruleBook.GetMinSpawnIntervalSec(), m_spawnIntervalSec);

		const int desiredAlive = (std::min)(m_targetAliveCount, m_maxAliveCount);
		if (livingEnemyCount < desiredAlive)
		{
			const float urgency = static_cast<float>(desiredAlive - livingEnemyCount) /
				static_cast<float>(std::max(1, desiredAlive));
			m_spawnCooldownSec = std::max(0.0f, m_spawnCooldownSec - dt * (0.70f + urgency * 0.90f));
		}
		else
		{
			m_spawnCooldownSec = (std::min)(m_spawnCooldownSec, m_spawnIntervalSec);
		}

		m_waveBreak = false;
		m_completed = false;
	}

	std::vector<EnemyState> SurvivalDirector::BuildInitialSpawn(
		const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints)
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		// Use stage-defined opening count directly (do not force a minimum here)
		const int openingFromRule = ruleBook.GetOpeningAliveCount();
		const int openingCount = (std::min)(openingFromRule, m_maxAliveCount);
		m_spawnCooldownSec = std::max(0.40f, m_spawnIntervalSec * 0.60f);

		char buf[256];
		sprintf_s(buf, "BuildInitialSpawn - Stage=%d openingRule=%d m_maxAlive=%d openingCount=%d\n",
			ruleBook.GetActiveStageIndex(), openingFromRule, m_maxAliveCount, openingCount);
		DebugPrint(buf);

		std::vector<EnemyState> spawned = BuildEnemies(spawnPoints, openingCount);

		// Debug: composition counts
		int melee = 0, laser = 0, dash = 0, heavy = 0;
		for (const auto& e : spawned)
		{
			if (e.isLaserEnemy) ++laser;
			if (e.isDashingEnemy) ++dash;
			if (e.isHeavyEnemy) ++heavy;
			if (!e.isLaserEnemy && !e.isDashingEnemy && !e.isHeavyEnemy) ++melee;
		}
		sprintf_s(buf, "Initial spawn composition - melee=%d laser=%d dash=%d heavy=%d total=%zu\n",
			melee, laser, dash, heavy, spawned.size());
		DebugPrint(buf);

		return spawned;
	}

	std::vector<EnemyState> SurvivalDirector::BuildSpawnBatch(
		const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints,
		int livingEnemyCount)
	{
		if (!CanSpawn(livingEnemyCount))
		{
			return {};
		}

		const int desiredAlive = (std::min)(m_targetAliveCount, m_maxAliveCount);
		const int deficit = std::max(0, desiredAlive - livingEnemyCount);
		const int roomToMax = std::max(0, m_maxAliveCount - livingEnemyCount);
		const bool emergencyFill = livingEnemyCount <= std::max(1, desiredAlive / 3);
		const int earlyWaveCap = (m_currentWave <= 2) ? 2 : 3;
		const int batchCap = std::max(1, std::min(m_spawnBatch + (emergencyFill ? 1 : 0), earlyWaveCap));
		const int spawnCount = (std::min)(roomToMax, (std::min)(deficit, batchCap));

		if (spawnCount <= 0)
		{
			return {};
		}

		m_spawnCooldownSec = emergencyFill ? (m_spawnIntervalSec * 0.72f) : m_spawnIntervalSec;
		return BuildEnemies(spawnPoints, spawnCount);
	}

	int SurvivalDirector::GetDangerLevel() const
	{
		return m_dangerLevel;
	}

	int SurvivalDirector::GetPeakDangerLevel() const
	{
		return m_peakDangerLevel;
	}

	float SurvivalDirector::GetSpawnCooldownRatio() const
	{
		if (m_spawnIntervalSec <= 0.0f)
		{
			return 1.0f;
		}

		return Utility::MathEx::Clamp(1.0f - (m_spawnCooldownSec / m_spawnIntervalSec), 0.0f, 1.0f);
	}

	int SurvivalDirector::GetTargetAliveCount() const
	{
		return m_targetAliveCount;
	}

	bool SurvivalDirector::CanSpawn(int livingEnemyCount) const
	{
		if (m_completed || m_waveBreak)
		{
			return false;
		}

		if (livingEnemyCount >= m_targetAliveCount || livingEnemyCount >= m_maxAliveCount)
		{
			return false;
		}

		const int desiredAlive = (std::min)(m_targetAliveCount, m_maxAliveCount);
		const bool emergencyFill = livingEnemyCount <= std::max(1, desiredAlive / 3);
		if (emergencyFill)
		{
			return m_spawnCooldownSec <= (m_spawnIntervalSec * 0.35f);
		}

		if (livingEnemyCount < desiredAlive)
		{
			return m_spawnCooldownSec <= 0.0f;
		}

		return m_spawnCooldownSec <= 0.0f;
	}

	int SurvivalDirector::GetCurrentWave() const
	{
		return m_currentWave;
	}

	int SurvivalDirector::GetTotalWaveCount() const
	{
		return m_totalWaveCount;
	}

	bool SurvivalDirector::IsWaveBreak() const
	{
		return m_waveBreak;
	}

	float SurvivalDirector::GetWaveBreakRatio() const
	{
		return 0.0f;
	}

	int SurvivalDirector::GetRemainingEnemiesInWave(int livingEnemyCount) const
	{
		return std::max(0, m_targetAliveCount - livingEnemyCount);
	}

	bool SurvivalDirector::IsCompleted() const
	{
		return m_completed;
	}

	std::vector<EnemyState> SurvivalDirector::BuildEnemies(
		const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints,
		int spawnCount)
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

		// Debug: print spawn serial and spawnCount
		{
			char buf[128];
			sprintf_s(buf, "BuildEnemies - spawnSerial=%d spawnCount=%d totalSerialAfter=%d\n", m_spawnSerial - spawnCount, spawnCount, m_spawnSerial);
			DebugPrint(buf);
		}

		return enemies;
	}

	int SurvivalDirector::BuildWaveTargetAliveCount(int waveIndex) const
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		const int baseAlive = std::max(1, ruleBook.GetBaseAliveCount());
		const int maxAlive = std::max(baseAlive, ruleBook.GetMaxAliveCount());
		if (m_totalWaveCount <= 1)
		{
			return maxAlive;
		}

		const float t = static_cast<float>(Utility::MathEx::Clamp(waveIndex, 1, m_totalWaveCount) - 1) /
			static_cast<float>(m_totalWaveCount - 1);
		int target = Utility::MathEx::Clamp(
			static_cast<int>(std::round(baseAlive + static_cast<float>(maxAlive - baseAlive) * t)),
			baseAlive,
			maxAlive);

		// Prefer moderate combat density: clamp target into a sensible range when supported by maxAlive
		if (maxAlive >= 8)
		{
			target = Utility::MathEx::Clamp(target, 6, std::min(20, maxAlive));
		}

		return target;
	}

	void SurvivalDirector::BeginWave(int waveIndex)
	{
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		m_currentWave = Utility::MathEx::Clamp(waveIndex, 1, m_totalWaveCount);
		// Use explicit max from ruleBook
		m_maxAliveCount = ruleBook.GetMaxAliveCount();
		m_targetAliveCount = BuildWaveTargetAliveCount(m_currentWave);
		m_spawnBatch = Utility::MathEx::Clamp(
			ruleBook.GetBaseSpawnBatch() + ((m_currentWave - 1) / 2),
			ruleBook.GetBaseSpawnBatch(),
			ruleBook.GetMaxSpawnBatch());
		m_spawnCooldownSec = std::max(0.45f, m_spawnIntervalSec * 0.85f);
		m_waveBreak = false;
		m_completed = false;

		char buf[256];
		sprintf_s(buf, "BeginWave - Stage=%d wave=%d targetAlive=%d maxAlive=%d spawnBatch=%d\n",
			ruleBook.GetActiveStageIndex(), m_currentWave, m_targetAliveCount, m_maxAliveCount, m_spawnBatch);
		DebugPrint(buf);
	}
}
