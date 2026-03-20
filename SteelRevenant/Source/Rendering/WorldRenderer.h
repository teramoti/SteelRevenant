#pragma once

//=============================================================================
// WorldRenderer.h
//
// 【役割】
//   ゲームの 3D ワールド全体の描画を担うクラス。
//   以前は GameScene の Draw* メソッド群として散在していた描画責務を集約した:
//     - DrawWorldBackdrop : 背景（スカイ・遠景・雲）
//     - DrawWorldArena    : 床・障害物・アリーナギミック
//     - DrawWorldActors   : プレイヤー・敵・斬撃エフェクト
//
// 【設計パターン】
//   - Single Responsibility Principle:
//       GameScene は描画に必要なデータを WorldRenderContext にまとめて渡すだけ。
//       WorldRenderer はそのデータを元に描画命令を発行する。
//   - Strategy（間接利用）:
//       アリーナ床のテーマは IStageFloorStyle を介して切り替える。
//       WorldRenderer は IStageFloorStyle のインタフェースのみを参照し、
//       具体的なテーマ実装（ネオン床、砂漠床など）を知らない。
//=============================================================================

#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

#include "../Core/Combat/CombatTypes.h"
#include "../Stage/ArenaObjective.h"
#include "../Stage/StageFloorStyle.h"

namespace Rendering
{
    //=========================================================================
    // WorldRenderContext  ― 3D ワールド描画に必要な全データ
    //
    // GameScene が毎フレーム構築して WorldRenderer::Draw() へ渡す。
    // WorldRenderer はこの構造体を読み取るだけで、ゲーム状態を変更しない。
    //=========================================================================
    struct WorldRenderContext
    {
        // カメラ
        DirectX::SimpleMath::Matrix view;   ///< ビュー行列
        DirectX::SimpleMath::Matrix proj;   ///< 投影行列

        // プレイヤー・敵
        const Core::PlayerState*             player  = nullptr;
        const std::vector<Core::EnemyState>* enemies = nullptr;

        // アリーナ
        const Stage::ArenaObjective*                  objective     = nullptr;
        const Stage::IStageFloorStyle*                floorStyle    = nullptr;
        const std::vector<DirectX::SimpleMath::Matrix>* obstacleWorlds = nullptr;

        // 斬撃エフェクト
        const struct SceneFx::SlashHitEffectSystem* slashEffects = nullptr;

        // 時間・演出
        float sceneTime      = 0.0f; ///< シーン開始からの累積時間 (秒)
        float attackBlend    = 0.0f; ///< 攻撃演出補間率 (0-1)
        float hitBloodTimer  = 0.0f; ///< ヒット血しぶきタイマー (0-1)

        // デバッグ
        bool showPathDebug = false;  ///< A* グリッド可視化フラグ
        const std::vector<Navigation::PathGrid::GridCoord>* blockedCells = nullptr;
    };

    //=========================================================================
    // WorldRenderer
    //=========================================================================
    class WorldRenderer
    {
    public:
        //---------------------------------------------------------------------
        // GeometrySet  ― 描画に使う GeometricPrimitive セット
        //
        // ゲームはテクスチャ・3D モデルを使用せず、
        // DirectXTK の GeometricPrimitive（球・ボックス・円柱など）のみで
        // すべてのオブジェクトを表現する。
        // これにより「アセット管理なしでここまで作れる」という
        // エンジニアリング力を示す。
        //---------------------------------------------------------------------
        struct GeometrySet
        {
            std::unique_ptr<DirectX::GeometricPrimitive> floor;       ///< アリーナ床
            std::unique_ptr<DirectX::GeometricPrimitive> sky;         ///< スカイドーム
            std::unique_ptr<DirectX::GeometricPrimitive> player;      ///< プレイヤー
            std::unique_ptr<DirectX::GeometricPrimitive> enemy;       ///< 敵
            std::unique_ptr<DirectX::GeometricPrimitive> weapon;      ///< 武器
            std::unique_ptr<DirectX::GeometricPrimitive> obstacle;    ///< 障害物
            std::unique_ptr<DirectX::GeometricPrimitive> effectOrb;   ///< エフェクト球
            std::unique_ptr<DirectX::GeometricPrimitive> effectTrail; ///< エフェクトトレイル
            std::unique_ptr<DirectX::GeometricPrimitive> debugCell;   ///< デバッググリッド
        };

    public:
        /// @brief 描画に使うジオメトリセットを受け取って構築する。
        /// @param geometry  GameScene が生成・所有する GeometricPrimitive 群
        explicit WorldRenderer(GeometrySet& geometry);

        //---------------------------------------------------------------------
        // Draw  ― メイン描画エントリポイント
        //---------------------------------------------------------------------

        /// @brief 3D ワールド全体を描画する。
        /// @param context  このフレームの描画コンテキスト
        void Draw(const WorldRenderContext& context) const;

    private:
        //---------------------------------------------------------------------
        // 内部描画レイヤー
        //---------------------------------------------------------------------

        /// @brief 背景（スカイドーム・遠景・雲）を描画する。
        void DrawBackdrop(const WorldRenderContext& ctx) const;

        /// @brief 床・障害物・アリーナギミックを描画する。
        void DrawArena(const WorldRenderContext& ctx) const;

        /// @brief プレイヤー・敵・斬撃エフェクトを描画する。
        void DrawActors(const WorldRenderContext& ctx) const;

        //---------------------------------------------------------------------
        // 個別描画ヘルパー
        //---------------------------------------------------------------------

        /// @brief アリーナ床面を描画する（床テーマに従う）。
        void DrawFloor(const WorldRenderContext& ctx) const;

        /// @brief 障害物群をボックスで描画する。
        void DrawObstacles(const WorldRenderContext& ctx) const;

        /// @brief 中継地点マーカーを描画する。
        void DrawRelayNodes(const WorldRenderContext& ctx) const;

        /// @brief ハザードゾーンを半透明で描画する。
        void DrawHazardZones(const WorldRenderContext& ctx) const;

        /// @brief 回復ビーコンを描画する。
        void DrawRecoveryBeacons(const WorldRenderContext& ctx) const;

        /// @brief プレイヤーキャラクターを描画する。
        void DrawPlayer(const WorldRenderContext& ctx) const;

        /// @brief 全敵キャラクターを描画する。
        void DrawEnemies(const WorldRenderContext& ctx) const;

        /// @brief 敵 1 体を描画する。
        /// @param enemy      対象の敵状態
        /// @param ctx        描画コンテキスト
        void DrawSingleEnemy(
            const Core::EnemyState& enemy,
            const WorldRenderContext& ctx) const;

        /// @brief 斬撃ヒットエフェクトを描画する。
        void DrawSlashEffects(const WorldRenderContext& ctx) const;

        /// @brief デバッグ用のパスグリッドを描画する。
        void DrawPathDebug(const WorldRenderContext& ctx) const;

    private:
        GeometrySet& m_geometry; ///< 描画に使う GeometricPrimitive セット（非所有）
    };

} // namespace Rendering
