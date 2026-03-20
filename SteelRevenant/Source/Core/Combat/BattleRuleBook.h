#pragma once

//=============================================================================
// BattleRuleBook.h
//
// 【役割】
//   戦闘バランスのデフォルト値とスコア計算ルールを一箇所に集約する。
//   「マジックナンバーをコード中に散らさない」という原則に基づき、
//   全チューニング値の生成元をここに限定する。
//
// 【設計パターン】
//   - Factory Method（MakeDefaultTuning）:
//       CombatTuning の生成責任を CombatSystem から分離する。
//       将来的にステージ難易度ごとのチューニングが必要になった場合、
//       このクラスを拡張するだけで対応できる。
//
// 【使い方】
//   CombatTuning tuning = BattleRuleBook::MakeDefaultTuning();
//   combatSystem.SetTuning(tuning);
//   int score = BattleRuleBook::ComputeScore(killCount, survivalTimeSec, ...);
//=============================================================================

#include "CombatTypes.h"

namespace Core
{
    //=========================================================================
    // BattleRuleBook
    //=========================================================================
    class BattleRuleBook
    {
    public:
        //---------------------------------------------------------------------
        // チューニング値生成
        //---------------------------------------------------------------------

        /// @brief 標準難易度のチューニング値を生成して返す。
        /// @return デフォルト CombatTuning
        static CombatTuning MakeDefaultTuning();

        //---------------------------------------------------------------------
        // スコア計算
        //---------------------------------------------------------------------

        /// @brief 戦績からステージ最終スコアを算出する。
        /// @param killCount        撃破数
        /// @param survivalTimeSec  生存時間 (秒)
        /// @param peakDangerLevel  到達最大危険度
        /// @param damageTaken      被ダメージ累計
        /// @param relayCaptured    制圧した中継地点数
        /// @param relayRequired    クリア必要中継地点数
        /// @return 算出スコア
        static int ComputeScore(
            int   killCount,
            float survivalTimeSec,
            int   peakDangerLevel,
            int   damageTaken,
            int   relayCaptured,
            int   relayRequired);

        //---------------------------------------------------------------------
        // ステージ設定
        //---------------------------------------------------------------------

        /// @brief ステージ番号に対応する制限時間を返す。
        /// @param stageIndex  ステージ番号 (0〜)
        /// @return 制限時間 (秒)
        static float GetStageDurationSec(int stageIndex);

        /// @brief ステージ番号に対応するクリア必要中継地点数を返す。
        /// @param stageIndex  ステージ番号 (0〜)
        /// @return 必要制圧数
        static int GetRequiredRelayCount(int stageIndex);

    private:
        BattleRuleBook() = delete; ///< 全メソッドが static のためインスタンス化を禁止する
    };

} // namespace Core
