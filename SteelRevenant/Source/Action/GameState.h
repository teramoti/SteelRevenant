//------------------------//------------------------
// Contents(処理内容) サバイバル進行で共有する実行時状態を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
#pragma once

namespace Action
{
	struct GameState
	{
		// 現在の残り時間(秒)。
		float stageTimer;
		// 開始からの経過時間(秒)。
		float survivalTimeSec;
		// 現在の総合スコア。
		int score;
		// 現在の危険度。
		int dangerLevel;
		// 到達済みの最大危険度。
		int peakDangerLevel;
		// 現在撃破数。
		int killCount;
		// 被ダメージ累計。
		int damageTaken;
		// 制限時間が尽きたか。
		bool timeExpired;

		// サバイバル進行状態を既定値で初期化する。
		GameState()
			: stageTimer(300.0f)
			, survivalTimeSec(0.0f)
			, score(0)
			, dangerLevel(1)
			, peakDangerLevel(1)
			, killCount(0)
			, damageTaken(0)
			, timeExpired(false)
		{
		}

		// ステージ開始時の状態へ戻す。
		void Reset(float startTimer)
		{
			stageTimer = startTimer;
			survivalTimeSec = 0.0f;
			score = 0;
			dangerLevel = 1;
			peakDangerLevel = 1;
			killCount = 0;
			damageTaken = 0;
			timeExpired = false;
		}

		// 時間経過を反映し、時間切れで終了状態へ遷移する。
		void Tick(float dt)
		{
			if (timeExpired)
			{
				return;
			}

			stageTimer -= dt;
			survivalTimeSec += dt;
			if (stageTimer <= 0.0f)
			{
				stageTimer = 0.0f;
				timeExpired = true;
			}
		}

		// リザルト遷移条件を満たしたか返す。
		bool IsFinished() const
		{
			return timeExpired;
		}
	};
}
