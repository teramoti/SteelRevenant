#pragma once

//=============================================================================
// CombatSystem.h
//
// 【役割】
//   プレイヤーと敵の戦闘ロジックを一元管理するクラス。
//   GameScene から純粋なゲームロジックを切り離し、
//   描画・入力・シーン管理への依存をゼロにしている。
//
// 【責務】
//   - プレイヤーの移動・旋回・攻撃・ガード・コンボゲージ更新
//   - 敵 AI の BehaviorTree 評価と状態遷移 (Idle/Wander/Chase/Attack/Return/Dead)
//   - A* 経路追従による敵の移動処理
//   - 攻撃ヒット判定とダメージ解決
//   - ロックオン・攻撃アシスト処理
//
// 【設計パターン】
//   - Strategy（BehaviorTree）:
//       敵 AI の意思決定を BehaviorTree へ委譲することで、
//       新しい行動タイプを追加するときに CombatSystem を変更しない。
//   - Data-Oriented:
//       PlayerState / EnemyState は純粋なデータ構造体。
//       CombatSystem はそれらを受け取って更新するだけで、
//       自身にゲーム状態を持たない（チューニング値と RNG を除く）。
//
// 【依存関係】
//   CombatTypes.h → PathGrid.h → AStarSolver.h のみ。
//   DirectX / シーン / 描画 への依存はない。
//=============================================================================

#include <random>
#include <vector>

#include "CombatTypes.h"
#include "GameState.h"
#include "../Navigation/PathGrid.h"
#include "../Navigation/AStarSolver.h"

namespace Core
{
    //=========================================================================
    // CombatSystem
    //=========================================================================
    class CombatSystem
    {
    public:
        /// @brief チューニング値と攻撃アシスト設定を既定値で構築する。
        CombatSystem();

        //---------------------------------------------------------------------
        // 設定
        //---------------------------------------------------------------------

        /// @brief 戦闘チューニング値をまとめて更新する。
        /// @param tuning  新しいチューニング値
        void SetTuning(const CombatTuning& tuning);

        /// @brief 現在のチューニング値を返す。
        const CombatTuning& GetTuning() const;

        /// @brief 攻撃アシストの距離と方向閾値を更新する。
        /// @param range         アシスト有効距離 (m)
        /// @param dotThreshold  アシスト有効方向閾値（内積値）
        void SetAttackAssistConfig(float range, float dotThreshold);

        //---------------------------------------------------------------------
        // 更新
        //---------------------------------------------------------------------

        /// @brief プレイヤーの移動・旋回・攻撃・コンボを 1 フレーム更新する。
        /// @param player        プレイヤー状態（読み書き）
        /// @param input         このフレームの入力スナップショット
        /// @param cameraForward カメラ前方ベクトル（移動方向算出に使用）
        /// @param cameraRight   カメラ右方向ベクトル（移動方向算出に使用）
        /// @param gameState     ゲーム進行状態（スコア加算など）
        /// @param groundHeight  プレイヤー足元の地面高さ (m)
        void UpdatePlayer(
            PlayerState&       player,
            const InputSnapshot& input,
            const DirectX::SimpleMath::Vector3& cameraForward,
            const DirectX::SimpleMath::Vector3& cameraRight,
            GameState&         gameState,
            float              groundHeight = 0.8f) const;

        /// @brief プレイヤーの攻撃判定を解決し、ヒットした敵にダメージを与える。
        /// @param player    プレイヤー状態（コンボゲージ更新など）
        /// @param enemies   全敵状態リスト（HP・hitByCurrentSwing を更新）
        /// @param gameState ゲーム進行状態（killCount・score 更新）
        void ResolvePlayerAttack(
            PlayerState&              player,
            std::vector<EnemyState>&  enemies,
            GameState&                gameState) const;

        /// @brief 全敵の AI 判断・移動・攻撃を 1 フレーム更新する。
        /// @param enemies   全敵状態リスト（読み書き）
        /// @param player    プレイヤー状態（攻撃対象として参照）
        /// @param input     このフレームの入力スナップショット
        /// @param grid      ナビゲーション用グリッド
        /// @param solver    A* ソルバー
        /// @param gameState ゲーム進行状態（被ダメージ記録）
        void UpdateEnemies(
            std::vector<EnemyState>&   enemies,
            PlayerState&               player,
            const InputSnapshot&       input,
            const Navigation::PathGrid&    grid,
            const Navigation::AStarSolver& solver,
            GameState&                 gameState) const;

        //---------------------------------------------------------------------
        // ユーティリティ
        //---------------------------------------------------------------------

        /// @brief 指定座標から最も近い生存敵のインデックスを返す。
        /// @param enemies     全敵状態リスト
        /// @param origin      基準座標
        /// @param maxDistance この距離以内の敵のみ対象 (m)
        /// @return インデックス（該当なし = -1）
        int FindNearestEnemy(
            const std::vector<EnemyState>& enemies,
            const DirectX::SimpleMath::Vector3& origin,
            float maxDistance) const;

        /// @brief 生存している敵が 1 体以上存在するか返す。
        bool HasLivingEnemy(const std::vector<EnemyState>& enemies) const;

    private:
        CombatTuning     m_tuning;              ///< 戦闘パラメータ調整値
        float            m_attackRange;         ///< 攻撃アシスト有効距離 (m)
        float            m_attackDotThreshold;  ///< 攻撃アシスト方向閾値
        mutable std::mt19937 m_rng;             ///< 敵 AI 乱数生成器（mutable: const メソッドから使用）
    };

} // namespace Core
