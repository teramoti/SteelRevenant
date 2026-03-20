//------------------------//------------------------
// Contents(処理内容) サバイバル進行と敵補充制御を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 18
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
#pragma once

#include <SimpleMath.h>
#include <vector>

#include "CombatSystem.h"

namespace Action
{
	class SurvivalDirector
	{
	public:
		// サバイバル制御の内部状態を既定値で初期化する。
		SurvivalDirector();

		// ステージ開始時の進行状態へ戻す。
		void Reset();
		// 撃破数、経過時間、生存敵数から危険度と補充量を更新する。
		void Update(float dt, int killCount, float survivalTimeSec, int livingEnemyCount);
		// ステージ開始直後に配置する敵群を構築する。
		std::vector<EnemyState> BuildInitialSpawn(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints);
		// 現在不足している敵を補充する。
		std::vector<EnemyState> BuildSpawnBatch(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints, int livingEnemyCount);
		// 現在の危険度を返す。
		int GetDangerLevel() const;
		// これまでに到達した最大危険度を返す。
		int GetPeakDangerLevel() const;
		// 補充クールダウンの進行率を返す。
		float GetSpawnCooldownRatio() const;
		// 現在の目標生存敵数を返す。
		int GetTargetAliveCount() const;
		// 補充条件を満たしているか返す。
		bool CanSpawn(int livingEnemyCount) const;

	private:
		// 指定数の敵を構築し、内部スポーン通番を進める。
		std::vector<EnemyState> BuildEnemies(const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints, int spawnCount);
		// 現在危険度に応じた補充間隔を返す。
		float GetCurrentSpawnIntervalSec() const;

	private:
		int m_dangerLevel;
		int m_peakDangerLevel;
		int m_targetAliveCount;
		int m_spawnBatch;
		int m_spawnSerial;
		float m_spawnCooldownSec;
		float m_spawnIntervalSec;
	};
}
