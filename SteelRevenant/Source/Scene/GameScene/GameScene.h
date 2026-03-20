#pragma once
//------------------------//------------------------
// Contents(処理内容) ゲームメインシーンを宣言する。
// Visual/サブファイル (WorldActors/Arena/Backdrop) が参照する
// 全メンバ変数・メソッドをここに集約する。
//------------------------//------------------------
#include "../IScene.h"
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <GeometricPrimitive.h>

#include "GameSceneWorldArena.h"
#include "GameSceneArenaLayer.h"
#include "GameSceneUpdate.h"
#include "GameSceneHud.h"
#include "GameSceneFlow.h"
#include "SlashHitEffectSystem.h"
#include "StageFloorStyle.h"

#include "../../Action/CombatSystem.h"
#include "../../Action/PathGrid.h"
#include "../../Action/AStarSolver.h"
#include "../../Action/SurvivalDirector.h"
#include "../../Action/BattleRuleBook.h"
#include "../../Action/GameState.h"

class GameScene : public IScene
{
public:
    GameScene();
    ~GameScene() override = default;

    void Initialize() override;
    void Update(const DX::StepTimer& timer) override;
    void Render()     override;
    void Finalize()   override;

private:
    // --- 更新 ---
    void UpdateBattle(float dt);
    void UpdateCamera();
    void BuildPathGrid();
    void SpawnInitialEnemies();
    void ProcessSpawn();
    void UpdateScore(float dt);

    // --- 描画 (GameSceneWorldRender.cpp / Visual/ サブファイルから呼ばれる) ---
    void DrawWorld();
    void DrawWorldBackdrop();
    void DrawWorldArena();
    void DrawWorldActors();
    void DrawSpeedUpItems();

    // ================================================================
    // プリミティブメッシュ (Visual/ サブファイルが直接参照)
    // ================================================================
    std::unique_ptr<DirectX::GeometricPrimitive> m_floorMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_playerMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_enemyMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_weaponMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_skyMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_effectOrbMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_effectTrailMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_obstacleMesh;

    // ================================================================
    // サブシステム
    // ================================================================
    GameSceneWorldArena             m_arena;
    GameSceneArenaLayer             m_arenaLayer;
    GameSceneUpdate                 m_dropSystem;
    GameSceneHud                    m_hud;
    GameSceneFlow                   m_flow;
    SceneFx::SlashHitEffectSystem   m_slashHitEffects;
    std::unique_ptr<IFloorStyle>    m_floorStyle;

    Action::CombatSystem            m_combat;
    Action::PathGrid                m_pathGrid;
    Action::AStarSolver             m_aStar;
    Action::PlayerState             m_player;
    Action::GameState               m_gameState;
    std::vector<Action::EnemyState> m_enemies;
    std::vector<Action::EnemyStateType> m_enemyPrevStates;
    std::vector<DirectX::SimpleMath::Matrix> m_obstacleWorlds;

    Action::SurvivalDirector        m_director;

    // ================================================================
    // カメラ
    // ================================================================
    DirectX::SimpleMath::Matrix     m_view;
    DirectX::SimpleMath::Matrix     m_proj;       // Visual/側が m_proj で参照
    DirectX::SimpleMath::Matrix     m_projection; // 旧名エイリアス用（互換）
    DirectX::SimpleMath::Vector3    m_cameraPos;
    DirectX::SimpleMath::Vector3    m_cameraTarget;

    // ================================================================
    // タイミング・状態
    // ================================================================
    float m_sceneTime      = 0.0f; // Visual/サブファイルが使用するシーン累積時間
    float m_hitstopTimer   = 0.0f;
    float m_hitFlashTimer  = 0.0f;
    float m_lockOnPulse    = 0.0f;

    int   m_stageIndex     = 1;
    int   m_stageThemeIndex= 0;    // Visual/サブファイルがパレット選択に使用
    bool  m_gridBuilt      = false;
    float m_scoreAccum     = 0.0f;

    // SpriteBatch/Font は Game から借用
    DirectX::SpriteBatch* m_spriteBatch = nullptr;
    DirectX::SpriteFont*  m_font        = nullptr;
};
