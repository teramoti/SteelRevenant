//------------------------//------------------------
// Contents(処理内容) ゲームメインシーンを実装する。全サブシステムを統合する。
// 追加機能: ヒットストップ・スクリーンシェイク・ロックオンリング
//           コンボポップアップ・ウェーブバナー・SlashHitEffect
//------------------------//------------------------
#include "GameScene.h"
#include "../../Utility/DirectX11.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/InputManager.h"
#include "../../Utility/Sound/AudioSystem.h"
#include "../SceneManager/SceneManager.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

namespace
{
    constexpr float kGridCellSize = 1.0f;
    constexpr int   kGridHalfExt  = 30;
    constexpr float kCameraHeight = 12.0f;
    constexpr float kCameraBack   = 10.0f;
    constexpr float kHitstopSec   = 0.06f;
    constexpr float kHitFlashSec  = 0.15f;
    constexpr float kShakeOnHit   = 4.0f;
    constexpr float kShakeOnKill  = 2.0f;
}

SceneManager* g_sceneManager = nullptr;

GameScene::GameScene() = default;

void GameScene::Initialize()
{
    DirectX11& dx = DirectX11::Get();
    ID3D11Device*        device  = dx.GetDevice().Get();
    ID3D11DeviceContext* context = dx.GetContext().Get();

    m_stageIndex      = Action::BattleRuleBook::GetInstance().GetActiveStageIndex();
    m_stageThemeIndex = m_stageIndex - 1;
    m_sceneTime       = 0.0f;

    // プリミティブメッシュ生成
    m_floorMesh       = DirectX::GeometricPrimitive::CreateBox(context, {2.0f,0.15f,2.0f});
    m_playerMesh      = DirectX::GeometricPrimitive::CreateBox(context, {0.32f,1.6f,0.32f});
    m_enemyMesh       = DirectX::GeometricPrimitive::CreateBox(context, {0.30f,1.5f,0.30f});
    m_weaponMesh      = DirectX::GeometricPrimitive::CreateCylinder(context, 1.0f, 0.06f, 8);
    m_skyMesh         = DirectX::GeometricPrimitive::CreateSphere(context, 2.0f, 8, false);
    m_effectOrbMesh   = DirectX::GeometricPrimitive::CreateSphere(context, 0.3f, 8);
    m_effectTrailMesh = DirectX::GeometricPrimitive::CreateBox(context, {0.06f,0.06f,1.0f});
    m_obstacleMesh    = DirectX::GeometricPrimitive::CreateBox(context, {1.0f,1.0f,1.0f});

    m_floorStyle = CreateFloorStyle(m_stageIndex);

    m_arena.Initialize(device, context, m_stageIndex);
    m_arenaLayer.Initialize(device, context, m_stageIndex);
    m_dropSystem.Initialize(device, context);
    m_slashHitEffects.Reset();

    BuildPathGrid();

    const float startTime = Action::BattleRuleBook::GetInstance().GetStageStartTimeSec();
    m_gameState.Reset(startTime);
    m_flow.Reset(startTime);

    Action::BattleRuleBook::GetInstance().SetActiveStage(m_stageIndex);
    m_director.Reset();
    m_enemies.clear();
    m_enemyPrevStates.clear();
    m_obstacleWorlds.clear();

    SpawnInitialEnemies();

    m_player      = Action::PlayerState();
    m_lockOnPulse = 0.0f;
    (void)device;
}

void GameScene::BuildPathGrid()
{
    const int   gridSize = kGridHalfExt * 2;
    const float origin   = -static_cast<float>(kGridHalfExt);
    m_pathGrid.Initialize(gridSize, gridSize, kGridCellSize,
        DirectX::SimpleMath::Vector2(origin, origin));
    m_arena.BuildPathGrid(m_pathGrid);
    m_gridBuilt = true;
}

void GameScene::SpawnInitialEnemies()
{
    const auto& spawnPts = m_arena.GetSpawnPoints();
    if (spawnPts.empty()) return;
    auto newEnemies = m_director.BuildInitialSpawn(spawnPts);
    m_enemies.insert(m_enemies.end(), newEnemies.begin(), newEnemies.end());
}

void GameScene::ProcessSpawn()
{
    const int living = static_cast<int>(std::count_if(m_enemies.begin(), m_enemies.end(),
        [](const Action::EnemyState& e){ return e.state != Action::EnemyStateType::Dead; }));
    auto batch = m_director.BuildSpawnBatch(m_arena.GetSpawnPoints(), living);
    if (!batch.empty())
        m_enemies.insert(m_enemies.end(), batch.begin(), batch.end());
}

void GameScene::UpdateScore(float dt)
{
    m_gameState.score = m_gameState.killCount * 100
        + static_cast<int>(m_gameState.survivalTimeSec) * 2
        + m_gameState.peakDangerLevel * 50;
    (void)dt;
}

void GameScene::Update(const DX::StepTimer& timer)
{
    const float dt = static_cast<float>(timer.GetElapsedSeconds());

    m_sceneTime  += dt;
    m_hud.Update(dt);
    m_lockOnPulse += dt;
    m_slashHitEffects.Update(dt);
    if (m_hitFlashTimer > 0.0f) m_hitFlashTimer -= dt;

    if (m_hitstopTimer > 0.0f)
    {
        m_hitstopTimer -= dt;
        UpdateCamera();
        return;
    }

    m_flow.Update(dt, m_gameState.IsFinished());

    if (m_flow.GetPhase() == BattlePhase::Battle)
        UpdateBattle(dt);

    UpdateCamera();
    UpdateScore(dt);

    if (m_flow.IsReadyToExit())
    {
        if (g_sceneManager) g_sceneManager->RequestTransition(RESULT_SCENE);
    }
}

void GameScene::UpdateBattle(float dt)
{
    m_gameState.Tick(dt);

    const System::InputManager& input = System::InputManager::GetInstance();
    const Vector2 mouseDelta = input.GetMouseDelta();

    const Vector3 camFwd   = Vector3::Transform(Vector3(0,0,1), Matrix::CreateRotationY(m_player.yaw));
    const Vector3 camRight = Vector3::Transform(Vector3(1,0,0), Matrix::CreateRotationY(m_player.yaw));

    Action::InputSnapshot snap;
    snap.dt           = dt;
    snap.mouseDelta   = mouseDelta;
    snap.moveForward  = input.IsKeyDown(DirectX::Keyboard::W);
    snap.moveBack     = input.IsKeyDown(DirectX::Keyboard::S);
    snap.moveLeft     = input.IsKeyDown(DirectX::Keyboard::A);
    snap.moveRight    = input.IsKeyDown(DirectX::Keyboard::D);
    snap.attackPressed= input.IsMouseButtonPressed(0);
    snap.guardHeld    = input.IsMouseButtonDown(1);
    snap.lockTogglePressed = input.IsKeyPressed(DirectX::Keyboard::Tab);

    if (snap.lockTogglePressed)
    {
        m_player.lockEnemyIndex = (m_player.lockEnemyIndex >= 0)
            ? -1
            : m_combat.FindNearestEnemy(m_enemies, m_player.position, 15.0f);
    }

    const int prevKill = m_gameState.killCount;
    m_combat.UpdatePlayer(m_player, snap, camFwd, camRight, m_gameState);
    m_combat.ResolvePlayerAttack(m_player, m_enemies, m_gameState);

    // ヒット時: SlashHitEffect + ヒットストップ + シェイク + コンボポップアップ
    if (m_player.attackPhase == Action::PlayerAttackPhase::Active)
    {
        for (size_t i = 0; i < m_enemies.size(); ++i)
        {
            if (m_enemies[i].hitByCurrentSwing && m_enemies[i].state != Action::EnemyStateType::Dead)
            {
                m_slashHitEffects.Spawn(m_enemies[i].position, m_player.yaw,
                    DirectX::SimpleMath::Color(1.0f, 0.7f, 0.2f, 1.0f));
                m_hitstopTimer = kHitstopSec;
                m_hud.TriggerScreenShake(kShakeOnKill);
                m_hud.TriggerComboPopup(m_player.comboIndex, m_player.comboLevel);
                break;
            }
        }
    }

    if (m_gameState.killCount > prevKill)
        m_hud.TriggerScreenShake(kShakeOnKill);

    if (m_player.damageGraceTimer > 0.0f && m_hitFlashTimer <= 0.0f)
    {
        m_hitFlashTimer = kHitFlashSec;
        m_hud.TriggerScreenShake(kShakeOnHit);
    }

    const int living = static_cast<int>(std::count_if(m_enemies.begin(), m_enemies.end(),
        [](const Action::EnemyState& e){ return e.state != Action::EnemyStateType::Dead; }));

    m_director.Update(dt, m_gameState.killCount, m_gameState.survivalTimeSec, living);
    m_gameState.dangerLevel     = m_director.GetDangerLevel();
    m_gameState.peakDangerLevel = m_director.GetPeakDangerLevel();

    static int s_prevWave = 1;
    if (m_director.GetCurrentWave() != s_prevWave)
    {
        s_prevWave = m_director.GetCurrentWave();
        m_hud.TriggerWaveBanner(s_prevWave, m_director.GetTotalWaveCount());
        GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::WaveClear, 0.8f);
    }

    if (m_director.IsCompleted()) m_gameState.stageCleared = true;

    m_combat.UpdateEnemies(m_enemies, m_player, snap, m_pathGrid, m_aStar, m_gameState);
    ProcessSpawn();

    m_arenaLayer.UpdateSpeedUpItems(m_player, m_combat, dt);
    m_dropSystem.ProcessEnemyDrops(m_enemies, m_enemyPrevStates);
    m_dropSystem.UpdateDropItems(m_player, m_gameState, dt);
}

void GameScene::UpdateCamera()
{
    const float yaw = m_player.yaw;
    const Vector3 back(-std::sin(yaw)*kCameraBack, kCameraHeight, -std::cos(yaw)*kCameraBack);
    const Vector2 shake = m_hud.GetShakeOffset();

    m_cameraPos    = m_player.position + back + Vector3(shake.x*0.05f, shake.y*0.05f, 0.0f);
    m_cameraTarget = m_player.position + Vector3(0.0f, 0.8f, 0.0f);
    m_view         = Matrix::CreateLookAt(m_cameraPos, m_cameraTarget, Vector3::Up);

    const DirectX11& dx = DirectX11::Get();
    const float aspect  = static_cast<float>(dx.GetWidth()) / static_cast<float>(std::max(1, dx.GetHeight()));
    m_proj       = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI/4.0f, aspect, 0.1f, 1000.0f);
    m_projection = m_proj;
}

void GameScene::Render()
{
    // Visual/ サブファイルへ委譲
    DrawWorld();

    // HUD
    if (m_spriteBatch && m_font)
    {
        m_hud.Render(m_gameState, m_player, m_director,
            m_arenaLayer.IsSpeedActive(), m_arenaLayer.GetSpeedRemaining(),
            DirectX11::Get().GetWidth(), DirectX11::Get().GetHeight());
    }

    // カウントダウン
    if (m_flow.GetPhase() == BattlePhase::Countdown && m_font && m_spriteBatch)
    {
        const int cnt = static_cast<int>(m_flow.GetCountdownTimer()) + 1;
        wchar_t buf[8]; swprintf_s(buf, L"%d", cnt);
        const float cx = static_cast<float>(DirectX11::Get().GetWidth())  * 0.5f - 20.0f;
        const float cy = static_cast<float>(DirectX11::Get().GetHeight()) * 0.5f - 30.0f;
        m_font->DrawString(m_spriteBatch, buf,
            DirectX::XMFLOAT2(cx, cy), DirectX::Colors::White, 0.0f,
            DirectX::XMFLOAT2(0,0), 2.5f);
    }
}

void GameScene::Finalize()
{
    m_enemies.clear();
    m_enemyPrevStates.clear();
    m_obstacleWorlds.clear();
    m_slashHitEffects.Reset();
    m_floorMesh.reset();
    m_playerMesh.reset();
    m_enemyMesh.reset();
    m_weaponMesh.reset();
    m_skyMesh.reset();
    m_effectOrbMesh.reset();
    m_effectTrailMesh.reset();
    m_obstacleMesh.reset();
}
