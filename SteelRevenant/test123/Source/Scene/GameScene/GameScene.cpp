#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// Contents(処理内容) ゲームメインシーンを実装する。全サブシステムを統合する。
// 追加機能: ヒットストップ・スクリーンシェイク・ロックオンリング
//           コンボポップアップ・ウェーブバナー・SlashHitEffect
//------------------------
#include "GameScene.h"
#include <chrono>

#include "../../Utility/DirectX11.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/InputManager.h"
#include <SpriteFont.h>
#include "../../Utility/SimpleMathEx.h"
#include "../../Utility/Sound/AudioSystem.h"
#include "../SceneManager/SceneManager.h"
#include "../ResultScene/ResultScene.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <vector>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

namespace
{
    constexpr float kGridCellSize = 1.0f;
    constexpr int   kGridHalfExt  = 30;
    // adjust camera to slightly angled rear/above for better forward view
constexpr float kCameraHeight = 5.0f;
constexpr float kCameraBack   = 8.0f;
constexpr float kHitstopSec   = 0.0004f;
constexpr float kKillHitstopSec = 0.0008f;
constexpr float kHitFlashSec  = 0.15f;
constexpr float kShakeOnHit   = 2.4f;
constexpr float kShakeOnKill  = 4.8f;

    void AppendPlayLog(const Action::GameState& gs, int stageIndex, float durationSec, int kills, int damage, int score)
    {
        try
        {
            std::ofstream ofs("play_log.csv", std::ios::app);
            if (!ofs) return;
            // header is not written here; consumer can inspect file
            ofs << stageIndex << ',' << durationSec << ',' << kills << ',' << damage << ',' << score << ',' << (gs.stageCleared ? 1 : 0) << ',' << (gs.timeExpired ? 1 : 0) << '\n';
        }
        catch (...) {}
    }
}

SceneManager* g_sceneManager = nullptr;

GameScene::GameScene() = default;

void GameScene::Initialize()
{
    // ステージ構成差分取得
    m_stageIndex      = Action::BattleRuleBook::GetInstance().GetActiveStageIndex();
    const auto& stageRule = Action::BattleRuleBook::GetInstance().GetStageRule(m_stageIndex);

    DirectX11& dx = DirectX11::Get();
    ID3D11Device*        device  = dx.GetDevice().Get();
    ID3D11DeviceContext* context = dx.GetContext().Get();

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
    m_survivalDirector.Reset();
    m_announcedWave = m_survivalDirector.GetCurrentWave();
    m_enemies.clear();
    m_enemyPrevStates.clear();
    m_obstacleWorlds.clear();

    SpawnInitialEnemies();

    m_player      = Action::PlayerState();
    m_lockOnPulse = 0.0f;

    // SpriteBatch は DrawManager から借用
    m_spriteBatch = System::DrawManager::GetInstance().GetSprite();

    // SpriteFont がない場合はロード
    if (!m_fontOwned)
    {
        static const wchar_t* kFontCandidates[] = {
            L"JapaneseUI_18.spritefont", L"../JapaneseUI_18.spritefont",
            L"../../JapaneseUI_18.spritefont", L"SegoeUI_18.spritefont",
            L"../SegoeUI_18.spritefont", L"../../SegoeUI_18.spritefont",
            nullptr
        };
        for (int fi = 0; kFontCandidates[fi]; ++fi)
        {
            try {
                m_fontOwned = std::make_unique<DirectX::SpriteFont>(
                    DirectX11::Get().GetDevice().Get(), kFontCandidates[fi]);
                break;
            } catch (...) {}
        }
    }
    m_font = m_fontOwned.get();

    // HUD に SpriteBatch / Font を渡す
    m_hud.Initialize(device, DirectX11::Get().GetContext().Get(), m_spriteBatch, m_font);

    // play stage-specific BGM if available
    if (m_stageIndex == 1)
    {
        GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::DefenseCorridor);
    }
    else
    {
        GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::CoreSector);
    }

    // TitleScene / StageSelectScene が MODE_ABSOLUTE に切り替えるため、ゲーム開始時に戻す
    System::InputManager::GetInstance().SetMouseSensitivity(GameSaveData::GetInstance().GetMouseSensitivity());
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
    DirectX::Mouse::Get().SetVisible(false);
    System::InputManager::GetInstance().ResetMouseDelta();
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
    auto newEnemies = m_survivalDirector.BuildInitialSpawn(spawnPts);
    m_enemies.insert(m_enemies.end(), newEnemies.begin(), newEnemies.end());
}

void GameScene::ProcessSpawn()
{
    const int living = static_cast<int>(std::count_if(m_enemies.begin(), m_enemies.end(),
        [](const Action::EnemyState& e){ return e.state != Action::EnemyStateType::Dead; }));
    auto batch = m_survivalDirector.BuildSpawnBatch(m_arena.GetSpawnPoints(), living);
    if (!batch.empty()) {
        const size_t oldSize = m_enemies.size();
        m_enemies.insert(m_enemies.end(), batch.begin(), batch.end());
    }
}

void GameScene::HandleEnemyDefeatEvents()
{
    if (m_enemyPrevStates.size() < m_enemies.size())
    {
        m_enemyPrevStates.resize(m_enemies.size(), Action::EnemyStateType::Idle);
    }

    for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
    {
        const bool wasAlive = m_enemyPrevStates[enemyIndex] != Action::EnemyStateType::Dead;
        const bool isDead = m_enemies[enemyIndex].state == Action::EnemyStateType::Dead;
        if (wasAlive && isDead)
        {
            const Action::EnemyState& enemy = m_enemies[enemyIndex];
            DirectX::SimpleMath::Color killTint(1.0f, 0.82f, 0.34f, 1.0f);
            if (enemy.isLaserEnemy)
            {
                killTint = DirectX::SimpleMath::Color(0.22f, 0.95f, 1.0f, 1.0f);
            }
            else if (enemy.isHeavyEnemy)
            {
                killTint = DirectX::SimpleMath::Color(0.72f, 0.96f, 0.74f, 1.0f);
            }

            m_slashHitEffects.SpawnKill(enemy.position, enemy.yaw, killTint);
            m_hud.TriggerKillPulse(m_gameState.killCount);
            m_hitFlashTimer = std::max(m_hitFlashTimer, kHitFlashSec + (enemy.isHeavyEnemy ? 0.08f : 0.03f));
            m_arenaLayer.TrySpawnSpeedDrop(enemy.position);
        }

        m_enemyPrevStates[enemyIndex] = m_enemies[enemyIndex].state;
    }
}

void GameScene::CompactEnemyStateArrays()
{
    if (m_enemies.empty())
    {
        m_enemyPrevStates.clear();
        m_player.lockEnemyIndex = -1;
        return;
    }

    std::vector<Action::EnemyState> livingEnemies;
    std::vector<Action::EnemyStateType> livingPrevStates;
    livingEnemies.reserve(m_enemies.size());
    livingPrevStates.reserve(m_enemies.size());
    const size_t beforeCount = m_enemies.size();

    for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
    {
        if (m_enemies[enemyIndex].state == Action::EnemyStateType::Dead)
        {
            continue;
        }

        livingEnemies.push_back(m_enemies[enemyIndex]);
        if (enemyIndex < m_enemyPrevStates.size())
        {
            livingPrevStates.push_back(m_enemyPrevStates[enemyIndex]);
        }
        else
        {
            livingPrevStates.push_back(m_enemies[enemyIndex].state);
        }
    }

    m_enemies.swap(livingEnemies);
    m_enemyPrevStates.swap(livingPrevStates);
    if (m_player.lockEnemyIndex >= static_cast<int>(m_enemies.size()) || m_enemies.size() != beforeCount)
    {
        m_player.lockEnemyIndex = -1;
    }
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
    using namespace std::chrono;
    const auto frameStart = steady_clock::now();

    const float dt = static_cast<float>(timer.GetElapsedSeconds());

    m_sceneTime  += dt;
    m_hud.Update(dt);

    const System::InputManager& input = System::InputManager::GetInstance();

    // Pause toggle handling
    if (input.IsKeyPressed(DirectX::Keyboard::Escape))
    {
        m_isPaused = !m_isPaused;
        m_pauseSelected = 0;
        if (m_isPaused)
        {
            // show cursor in pause
            DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
            DirectX::Mouse::Get().SetVisible(true);
            System::InputManager::GetInstance().ResetMouseDelta();
        }
        else
        {
            DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
            DirectX::Mouse::Get().SetVisible(false);
            System::InputManager::GetInstance().ResetMouseDelta();
        }
    }

    // If paused, process pause menu input and skip regular updates
    if (m_isPaused)
    {
        const DirectX::Mouse::State* mouseState = input.GetMouseState();
        if (mouseState != nullptr)
        {
            const float width = static_cast<float>(DirectX11::Get().GetWidth());
            const float height = static_cast<float>(DirectX11::Get().GetHeight());
            const float boxW = 420.0f;
            const float boxH = 220.0f;
            const float bx = width * 0.5f - boxW * 0.5f;
            const float by = height * 0.5f - boxH * 0.5f;
            const float mx = static_cast<float>(mouseState->x);
            const float my = static_cast<float>(mouseState->y);
            for (int index = 0; index < 4; ++index)
            {
                const float iy = by + 68.0f + static_cast<float>(index) * 36.0f;
                if (mx >= bx + 16.0f && mx <= bx + boxW - 16.0f &&
                    my >= iy - 6.0f && my <= iy + 28.0f)
                {
                    m_pauseSelected = index;
                    break;
                }
            }
        }

        // navigate pause menu
        if (input.IsKeyPressed(DirectX::Keyboard::Up) || input.IsKeyPressed(DirectX::Keyboard::W))
        {
            m_pauseSelected = (m_pauseSelected + 4 - 1) % 4;
        }
        if (input.IsKeyPressed(DirectX::Keyboard::Down) || input.IsKeyPressed(DirectX::Keyboard::S))
        {
            m_pauseSelected = (m_pauseSelected + 1) % 4;
        }

        if (input.IsKeyPressed(DirectX::Keyboard::Enter) || input.IsKeyPressed(DirectX::Keyboard::Space) || input.IsMouseButtonPressed(0))
        {
            // 0: resume, 1: title, 2: settings, 3: quit
            if (m_pauseSelected == 0)
            {
                m_isPaused = false;
                DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
                DirectX::Mouse::Get().SetVisible(false);
                System::InputManager::GetInstance().ResetMouseDelta();
            }
            else if (m_pauseSelected == 1)
            {
                if (g_sceneManager) g_sceneManager->RequestTransition(TITLE_SCENE);
                return;
            }
            else if (m_pauseSelected == 2)
            {
                if (g_sceneManager) g_sceneManager->RequestTransition(SETTINGS_SCENE);
                return;
            }
            else if (m_pauseSelected == 3)
            {
                extern void ExitGame();
                ExitGame();
                return;
            }
        }

        return;
    }

    m_lockOnPulse += dt;
    m_slashHitEffects.Update(dt);
    if (m_hitFlashTimer > 0.0f) m_hitFlashTimer -= dt;

    if (m_hitstopTimer > 0.0f)
    {
        m_hitstopTimer = std::max(0.0f, m_hitstopTimer - dt);
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
        // append run summary for tuning
        AppendPlayLog(m_gameState, m_stageIndex, m_gameState.survivalTimeSec, m_gameState.killCount, m_gameState.damageTaken, m_gameState.score);
        // pass final game state and basic meta to ResultScene for display
        ResultScene::s_lastResult = m_gameState;
        ResultScene::s_stageIndex = m_stageIndex;
        ResultScene::s_reachedWave = m_survivalDirector.GetCurrentWave();
        if (g_sceneManager) g_sceneManager->RequestTransition(RESULT_SCENE);
    }

    (void)frameStart;
}

void GameScene::UpdateBattle(float dt)
{
    m_gameState.Tick(dt);

    const System::InputManager& input = System::InputManager::GetInstance();
    const Vector2 mouseDelta = input.GetMouseDelta();

    const Vector3 camFwd   = Vector3::Transform(Vector3(0,0,1), Matrix::CreateRotationY(m_player.yaw));
    const Vector3 camRight = Utility::MathEx::SafeNormalize(camFwd.Cross(Vector3::Up));

    Action::InputSnapshot snap;
    snap.dt           = dt;
    snap.mouseDelta   = mouseDelta;
    snap.moveForward  = input.IsKeyDown(DirectX::Keyboard::W);
    snap.moveBack     = input.IsKeyDown(DirectX::Keyboard::S);
    snap.moveLeft     = input.IsKeyDown(DirectX::Keyboard::A);
    snap.moveRight    = input.IsKeyDown(DirectX::Keyboard::D);
    // slide: Shift 押下で発動 (LeftShift または RightShift)
    snap.slideHeld    = input.IsKeyDown(DirectX::Keyboard::LeftShift) || input.IsKeyDown(DirectX::Keyboard::RightShift);
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

    bool swingHit = false;
    bool swingKilled = false;
    Vector3 swingHitPosition = Vector3::Zero;
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        if (!m_enemies[i].hitByCurrentSwing)
        {
            continue;
        }
        if (!swingHit)
        {
            swingHit = true;
            swingHitPosition = m_enemies[i].position;
        }
        if (m_enemies[i].state == Action::EnemyStateType::Dead)
        {
            swingKilled = true;
        }
    }

    if (swingHit)
    {
        m_slashHitEffects.Spawn(
            swingHitPosition,
            m_player.yaw,
            DirectX::SimpleMath::Color(1.0f, 0.78f, 0.28f, 1.0f));
        m_hitstopTimer = swingKilled ? kKillHitstopSec : kHitstopSec;
        m_hud.TriggerScreenShake(swingKilled ? kShakeOnKill : kShakeOnHit);
        if (swingKilled || m_player.comboIndex >= 2 || m_player.comboLevel > 0)
        {
            m_hud.TriggerComboPopup(m_player.comboIndex, m_player.comboLevel);
        }
    }

    if (m_gameState.killCount > prevKill)
    {
        m_hitFlashTimer = kHitFlashSec; // brief flash on kill to strengthen feedback
    }

    if (m_player.damageGraceTimer > 0.0f && m_hitFlashTimer <= 0.0f)
    {
        m_hitFlashTimer = kHitFlashSec;
        m_hud.TriggerScreenShake(kShakeOnHit);
    }

    const int living = static_cast<int>(std::count_if(m_enemies.begin(), m_enemies.end(),
        [](const Action::EnemyState& e){ return e.state != Action::EnemyStateType::Dead; }));

    m_survivalDirector.Update(dt, m_gameState.killCount, m_gameState.survivalTimeSec, living);
    m_gameState.dangerLevel     = m_survivalDirector.GetDangerLevel();
    m_gameState.peakDangerLevel = m_survivalDirector.GetPeakDangerLevel();

    if (m_survivalDirector.GetCurrentWave() != m_announcedWave)
    {
        m_announcedWave = m_survivalDirector.GetCurrentWave();
        m_hud.TriggerWaveBanner(m_announcedWave, m_survivalDirector.GetTotalWaveCount());
        GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::WaveClear, 0.8f);
    }

    m_combat.UpdateEnemies(m_enemies, m_player, snap, m_pathGrid, m_aStar, m_gameState);
    HandleEnemyDefeatEvents();
    CompactEnemyStateArrays();
    ProcessSpawn();

    std::vector<DirectX::SimpleMath::Vector3> pickedItems;
    std::vector<DirectX::SimpleMath::Vector3> pickedSpeedItems;
    m_arenaLayer.UpdateSpeedUpItems(m_player, m_combat, dt, pickedSpeedItems);
    m_dropSystem.UpdateDropItems(m_player, m_gameState, dt, pickedItems);
    for (const auto& pos : pickedItems)
    {
        m_slashHitEffects.Spawn(pos, m_player.yaw, DirectX::SimpleMath::Color(0.24f, 0.88f, 0.24f, 1.0f));
        GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::ItemPickup, 1.0f);
    }
    for (const auto& pos : pickedSpeedItems)
    {
        m_slashHitEffects.Spawn(pos, m_player.yaw, DirectX::SimpleMath::Color(0.18f, 1.0f, 0.72f, 1.0f));
        GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::ItemPickup, 1.0f);
    }

    // audio for enemy events is handled in CombatSystem to avoid duplication
}

void GameScene::UpdateCamera()
{
    // camera placed behind player relative to player's facing direction
    float cameraHeight = kCameraHeight;
    const auto& stageRule = Action::BattleRuleBook::GetInstance().GetStageRule(m_stageIndex);
    if (stageRule.cameraHeightOverride > 0.0f) {
        cameraHeight = stageRule.cameraHeightOverride;
    }

    // forward vector from player yaw
    Vector3 forward = Vector3::Transform(Vector3(0, 0, 1), Matrix::CreateRotationY(m_player.yaw));
    forward = forward; // already unit-length when yaw valid

    // desired camera position: behind player along forward, elevated by cameraHeight
    const Vector3 desiredCamPos = m_player.position - forward * kCameraBack + Vector3(0.0f, cameraHeight, 0.0f);
    // look target slightly ahead of player to show forward area
    const Vector3 desiredTarget = m_player.position + forward * 6.0f + Vector3(0.0f, 0.8f, 0.0f);

    const Vector2 shake = m_hud.GetShakeOffset();

    // smooth camera position (lerp)
    const float smoothFactor = 0.12f; // 0..1, higher is snappier
    m_cameraPos = DirectX::SimpleMath::Vector3(
        m_cameraPos.x + (desiredCamPos.x + shake.x * 0.05f - m_cameraPos.x) * smoothFactor,
        m_cameraPos.y + (desiredCamPos.y + shake.y * 0.05f - m_cameraPos.y) * smoothFactor,
        m_cameraPos.z + (desiredCamPos.z - m_cameraPos.z) * smoothFactor);

    m_cameraTarget = m_player.position + Vector3(0.0f, 0.8f, 0.0f);
    // use smoothed look target as well to avoid jitter
    m_cameraTarget.x += (desiredTarget.x - m_cameraTarget.x) * smoothFactor;
    m_cameraTarget.y += (desiredTarget.y - m_cameraTarget.y) * smoothFactor;
    m_cameraTarget.z += (desiredTarget.z - m_cameraTarget.z) * smoothFactor;

    m_view = Matrix::CreateLookAt(m_cameraPos, m_cameraTarget, Vector3::Up);

    const DirectX11& dx = DirectX11::Get();
    const float aspect  = static_cast<float>(dx.GetWidth()) / static_cast<float>(std::max(1, dx.GetHeight()));
    m_proj       = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PI/4.0f, aspect, 0.1f, 1000.0f);
    m_projection = m_proj;
}

void GameScene::Render()
{
    // Visual/ サブファイルへ委譲
    DrawWorld();

    // HUD（SpriteBatch 2D描画）
    if (m_spriteBatch && m_font)
    {
        System::DrawManager::GetInstance().Begin();
        m_hud.Render(m_gameState, m_player, m_survivalDirector,
            m_arena.GetSpawnPoints(), m_arena.GetWallRects(), m_enemies,
            m_arenaLayer.IsSpeedActive(), m_arenaLayer.GetSpeedRemaining(),
            DirectX11::Get().GetWidth(), DirectX11::Get().GetHeight());

        // カウントダウン
        if (m_flow.GetPhase() == BattlePhase::Countdown && m_font)
        {
            const int cnt = static_cast<int>(m_flow.GetCountdownTimer()) + 1;
            wchar_t buf[8]; swprintf_s(buf, L"%d", cnt);
            const float cx = static_cast<float>(DirectX11::Get().GetWidth())  * 0.5f - 20.0f;
            const float cy = static_cast<float>(DirectX11::Get().GetHeight()) * 0.5f - 30.0f;
            m_font->DrawString(m_spriteBatch, buf,
                DirectX::XMFLOAT2(cx, cy), DirectX::Colors::White, 0.0f,
                DirectX::XMFLOAT2(0,0), 2.5f);
        }

        // pause modal
        if (m_isPaused)
        {
            m_hud.RenderPauseMenu(m_pauseSelected, DirectX11::Get().GetWidth(), DirectX11::Get().GetHeight());
        }

        System::DrawManager::GetInstance().End();
    }
}

void GameScene::Finalize()
{
    m_enemies.clear();
    m_enemyPrevStates.clear();
    m_obstacleWorlds.clear();
    m_slashHitEffects.Reset();
    m_fontOwned.reset();
    m_font        = nullptr;
    m_spriteBatch = nullptr;
    // タイトル等に戻る際はカーソルを解放する
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    DirectX::Mouse::Get().SetVisible(true);
    System::InputManager::GetInstance().ResetMouseDelta();
    m_floorMesh.reset();
    m_playerMesh.reset();
    m_enemyMesh.reset();
    m_weaponMesh.reset();
    m_skyMesh.reset();
    m_effectOrbMesh.reset();
    m_effectTrailMesh.reset();
    m_obstacleMesh.reset();
}
