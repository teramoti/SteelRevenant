//------------------------//------------------------
// Contents(処理内容) ステージ別サバイバルルールと参照 API を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "BattleRuleBook.h"

#include "../Utility/SimpleMathEx.h"

namespace Action
{
	// ステージごとの固定ルールを構築する。
	BattleRuleBook::BattleRuleBook()
		: m_stageRules
		{
			StageRuleDefinition{
				L"\u5916\u7e01\u533a\u753b",
				L"\u5916\u5468\u30c7\u30c3\u30ad + \u4e2d\u592e\u30c7\u30c3\u30ad",
				L"\u8fd1\u63a5\u4e3b\u4f53 / \u6642\u9593\u5727: \u4f4e",
				L"1\u5206\u77ed\u671f\u6226\u3067\u6575\u5727\u3068\u5ef6\u547d\u5224\u65ad\u3092\u78ba\u8a8d\u305b\u3088",
				L"1\u5206\u77ed\u671f\u6226  Relay 1 / Beacon +2 sec",
				60.0f,
				2.0f,
				1,
				4.0f,
				2.0f,
				6,
				20.0f,
				20,
				20,
				30,
				4,
				0.65f,
				0.28f,
				6 },
			StageRuleDefinition{
				L"\u9632\u885b\u56de\u5eca",
				L"\u76f4\u7dda\u56de\u5eca + \u5074\u9762\u969c\u5bb3",
				L"\u8fd1\u63a5\u4e3b\u4f53 / \u6642\u9593\u5727: \u4e2d",
				L"3\u5206\u5236\u5727\u6226\u3067\u6b63\u9762\u5727\u3068\u6a2a\u5727\u3092\u7ba1\u7406\u305b\u3088",
				L"3\u5206\u5236\u5727\u6226  Relay 2 / Beacon +7 sec",
				180.0f,
				2.1f,
				2,
				11.0f,
				7.0f,
				5,
				35.0f,
				35,
				35,
				45,
				6,
				0.48f,
				0.20f,
				9 },
			StageRuleDefinition{
				L"\u4e2d\u592e\u533a\u4e2d\u67a2",
				L"\u4e2d\u592e\u30b3\u30a2 + \u56db\u9685\u969c\u5bb3",
				L"\u8fd1\u63a5\u4e3b\u4f53 / \u6642\u9593\u5727: \u9ad8",
				L"5\u5206\u8010\u4e45\u6226\u3067\u5305\u56f2\u5727\u3068\u5371\u967a\u5730\u5e2f\u3092\u634c\u3051",
				L"5\u5206\u8010\u4e45\u6226  Relay 3 / Beacon +12 sec",
				300.0f,
				2.3f,
				3,
				18.0f,
				12.0f,
				4,
				45.0f,
				50,
				50,
				65,
				8,
				0.36f,
				0.14f,
				12 }
		}
		, m_activeStageIndex(1)
	{
	}

	// 追加解放は持たないため特別な後処理は行わない。
	BattleRuleBook::~BattleRuleBook() = default;

	// 現在参照するステージ番号を設定する。
	void BattleRuleBook::SetActiveStage(int stageIndex)
	{
		m_activeStageIndex = Utility::MathEx::Clamp(stageIndex, 1, static_cast<int>(m_stageRules.size()));
	}

	// 現在参照しているステージ番号を返す。
	int BattleRuleBook::GetActiveStageIndex() const
	{
		return m_activeStageIndex;
	}

	// 現在ステージのルールを返す。
	const StageRuleDefinition& BattleRuleBook::GetActiveRule() const
	{
		return m_stageRules[static_cast<size_t>(Utility::MathEx::Clamp(m_activeStageIndex, 1, static_cast<int>(m_stageRules.size())) - 1)];
	}

	// 指定ステージ番号のルールを返す。
	const StageRuleDefinition& BattleRuleBook::GetStageRule(int stageIndex) const
	{
		const int index = Utility::MathEx::Clamp(stageIndex, 1, static_cast<int>(m_stageRules.size())) - 1;
		return m_stageRules[static_cast<size_t>(index)];
	}

	// ステージ開始時の制限時間を返す。
	float BattleRuleBook::GetStageStartTimeSec() const
	{
		return GetActiveRule().stageStartTimeSec;
	}

	// 結果画面へ遷移するまでの待機秒数を返す。
	float BattleRuleBook::GetResultDelaySec() const
	{
		return GetActiveRule().resultDelaySec;
	}

	// ステージに配置する Relay 数を返す。
	int BattleRuleBook::GetRequiredRelayCount() const
	{
		return GetActiveRule().requiredRelayCount;
	}

	// Relay 制圧時に加算する秒数を返す。
	float BattleRuleBook::GetRelayTimeBonusSec() const
	{
		return GetActiveRule().relayTimeBonusSec;
	}

	// Beacon 使用時に加算する秒数を返す。
	float BattleRuleBook::GetBeaconTimeBonusSec() const
	{
		return GetActiveRule().beaconTimeBonusSec;
	}

	// dangerLevel を撃破数で上げる刻みを返す。
	int BattleRuleBook::GetKillRampStep() const
	{
		return GetActiveRule().killRampStep;
	}

	// dangerLevel を生存時間で上げる刻み秒を返す。
	float BattleRuleBook::GetTimeRampStepSec() const
	{
		return GetActiveRule().timeRampStepSec;
	}

	// 開始直後に出現させる敵数を返す。
	int BattleRuleBook::GetOpeningAliveCount() const
	{
		return GetActiveRule().openingAliveCount;
	}

	// 維持したい同時生存数の下限を返す。
	int BattleRuleBook::GetBaseAliveCount() const
	{
		return GetActiveRule().baseAliveCount;
	}

	// 維持する同時生存数の上限を返す。
	int BattleRuleBook::GetMaxAliveCount() const
	{
		return GetActiveRule().maxAliveCount;
	}

	// 1 回の補充で出す敵数の下限を返す。
	int BattleRuleBook::GetBaseSpawnBatch() const
	{
		return GetActiveRule().baseSpawnBatch;
	}

	// 補充間隔の初期秒数を返す。
	float BattleRuleBook::GetBaseSpawnIntervalSec() const
	{
		return GetActiveRule().baseSpawnIntervalSec;
	}

	// 補充間隔の下限秒数を返す。
	float BattleRuleBook::GetMinSpawnIntervalSec() const
	{
		return GetActiveRule().minSpawnIntervalSec;
	}

	// 1 回の補充で出す敵数の上限を返す。
	int BattleRuleBook::GetMaxSpawnBatch() const
	{
		return GetActiveRule().maxSpawnBatch;
	}
}
