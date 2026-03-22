#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <memory>
#include <vector>

#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

#include "../IScene.h"
#include "GameSceneWorldArena.h"
#include "GameSceneArenaLayer.h"
#include "GameSceneUpdate.h"
#include "GameSceneHud.h"
#include "GameSceneFlow.h"
#include "GameScenePauseController.h"
#include "FollowCameraController.h"
#include "SlashHitEffectSystem.h"
#include "StageFloorStyle.h"

#include "../../Action/AStarSolver.h"
#include "../../Action/BattleRuleBook.h"
#include "../../Action/CombatSystem.h"
#include "../../Action/GameState.h"
#include "../../Action/PathGrid.h"
#include "../../Action/SurvivalDirector.h"
#include "../../GameSystem/Camera.h"

class GameScene : public IScene
{
public:
    GameScene();
    ~GameScene() override = default;

    void Initialize() override;
    void Update(const DX::StepTimer& timer) override;
    void Render() override;
    void Finalize() override;

private:
    void UpdateBattle(float dt);
    void UpdateCamera();
    void BuildPathGrid();
    void SpawnInitialEnemies();
    void ProcessSpawn();
    void HandleEnemyDefeatEvents();
    void CompactEnemyStateArrays();
    void UpdateScore(float dt);

    // UpdateBattle から分離したサブ責務
    Action::InputSnapshot BuildInputSnapshot(float dt) const;
    void ToggleLockOn(const Action::InputSnapshot& snap);
    void ApplySwingHitFeedback(int prevKillCount);
    void SyncWaveProgress(float dt);
    void CollectDropItems(float dt);

    void DrawWorld();
    void DrawWorldBackdrop();
    void DrawWorldArena();
    void DrawWorldActors();
    void DrawSpeedUpItems();

    void SetupStage();
    void BuildObstacleBounds();
    bool IsInsideObstacle(const DirectX::SimpleMath::Vector3& position, float radius) const;
    float ResolveGroundHeight(const DirectX::SimpleMath::Vector3& position) const;
    void ResolvePlayerObstacleCollision(const DirectX::SimpleMath::Vector3& previousPosition);
    std::vector<DirectX::SimpleMath::Vector3> BuildSpawnCandidates(bool initialSpawn) const;
    void SpawnEnemyBatch(bool initialSpawn);
    int CountLivingEnemies() const;

    float GetAttackBlend() const
    {
        if (m_player.attackPhase == Action::PlayerAttackPhase::Active)
        {
            return m_player.attackPhaseBlend;
        }
        if (m_player.attackPhase == Action::PlayerAttackPhase::Windup)
        {
            return m_player.attackPhaseBlend * 0.5f;
        }
        return 0.0f;
    }

    std::unique_ptr<DirectX::GeometricPrimitive> m_floorMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_playerMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_enemyMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_weaponMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_skyMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_effectOrbMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_effectTrailMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_obstacleMesh;
    std::unique_ptr<DirectX::GeometricPrimitive> m_debugCellMesh;

    GameSceneWorldArena m_arena;
    GameSceneArenaLayer m_arenaLayer;
    GameSceneUpdate m_dropSystem;
    GameSceneHud m_hud;
    GameSceneFlow m_flow;
    GameScenePauseController m_pauseController;
    Camera::FollowCameraController m_cameraController;
    SceneFx::SlashHitEffectSystem m_slashHitEffects;
    std::unique_ptr<IFloorStyle> m_floorStyle;

    Action::CombatSystem m_combat;
    Action::PathGrid m_pathGrid;
    Action::AStarSolver m_aStar;
    Action::PlayerState m_player;
    Action::GameState m_gameState;
    std::vector<Action::EnemyState> m_enemies;
    std::vector<Action::EnemyStateType> m_enemyPrevStates;
    std::vector<DirectX::SimpleMath::Matrix> m_obstacleWorlds;

    Action::SurvivalDirector m_survivalDirector;
    Action::PathGrid m_grid;
    Action::AStarSolver m_solver;

    std::vector<DirectX::SimpleMath::Vector3> m_spawnPoints;

    struct ObstacleBounds
    {
        float minX;
        float minZ;
        float maxX;
        float maxZ;
    };

    std::vector<ObstacleBounds> m_obstacleBounds;
    std::vector<Action::PathGrid::GridCoord> m_minimapBlockedCells;

    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;
    DirectX::SimpleMath::Matrix m_projection;

    float m_sceneTime = 0.0f;
    float m_hitstopTimer = 0.0f;
    float m_hitFlashTimer = 0.0f;
    float m_lockOnPulse = 0.0f;

    int m_stageIndex = 1;
    int m_stageThemeIndex = 0;
    bool m_gridBuilt = false;
    bool m_showPathDebug = false;
    float m_scoreAccum = 0.0f;
    int m_announcedWave = 1;

    DirectX::SpriteBatch*                  m_spriteBatch = nullptr;
    std::unique_ptr<DirectX::SpriteFont>   m_fontOwned;
    DirectX::SpriteFont*                   m_font        = nullptr;
};
