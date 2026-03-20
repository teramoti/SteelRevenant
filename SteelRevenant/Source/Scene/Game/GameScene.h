#pragma once

//=============================================================================
// GameScene.h
//
// 【役割】
//   本編ゲームプレイのシーンライフサイクル（初期化・更新・描画・終了）を
//   オーケストレーションする中核クラス。
//
// 【設計方針】
//   旧 GameScene は 1 クラスに以下の責務が混在していた（神クラス問題）:
//     - ゲームロジック（戦闘・AI・経路探索）
//     - ステージ配置（中継地点・ハザード・障害物）
//     - HUD 描画（HP バー・ミニマップ・ポーズ UI）
//     - 3D 描画（背景・アリーナ・キャラクター）
//
//   リファクタ後は Single Responsibility Principle に従い、
//   各責務を専用クラスへ委譲する:
//     - Core::CombatSystem    ← ゲームロジック
//     - Core::SurvivalDirector← 動的難易度・補充制御
//     - Stage::ArenaObjective ← 中継地点・ハザード・ビーコン管理
//     - Rendering::WorldRenderer ← 3D 描画
//     - Rendering::HudRenderer   ← HUD / UI 描画
//
//   GameScene 自身は「何を・いつ・どの順で呼ぶか」だけを知り、
//   個別の処理は上記クラスへ委譲する。
//
// 【技術的アピールポイント】
//   - A* 経路探索 + NavMesh グリッドによる敵 AI ナビゲーション
//   - BehaviorTree による宣言的 AI 行動記述
//   - SurvivalDirector による動的難易度調整（DDA）
//   - Strategy パターン（IStageFloorStyle）による床テーマ切り替え
//   - テクスチャ・3D モデルなしで GeometricPrimitive のみで表現
//=============================================================================

#include <memory>
#include <vector>
#include <wrl/client.h>
#include <GeometricPrimitive.h>
#include <CommonStates.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <Keyboard.h>
#include <Mouse.h>

#include "../Base/ActionSceneBase.h"
#include "../../Core/Combat/CombatSystem.h"
#include "../../Core/Combat/SurvivalDirector.h"
#include "../../Core/Combat/GameState.h"
#include "../../Core/Navigation/PathGrid.h"
#include "../../Core/Navigation/AStarSolver.h"
#include "../../Stage/ArenaObjective.h"
#include "../../Stage/StageFloorStyle.h"
#include "../../Rendering/WorldRenderer.h"
#include "../../Rendering/HudRenderer.h"
#include "SlashHitEffectSystem.h"

//=============================================================================
// GameScene  ― 本編ゲームプレイシーン
//=============================================================================
class GameScene : public ActionSceneBase
{
public:
    /// @brief シーン管理への参照と内部状態を初期化する。
    /// @param sceneManager  シーン遷移要求先
    explicit GameScene(SceneManager* sceneManager);

    /// @brief シーンが保持する一時リソースを解放する。
    ~GameScene();

public:
    //-------------------------------------------------------------------------
    // ライフサイクル（SceneBase インタフェース実装）
    //-------------------------------------------------------------------------

    /// @brief ステージ配置・戦闘状態・描画リソースをすべて初期化する。
    void Initialize() override;

    /// @brief 入力・戦闘・演出・進行を 1 フレーム更新する。
    void Update(const DX::StepTimer& stepTimer) override;

    /// @brief 3D ワールドと HUD を描画する。
    void Render() override;

    /// @brief シーンの一時リソースと状態を破棄する。
    void Finalize() override;

private:
    //-------------------------------------------------------------------------
    // 初期化ヘルパー
    //-------------------------------------------------------------------------

    /// @brief GeometricPrimitive 群を生成して WorldRenderer へ渡す準備をする。
    void SetupGeometry();

    /// @brief 現在ステージの障害物・スポーン地点を配置し、NavMesh を構築する。
    void SetupStage();

    /// @brief ナビゲーション用グリッドと NavMesh を（再）構築する。
    void BuildNavMesh();

    /// @brief 障害物の AABB キャッシュを更新する。
    void BuildObstacleBounds();

    //-------------------------------------------------------------------------
    // 更新ヘルパー
    //-------------------------------------------------------------------------

    /// @brief このフレームの入力を InputSnapshot にまとめる。
    /// @param dt  フレーム経過時間 (秒)
    Core::InputSnapshot BuildInputSnapshot(float dt) const;

    /// @brief ポーズ切り替え入力を処理する。
    /// @return ポーズ状態が変化した場合 true
    bool HandlePauseToggle(const DirectX::Keyboard::KeyboardStateTracker& keyTracker);

    /// @brief ポーズ中の入力とメニュー状態を更新する。
    /// @return ポーズ中であれば true（以降の更新をスキップ）
    bool UpdatePausedState(
        float dt,
        const DirectX::Mouse::ButtonStateTracker& mouseTracker,
        const DirectX::Mouse::State& mouseState);

    /// @brief デバッグ用チューニングホットキーを処理する。
    void UpdateDebugTuningHotkeys(
        const DirectX::Keyboard::KeyboardStateTracker& keyTracker);

    /// @brief ロックオン対象の選択状態を更新する。
    void UpdateLockOn(const Core::InputSnapshot& input);

    /// @brief プレイヤー・敵・アリーナ目標を 1 フレーム更新する。
    void UpdateCombatFrame(float dt, const Core::InputSnapshot& input);

    /// @brief サバイバル進行と敵補充タイミングを更新する。
    void UpdateSurvivalFlow();

    /// @brief 入力とプレイヤー状態に応じてカメラを更新する。
    void UpdateCamera(float dt, const Core::InputSnapshot& input);

    //-------------------------------------------------------------------------
    // コリジョン
    //-------------------------------------------------------------------------

    /// @brief プレイヤーと障害物のめり込みを補正する。
    /// @param prevPos  移動前のプレイヤー座標
    void ResolvePlayerObstacleCollision(
        const DirectX::SimpleMath::Vector3& prevPos);

    /// @brief 指定座標が障害物内部か判定する。
    bool IsInsideObstacle(
        const DirectX::SimpleMath::Vector3& pos, float radius) const;

    /// @brief 指定座標の地面高さを返す。
    float ResolveGroundHeight(
        const DirectX::SimpleMath::Vector3& pos) const;

    //-------------------------------------------------------------------------
    // 結果・スコア
    //-------------------------------------------------------------------------

    /// @brief 現在の戦績からステージスコアを算出する。
    int ComputeStageScore() const;

    /// @brief 戦績を保存してリザルトシーンへ遷移する。
    void PushResultAndExit();

    //-------------------------------------------------------------------------
    // カメラシェイク
    //-------------------------------------------------------------------------

    /// @brief カメラシェイクの強度と継続時間を加算する。
    /// @param intensity    シェイク強度
    /// @param durationSec  継続時間 (秒)
    void AddCameraShake(float intensity, float durationSec);

private:
    //-------------------------------------------------------------------------
    // ゲームロジック（委譲先）
    //-------------------------------------------------------------------------
    Core::GameState         m_gameState;        ///< サバイバル進行状態
    Core::CombatSystem      m_combat;           ///< 戦闘ロジック
    Core::SurvivalDirector  m_survivalDirector; ///< 動的難易度・補充制御
    Navigation::PathGrid    m_grid;             ///< ナビゲーショングリッド
    Navigation::AStarSolver m_solver;           ///< A* 経路探索ソルバー

    //-------------------------------------------------------------------------
    // ゲームオブジェクト状態
    //-------------------------------------------------------------------------
    Core::PlayerState              m_player;   ///< プレイヤー実行時状態
    std::vector<Core::EnemyState>  m_enemies;  ///< 全敵の実行時状態リスト

    //-------------------------------------------------------------------------
    // ステージ・アリーナ
    //-------------------------------------------------------------------------
    Stage::ArenaObjective                    m_objective;     ///< 中継地点・ハザード・ビーコン管理
    std::unique_ptr<Stage::IStageFloorStyle> m_floorStyle;    ///< 床テーマ（Strategyパターン）
    std::vector<DirectX::SimpleMath::Matrix> m_obstacleWorlds;///< 障害物ワールド行列リスト
    std::vector<DirectX::SimpleMath::Vector3> m_spawnPoints;  ///< 敵スポーン候補座標

    //-------------------------------------------------------------------------
    // 障害物コリジョンキャッシュ
    //-------------------------------------------------------------------------
    struct ObstacleBounds
    {
        float minX; ///< AABB X 下限
        float minZ; ///< AABB Z 下限
        float maxX; ///< AABB X 上限
        float maxZ; ///< AABB Z 上限
    };
    std::vector<ObstacleBounds> m_obstacleBounds; ///< 障害物 AABB キャッシュ

    //-------------------------------------------------------------------------
    // 描画（委譲先）
    //-------------------------------------------------------------------------
    Rendering::WorldRenderer::GeometrySet m_geometry;   ///< GeometricPrimitive 群
    std::unique_ptr<Rendering::WorldRenderer> m_worldRenderer; ///< 3D 描画
    std::unique_ptr<Rendering::HudRenderer>   m_hudRenderer;   ///< HUD 描画
    SceneFx::SlashHitEffectSystem             m_slashHitEffects; ///< 斬撃エフェクト

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uiSolidTexture; ///< 単色矩形用テクスチャ

    //-------------------------------------------------------------------------
    // カメラ
    //-------------------------------------------------------------------------
    DirectX::SimpleMath::Matrix m_view;      ///< ビュー行列
    DirectX::SimpleMath::Matrix m_proj;      ///< 投影行列
    float m_cameraPitch;                     ///< カメラ仰俯角 (ラジアン)
    float m_cameraShakeTimer;                ///< シェイク残り時間 (秒)
    float m_cameraShakeStrength;             ///< シェイク強度

    //-------------------------------------------------------------------------
    // 演出・タイマー
    //-------------------------------------------------------------------------
    float m_sceneTime;          ///< シーン開始からの累積時間 (秒)
    float m_hitBloodTimer;      ///< 命中血しぶきタイマー (0-1)
    float m_damageBloodTimer;   ///< 被弾血しぶきタイマー (0-1)
    float m_hitStopTimer;       ///< ヒットストップ残り時間 (秒)
    float m_stageIntroTimer;    ///< ステージイントロ演出タイマー (秒)
    float m_finishDelay;        ///< 結果遷移までの待機時間 (秒)

    //-------------------------------------------------------------------------
    // ポーズ
    //-------------------------------------------------------------------------
    bool  m_isPaused;               ///< ポーズ中フラグ
    int   m_pauseSelectedIndex;     ///< 現在選択中のポーズメニュー項目
    float m_pauseClickFxTimer;      ///< クリックエフェクトタイマー (秒)
    DirectX::SimpleMath::Vector2 m_pauseClickFxPos; ///< クリックエフェクト座標

    //-------------------------------------------------------------------------
    // その他
    //-------------------------------------------------------------------------
    int   m_stageThemeIndex;       ///< ステージテーマ番号（床スタイル選択に使用）
    bool  m_resultPushed;          ///< リザルト遷移済みフラグ
    float m_mouseSensitivityView;  ///< デバッグ表示用マウス感度
    float m_attackAssistRangeView; ///< デバッグ表示用攻撃アシスト距離
    float m_attackAssistDotView;   ///< デバッグ表示用攻撃アシスト閾値
    bool  m_showPathDebug;         ///< A* グリッド可視化フラグ
    bool  m_showHudDetail;         ///< デバッグ HUD 表示フラグ
    int   m_recoveryBeaconUseCount;///< 回復ビーコン累積使用回数
    bool  m_resultPushed2;         ///< 二重遷移防止フラグ（内部用）

    std::vector<Navigation::PathGrid::GridCoord> m_minimapBlockedCells; ///< ミニマップ用通行不可セルキャッシュ
};
