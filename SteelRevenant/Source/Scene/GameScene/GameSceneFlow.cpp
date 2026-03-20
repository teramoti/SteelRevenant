//------------------------//------------------------
// Contents(処理内容) ポーズ遷移とリザルト遷移の制御を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "GameScene.h"

#include "../../Action/BattleRuleBook.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/UiUtil.h"
#include "../../Utility/SimpleMathEx.h"
#include "../SceneManager/SceneManager.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

extern void ExitGame();

namespace
{
	constexpr float kAttackVisualDurationSec = 0.18f;
	constexpr int kPauseMenuCount = 4;
	constexpr float kPauseClickFxDurationSec = 0.24f;
}

// ポーズメニューの選択状態を更新する。
void GameScene::UpdatePauseMenu(
	const DirectX::Mouse::ButtonStateTracker& mouseTracker,
	const DirectX::Mouse::State& mouseState)
{
	if (!m_isPaused)
	{
		return;
	}

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const float uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));
	const float panelWidth = std::min(500.0f * uiScale, width - 80.0f * uiScale);
	const float panelHeight = 336.0f * uiScale;
	const Vector2 panelPos((width - panelWidth) * 0.5f, (height - panelHeight) * 0.5f);
	const float optionWidth = panelWidth - 54.0f * uiScale;
	const float optionHeight = 52.0f * uiScale;
	const float optionGap = 12.0f * uiScale;
	const float optionStartY = panelPos.y + 96.0f * uiScale;
	const float optionX = panelPos.x + 27.0f * uiScale;

	const Vector2 mousePoint(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));
	bool clickedOption = false;
	for (int optionIndex = 0; optionIndex < kPauseMenuCount; ++optionIndex)
	{
		const Vector2 optionPos(optionX, optionStartY + static_cast<float>(optionIndex) * (optionHeight + optionGap));
		if (!UiUtil::IsInsideRect(mousePoint, optionPos, Vector2(optionWidth, optionHeight)))
		{
			continue;
		}

		m_pauseSelectedIndex = optionIndex;
		if (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
		{
			clickedOption = true;
			m_pauseClickFxTimer = kPauseClickFxDurationSec;
			m_pauseClickFxPos = mousePoint;
		}
		break;
	}

	if (!clickedOption)
	{
		return;
	}

	switch (m_pauseSelectedIndex)
	{
	case 0:
		m_isPaused = false;
		DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
		SetSystemCursorVisible(false);
		break;

	case 1:
		DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
		SetSystemCursorVisible(true);
		m_sceneManager->PushScene(SETTINGS_SCENE);
		break;

	case 2:
		m_isPaused = false;
		DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
		SetSystemCursorVisible(true);
		m_sceneManager->SetScene(TITLE_SCENE);
		break;

	default:
		ExitGame();
		break;
	}
}

// ポーズ画面のオーバーレイ UI を描画する。
void GameScene::DrawPauseOverlay(
	DirectX::SpriteBatch* batch,
	System::UIShaderText* uiText,
	const System::UIShaderStyle& titleStyle,
	const System::UIShaderStyle& selectedStyle,
	const System::UIShaderStyle& normalStyle,
	const System::UIShaderStyle& helpStyle,
	float width,
	float height)
{
	if (!m_isPaused || batch == nullptr || uiText == nullptr)
	{
		return;
	}

	System::UIShaderStyle selectedMenuStyle = selectedStyle;
	selectedMenuStyle.blink = false;
	selectedMenuStyle.pulseAmount = 0.0f;

	System::UIShaderStyle help = helpStyle;
	help.blink = false;

	const float uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));
	const float safeMargin = 12.0f * uiScale;

	DrawSolidRect(batch, Vector2::Zero, Vector2(width, height), Color(0.0f, 0.0f, 0.0f, 0.62f));

	const float panelWidth = std::min(500.0f * uiScale, width - 80.0f * uiScale);
	const float panelHeight = std::min(360.0f * uiScale, height - safeMargin * 2.0f);
	const float panelX = (width - panelWidth) * 0.5f;
	const float panelY = std::max(safeMargin, std::min((height - panelHeight) * 0.5f, height - panelHeight - safeMargin));
	const Vector2 panelPos(panelX, panelY);
	DrawSolidRect(batch, panelPos, Vector2(panelWidth, panelHeight), Color(0.03f, 0.05f, 0.08f, 0.92f));
	DrawSolidRect(batch, panelPos, Vector2(panelWidth, std::max(1.0f, 2.0f * uiScale)), Color(0.78f, 0.88f, 1.0f, 0.88f));
	DrawSolidRect(batch, panelPos + Vector2(0.0f, panelHeight - std::max(1.0f, 2.0f * uiScale)), Vector2(panelWidth, std::max(1.0f, 2.0f * uiScale)), Color(0.3f, 0.44f, 0.62f, 0.6f));

	uiText->Draw(batch, L"\u4e00\u6642\u505c\u6b62", panelPos + Vector2(24.0f * uiScale, 18.0f * uiScale), titleStyle, 1.38f * uiScale);
	uiText->Draw(batch, L"\u518d\u958b / \u8a2d\u5b9a / \u30bf\u30a4\u30c8\u30eb\u623b\u308a / \u7d42\u4e86", panelPos + Vector2(24.0f * uiScale, 52.0f * uiScale), normalStyle, 0.56f * uiScale);

	struct PauseEntry
	{
		const wchar_t* label;
		const wchar_t* detail;
	};

	const PauseEntry entries[kPauseMenuCount] =
	{
		{ L"01 \u30b2\u30fc\u30e0\u3078\u623b\u308b", L"\u4e00\u6642\u505c\u6b62\u3092\u89e3\u9664\u3057\u3066\u672c\u6226\u306b\u5fa9\u5e30" },
		{ L"02 \u8a2d\u5b9a", L"\u8996\u70b9\u611f\u5ea6\u3068\u64cd\u4f5c\u8a2d\u5b9a\u3092\u8abf\u6574" },
		{ L"03 \u30bf\u30a4\u30c8\u30eb\u3078", L"\u30bf\u30a4\u30c8\u30eb\u753b\u9762\u3078\u623b\u308b" },
		{ L"04 \u7d42\u4e86", L"\u30a2\u30d7\u30ea\u30b1\u30fc\u30b7\u30e7\u30f3\u3092\u7d42\u4e86" }
	};

	const float optionWidth = panelWidth - 54.0f * uiScale;
	const float optionHeight = 52.0f * uiScale;
	const float optionGap = 12.0f * uiScale;
	const float optionStartY = panelPos.y + 96.0f * uiScale;
	const float optionX = panelPos.x + 27.0f * uiScale;
	for (int optionIndex = 0; optionIndex < kPauseMenuCount; ++optionIndex)
	{
		const bool selected = (m_pauseSelectedIndex == optionIndex);
		const Vector2 optionPos(optionX, optionStartY + static_cast<float>(optionIndex) * (optionHeight + optionGap));
		DrawSolidRect(batch, optionPos, Vector2(optionWidth, optionHeight), selected ? Color(0.10f, 0.24f, 0.15f, 0.92f) : Color(0.06f, 0.09f, 0.12f, 0.84f));
		DrawSolidRect(batch, optionPos, Vector2(optionWidth, std::max(1.0f, 1.5f * uiScale)), selected ? Color(0.55f, 1.0f, 0.72f, 0.94f) : Color(0.42f, 0.56f, 0.72f, 0.68f));
		uiText->Draw(batch, entries[optionIndex].label, optionPos + Vector2(14.0f * uiScale, 6.0f * uiScale), selected ? selectedMenuStyle : normalStyle, 0.80f * uiScale);
		uiText->Draw(batch, entries[optionIndex].detail, optionPos + Vector2(14.0f * uiScale, 28.0f * uiScale), selected ? selectedMenuStyle : normalStyle, 0.60f * uiScale);
	}

	const float helpY = std::max(panelPos.y + 12.0f * uiScale, std::min(panelPos.y + panelHeight - 20.0f * uiScale, height - safeMargin - 18.0f * uiScale));
	uiText->Draw(batch, L"\u30de\u30a6\u30b9\u3067\u9078\u629e  /  \u5de6\u30af\u30ea\u30c3\u30af\u3067\u6c7a\u5b9a  /  Esc\u3067\u518d\u958b", Vector2(panelPos.x + 20.0f * uiScale, helpY), help, 0.68f * uiScale);

	if (m_pauseClickFxTimer > 0.0f)
	{
		const float progress = 1.0f - (m_pauseClickFxTimer / kPauseClickFxDurationSec);
		const float radius = (8.0f + progress * 30.0f) * uiScale;
		const float alpha = (1.0f - progress) * 0.85f;
		const Color fxColor(0.5f, 1.0f, 0.68f, alpha);
		for (int particle = 0; particle < 12; ++particle)
		{
			const float fp = static_cast<float>(particle);
			const float angle = (DirectX::XM_2PI / 12.0f) * fp + progress * 1.4f;
			const Vector2 dir(std::cosf(angle), std::sinf(angle));
			const Vector2 point = m_pauseClickFxPos + dir * radius;
			const float size = (2.0f + std::fmod(fp, 2.0f)) * uiScale;
			DrawSolidRect(batch, point - Vector2(size * 0.5f, size * 0.5f), Vector2(size, size), fxColor);
		}

		const float coreRadius = (3.0f + (1.0f - progress) * 8.0f) * uiScale;
		DrawSolidRect(batch, m_pauseClickFxPos + Vector2(-coreRadius, -uiScale), Vector2(coreRadius * 2.0f, 2.0f * uiScale), Color(0.95f, 1.0f, 0.97f, alpha * 0.88f));
		DrawSolidRect(batch, m_pauseClickFxPos + Vector2(-uiScale, -coreRadius), Vector2(2.0f * uiScale, coreRadius * 2.0f), Color(0.95f, 1.0f, 0.97f, alpha * 0.88f));
	}
}

// 現在状態からライブスコアを算出する。
int GameScene::ComputeStageScore() const
{
	const int relayCaptured = 0;
	const int rawScore =
		m_gameState.killCount * 100 +
		static_cast<int>(std::floor(std::max(0.0f, m_gameState.survivalTimeSec) * 10.0f)) +
		(m_gameState.stageCleared ? 2000 : 0) +
		relayCaptured * 500 -
		m_gameState.damageTaken * 15 -
		0 * 120;
	return std::max(0, rawScore);
}

// 集計した戦闘結果を保存して結果シーンへ進める。
void GameScene::PushResultAndExit()
{
	if (m_resultPushed)
	{
		return;
	}

	m_resultPushed = true;

	const int relayCaptured = 0;
	const int relayRequired = 0;
	m_gameState.score = ComputeStageScore();
	const bool isNewRecord = GameSaveData::GetInstance().UpdateBestScore(m_stageThemeIndex, m_gameState.score);

	BattleResultData result = {};
	result.stageIndex = m_stageThemeIndex;
	result.killCount = m_gameState.killCount;
	result.survivalTimeSec = m_gameState.survivalTimeSec;
	result.damageTaken = m_gameState.damageTaken;
	result.relayCaptured = 0;
	result.relayRequired = relayRequired;
	result.totalScore = m_gameState.score;
	result.peakDangerLevel = m_gameState.peakDangerLevel;
	result.bestScore = GameSaveData::GetInstance().GetBestScore(m_stageThemeIndex);
	result.isNewRecord = isNewRecord;
	GameSaveData::GetInstance().SetBattleResult(result);

	m_isPaused = false;
	m_sceneManager->SetScene(RESULT_SCENE);
}

// 一時的な視覚演出状態を初期値へ戻す。
void GameScene::ResetTransientVisualState()
{
	m_sceneTime = 0.0f;
	m_hitBloodTimer = 0.0f;
	m_damageBloodTimer = 0.0f;
	m_cameraShakeTimer = 0.0f;
	m_cameraShakeStrength = 0.0f;
	m_hitStopTimer = 0.0f;
	m_pauseClickFxTimer = 0.0f;
	m_pauseClickFxPos = Vector2::Zero;
	m_slashHitEffects.Reset();
}

// カメラシェイク量を加算する。
void GameScene::AddCameraShake(float intensity, float durationSec)
{
	m_cameraShakeStrength = Utility::MathEx::Clamp(std::max(m_cameraShakeStrength, intensity), 0.0f, 1.0f);
	m_cameraShakeTimer = std::max(m_cameraShakeTimer, durationSec);
}

// 本編シーンの一時リソースと状態を破棄する。
void GameScene::Finalize()
{
	if (m_isFinalized)
	{
		return;
	}
	m_isFinalized = true;
	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	SetSystemCursorVisible(true);

	m_enemies.clear();
	m_obstacleWorlds.clear();
	m_spawnPoints.clear();
	m_minimapBlockedCells.clear();
	m_obstacleBounds.clear();
	ResetTransientVisualState();
	m_mouseSensitivityView = 0.08f;
	const Action::CombatTuning& tuning = m_combat.GetTuning();
	m_attackAssistRangeView = tuning.attackAssistRangeDefault;
	m_attackAssistDotView = tuning.attackAssistDotDefault;
	m_stageThemeIndex = 1;
	m_isPaused = false;
	m_pauseSelectedIndex = 0;
	m_showHudDetail = false;

	m_floorMesh.reset();
	m_skyMesh.reset();
	m_playerMesh.reset();
	m_enemyMesh.reset();
	m_weaponMesh.reset();
	m_obstacleMesh.reset();
	m_effectOrbMesh.reset();
	m_effectTrailMesh.reset();
	m_debugCellMesh.reset();
	m_uiSolidTexture.Reset();
	m_floorStyle.reset();
	m_textRenderer.reset();
}

// 攻撃演出用の補間率を返す。
float GameScene::GetAttackBlend() const
{
	if (m_player.comboIndex <= 0 || m_player.attackPhase == Action::PlayerAttackPhase::Idle)
	{
		return 0.0f;
	}

	const Action::CombatTuning& tuning = m_combat.GetTuning();
	const float totalSec =
		std::max(0.0f, tuning.comboAttackWindupSec) +
		std::max(0.0f, tuning.comboAttackActiveSec) +
		std::max(0.0f, tuning.comboAttackFollowThroughSec) +
		std::max(0.0f, tuning.comboAttackRecoverSec);
	if (totalSec <= 0.0f)
	{
		return 0.0f;
	}

	float elapsedSec = 0.0f;
	switch (m_player.attackPhase)
	{
	case Action::PlayerAttackPhase::Windup:
		elapsedSec = tuning.comboAttackWindupSec * m_player.attackPhaseBlend;
		break;

	case Action::PlayerAttackPhase::Active:
		elapsedSec =
			tuning.comboAttackWindupSec +
			tuning.comboAttackActiveSec * m_player.attackPhaseBlend;
		break;

	case Action::PlayerAttackPhase::FollowThrough:
		elapsedSec =
			tuning.comboAttackWindupSec +
			tuning.comboAttackActiveSec +
			tuning.comboAttackFollowThroughSec * m_player.attackPhaseBlend;
		break;

	case Action::PlayerAttackPhase::Recover:
		elapsedSec =
			tuning.comboAttackWindupSec +
			tuning.comboAttackActiveSec +
			tuning.comboAttackFollowThroughSec +
			tuning.comboAttackRecoverSec * m_player.attackPhaseBlend;
		break;

	case Action::PlayerAttackPhase::Idle:
	default:
		break;
	}

	return Utility::MathEx::Clamp(elapsedSec / totalSec, 0.0f, 1.0f);
}


