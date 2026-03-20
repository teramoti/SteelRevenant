//------------------------//------------------------
// Contents(処理内容) HUD と目的表示の描画処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "GameScene.h"

#include "../../Action/BattleRuleBook.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/UiUtil.h"
#include "../../Utility/SimpleMathEx.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kMiniMapMarginPx = 18.0f;
	constexpr float kMiniMapMarkerEnemyPx = 2.6f;
	constexpr float kMiniMapMarkerPlayerPx = 5.0f;
	constexpr float kMiniMapPlayerHeadingPx = 9.0f;
	constexpr float kObjectiveBannerDurationSec = 2.8f;
	constexpr float kStageIntroDurationSec = 1.15f;
}

// HUD とポーズ UI を描画する。
void GameScene::DrawUI()
{
	System::UIShaderText* uiText = EnsureTextRenderer();
	if (uiText == nullptr)
	{
		return;
	}

	DirectX::SpriteBatch* batch = System::DrawManager::GetInstance().GetSprite();
	if (batch == nullptr)
	{
		return;
	}

	BeginSpriteLayer();

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const float uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));

	System::UIShaderStyle hudStyle;
	hudStyle.baseColor = Color(0.95f, 0.95f, 0.95f, 1.0f);
	hudStyle.outlineColor = Color(0.05f, 0.05f, 0.05f, 1.0f);
	hudStyle.pulseAmount = 0.0f;

	System::UIShaderStyle warningStyle;
	warningStyle.baseColor = Color(1.0f, 0.35f, 0.35f, 1.0f);
	warningStyle.outlineColor = Color(0.1f, 0.0f, 0.0f, 1.0f);
	warningStyle.pulseAmount = 0.0f;
	warningStyle.pulseSpeed = 0.0f;
	warningStyle.blink = false;
	warningStyle.blinkPeriod = 0.0f;

	System::UIShaderStyle timerStyle = hudStyle;
	timerStyle.baseColor = Color(1.0f, 0.95f, 0.75f, 1.0f);
	timerStyle.outlineColor = Color(0.12f, 0.09f, 0.0f, 1.0f);
	timerStyle.pulseAmount = 0.0f;
	timerStyle.pulseSpeed = 0.0f;

	const std::wstring timerText = BuildTimerMMSS(m_gameState.stageTimer);
	const std::wstring killText = L"\u6483\u7834 " + std::to_wstring(m_gameState.killCount);
	const std::wstring dangerText = L"DANGER LV " + std::to_wstring(m_gameState.dangerLevel);
	const std::wstring stageText = Action::BattleRuleBook::GetInstance().GetActiveRule().missionName;
	const std::wstring sensText = L"\u8996\u70b9\u611f\u5ea6 " + UiUtil::ToWStringFixed(m_mouseSensitivityView, 2) + L"  (PgUp/PgDn)";
	const std::wstring assistRangeText = L"\u88dc\u6b63\u8ddd\u96e2 " + UiUtil::ToWStringFixed(m_attackAssistRangeView, 2) + L"  (Ins/Del)";
	const std::wstring assistAngleText = L"\u88dc\u6b63\u89d2\u5ea6 " + UiUtil::ToWStringFixed(m_attackAssistDotView, 2) + L"  (Home/End)";
	const std::wstring comboText = L"\u30b3\u30f3\u30dc \u6bb5\u968e " + std::to_wstring(m_player.comboLevel) + L"  " + UiUtil::ToWStringFixed(m_player.comboGauge, 0) + L"%";

	const Vector2 coreHudPos(18.0f * uiScale, 16.0f * uiScale);
	const Vector2 coreHudSize(248.0f * uiScale, 76.0f * uiScale);
	DrawSolidRect(batch, coreHudPos, coreHudSize, Color(0.02f, 0.04f, 0.07f, 0.74f));
	DrawSolidRect(batch, coreHudPos, Vector2(coreHudSize.x, std::max(1.0f, 2.0f * uiScale)), Color(0.92f, 0.95f, 1.0f, 0.86f));

	if (m_uiSolidTexture != nullptr)
	{
		const Vector2 clockCenter = coreHudPos + Vector2(24.0f * uiScale, 23.0f * uiScale);
		const float clockRadius = 11.5f * uiScale;
		const auto drawClockBar =
			[&](const Vector2& center, const Vector2& size, float rotation, const Color& color)
			{
				batch->Draw(
					m_uiSolidTexture.Get(),
					center,
					nullptr,
					color,
					rotation,
					Vector2(0.5f, 0.5f),
					DirectX::XMFLOAT2(size.x, size.y));
			};

		for (int tick = 0; tick < 8; ++tick)
		{
			const float tickAngle = (DirectX::XM_2PI / 8.0f) * static_cast<float>(tick);
			const Vector2 tickDir(std::sinf(tickAngle), -std::cosf(tickAngle));
			const Vector2 tickCenter = clockCenter + tickDir * (clockRadius - 1.8f * uiScale);
			drawClockBar(tickCenter, Vector2(2.0f * uiScale, 4.0f * uiScale), tickAngle, Color(0.94f, 0.97f, 1.0f, 0.90f));
		}

		drawClockBar(clockCenter, Vector2(2.2f * uiScale, 12.0f * uiScale), DirectX::XM_PIDIV2, Color(0.94f, 0.97f, 1.0f, 0.94f));
		drawClockBar(clockCenter + Vector2(-1.2f * uiScale, 1.8f * uiScale), Vector2(2.6f * uiScale, 8.0f * uiScale), -0.78f, Color(0.94f, 0.97f, 1.0f, 0.94f));
		DrawSolidRect(batch, clockCenter - Vector2(1.7f * uiScale, 1.7f * uiScale), Vector2(3.4f * uiScale, 3.4f * uiScale), Color(0.94f, 0.97f, 1.0f, 0.96f));
	}

	uiText->Draw(batch, L"\u6b8b\u308a\u6642\u9593", coreHudPos + Vector2(74.0f * uiScale, 8.0f * uiScale), hudStyle, 0.64f * uiScale);
	uiText->Draw(batch, timerText, coreHudPos + Vector2(74.0f * uiScale, 21.0f * uiScale), timerStyle, 1.28f * uiScale);

	const float safeMargin = 12.0f * uiScale;
	const Vector2 progressPanelSize(252.0f * uiScale, 106.0f * uiScale);
	const float progressPanelX = std::max(safeMargin, width - safeMargin - progressPanelSize.x);
	const Vector2 progressPanelPos(progressPanelX, std::max(safeMargin, 16.0f * uiScale));
	DrawSolidRect(batch, progressPanelPos, progressPanelSize, Color(0.02f, 0.04f, 0.07f, 0.74f));
	DrawSolidRect(batch, progressPanelPos, Vector2(progressPanelSize.x, std::max(1.0f, 2.0f * uiScale)), Color(0.55f, 0.88f, 1.0f, 0.86f));
	uiText->Draw(batch, L"\u6226\u95d8\u9032\u884c", progressPanelPos + Vector2(14.0f * uiScale, 10.0f * uiScale), hudStyle, 0.60f * uiScale);
	uiText->Draw(batch, stageText, progressPanelPos + Vector2(14.0f * uiScale, 28.0f * uiScale), hudStyle, 0.72f * uiScale);
	uiText->Draw(batch, dangerText, progressPanelPos + Vector2(14.0f * uiScale, 49.0f * uiScale), timerStyle, 0.86f * uiScale);
	uiText->Draw(batch, killText, progressPanelPos + Vector2(14.0f * uiScale, 76.0f * uiScale), hudStyle, 0.66f * uiScale);

	if (GetRequiredRelayCount() > 0)
	{
		const std::wstring relayText = L"Relay " + std::to_wstring(GetCapturedRelayCount()) + L" / " + std::to_wstring(GetRequiredRelayCount());
		const Vector2 relayPanelSize(252.0f * uiScale, 102.0f * uiScale);
		const float relayPanelX = std::max(safeMargin, width - safeMargin - relayPanelSize.x);
		const Vector2 relayPanelPos(relayPanelX, progressPanelPos.y + progressPanelSize.y + 8.0f * uiScale);
		DrawSolidRect(batch, relayPanelPos, relayPanelSize, Color(0.02f, 0.05f, 0.08f, 0.72f));
		DrawSolidRect(batch, relayPanelPos, Vector2(relayPanelSize.x, std::max(1.0f, 2.0f * uiScale)), Color(0.35f, 0.95f, 0.78f, 0.86f));
		uiText->Draw(batch, L"\u526f\u76ee\u6a19", relayPanelPos + Vector2(14.0f * uiScale, 10.0f * uiScale), hudStyle, 0.56f * uiScale);
		uiText->Draw(batch, relayText, relayPanelPos + Vector2(14.0f * uiScale, 32.0f * uiScale), timerStyle, 0.78f * uiScale);
		uiText->Draw(batch, L"Relay\u5236\u5727 / \u88dc\u7d66 / \u5ef6\u547d", relayPanelPos + Vector2(14.0f * uiScale, 56.0f * uiScale), hudStyle, 0.46f * uiScale);
		const float relayRatio = Utility::MathEx::Clamp(static_cast<float>(GetCapturedRelayCount()) / static_cast<float>(std::max(1, GetRequiredRelayCount())), 0.0f, 1.0f);
		DrawSolidRect(batch, relayPanelPos + Vector2(14.0f * uiScale, 80.0f * uiScale), Vector2(relayPanelSize.x - 28.0f * uiScale, 7.0f * uiScale), Color(0.08f, 0.08f, 0.1f, 0.88f));
		DrawSolidRect(batch, relayPanelPos + Vector2(14.0f * uiScale, 80.0f * uiScale), Vector2((relayPanelSize.x - 28.0f * uiScale) * relayRatio, 7.0f * uiScale), Color(0.34f, 1.0f, 0.58f, 0.92f));
	}
	if (m_showHudDetail)
	{
		const Vector2 detailPos(18.0f * uiScale, height - 106.0f * uiScale);
		const Vector2 detailSize(332.0f * uiScale, 86.0f * uiScale);
		DrawSolidRect(batch, detailPos, detailSize, Color(0.03f, 0.05f, 0.08f, 0.5f));
		DrawSolidRect(batch, detailPos, Vector2(detailSize.x, std::max(1.0f, 1.5f * uiScale)), Color(0.7f, 0.86f, 1.0f, 0.62f));
		uiText->Draw(batch, sensText, detailPos + Vector2(10.0f * uiScale, 10.0f * uiScale), hudStyle, 0.64f * uiScale);
		uiText->Draw(batch, assistRangeText, detailPos + Vector2(10.0f * uiScale, 30.0f * uiScale), hudStyle, 0.62f * uiScale);
		uiText->Draw(batch, assistAngleText, detailPos + Vector2(10.0f * uiScale, 48.0f * uiScale), hudStyle, 0.62f * uiScale);
		uiText->Draw(batch, comboText, detailPos + Vector2(10.0f * uiScale, 66.0f * uiScale), hudStyle, 0.58f * uiScale);
	}
	else
	{
		const float safeMargin = 12.0f * uiScale;
		const float helpX = std::max(safeMargin, width - 124.0f * uiScale);
		const float helpY = std::max(safeMargin, std::min(height - 22.0f * uiScale, height - safeMargin - 18.0f * uiScale));
		uiText->Draw(batch, L"F4: \u8a73\u7d30HUD", Vector2(helpX, helpY), hudStyle, 0.44f * uiScale);
	}

	DrawMiniMap(batch, uiText, hudStyle);

	if (m_objectiveBannerTimer > 0.0f && !m_objectiveBannerText.empty())
	{
		const float t = Utility::MathEx::Clamp(m_objectiveBannerTimer / kObjectiveBannerDurationSec, 0.0f, 1.0f);
		const float alpha = 0.22f + t * 0.62f;
		const float bannerWidth = std::min(560.0f * uiScale, width - 54.0f * uiScale);
		const float bannerLeft = (width - bannerWidth) * 0.5f;
		DrawSolidRect(batch, Vector2(bannerLeft, 40.0f * uiScale), Vector2(bannerWidth, 40.0f * uiScale), Color(0.02f, 0.08f, 0.1f, alpha));
		System::UIShaderStyle bannerStyle = warningStyle;
		bannerStyle.baseColor = Color(0.86f, 0.98f, 1.0f, 1.0f);
		bannerStyle.outlineColor = Color(0.02f, 0.04f, 0.06f, 1.0f);
		bannerStyle.blink = false;
		uiText->Draw(batch, m_objectiveBannerText, Vector2(bannerLeft + 14.0f * uiScale, 50.0f * uiScale), bannerStyle, 0.66f * uiScale);
	}

	if (m_showPathDebug)
	{
		uiText->Draw(batch, L"\u7d4c\u8def\u30c7\u30d0\u30c3\u30b0 ON  F3", Vector2(24.0f * uiScale, 200.0f * uiScale), warningStyle, 0.68f * uiScale);
	}

	if (m_player.lockEnemyIndex >= 0)
	{
		uiText->Draw(batch, L"\u30ed\u30c3\u30af\u30aa\u30f3", Vector2(width * 0.5f - 22.0f * uiScale, height * 0.38f), warningStyle, 0.76f * uiScale);
	}

	if (m_gameState.timeExpired)
	{
		DrawSolidRect(batch, Vector2(width * 0.5f - 180.0f * uiScale, height * 0.41f), Vector2(360.0f * uiScale, 82.0f * uiScale), Color(0.10f, 0.02f, 0.02f, 0.66f));
		uiText->Draw(batch, L"TIME OVER", Vector2(width * 0.5f - 126.0f * uiScale, height * 0.45f), warningStyle, 1.24f * uiScale);
	}

	if (m_stageIntroTimer > 0.0f)
	{
		const float introT = Utility::MathEx::Clamp(m_stageIntroTimer / kStageIntroDurationSec, 0.0f, 1.0f);
		const float fadeAlpha = (introT > 0.55f)
			? Utility::MathEx::Clamp((introT - 0.55f) / 0.45f, 0.0f, 1.0f)
			: Utility::MathEx::Clamp(introT / 0.55f, 0.0f, 1.0f);
		DrawSolidRect(batch, Vector2::Zero, Vector2(width, height), Color(0.01f, 0.02f, 0.03f, 0.78f * fadeAlpha));

		System::UIShaderStyle introTitleStyle = timerStyle;
		introTitleStyle.blink = false;
		introTitleStyle.pulseAmount = 0.0f;
		System::UIShaderStyle introBodyStyle = hudStyle;
		introBodyStyle.blink = false;
		introBodyStyle.pulseAmount = 0.0f;

		const Action::StageRuleDefinition& activeRule = Action::BattleRuleBook::GetInstance().GetActiveRule();
		const float panelWidth = std::min(520.0f * uiScale, width - 48.0f * uiScale);
		const float panelHeight = 112.0f * uiScale;
		const Vector2 panelPos((width - panelWidth) * 0.5f, (height - panelHeight) * 0.5f - 18.0f * uiScale);
		DrawSolidRect(batch, panelPos, Vector2(panelWidth, panelHeight), Color(0.02f, 0.05f, 0.08f, 0.84f * fadeAlpha));
		DrawSolidRect(batch, panelPos, Vector2(panelWidth, std::max(1.0f, 2.0f * uiScale)), Color(0.70f, 0.88f, 1.0f, 0.90f * fadeAlpha));
		uiText->Draw(batch, L"DEPLOYING", panelPos + Vector2(18.0f * uiScale, 14.0f * uiScale), introBodyStyle, 0.58f * uiScale);
		uiText->Draw(batch, activeRule.missionName, panelPos + Vector2(18.0f * uiScale, 36.0f * uiScale), introTitleStyle, 1.08f * uiScale);
		uiText->Draw(batch, activeRule.missionSummary, panelPos + Vector2(18.0f * uiScale, 70.0f * uiScale), introBodyStyle, 0.60f * uiScale);
	}

	if (m_isPaused && !m_gameState.IsFinished())
	{
		DrawPauseOverlay(batch, uiText, timerStyle, warningStyle, hudStyle, warningStyle, width, height);
	}

	EndSpriteLayer();
}

// ワールド座標をミニマップ座標へ変換する。
Vector2 GameScene::ToMiniMapPoint(
	const Vector3& worldPos,
	const Vector2& mapTopLeft,
	float mapSize) const
{
	const int gridWidth = std::max(1, m_grid.GetWidth());
	const int gridHeight = std::max(1, m_grid.GetHeight());

	Action::PathGrid::GridCoord coord = m_grid.WorldToGrid(worldPos);
	coord.x = Utility::MathEx::Clamp(coord.x, 0, gridWidth - 1);
	coord.y = Utility::MathEx::Clamp(coord.y, 0, gridHeight - 1);

	const float normalizedX = (static_cast<float>(coord.x) + 0.5f) / static_cast<float>(gridWidth);
	const float normalizedY = (static_cast<float>(coord.y) + 0.5f) / static_cast<float>(gridHeight);

	return Vector2(
		mapTopLeft.x + normalizedX * mapSize,
		mapTopLeft.y + (1.0f - normalizedY) * mapSize);
}

// ミニマップと各種マーカーを描画する。
void GameScene::DrawMiniMap(DirectX::SpriteBatch* batch, System::UIShaderText* uiText, const System::UIShaderStyle& style)
{
	if (batch == nullptr || uiText == nullptr)
	{
		return;
	}

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const float uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));
	const float miniMapSize = std::min(142.0f, static_cast<float>(std::max(320, m_directX.GetWidth())) * 0.23f);
	const float safeMargin = 12.0f * uiScale;
	const float miniMapMargin = kMiniMapMarginPx * uiScale;
	const float mapLeft = std::max(safeMargin, width - safeMargin - miniMapSize);
	const float mapTop = std::max(safeMargin, height - safeMargin - miniMapSize);
	const Vector2 mapTopLeft(mapLeft, mapTop);

	DrawSolidRect(batch, Vector2(mapLeft - 8.0f * uiScale, mapTop - 32.0f * uiScale), Vector2(miniMapSize + 16.0f * uiScale, miniMapSize + 40.0f * uiScale), Color(0.02f, 0.03f, 0.05f, 0.62f));
	DrawSolidRect(batch, mapTopLeft, Vector2(miniMapSize, miniMapSize), Color(0.05f, 0.09f, 0.13f, 0.74f));
	DrawSolidRect(batch, Vector2(mapLeft, mapTop), Vector2(miniMapSize, 1.5f), Color(0.77f, 0.85f, 1.0f, 0.78f));
	DrawSolidRect(batch, Vector2(mapLeft, mapTop + miniMapSize - 1.5f), Vector2(miniMapSize, 1.5f), Color(0.77f, 0.85f, 1.0f, 0.78f));
	DrawSolidRect(batch, Vector2(mapLeft, mapTop), Vector2(1.5f, miniMapSize), Color(0.77f, 0.85f, 1.0f, 0.78f));
	DrawSolidRect(batch, Vector2(mapLeft + miniMapSize - 1.5f, mapTop), Vector2(1.5f, miniMapSize), Color(0.77f, 0.85f, 1.0f, 0.78f));
	uiText->Draw(batch, L"\u30df\u30cb\u30de\u30c3\u30d7", Vector2(mapLeft + 8.0f * uiScale, mapTop - 24.0f * uiScale), style, 0.68f * uiScale);

	const int gridWidth = std::max(1, m_grid.GetWidth());
	const int gridHeight = std::max(1, m_grid.GetHeight());
	const float cellWidth = miniMapSize / static_cast<float>(gridWidth);
	const float cellHeight = miniMapSize / static_cast<float>(gridHeight);

	for (size_t i = 0; i < m_minimapBlockedCells.size(); ++i)
	{
		const Action::PathGrid::GridCoord& block = m_minimapBlockedCells[i];
		const float x = mapLeft + static_cast<float>(block.x) * cellWidth;
		const float y = mapTop + static_cast<float>(gridHeight - 1 - block.y) * cellHeight;
		DrawSolidRect(batch, Vector2(x, y), Vector2(cellWidth + 0.5f, cellHeight + 0.5f), Color(0.23f, 0.27f, 0.31f, 0.95f));
	}

	int livingEnemies = 0;
	for (size_t i = 0; i < m_enemies.size(); ++i)
	{
		const Action::EnemyState& enemy = m_enemies[i];
		if (enemy.state == Action::EnemyStateType::Dead)
		{
			continue;
		}

		++livingEnemies;
		const Vector2 enemyPoint = ToMiniMapPoint(enemy.position, mapTopLeft, miniMapSize);
		DrawSolidRect(
			batch,
			enemyPoint - Vector2(kMiniMapMarkerEnemyPx, kMiniMapMarkerEnemyPx),
			Vector2(kMiniMapMarkerEnemyPx * 2.0f, kMiniMapMarkerEnemyPx * 2.0f),
			Color(0.95f, 0.28f, 0.28f, 0.95f));
	}

	for (size_t relayIndex = 0; relayIndex < m_relayNodes.size(); ++relayIndex)
	{
		const RelayNodeInfo& relay = m_relayNodes[relayIndex];
		const Vector2 relayPoint = ToMiniMapPoint(relay.position, mapTopLeft, miniMapSize);
		const Color relayColor = relay.captured ? Color(0.34f, 1.0f, 0.58f, 0.96f) : Color(0.32f, 0.92f, 1.0f, 0.94f);
		DrawSolidRect(batch, relayPoint - Vector2(4.2f, 4.2f), Vector2(8.4f, 8.4f), relayColor);
	}

	for (size_t beaconIndex = 0; beaconIndex < m_recoveryBeacons.size(); ++beaconIndex)
	{
		const RecoveryBeaconInfo& beacon = m_recoveryBeacons[beaconIndex];
		const Vector2 beaconPoint = ToMiniMapPoint(beacon.position, mapTopLeft, miniMapSize);
		const Color beaconColor = (beacon.cooldownSec <= 0.0f) ? Color(0.5f, 1.0f, 0.92f, 0.96f) : Color(0.46f, 0.62f, 0.74f, 0.82f);
		DrawSolidRect(batch, beaconPoint - Vector2(3.2f, 3.2f), Vector2(6.4f, 6.4f), beaconColor);
	}

	for (size_t patrolIndex = 0; patrolIndex < m_patrolHazards.size(); ++patrolIndex)
	{
		const Vector2 patrolPoint = ToMiniMapPoint(EvaluatePatrolHazardPosition(patrolIndex), mapTopLeft, miniMapSize);
		DrawSolidRect(batch, patrolPoint - Vector2(3.8f, 3.8f), Vector2(7.6f, 7.6f), Color(1.0f, 0.72f, 0.18f, 0.96f));
	}

	if (m_player.lockEnemyIndex >= 0 && m_player.lockEnemyIndex < static_cast<int>(m_enemies.size()))
	{
		const Action::EnemyState& lockEnemy = m_enemies[static_cast<size_t>(m_player.lockEnemyIndex)];
		if (lockEnemy.state != Action::EnemyStateType::Dead)
		{
			const Vector2 lockPoint = ToMiniMapPoint(lockEnemy.position, mapTopLeft, miniMapSize);
			DrawSolidRect(batch, lockPoint - Vector2(5.5f, 5.5f), Vector2(11.0f, 1.4f), Color(1.0f, 0.95f, 0.2f, 0.92f));
			DrawSolidRect(batch, lockPoint - Vector2(5.5f, -4.1f), Vector2(11.0f, 1.4f), Color(1.0f, 0.95f, 0.2f, 0.92f));
			DrawSolidRect(batch, lockPoint - Vector2(5.5f, 5.5f), Vector2(1.4f, 11.0f), Color(1.0f, 0.95f, 0.2f, 0.92f));
			DrawSolidRect(batch, lockPoint - Vector2(-4.1f, 5.5f), Vector2(1.4f, 11.0f), Color(1.0f, 0.95f, 0.2f, 0.92f));
		}
	}

	const Vector2 playerPoint = ToMiniMapPoint(m_player.position, mapTopLeft, miniMapSize);
	DrawSolidRect(
		batch,
		playerPoint - Vector2(kMiniMapMarkerPlayerPx, kMiniMapMarkerPlayerPx),
		Vector2(kMiniMapMarkerPlayerPx * 2.0f, kMiniMapMarkerPlayerPx * 2.0f),
		Color(0.32f, 1.0f, 0.46f, 1.0f));

	const Vector2 heading(std::sin(m_player.yaw), -std::cos(m_player.yaw));
	DrawSolidRect(batch, playerPoint + heading * (kMiniMapPlayerHeadingPx - 3.5f) - Vector2(1.4f, 1.4f), Vector2(2.8f, 2.8f), Color(1.0f, 1.0f, 1.0f, 0.94f));
	DrawSolidRect(batch, playerPoint + heading * kMiniMapPlayerHeadingPx - Vector2(1.8f, 1.8f), Vector2(3.6f, 3.6f), Color(1.0f, 1.0f, 1.0f, 0.94f));

	const std::wstring enemyCountText = L"\u6575 " + std::to_wstring(livingEnemies);
	uiText->Draw(batch, enemyCountText, Vector2(mapLeft + miniMapSize - 28.0f * uiScale, mapTop - 24.0f * uiScale), style, 0.54f * uiScale);
}

