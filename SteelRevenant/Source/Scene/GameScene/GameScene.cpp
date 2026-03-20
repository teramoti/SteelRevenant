//------------------------//------------------------
// Contents(処理内容) 本編ゲームシーンの共通初期化と共有状態を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "GameScene.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <cstdint>

#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/UiUtil.h"
#include "../../Action/BattleRuleBook.h"
#include "../../Utility/SimpleMathEx.h"
#include "../SceneManager/SceneManager.h"
#include "EnemyVisualProfile.h"
#include "PlayerVisualProfile.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

extern void ExitGame();

namespace
{
	constexpr float kCameraPitchMin = -0.75f;
	constexpr float kCameraPitchMax = 0.35f;
	constexpr float kAttackVisualDurationSec = 0.18f;
	constexpr float kWeaponSwingYawRad = 1.45f;
	constexpr float kWeaponSwingPitchRad = 1.15f;
	constexpr float kPlayerRunCycleSpeed = 7.6f;
	constexpr float kPlayerStrideRad = 0.38f;
	constexpr float kEnemyBreathSpeed = 4.8f;
	constexpr float kEnemySpawnDurationSec = 0.4f;
	constexpr float kEnemyAttackWindupVisualSec = 0.45f;
	constexpr float kMiniMapSizePx = 156.0f;
	constexpr float kMiniMapMarginPx = 18.0f;
	constexpr float kMiniMapMarkerEnemyPx = 2.6f;
	constexpr float kMiniMapMarkerPlayerPx = 5.0f;
	constexpr float kMiniMapPlayerHeadingPx = 9.0f;
	constexpr float kFloorPulseSpeed = 1.6f;
	constexpr float kFloorSweepSpeed = 0.42f;
	constexpr float kCameraShakeSettleSec = 0.18f;
	constexpr float kHitStopSecPerHit = 0.026f;
	constexpr float kHitStopSecMax = 0.055f;
	constexpr int kPauseMenuCount = 4;
	constexpr float kPauseClickFxDurationSec = 0.24f;
	constexpr float kObjectiveBannerDurationSec = 2.8f;
}

// ゲームシーンの共有参照と初期状態を構築する。
GameScene::GameScene(SceneManager* scenemaneger)
	: ActionSceneBase(scenemaneger, false)
	, m_cameraPitch(-0.18f)
	, m_sceneTime(0.0f)
	, m_hitBloodTimer(0.0f)
	, m_damageBloodTimer(0.0f)
	, m_mouseSensitivityView(0.08f)
	, m_attackAssistRangeView(2.6f)
	, m_attackAssistDotView(0.30f)
	, m_cameraShakeTimer(0.0f)
	, m_cameraShakeStrength(0.0f)
	, m_hitStopTimer(0.0f)
	, m_isPaused(false)
	, m_pauseSelectedIndex(0)
	, m_pauseClickFxTimer(0.0f)
	, m_pauseClickFxPos(Vector2::Zero)
	, m_recoveryBeaconUseCount(0)
	, m_requiredRelayCount(0)
	, m_objectiveBannerTimer(0.0f)
	, m_objectiveBannerText()
	, m_finishDelay(Action::BattleRuleBook::GetInstance().GetResultDelaySec())
	, m_stageThemeIndex(1)
	, m_resultPushed(false)
	, m_showPathDebug(false)
	, m_showHudDetail(false)
{
}

// ゲームシーンが保持する一時リソースを解放する。
GameScene::~GameScene()
{
	Finalize();
}

// 本編シーンの描画、戦闘、進行状態を初期化する。
void GameScene::Initialize()
{
	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
	SetSystemCursorVisible(false);

	int requestedStage = GameSaveData::GetInstance().GetStage();
	if (requestedStage < 1 || requestedStage > 3)
	{
		requestedStage = 1;
		GameSaveData::GetInstance().SetStageNum(1);
	}
	m_stageThemeIndex = requestedStage;
	Action::BattleRuleBook::GetInstance().SetActiveStage(m_stageThemeIndex);
	m_floorStyle = SceneFx::StageFloorStyleFactory::Create(m_stageThemeIndex);

	m_survivalDirector.Reset();
	m_gameState.Reset(Action::BattleRuleBook::GetInstance().GetStageStartTimeSec());
	m_gameState.dangerLevel = m_survivalDirector.GetDangerLevel();
	m_gameState.peakDangerLevel = m_survivalDirector.GetPeakDangerLevel();
	m_player = Action::PlayerState();
	switch (m_stageThemeIndex)
	{
	case 2:
		m_player.position = Vector3(0.0f, 0.8f, 0.0f);
		break;
	case 3:
		m_player.position = Vector3(0.0f, 0.8f, -8.0f);
		break;
	default:
		m_player.position = Vector3(0.0f, 0.8f, -2.5f);
		break;
	}
	m_enemies.clear();

	m_resultPushed = false;
	m_showPathDebug = false;
	m_showHudDetail = false;
	m_finishDelay = Action::BattleRuleBook::GetInstance().GetResultDelaySec();
	m_mouseSensitivityView = System::InputManager::GetInstance().GetMouseSensitivity();
	const Action::CombatTuning& tuning = m_combat.GetTuning();
	m_attackAssistRangeView = tuning.attackAssistRangeDefault;
	m_attackAssistDotView = tuning.attackAssistDotDefault;
	m_combat.SetAttackAssistConfig(m_attackAssistRangeView, m_attackAssistDotView);
	ResetTransientVisualState();
	m_isPaused = false;
	m_pauseSelectedIndex = 0;
	m_pauseClickFxTimer = 0.0f;
	m_pauseClickFxPos = Vector2::Zero;
	m_recoveryBeaconUseCount = 0;
	m_requiredRelayCount = 0;
	m_objectiveBannerTimer = kObjectiveBannerDurationSec;
	m_objectiveBannerText = Action::BattleRuleBook::GetInstance().GetActiveRule().missionSummary;

	m_floorMesh = DirectX::GeometricPrimitive::CreateCube(m_directX.GetContext().Get());
	m_skyMesh = DirectX::GeometricPrimitive::CreateSphere(m_directX.GetContext().Get(), 1.0f, 18);
	m_playerMesh = DirectX::GeometricPrimitive::CreateCylinder(m_directX.GetContext().Get(), 1.2f, 0.9f, 18);
	m_enemyMesh = DirectX::GeometricPrimitive::CreateSphere(m_directX.GetContext().Get(), 1.1f, 16);
	m_weaponMesh = DirectX::GeometricPrimitive::CreateBox(m_directX.GetContext().Get(), DirectX::XMFLOAT3(0.09f, 1.48f, 0.14f));
	m_obstacleMesh = DirectX::GeometricPrimitive::CreateBox(m_directX.GetContext().Get(), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
	m_effectOrbMesh = DirectX::GeometricPrimitive::CreateSphere(m_directX.GetContext().Get(), 1.0f, 16);
	m_effectTrailMesh = DirectX::GeometricPrimitive::CreateSphere(m_directX.GetContext().Get(), 1.0f, 12);
	m_debugCellMesh = DirectX::GeometricPrimitive::CreateCube(m_directX.GetContext().Get(), 1.0f);
	m_uiSolidTexture = UiUtil::CreateSolidTexture(m_directX.GetDevice().Get(), 0xffffffffu);

	SetupStage();
	SpawnEnemyBatch(true);
	m_gameState.score = ComputeStageScore();
	Action::InputSnapshot initialInput = {};
	UpdateCamera(0.0f, initialInput);
}

// 入力とプレイヤー状態に応じてカメラを更新する。
void GameScene::UpdateCamera(float dt, const Action::InputSnapshot& input)
{
    m_cameraPitch = Utility::MathEx::Clamp(m_cameraPitch - input.mouseDelta.y * 0.03f, kCameraPitchMin, kCameraPitchMax);

    m_proj = BuildPerspective(60.0f, 0.1f, 500.0f);

    Vector3 target = m_player.position + Vector3(0.0f, 0.8f, 0.0f);
    Vector3 forward(std::sin(m_player.yaw), std::sin(m_cameraPitch), std::cos(m_player.yaw));
    forward.Normalize();

    if (m_player.lockEnemyIndex >= 0 && m_player.lockEnemyIndex < static_cast<int>(m_enemies.size()))
    {
        const Action::EnemyState& lockEnemy = m_enemies[static_cast<size_t>(m_player.lockEnemyIndex)];
        if (lockEnemy.state != Action::EnemyStateType::Dead)
        {
            const Vector3 enemyPos = lockEnemy.position + Vector3(0.0f, 0.8f, 0.0f);
            target = (target + enemyPos) * 0.5f;
            Vector3 dir = target - m_player.position;
            if (dir.LengthSquared() > 0.0001f)
            {
                dir.Normalize();
                forward = dir;
            }
        }
    }

    Vector3 cameraPos = m_player.position - Vector3(forward.x, 0.0f, forward.z) * 6.0f + Vector3(0.0f, 3.2f, 0.0f);

    if (m_cameraShakeTimer > 0.0f && m_cameraShakeStrength > 0.0f)
    {
        const float shakeFade = Utility::MathEx::Clamp(m_cameraShakeTimer / kCameraShakeSettleSec, 0.0f, 1.0f);
        const float amp = m_cameraShakeStrength * (0.04f + shakeFade * 0.05f);
        const float t = m_sceneTime + dt * 0.5f;
        const Vector3 shakeOffset(
            std::sinf(t * 37.0f) * amp,
            std::cosf(t * 53.0f) * amp * 0.72f,
            std::sinf(t * 29.0f + 1.2f) * amp * 0.45f);
        cameraPos += shakeOffset;
        target += shakeOffset * 0.2f;
    }

    m_view = Matrix::CreateLookAt(cameraPos, target, Vector3::Up);
}

// 本編シーンのワールド描画と UI 描画を実行する。
void GameScene::Render()
{
    DrawWorld();
    DrawUI();
}

// 残り時間を MM:SS 形式の文字列へ整形する。
std::wstring GameScene::BuildTimerMMSS(float seconds) const
{
    const float safeSeconds = std::max(0.0f, seconds);
    const int totalSeconds = static_cast<int>(std::ceil(safeSeconds - 0.0001f));
    const int minutes = totalSeconds / 60;
    const int remainSeconds = totalSeconds % 60;

    std::wstringstream ss;
    ss << std::setfill(L'0') << std::setw(2) << minutes << L":" << std::setw(2) << remainSeconds;
    return ss.str();
}

// 単色矩形を UI 描画用に描く。
void GameScene::DrawSolidRect(
    DirectX::SpriteBatch* batch,
    const Vector2& position,
    const Vector2& size,
    const Color& color) const
{
	if (batch == nullptr || m_uiSolidTexture == nullptr)
	{
		return;
	}
	if (size.x <= 0.0f || size.y <= 0.0f)
	{
		return;
	}

	const DirectX::XMFLOAT2 scale(size.x, size.y);
	batch->Draw(m_uiSolidTexture.Get(), position, nullptr, color, 0.0f, Vector2::Zero, scale);
}


