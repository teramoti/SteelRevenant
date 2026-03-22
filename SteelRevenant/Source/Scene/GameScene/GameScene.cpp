#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// ゲームメインシーンを実装する。全サブシステムを統合する。
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
#include <filesystem>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

namespace
{
    constexpr float kGridCellSize = 1.0f;
    constexpr int   kGridHalfExt  = 30;
    // ヒットストップ無効
    constexpr float kHitstopSec     = 0.0f;
    constexpr float kKillHitstopSec = 0.0f;
    constexpr float kHitFlashSec    = 0.15f;
    constexpr float kShakeOnHit     = 2.4f;
    constexpr float kShakeOnKill    = 4.8f;
    // デフォルト画角 (ラジアン)
    constexpr float kDefaultFov = DirectX::XM_PI / 4.0f;

    // プレイログを CSV ファイルへ追記する
    void AppendPlayLog(const Action::GameState& gs, int stageIndex, float durationSec, int kills, int damage, int score)
    {
        try
        {
            const std::filesystem::path logDir = std::filesystem::path("logs");
            std::error_code ec;
            std::filesystem::create_directories(logDir, ec);
            const std::filesystem::path logPath = logDir / "play_log.csv";
            std::ofstream ofs(logPath, std::ios::app);
            if (!ofs)
            {
                return;
            }
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

    // カメラコントローラーの初期化
    {
        const float width  = static_cast<float>(DirectX11::Get().GetWidth());
        const float height = static_cast<float>(DirectX11::Get().GetHeight());
        Camera::FollowCameraSettings camSettings;
        m_cameraController.Initialize(camSettings, m_player.yaw, width, height);
    }

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

    // ステージ別 BGM を再生する
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

    m_pauseController.Reset();
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

// --- UpdateBattle から分離したサブ責務 ---

Action::InputSnapshot GameScene::BuildInputSnapshot(float dt) const
{
    const System::InputManager& input = System::InputManager::GetInstance();

    Action::InputSnapshot snap;
    snap.dt           = dt;
    snap.mouseDelta   = input.GetMouseDelta();
    snap.moveForward  = input.IsKeyDown(DirectX::Keyboard::W);
    snap.moveBack     = input.IsKeyDown(DirectX::Keyboard::S);
    snap.moveLeft     = input.IsKeyDown(DirectX::Keyboard::A);
    snap.moveRight    = input.IsKeyDown(DirectX::Keyboard::D);
    snap.slideHeld    = input.IsKeyDown(DirectX::Keyboard::LeftShift) || input.IsKeyDown(DirectX::Keyboard::RightShift);
    snap.attackPressed= input.IsMouseButtonPressed(0);
    snap.guardHeld    = input.IsMouseButtonDown(1);
    snap.lockTogglePressed = input.IsKeyPressed(DirectX::Keyboard::Tab);
    return snap;
}

void GameScene::ToggleLockOn(const Action::InputSnapshot& snap)
{
    if (!snap.lockTogglePressed)
    {
        return;
    }

    m_player.lockEnemyIndex = (m_player.lockEnemyIndex >= 0)
        ? -1
        : m_combat.FindNearestEnemy(m_enemies, m_player.position, 15.0f);
}

void GameScene::ApplySwingHitFeedback(int prevKillCount)
{
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

    // キル時の短時間フラッシュ
    if (m_gameState.killCount > prevKillCount)
    {
        m_hitFlashTimer = kHitFlashSec;
    }

    // 被ダメージ時のフラッシュ
    if (m_player.damageGraceTimer > 0.0f && m_hitFlashTimer <= 0.0f)
    {
        m_hitFlashTimer = kHitFlashSec;
        m_hud.TriggerScreenShake(kShakeOnHit);
    }
}

void GameScene::SyncWaveProgress(float dt)
{
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
}

void GameScene::CollectDropItems(float dt)
{
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
}

// --- メインループ ---

void GameScene::Update(const DX::StepTimer& timer)
{
    const float dt = static_cast<float>(timer.GetElapsedSeconds());

    m_sceneTime += dt;
    m_hud.Update(dt);

    // ポーズ切替判定
    m_pauseController.UpdateToggle();

    // ポーズ中はメニュー入力のみ処理する
    if (m_pauseController.IsPaused())
    {
        const int transition = m_pauseController.UpdateMenuInput(
            DirectX11::Get().GetWidth(), DirectX11::Get().GetHeight());
        if (transition == -2)
        {
            extern void ExitGame();
            ExitGame();
            return;
        }
        if (transition >= 0)
        {
            if (g_sceneManager) g_sceneManager->RequestTransition(static_cast<SceneID>(transition));
            return;
        }
        return;
    }

    m_lockOnPulse += dt;
    m_slashHitEffects.Update(dt);
    if (m_hitFlashTimer > 0.0f) m_hitFlashTimer -= dt;

    // ヒットストップ中はカメラのみ更新する
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

    // リザルト遷移判定
    if (m_flow.IsReadyToExit())
    {
        AppendPlayLog(m_gameState, m_stageIndex, m_gameState.survivalTimeSec, m_gameState.killCount, m_gameState.damageTaken, m_gameState.score);
        ResultScene::s_lastResult = m_gameState;
        ResultScene::s_stageIndex = m_stageIndex;
        ResultScene::s_reachedWave = m_survivalDirector.GetCurrentWave();
        if (g_sceneManager) g_sceneManager->RequestTransition(RESULT_SCENE);
    }
}

void GameScene::UpdateBattle(float dt)
{
    m_gameState.Tick(dt);

    // 入力スナップショット構築
    Action::InputSnapshot snap = BuildInputSnapshot(dt);
    ToggleLockOn(snap);

    const int prevKill = m_gameState.killCount;

    // カメラ基底からプレイヤー移動方向を算出
    const Vector3 camFwd   = m_cameraController.GetForward();
    const Vector3 camRight = m_cameraController.GetRight();
    m_combat.UpdatePlayer(m_player, snap, camFwd, camRight, m_gameState);
    m_combat.ResolvePlayerAttack(m_player, m_enemies, m_gameState);

    // 攻撃ヒット演出フィードバック
    ApplySwingHitFeedback(prevKill);

    // ウェーブ進行・危険度同期
    SyncWaveProgress(dt);

    // 敵 AI 更新
    m_combat.UpdateEnemies(m_enemies, m_player, snap, m_pathGrid, m_aStar, m_gameState);
    HandleEnemyDefeatEvents();
    CompactEnemyStateArrays();
    ProcessSpawn();

    // アイテム回収
    CollectDropItems(dt);
}

void GameScene::UpdateCamera()
{
    const auto& stageRule = Action::BattleRuleBook::GetInstance().GetStageRule(m_stageIndex);
    const DirectX11& dx   = DirectX11::Get();
    const float aspect    = static_cast<float>(dx.GetWidth()) / static_cast<float>(std::max(1, dx.GetHeight()));

    m_cameraController.Update(
        m_player.position,
        m_player.yaw,
        m_enemies,
        m_player.lockEnemyIndex,
        m_arena.GetWallRects(),
        m_hud.GetShakeOffset(),
        stageRule.cameraHeightOverride,
        aspect);

    m_view       = m_cameraController.GetView();
    m_proj       = m_cameraController.GetProj();
    m_projection = m_proj;
}

void GameScene::Render()
{
    // ワールド描画（サブファイルへ委譲）
    DrawWorld();

    // HUD（SpriteBatch 2D 描画）
    if (m_spriteBatch && m_font)
    {
        System::DrawManager::GetInstance().Begin();
        m_hud.Render(m_gameState, m_player, m_survivalDirector,
            m_arena.GetSpawnPoints(), m_arena.GetWallRects(), m_enemies,
            m_arenaLayer.IsSpeedActive(), m_arenaLayer.GetSpeedRemaining(),
            DirectX11::Get().GetWidth(), DirectX11::Get().GetHeight());

        // カウントダウン表示
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

        // ポーズメニュー描画
        if (m_pauseController.IsPaused())
        {
            m_hud.RenderPauseMenu(m_pauseController.GetSelectedIndex(), DirectX11::Get().GetWidth(), DirectX11::Get().GetHeight());
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
