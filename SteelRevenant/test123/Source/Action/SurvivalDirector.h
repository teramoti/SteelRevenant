#pragma once

#include <SimpleMath.h>
#include <vector>

#include "CombatSystem.h"

namespace Action
{
	// ウェーブ進行と増援供給を管理するディレクタ。
	class SurvivalDirector
	{
	public:
		SurvivalDirector();

		void Reset();
		void Update(float dt, int killCount, float survivalTimeSec, int livingEnemyCount);
		std::vector<EnemyState> BuildInitialSpawn(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints);
		std::vector<EnemyState> BuildSpawnBatch(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints, int livingEnemyCount);
		int GetDangerLevel() const;
		int GetPeakDangerLevel() const;
		float GetSpawnCooldownRatio() const;
		int GetTargetAliveCount() const;
		bool CanSpawn(int livingEnemyCount) const;
		int GetCurrentWave() const;
		int GetTotalWaveCount() const;
		bool IsWaveBreak() const;
		float GetWaveBreakRatio() const;
		int GetRemainingEnemiesInWave(int livingEnemyCount) const;
		bool IsCompleted() const;

	private:
		std::vector<EnemyState> BuildEnemies(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints, int spawnCount);
		int BuildWaveTargetAliveCount(int waveIndex) const;
		void BeginWave(int waveIndex);

	private:
		int m_dangerLevel;
		int m_peakDangerLevel;
		int m_targetAliveCount;
		int m_maxAliveCount;
		int m_spawnBatch;
		int m_spawnSerial;
		float m_spawnCooldownSec;
		float m_spawnIntervalSec;
		float m_waveDurationSec;
		int m_currentWave;
		int m_totalWaveCount;
		bool m_waveBreak;
		bool m_completed;
	};
}

