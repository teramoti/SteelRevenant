//------------------------//------------------------
// Contents(処理内容) リザルト画面の更新と描画処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "ResultScene.h"

#include <Keyboard.h>
#include <Mouse.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>

#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/UiUtil.h"
#include "../../Action/BattleRuleBook.h"
#include "../Base/SceneUiSound.h"
#include "../SceneManager/SceneManager.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

extern void ExitGame();

namespace
{
	constexpr int kResultMenuCount = 3;
	constexpr float kClickFxDurationSec = 0.22f;
}

// コンストラクタ。
ResultScene::ResultScene(SceneManager* scenemaneger)
	: ActionSceneBase(scenemaneger, false)
	, m_elapsed(0.0f)
	, m_selectedItem(0)
	, m_clickFxTimer(0.0f)
	, m_clickFxPos(Vector2::Zero)
{
	m_SceneFlag = false;
}

// 終了時に保持リソースを解放する。
ResultScene::~ResultScene()
{
	Finalize();
}

// 結果表示に必要な状態を初期化する。
void ResultScene::Initialize()
{
	m_elapsed = 0.0f;
	m_selectedItem = 0;
	m_clickFxTimer = 0.0f;
	m_clickFxPos = Vector2::Zero;
	m_uiSolidTexture = UiUtil::CreateSolidTexture(m_directX.GetDevice().Get());
	EnsureTextRenderer();
	// 結果画面はクリックUIなので絶対座標モードへ切り替える。
	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	SetSystemCursorVisible(true);
}

// 入力処理と遷移を更新する。
void ResultScene::Update(const DX::StepTimer& step)
{
	const float dt = static_cast<float>(step.GetElapsedSeconds());
	if (dt <= 0.0f)
	{
		return;
	}

	m_elapsed += dt;
	m_clickFxTimer = std::max(0.0f, m_clickFxTimer - dt);

	if (m_textRenderer != nullptr)
	{
		m_textRenderer->Update(dt);
	}

	const System::InputManager& input = System::InputManager::GetInstance();
	const DirectX::Keyboard::KeyboardStateTracker keyTracker = input.GetKeyboardTracker();
	const DirectX::Mouse::ButtonStateTracker mouseTracker = input.GetMouseTracker();
	const DirectX::Mouse::State mouseState = input.GetMouseState();
	const Vector2 mousePoint(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));
	const int previousSelectedItem = m_selectedItem;

	if (keyTracker.pressed.Up || keyTracker.pressed.W)
	{
		m_selectedItem = (m_selectedItem + kResultMenuCount - 1) % kResultMenuCount;
	}
	if (keyTracker.pressed.Down || keyTracker.pressed.S)
	{
		m_selectedItem = (m_selectedItem + 1) % kResultMenuCount;
	}

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const float uiScale = std::max(0.78f, std::min(1.4f, std::min(width / 1280.0f, height / 720.0f)));
	const float panelWidth = std::min(760.0f * uiScale, width - 64.0f * uiScale);
	const float panelHeight = std::min(560.0f * uiScale, height - 56.0f * uiScale);
	const Vector2 panelPos((width - panelWidth) * 0.5f, (height - panelHeight) * 0.5f);
	const float menuY = panelPos.y + panelHeight - 72.0f * uiScale;
	const float buttonWidth = std::min(190.0f * uiScale, (panelWidth - 72.0f * uiScale) / 3.0f);
	const float buttonGap = 18.0f * uiScale;
	const float buttonStartX = panelPos.x + (panelWidth - (buttonWidth * 3.0f + buttonGap * 2.0f)) * 0.5f;

	bool clicked = false;
	for (int i = 0; i < kResultMenuCount; ++i)
	{
		const Vector2 buttonPos(buttonStartX + static_cast<float>(i) * (buttonWidth + buttonGap), menuY);
		const Vector2 buttonSize(buttonWidth, 42.0f * uiScale);
		if (!UiUtil::IsInsideRect(mousePoint, buttonPos, buttonSize))
		{
			continue;
		}

		m_selectedItem = i;
		if (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
		{
			clicked = true;
			m_clickFxTimer = kClickFxDurationSec;
			m_clickFxPos = mousePoint;
		}
		break;
	}

	const bool decidedByKey = keyTracker.pressed.Enter || keyTracker.pressed.Space;
	if (keyTracker.pressed.Escape || keyTracker.pressed.Z)
	{
		SceneUiSound::PlayBack();
		m_sceneManager->SetScene(TITLE_SCENE);
		return;
	}
	if (!decidedByKey && !clicked)
	{
		if (m_selectedItem != previousSelectedItem)
		{
			SceneUiSound::PlayMove();
		}
		return;
	}

	switch (m_selectedItem)
	{
	case 0:
		SceneUiSound::PlayConfirm();
		m_sceneManager->SetScene(TITLE_SCENE);
		return;
	case 1:
		SceneUiSound::PlayConfirm();
		m_sceneManager->SetScene(SELECT_SCENE);
		return;
	default:
		SceneUiSound::PlayConfirm();
		ExitGame();
		return;
	}
}

// 結果画面を描画する。
void ResultScene::Render()
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

	const GameSaveData& data = GameSaveData::GetInstance();
	const BattleResultData& result = data.GetBattleResult();

	System::UIShaderStyle titleStyle;
	titleStyle.baseColor = Color(1.0f, 0.42f, 0.38f, 1.0f);
	titleStyle.outlineColor = Color(0.04f, 0.04f, 0.04f, 1.0f);
	titleStyle.pulseAmount = 0.0f;
	titleStyle.pulseSpeed = 0.0f;

	System::UIShaderStyle normalStyle;
	normalStyle.baseColor = Color(0.92f, 0.95f, 0.98f, 1.0f);
	normalStyle.outlineColor = Color(0.05f, 0.05f, 0.07f, 1.0f);
	normalStyle.pulseAmount = 0.0f;

	System::UIShaderStyle valueStyle = normalStyle;
	valueStyle.baseColor = Color(1.0f, 0.95f, 0.75f, 1.0f);
	valueStyle.outlineColor = Color(0.12f, 0.08f, 0.02f, 1.0f);
	valueStyle.pulseAmount = 0.0f;

	System::UIShaderStyle selectedStyle;
	selectedStyle.baseColor = Color(0.72f, 0.92f, 1.0f, 1.0f);
	selectedStyle.outlineColor = Color(0.02f, 0.08f, 0.14f, 1.0f);
	selectedStyle.pulseAmount = 0.0f;

	System::UIShaderStyle warningStyle = titleStyle;
	warningStyle.blink = false;
	warningStyle.blinkPeriod = 0.0f;

	BeginSpriteLayer();

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const float uiScale = std::max(0.78f, std::min(1.4f, std::min(width / 1280.0f, height / 720.0f)));

	for (int band = 0; band < 18; ++band)
	{
		const float t = static_cast<float>(band) / 17.0f;
		const float y = height * t;
		const float h = (height / 18.0f) + 2.0f;
		DrawSolidRect(
			batch,
			Vector2(0.0f, y),
			Vector2(width, h),
			Color(0.016f + t * 0.058f, 0.032f + t * 0.078f, 0.072f + t * 0.116f, 0.92f));
	}

	const float panelWidth = std::min(760.0f * uiScale, width - 64.0f * uiScale);
	const float panelHeight = std::min(560.0f * uiScale, height - 56.0f * uiScale);
	const Vector2 panelPos((width - panelWidth) * 0.5f, (height - panelHeight) * 0.5f);
	DrawSolidRect(batch, panelPos, Vector2(panelWidth, panelHeight), Color(0.03f, 0.05f, 0.08f, 0.82f));
	DrawSolidRect(batch, panelPos, Vector2(panelWidth, std::max(1.0f, 2.0f * uiScale)), Color(0.74f, 0.86f, 1.0f, 0.9f));
	DrawSolidRect(batch, panelPos + Vector2(0.0f, 92.0f * uiScale), Vector2(panelWidth, std::max(1.0f, 1.5f * uiScale)), Color(0.36f, 0.5f, 0.65f, 0.58f));

	uiText->Draw(batch, L"TIME OVER", panelPos + Vector2(26.0f * uiScale, 24.0f * uiScale), titleStyle, 1.36f * uiScale);
	uiText->Draw(batch, Action::BattleRuleBook::GetInstance().GetStageRule(std::max(1, std::min(3, result.stageIndex))).missionName, panelPos + Vector2(28.0f * uiScale, 64.0f * uiScale), normalStyle, 0.8f * uiScale);

	const float cardWidth = (panelWidth - 106.0f * uiScale) / 4.0f;
	const float cardHeight = 78.0f * uiScale;
	const float topY = panelPos.y + 114.0f * uiScale;
	const float secondY = topY + cardHeight + 14.0f * uiScale;
	const float leftX = panelPos.x + 26.0f * uiScale;
	const float midX = leftX + cardWidth + 18.0f * uiScale;
	const float rightX = midX + cardWidth + 18.0f * uiScale;
	const float farRightX = rightX + cardWidth + 18.0f * uiScale;

	const auto DrawStatCard = [this, batch, uiText, &normalStyle, &valueStyle, cardWidth, cardHeight, uiScale](
		const Vector2& pos,
		const wchar_t* title,
		const std::wstring& value,
		const Color& lineColor)
	{
		DrawSolidRect(batch, pos, Vector2(cardWidth, cardHeight), Color(0.05f, 0.08f, 0.12f, 0.72f));
		DrawSolidRect(batch, pos, Vector2(cardWidth, std::max(1.0f, 1.5f * uiScale)), lineColor);
		uiText->Draw(batch, title, pos + Vector2(12.0f * uiScale, 9.0f * uiScale), normalStyle, 0.62f * uiScale);
		uiText->Draw(batch, value, pos + Vector2(12.0f * uiScale, 37.0f * uiScale), valueStyle, 0.92f * uiScale);
	};

	DrawStatCard(Vector2(leftX, topY), L"総合スコア", std::to_wstring(result.totalScore), Color(1.0f, 0.78f, 0.42f, 0.90f));
	DrawStatCard(Vector2(midX, topY), L"BEST SCORE", std::to_wstring(result.bestScore), Color(0.92f, 0.95f, 0.48f, 0.90f));
	DrawStatCard(Vector2(rightX, topY), L"撃破数", std::to_wstring(result.killCount) + L" 体", Color(0.48f, 1.0f, 0.66f, 0.86f));
	DrawStatCard(Vector2(farRightX, topY), L"生存時間", UiUtil::ToWStringFixed(result.survivalTimeSec, 1) + L" sec", Color(0.96f, 0.88f, 0.42f, 0.86f));
	DrawStatCard(Vector2(leftX, secondY), L"Relay", std::to_wstring(result.relayCaptured) + L" / " + std::to_wstring(result.relayRequired), Color(0.52f, 0.86f, 1.0f, 0.86f));
	DrawStatCard(Vector2(midX, secondY), L"補給使用", std::to_wstring(result.beaconUseCount) + L" 回", Color(0.62f, 1.0f, 0.92f, 0.86f));
	DrawStatCard(Vector2(rightX, secondY), L"被弾値", std::to_wstring(result.damageTaken), Color(1.0f, 0.62f, 0.5f, 0.86f));
	DrawStatCard(Vector2(farRightX, secondY), L"PEAK DANGER", std::to_wstring(result.peakDangerLevel), Color(1.0f, 0.64f, 0.28f, 0.90f));

	if (result.isNewRecord)
	{
		System::UIShaderStyle recordStyle = selectedStyle;
		recordStyle.baseColor = Color(0.72f, 0.92f, 1.0f, 1.0f);
		recordStyle.outlineColor = Color(0.02f, 0.08f, 0.14f, 1.0f);
		recordStyle.pulseAmount = 0.22f;
		uiText->Draw(batch, L"NEW RECORD", panelPos + Vector2(panelWidth - 232.0f * uiScale, 36.0f * uiScale), recordStyle, 0.92f * uiScale);
	}

	const float menuY = panelPos.y + panelHeight - 72.0f * uiScale;
	const float buttonWidth = std::min(190.0f * uiScale, (panelWidth - 72.0f * uiScale) / 3.0f);
	const float buttonGap = 18.0f * uiScale;
	const float buttonStartX = panelPos.x + (panelWidth - (buttonWidth * 3.0f + buttonGap * 2.0f)) * 0.5f;

	struct MenuItem
	{
		const wchar_t* label;
		const Color base;
		const Color edge;
	};
	const MenuItem menu[kResultMenuCount] =
	{
		{ L"タイトルへ", Color(0.08f, 0.16f, 0.22f, 0.84f), Color(0.54f, 0.76f, 1.0f, 0.88f) },
		{ L"ステージ選択", Color(0.08f, 0.16f, 0.22f, 0.84f), Color(0.58f, 0.84f, 1.0f, 0.88f) },
		{ L"終了", Color(0.22f, 0.11f, 0.11f, 0.84f), Color(1.0f, 0.62f, 0.56f, 0.88f) }
	};

	for (int i = 0; i < kResultMenuCount; ++i)
	{
		const bool selected = (i == m_selectedItem);
		const Vector2 pos(buttonStartX + static_cast<float>(i) * (buttonWidth + buttonGap), menuY);
		const Vector2 size(buttonWidth, 42.0f * uiScale);
		DrawSolidRect(batch, pos, size, selected ? Color(menu[i].base.x + 0.04f, menu[i].base.y + 0.05f, menu[i].base.z + 0.05f, 0.94f) : menu[i].base);
		DrawSolidRect(batch, pos, Vector2(size.x, std::max(1.0f, 1.5f * uiScale)), menu[i].edge);
		uiText->Draw(batch, menu[i].label, pos + Vector2(18.0f * uiScale, 10.0f * uiScale), selected ? selectedStyle : normalStyle, 0.74f * uiScale);
	}

	const float safeMargin = 12.0f * uiScale;
	const float helpY = std::max(panelPos.y + 10.0f * uiScale, std::min(panelPos.y + panelHeight - 26.0f * uiScale, height - safeMargin - 18.0f * uiScale));
	uiText->Draw(batch, L"\u4e0a\u4e0b / W S \u3067\u9078\u629e  Enter\u307e\u305f\u306f\u30af\u30ea\u30c3\u30af\u3067\u6c7a\u5b9a  Esc\u3067\u30bf\u30a4\u30c8\u30eb", Vector2(panelPos.x + 26.0f * uiScale, helpY), warningStyle, 0.6f * uiScale);

	if (m_clickFxTimer > 0.0f)
	{
		const float progress = 1.0f - (m_clickFxTimer / kClickFxDurationSec);
		const float radius = (8.0f + progress * 34.0f) * uiScale;
		const float alpha = (1.0f - progress) * 0.85f;
		const Color fxColor(0.62f, 0.9f, 1.0f, alpha);

		DrawSolidRect(batch, m_clickFxPos + Vector2(-radius, -radius), Vector2(radius * 2.0f, 2.0f), fxColor);
		DrawSolidRect(batch, m_clickFxPos + Vector2(-radius, radius), Vector2(radius * 2.0f, 2.0f), fxColor);
		DrawSolidRect(batch, m_clickFxPos + Vector2(-radius, -radius), Vector2(2.0f, radius * 2.0f), fxColor);
		DrawSolidRect(batch, m_clickFxPos + Vector2(radius, -radius), Vector2(2.0f, radius * 2.0f), fxColor);
	}

	EndSpriteLayer();
}

// 結果画面の保持リソースを解放する。
void ResultScene::Finalize()
{
	m_textRenderer.reset();
	m_uiSolidTexture.Reset();
	m_elapsed = 0.0f;
	m_selectedItem = 0;
	m_clickFxTimer = 0.0f;
	m_clickFxPos = Vector2::Zero;
}

// 単色矩形をSpriteBatchへ描画する。
void ResultScene::DrawSolidRect(
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

