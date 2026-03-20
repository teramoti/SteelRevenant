#pragma once

//=============================================================================
// ArenaObjective.h
//
// 【役割】
//   アリーナ内のゲーム目標オブジェクトを一括管理するクラス。
//   以前は GameScene に直接埋め込まれていた以下の責務を独立させた:
//     - 中継地点 (RelayNode) の配置と制圧進行管理
//     - 固定ハザードゾーン (HazardZone) のダメージ判定
//     - 巡回型ハザード (PatrolHazard) の移動とダメージ判定
//     - 回復ビーコン (RecoveryBeacon) のクールダウンと使用判定
//
// 【設計パターン】
//   - Single Responsibility Principle:
//       GameScene はシーンライフサイクルのみを担い、
//       アリーナロジックはこのクラスに委譲する。
//
// 【使い方】
//   1. ArenaObjective objective;
//   2. objective.Setup(stageThemeIndex);          // ステージ開始時に配置
//   3. objective.Update(dt, playerPos, enemies, gameState); // 毎フレーム更新
//   4. objective.DrawBanner(batch, uiText, ...);  // HUD バナー描画
//=============================================================================

#include <string>
#include <vector>
#include <SimpleMath.h>

#include "../Core/Combat/CombatTypes.h"
#include "../Core/Combat/GameState.h"

namespace Stage
{
    //=========================================================================
    // ArenaObjective
    //=========================================================================
    class ArenaObjective
    {
    public:
        //---------------------------------------------------------------------
        // RelayNode  ― 中継地点（制圧ポイント）
        //
        // プレイヤーが一定時間留まると制圧が進行し、
        // 規定数を制圧するとステージクリア条件のひとつを満たす。
        //---------------------------------------------------------------------
        struct RelayNode
        {
            DirectX::SimpleMath::Vector3 position;  ///< ワールド座標
            float radius;                           ///< 制圧判定半径 (m)
            float captureProgress;                  ///< 制圧進行度 (0-1)
            bool  captured;                         ///< 制圧完了フラグ
            float pulseSeed;                        ///< 描画パルスアニメ位相シード
        };

        //---------------------------------------------------------------------
        // HazardZone  ― 固定ハザードエリア
        //
        // 一定周期でウォームアップ → 活性化を繰り返し、
        // 活性中にプレイヤーがエリア内にいるとダメージを与える。
        //---------------------------------------------------------------------
        struct HazardZone
        {
            DirectX::SimpleMath::Vector3 center;    ///< エリア中心座標
            DirectX::SimpleMath::Vector2 halfExtent;///< XZ 方向の半幅 (m)
            float phaseOffsetSec;                   ///< 周期オフセット (秒)
            float warmupSec;                        ///< 活性化前の警告時間 (秒)
            float activeSec;                        ///< 活性化継続時間 (秒)
            float damagePerSec;                     ///< 活性中の毎秒ダメージ量
        };

        //---------------------------------------------------------------------
        // PatrolHazard  ― 巡回型ハザード
        //
        // start ↔ end を往復する球形ハザード。
        // 接触したプレイヤーと敵にダメージを与える。
        //---------------------------------------------------------------------
        struct PatrolHazard
        {
            DirectX::SimpleMath::Vector3 start;     ///< 巡回開始地点
            DirectX::SimpleMath::Vector3 end;       ///< 巡回終了地点
            float radius;                           ///< 接触判定半径 (m)
            float cycleSpeed;                       ///< 往復速度 (m/s)
            float damagePerSec;                     ///< 接触時毎秒ダメージ量
            float phaseSeed;                        ///< 往復アニメ位相シード
        };

        //---------------------------------------------------------------------
        // RecoveryBeacon  ― 回復ビーコン
        //
        // プレイヤーが接触するとHPを回復させ、クールダウンに入る。
        // クールダウン中は非活性表示になる。
        //---------------------------------------------------------------------
        struct RecoveryBeacon
        {
            DirectX::SimpleMath::Vector3 position;  ///< ワールド座標
            float radius;                           ///< 使用判定半径 (m)
            float cooldownSec;                      ///< 現在クールダウン残り時間 (秒)
            float maxCooldownSec;                   ///< 最大クールダウン時間 (秒)
            float pulseSeed;                        ///< 描画パルスアニメ位相シード
        };

    public:
        /// @brief 各コンテナを空にして初期状態で構築する。
        ArenaObjective();

        //---------------------------------------------------------------------
        // Setup / Reset
        //---------------------------------------------------------------------

        /// @brief ステージテーマ番号に応じてオブジェクトを配置する。
        /// @param stageThemeIndex  ステージテーマ番号 (0-2)
        void Setup(int stageThemeIndex);

        /// @brief すべてのオブジェクトを未制圧・未使用状態にリセットする。
        void Reset();

        //---------------------------------------------------------------------
        // Update
        //---------------------------------------------------------------------

        /// @brief 全オブジェクトを 1 フレーム更新する。
        /// @param dt         フレーム経過時間 (秒)
        /// @param playerPos  プレイヤーワールド座標
        /// @param playerHp   プレイヤー現在 HP（参照渡しでダメージを適用）
        /// @param sceneTime  シーン開始からの累積時間 (秒)
        void Update(
            float dt,
            const DirectX::SimpleMath::Vector3& playerPos,
            float& playerHp,
            float  sceneTime);

        //---------------------------------------------------------------------
        // HUD バナー
        //---------------------------------------------------------------------

        /// @brief 目標バナーテキストと表示タイマーを 1 フレーム更新する。
        /// @param dt  フレーム経過時間 (秒)
        void UpdateBanner(float dt);

        /// @brief 現在のバナーテキストを返す。
        /// @return 表示中のバナー文字列（空なら非表示）
        const std::wstring& GetBannerText() const { return m_bannerText; }

        /// @brief バナー表示中かどうかを返す。
        bool IsBannerVisible() const { return m_bannerTimer > 0.0f; }

        //---------------------------------------------------------------------
        // Accessors
        //---------------------------------------------------------------------

        /// @brief 中継地点リストへの読み取り参照を返す。
        const std::vector<RelayNode>& GetRelayNodes() const { return m_relayNodes; }

        /// @brief 固定ハザードゾーンリストへの読み取り参照を返す。
        const std::vector<HazardZone>& GetHazardZones() const { return m_hazardZones; }

        /// @brief 巡回型ハザードリストへの読み取り参照を返す。
        const std::vector<PatrolHazard>& GetPatrolHazards() const { return m_patrolHazards; }

        /// @brief 回復ビーコンリストへの読み取り参照を返す。
        const std::vector<RecoveryBeacon>& GetRecoveryBeacons() const { return m_recoveryBeacons; }

        /// @brief 制圧済み中継地点数を返す。
        int GetCapturedCount() const;

        /// @brief クリア条件に必要な中継地点数を返す。
        int GetRequiredCount() const { return m_requiredCount; }

        /// @brief クリア条件を満たしているか返す。
        bool IsClearConditionMet() const;

        /// @brief 回復ビーコンの累積使用回数を返す。
        int GetBeaconUseCount() const { return m_beaconUseCount; }

        /// @brief 現在の巡回型ハザードのワールド座標を返す。
        /// @param index  巡回型ハザードのインデックス
        DirectX::SimpleMath::Vector3 EvaluatePatrolPosition(size_t index, float sceneTime) const;

    private:
        //---------------------------------------------------------------------
        // 内部更新
        //---------------------------------------------------------------------

        /// @brief 中継地点の制圧進行を 1 フレーム更新する。
        /// @param dt         フレーム経過時間 (秒)
        /// @param playerPos  プレイヤーワールド座標
        void UpdateRelayNodes(float dt, const DirectX::SimpleMath::Vector3& playerPos);

        /// @brief 固定ハザードゾーンのダメージ判定を行う。
        /// @param dt         フレーム経過時間 (秒)
        /// @param playerPos  プレイヤーワールド座標
        /// @param playerHp   プレイヤー HP（参照渡しでダメージを適用）
        /// @param sceneTime  シーン累積時間 (秒)
        void UpdateHazardZones(
            float dt,
            const DirectX::SimpleMath::Vector3& playerPos,
            float& playerHp,
            float  sceneTime);

        /// @brief 巡回型ハザードの移動とダメージ判定を行う。
        /// @param dt         フレーム経過時間 (秒)
        /// @param playerPos  プレイヤーワールド座標
        /// @param playerHp   プレイヤー HP（参照渡しでダメージを適用）
        /// @param sceneTime  シーン累積時間 (秒)
        void UpdatePatrolHazards(
            float dt,
            const DirectX::SimpleMath::Vector3& playerPos,
            float& playerHp,
            float  sceneTime);

        /// @brief 回復ビーコンのクールダウンと使用判定を行う。
        /// @param dt         フレーム経過時間 (秒)
        /// @param playerPos  プレイヤーワールド座標
        /// @param playerHp   プレイヤー HP（参照渡しで回復を適用）
        void UpdateRecoveryBeacons(
            float dt,
            const DirectX::SimpleMath::Vector3& playerPos,
            float& playerHp);

        /// @brief 指定座標が固定ハザードゾーン内か判定する。
        /// @param pos        判定するワールド座標
        /// @param zoneIndex  ハザードゾーンのインデックス
        bool IsInsideHazardZone(
            const DirectX::SimpleMath::Vector3& pos,
            size_t zoneIndex) const;

        /// @brief バナーテキストを設定して表示タイマーをリセットする。
        /// @param text  表示するテキスト
        void ShowBanner(const std::wstring& text);

    private:
        std::vector<RelayNode>      m_relayNodes;     ///< 中継地点リスト
        std::vector<HazardZone>     m_hazardZones;    ///< 固定ハザードリスト
        std::vector<PatrolHazard>   m_patrolHazards;  ///< 巡回ハザードリスト
        std::vector<RecoveryBeacon> m_recoveryBeacons;///< 回復ビーコンリスト

        int m_requiredCount;        ///< クリアに必要な制圧数
        int m_beaconUseCount;       ///< 累積ビーコン使用回数

        std::wstring m_bannerText;  ///< 現在表示中の目標バナーテキスト
        float m_bannerTimer;        ///< バナー表示残り時間 (秒)
    };

} // namespace Stage
