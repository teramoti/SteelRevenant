#pragma once

//=============================================================================
// GameState.h
//
// 【役割】
//   サバイバル進行で共有する実行時ゲーム状態を保持する構造体。
//   CombatSystem / SurvivalDirector / GameScene の間でデータを受け渡す
//   中立的な値オブジェクトとして機能する。
//
// 【設計メモ】
//   純粋なデータ構造体であり、依存関係はゼロ。
//   Reset() / Tick() / IsFinished() の 3 メソッドだけを持ち、
//   ゲームロジックは CombatSystem と SurvivalDirector に委譲する。
//=============================================================================

namespace Core
{
    //=========================================================================
    // GameState  ― サバイバル進行状態
    //=========================================================================
    struct GameState
    {
        float stageTimer;       ///< 現在の残り時間 (秒)
        float survivalTimeSec;  ///< 開始からの経過時間 (秒)
        int   score;            ///< 現在の総合スコア
        int   dangerLevel;      ///< 現在の危険度 (1〜)
        int   peakDangerLevel;  ///< 到達済みの最大危険度
        int   killCount;        ///< 現在撃破数
        int   damageTaken;      ///< 被ダメージ累計
        bool  timeExpired;      ///< 制限時間が尽きたか

        /// @brief サバイバル進行状態を既定値で初期化する。
        GameState()
            : stageTimer(300.0f)
            , survivalTimeSec(0.0f)
            , score(0)
            , dangerLevel(1)
            , peakDangerLevel(1)
            , killCount(0)
            , damageTaken(0)
            , timeExpired(false)
        {}

        /// @brief ステージ開始時の状態へ戻す。
        /// @param startTimer  ステージ制限時間 (秒)
        void Reset(float startTimer)
        {
            stageTimer      = startTimer;
            survivalTimeSec = 0.0f;
            score           = 0;
            dangerLevel     = 1;
            peakDangerLevel = 1;
            killCount       = 0;
            damageTaken     = 0;
            timeExpired     = false;
        }

        /// @brief 時間経過を反映し、時間切れで終了状態へ遷移する。
        /// @param dt  フレーム経過時間 (秒)
        void Tick(float dt)
        {
            if (timeExpired) { return; }

            stageTimer      -= dt;
            survivalTimeSec += dt;

            if (stageTimer <= 0.0f)
            {
                stageTimer  = 0.0f;
                timeExpired = true;
            }
        }

        /// @brief リザルト遷移条件を満たしているか返す。
        bool IsFinished() const { return timeExpired; }
    };

} // namespace Core
