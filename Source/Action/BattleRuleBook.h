//------------------------//------------------------
// Contents(処理内容) ステージ別サバイバルルールと参照 API を定義する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#pragma once

#include <array>

#include "../Utility/SingletonBase.h"

namespace Action
{
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
	};

	class BattleRuleBook : public Utility::SingletonBase<BattleRuleBook>
	{
	public:
		friend class Utility::SingletonBase<BattleRuleBook>;

	private:
		// ステージごとの固定ルールを構築する。
		BattleRuleBook();
		// 追加解放は持たないため特別な後処理は行わない。
		~BattleRuleBook();

	public:
		// 現在参照するステージ番号を設定する。
		void SetActiveStage(int stageIndex);
		// 現在参照しているステージ番号を返す。
		int GetActiveStageIndex() const;
		// 現在ステージのルールを返す。
		const StageRuleDefinition& GetActiveRule() const;
		// 指定ステージ番号のルールを返す。
		const StageRuleDefinition& GetStageRule(int stageIndex) const;

		// ステージ開始時の制限時間を返す。
		float GetStageStartTimeSec() const;
		// 結果画面へ遷移するまでの待機秒数を返す。
		float GetResultDelaySec() const;
		// ステージに配置する Relay 数を返す。
		int GetRequiredRelayCount() const;
		// Relay 制圧時に加算する秒数を返す。
		float GetRelayTimeBonusSec() const;
		// Beacon 使用時に加算する秒数を返す。
		float GetBeaconTimeBonusSec() const;
		// dangerLevel を撃破数で上げる刻みを返す。
		int GetKillRampStep() const;
		// dangerLevel を生存時間で上げる刻み秒を返す。
		float GetTimeRampStepSec() const;
		// 開始直後に出現させる敵数を返す。
		int GetOpeningAliveCount() const;
		// 維持したい同時生存数の下限を返す。
		int GetBaseAliveCount() const;
		// 維持する同時生存数の上限を返す。
		int GetMaxAliveCount() const;
		// 1 回の補充で出す敵数の下限を返す。
		int GetBaseSpawnBatch() const;
		// 補充間隔の初期秒数を返す。
		float GetBaseSpawnIntervalSec() const;
		// 補充間隔の下限秒数を返す。
		float GetMinSpawnIntervalSec() const;
		// 1 回の補充で出す敵数の上限を返す。
		int GetMaxSpawnBatch() const;

	private:
		std::array<StageRuleDefinition, 3> m_stageRules;
		int m_activeStageIndex;
	};
}
