#pragma once

namespace Action
{
	// 戦闘ステージの進行集計をまとめる軽量ステート。
	struct GameState
	{
		// 残り制限時間。
		float stageTimer;
		// 経過時間。
		float survivalTimeSec;
		// ライブスコア。
		int score;
		// 現在危険度。
		int dangerLevel;
		// 到達した最大危険度。
		int peakDangerLevel;
		// 累計撃破数。
		int killCount;
		// 被ダメージ集計。
		int damageTaken;
		// 時間切れになったか。
		bool timeExpired;
		// 最終ウェーブを制圧したか。
		bool stageCleared;

		// 既定値で状態を構築する。
		GameState()
			: stageTimer(300.0f)
			, survivalTimeSec(0.0f)
			, score(0)
			, dangerLevel(1)
			, peakDangerLevel(1)
			, killCount(0)
			, damageTaken(0)
			, timeExpired(false)
			, stageCleared(false)
		{
		}

		// ステージ開始時の集計値へ戻す。
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
			stageCleared = false;
		}

		// 戦闘中のみタイマーと経過時間を進める。
		void Tick(float dt)
		{
			if (timeExpired || stageCleared)
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

		// 時間切れまたは制圧完了で終了扱いとする。
		bool IsFinished() const
		{
			return timeExpired || stageCleared;
		}
	};
}

