#pragma once

//=============================================================================
// SurvivalDirector.h
//
// 【役割】
//   サバイバルモードの進行制御と敵補充を管理するクラス。
//   「いつ・何体・どんな敵を出すか」という動的難易度調整（DDA）の
//   中心的な役割を担う。
//
// 【責務】
//   - 撃破数・生存時間・生存敵数に基づく危険度 (DangerLevel) の段階的上昇
//   - 危険度に連動した目標生存敵数・補充バッチ数・補充間隔の自動調整
//   - EnemyFactory を通じた補充敵インスタンスの生成
//
// 【設計パターン】
//   - Director（GoF Builder パターンの Director に相当）:
//       SurvivalDirector は「敵をいつ出すか」を決め、
//       EnemyFactory は「どんな敵を生成するか」を決める。
//       この分離により、補充ルールと生成ルールを独立して変更できる。
//
// 【動的難易度調整 (DDA) の概要】
//   危険度は killCount と survivalTimeSec のどちらか早い方で上昇する。
//   危険度が上がると:
//     - 目標生存敵数が増える
//     - 補充バッチサイズが増える
//     - 補充クールダウンが短くなる
//     - EnemyFactory に渡す dangerLevel が上がり、敵個体が強化される
//=============================================================================

#include <vector>
#include <SimpleMath.h>

#include "CombatTypes.h"

namespace Core
{
    //=========================================================================
    // SurvivalDirector
    //=========================================================================
    class SurvivalDirector
    {
    public:
        /// @brief サバイバル制御の内部状態を既定値で初期化する。
        SurvivalDirector();

        //---------------------------------------------------------------------
        // 初期化 / リセット
        //---------------------------------------------------------------------

        /// @brief ステージ開始時の進行状態へ戻す。
        void Reset();

        //---------------------------------------------------------------------
        // 更新
        //---------------------------------------------------------------------

        /// @brief 撃破数・経過時間・生存敵数から危険度と補充タイミングを更新する。
        /// @param dt                フレーム経過時間 (秒)
        /// @param killCount         現在の撃破数
        /// @param survivalTimeSec   ステージ開始からの経過時間 (秒)
        /// @param livingEnemyCount  現在の生存敵数
        void Update(float dt, int killCount, float survivalTimeSec, int livingEnemyCount);

        //---------------------------------------------------------------------
        // 補充生成
        //---------------------------------------------------------------------

        /// @brief ステージ開始直後に配置する初期敵群を生成する。
        /// @param spawnPoints  配置候補座標リスト
        /// @return 生成した EnemyState のリスト
        std::vector<EnemyState> BuildInitialSpawn(
            const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints);

        /// @brief 現在の不足分を補充する敵群を生成する。
        /// @param spawnPoints       配置候補座標リスト
        /// @param livingEnemyCount  現在の生存敵数
        /// @return 生成した EnemyState のリスト（補充不要なら空）
        std::vector<EnemyState> BuildSpawnBatch(
            const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints,
            int livingEnemyCount);

        //---------------------------------------------------------------------
        // Accessors
        //---------------------------------------------------------------------

        /// @brief 現在の危険度を返す。
        int GetDangerLevel() const { return m_dangerLevel; }

        /// @brief これまでに到達した最大危険度を返す。
        int GetPeakDangerLevel() const { return m_peakDangerLevel; }

        /// @brief 補充クールダウンの進行率を返す (0=待機中, 1=補充可能)。
        float GetSpawnCooldownRatio() const;

        /// @brief 現在の目標生存敵数を返す。
        int GetTargetAliveCount() const { return m_targetAliveCount; }

        /// @brief 補充条件を満たしているか返す。
        /// @param livingEnemyCount  現在の生存敵数
        bool CanSpawn(int livingEnemyCount) const;

    private:
        //---------------------------------------------------------------------
        // 内部生成
        //---------------------------------------------------------------------

        /// @brief 指定数の敵を生成し、内部スポーン通番を進める。
        /// @param spawnPoints  配置候補座標リスト
        /// @param count        生成数
        /// @return 生成した EnemyState のリスト
        std::vector<EnemyState> BuildEnemies(
            const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints,
            int count);

        /// @brief 現在危険度に応じた補充間隔を返す (秒)。
        float GetCurrentSpawnIntervalSec() const;

    private:
        int   m_dangerLevel;        ///< 現在の危険度 (1〜)
        int   m_peakDangerLevel;    ///< 到達済み最大危険度
        int   m_targetAliveCount;   ///< 現在の目標生存敵数
        int   m_spawnBatch;         ///< 現在の補充バッチサイズ
        int   m_spawnSerial;        ///< スポーン通番（EnemyFactory に渡す）
        float m_spawnCooldownSec;   ///< 補充クールダウン残り時間 (秒)
        float m_spawnIntervalSec;   ///< 現在の補充間隔 (秒)
    };

} // namespace Core
