//------------------------//------------------------
// Contents(処理内容) ステージ選択画面の更新と描画処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "StageSelectScene.h"

#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/UiUtil.h"
#include "../../Action/BattleRuleBook.h"
#include "../Base/SceneUiSound.h"
#include "../SceneManager/SceneManager.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <Mouse.h>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

namespace
{
	constexpr float kClickFxDurationSec = 0.22f;

	// 指定幅に収まる文字列へ省略記号付きで整形する。
	std::wstring FitTextWidth(System::UIShaderText* ui, const std::wstring& text, float scale, float maxWidth)
	{
		if (ui == nullptr || text.empty() || maxWidth <= 0.0f)
		{
			return text;
		}
		const std::wstring ellipsis = L"…";
		if (ui->Measure(text, scale).x <= maxWidth)
		{
			return text;
		}
		std::wstring result = text;
		while (!result.empty())
		{
			std::wstring candidate = result + ellipsis;
			if (ui->Measure(candidate, scale).x <= maxWidth)
			{
				return candidate;
			}
			result.pop_back();
		}
		return ellipsis;
	}
}

// ステージ選択シーンを生成する。
StageSelectScene::StageSelectScene(SceneManager* sceneManager)
	: ActionSceneBase(sceneManager, false)
	, m_selectedStage(0)
	, m_blinkTimer(0.0f)
	, m_clickFxTimer(0.0f)
	, m_clickFxPos(Vector2::Zero)
{
	m_SceneFlag = false;
}

// 確保済み資産を解放する。
StageSelectScene::~StageSelectScene()
{
	Finalize();
}

// 選択状態とUI描画資産を初期化する。
void StageSelectScene::Initialize()
{
	m_selectedStage = 0;
	m_blinkTimer = 0.0f;
	m_clickFxTimer = 0.0f;
	m_clickFxPos = Vector2::Zero;
	m_uiSolidTexture = UiUtil::CreateSolidTexture(m_directX.GetDevice().Get());
	EnsureTextRenderer();
	// UI操作はクリック前提のため、絶対座標モードへ切り替える。
	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	SetSystemCursorVisible(true);
}

// 入力に応じて選択カーソルと遷移先を更新する。
void StageSelectScene::Update(const DX::StepTimer& stepTimer)
{
	const float dt = static_cast<float>(stepTimer.GetElapsedSeconds());
	m_blinkTimer += dt;
	m_clickFxTimer = std::max(0.0f, m_clickFxTimer - dt);

	if (m_textRenderer != nullptr)
	{
		m_textRenderer->Update(dt);
	}

	const DirectX::Keyboard::KeyboardStateTracker tracker = System::InputManager::GetInstance().GetKeyboardTracker();
	const DirectX::Mouse::ButtonStateTracker mouseTracker = System::InputManager::GetInstance().GetMouseTracker();
	const DirectX::Mouse::State mouseState = System::InputManager::GetInstance().GetMouseState();
	const int previousSelectedStage = m_selectedStage;

	// 上下入力だけでカーソルを巡回移動する。
	if (tracker.pressed.Up || tracker.pressed.W || tracker.pressed.A)
	{
		m_selectedStage = (m_selectedStage + 2) % 3;
	}
	if (tracker.pressed.Down || tracker.pressed.S || tracker.pressed.D)
	{
		m_selectedStage = (m_selectedStage + 1) % 3;
	}

	// マウスホバーでカード選択、クリックで即出撃。
	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const float uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));
	const float frameX = width * 0.08f;
	const float safeMargin = 12.0f * uiScale;
	const float frameH = std::min(std::min(height * 0.88f, 620.0f * uiScale), height - safeMargin * 2.0f);
	const float frameY = std::max(safeMargin, std::min(38.0f * uiScale, height - frameH - safeMargin));
	const float frameW = width * 0.84f;
	const float leftX = frameX + 14.0f * uiScale;
	const float leftW = frameW * 0.42f;
	const float rightX = leftX + leftW + 18.0f * uiScale;
	const float rightY = frameY + 56.0f * uiScale;
	const float rightW = frameX + frameW - rightX - 14.0f * uiScale;
	const float cardX = rightX + 12.0f * uiScale;
	const float cardW = rightW - 24.0f * uiScale;
	const float cardH = 82.0f * uiScale;
	const float cardGap = 10.0f * uiScale;
	const float listStartY = rightY + 12.0f * uiScale;
	const Vector2 mousePoint(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));
	const bool leftClicked = (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED);
	for (int i = 0; i < 3; ++i)
	{
		const float y = listStartY + static_cast<float>(i) * (cardH + cardGap);
		const bool hover =
			(mousePoint.x >= cardX && mousePoint.x <= (cardX + cardW) &&
			 mousePoint.y >= y && mousePoint.y <= (y + cardH));
		if (!hover)
		{
			continue;
		}

		m_selectedStage = i;
		if (leftClicked)
		{
			m_clickFxTimer = kClickFxDurationSec;
			m_clickFxPos = mousePoint;
			SceneUiSound::PlayConfirm();
			GameSaveData::GetInstance().SetStageNum(m_selectedStage + 1);
			m_sceneManager->SetScene(GAME_SCENE);
			return;
		}
		break;
	}

	if (tracker.pressed.Enter || tracker.pressed.Space)
	{
		SceneUiSound::PlayConfirm();
		GameSaveData::GetInstance().SetStageNum(m_selectedStage + 1);
		m_sceneManager->SetScene(GAME_SCENE);
		return;
	}

	if (tracker.pressed.Escape)
	{
		SceneUiSound::PlayBack();
		m_sceneManager->SetScene(TITLE_SCENE);
		return;
	}

	if (m_selectedStage != previousSelectedStage)
	{
		SceneUiSound::PlayMove();
	}
}

// ステージカードUIを描画する。
void StageSelectScene::Render()
{
	System::UIShaderText* ui = EnsureTextRenderer();
	// UI操作はクリック前提のため、絶対座標モードへ切り替える。
	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	if (ui == nullptr)
	{
		return;
	}

	System::UIShaderStyle titleStyle;
	titleStyle.baseColor = Color(0.9f, 0.95f, 1.0f, 1.0f);
	titleStyle.outlineColor = Color(0.05f, 0.05f, 0.08f, 1.0f);
	titleStyle.pulseAmount = 0.1f;
	titleStyle.pulseSpeed = 2.8f;

	System::UIShaderStyle selectedStyle;
	selectedStyle.baseColor = Color(0.72f, 0.92f, 1.0f, 1.0f);
	selectedStyle.outlineColor = Color(0.02f, 0.08f, 0.14f, 1.0f);
	selectedStyle.pulseAmount = 0.15f;

	System::UIShaderStyle normalStyle;
	normalStyle.baseColor = Color(0.85f, 0.85f, 0.85f, 1.0f);
	normalStyle.outlineColor = Color(0.06f, 0.06f, 0.06f, 1.0f);

	System::UIShaderStyle subStyle;
	subStyle.baseColor = Color(0.68f, 0.76f, 0.86f, 1.0f);
	subStyle.outlineColor = Color(0.04f, 0.05f, 0.07f, 1.0f);

	System::UIShaderStyle helpStyle;
	helpStyle.baseColor = Color(1.0f, 0.9f, 0.25f, 1.0f);
	helpStyle.outlineColor = Color(0.15f, 0.1f, 0.0f, 1.0f);
	helpStyle.blink = true;
	helpStyle.blinkPeriod = 0.8f;

	BeginSpriteLayer();
	DirectX::SpriteBatch* batch = System::DrawManager::GetInstance().GetSprite();
	if (batch == nullptr)
	{
		EndSpriteLayer();
		return;
	}

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const float uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));
	const float frameX = width * 0.08f;
	const float safeMargin = 12.0f * uiScale;
	const float frameH = std::min(std::min(height * 0.84f, 540.0f * uiScale), height - safeMargin * 2.0f);
	const float frameY = std::max(safeMargin, std::min(38.0f * uiScale, height - frameH - safeMargin));
	const float frameW = width * 0.84f;

	// 単色背景を避けるため、縦グラデーション + 斜光 + 雲帯を重ねる。
	for (int band = 0; band < 16; ++band)
	{
		const float t = static_cast<float>(band) / 15.0f;
		const float y = height * t;
		const float h = (height / 16.0f) + 2.0f;
		const float wave = std::sinf(m_blinkTimer * 0.9f + t * 6.1f) * 0.03f;
		DrawSolidRect(
			batch,
			Vector2(0.0f, y),
			Vector2(width, h),
			Color(0.01f + t * 0.05f + wave * 0.6f, 0.025f + t * 0.08f + wave * 0.5f, 0.055f + t * 0.12f + wave * 0.4f, 0.9f));
	}

	for (int beam = 0; beam < 4; ++beam)
	{
		const float speed = 20.0f + static_cast<float>(beam) * 6.5f;
		float x = std::fmod(m_blinkTimer * speed + 230.0f * static_cast<float>(beam), width + 320.0f) - 160.0f;
		if (x < -160.0f)
		{
			x += width + 320.0f;
		}

		batch->Draw(
			m_uiSolidTexture.Get(),
			Vector2(x, -height * 0.2f),
			nullptr,
			(beam % 2 == 0) ? Color(0.14f, 0.34f, 0.7f, 0.14f) : Color(0.22f, 0.5f, 0.88f, 0.18f),
			0.33f,
			Vector2::Zero,
			DirectX::XMFLOAT2(220.0f, height * 1.35f));
	}

	for (int cloud = 0; cloud < 10; ++cloud)
	{
		const float fc = static_cast<float>(cloud);
		const float loop = std::fmod(m_blinkTimer * (7.0f + std::fmod(fc, 4.0f) * 1.1f) + fc * 13.7f, width + 160.0f) - 80.0f;
		const float y = height * 0.12f + std::fmod(fc * 41.0f, height * 0.62f);
		const float len = 70.0f + std::fmod(fc * 17.0f, 86.0f);
		DrawSolidRect(batch, Vector2(loop, y), Vector2(len, 8.0f), Color(0.22f, 0.24f, 0.3f, 0.24f));
	}

	const float leftX = frameX + 14.0f * uiScale;
	const float leftY = frameY + 52.0f * uiScale;
	const float leftW = frameW * 0.42f;
	const float leftH = frameH - 68.0f;

	const float rightX = leftX + leftW + 18.0f * uiScale;
	const float rightY = leftY;
	const float rightW = frameX + frameW - rightX - 14.0f * uiScale;
	const float rightH = leftH;

	DrawSolidRect(batch, Vector2(frameX, frameY), Vector2(frameW, frameH), Color(0.02f, 0.04f, 0.07f, 0.84f));
	DrawSolidRect(batch, Vector2(frameX, frameY), Vector2(frameW, 2.0f), Color(0.72f, 0.82f, 1.0f, 0.86f));

	// 文字コード依存を避けるため、画面文言は Unicode エスケープで固定する。
	ui->Draw(batch, L"\u30b9\u30c6\u30fc\u30b8\u9078\u629e", Vector2(frameX + 28.0f * uiScale, frameY + 20.0f * uiScale), titleStyle, 1.18f * uiScale);
	ui->Draw(batch, L"\u51fa\u6483\u5148\u3092\u9078\u3093\u3067 Enter \u3067\u958b\u59cb", Vector2(frameX + 260.0f * uiScale, frameY + 24.0f * uiScale), normalStyle, 0.76f * uiScale);

	std::array<const Action::StageRuleDefinition*, 3> stageRules =
	{
		&Action::BattleRuleBook::GetInstance().GetStageRule(1),
		&Action::BattleRuleBook::GetInstance().GetStageRule(2),
		&Action::BattleRuleBook::GetInstance().GetStageRule(3)
	};

	// 左パネル: 選択中ステージの詳細と攻略要点を表示する。

	DrawSolidRect(batch, Vector2(leftX, leftY), Vector2(leftW, leftH), Color(0.04f, 0.07f, 0.10f, 0.86f));
	DrawSolidRect(batch, Vector2(leftX, leftY), Vector2(leftW, std::max(1.0f, 1.5f * uiScale)), Color(0.64f, 0.8f, 1.0f, 0.8f));
	ui->Draw(batch, L"\u4efb\u52d9\u8a73\u7d30", Vector2(leftX + 14.0f * uiScale, leftY + 12.0f * uiScale), titleStyle, 0.9f * uiScale);

	const int infoIndex = std::max(0, std::min(2, m_selectedStage));
	const Action::StageRuleDefinition& selectedRule = *stageRules[static_cast<size_t>(infoIndex)];
	const std::array<Color, 3> stageAccentColors =
	{
		Color(0.68f, 0.84f, 1.0f, 0.94f),
		Color(1.0f, 0.72f, 0.28f, 0.94f),
		Color(0.38f, 0.94f, 1.0f, 0.94f)
	};
	const Color selectedAccent = stageAccentColors[static_cast<size_t>(infoIndex)];

	DrawSolidRect(batch, Vector2(leftX + 14.0f * uiScale, leftY + 44.0f * uiScale), Vector2(leftW - 28.0f * uiScale, 62.0f * uiScale), Color(0.05f, 0.11f, 0.14f, 0.82f));
	DrawSolidRect(batch, Vector2(leftX + 14.0f * uiScale, leftY + 44.0f * uiScale), Vector2(6.0f * uiScale, 62.0f * uiScale), selectedAccent);
	ui->Draw(batch, std::wstring(L"0") + std::to_wstring(infoIndex + 1), Vector2(leftX + 28.0f * uiScale, leftY + 48.0f * uiScale), selectedStyle, 1.42f * uiScale);
	ui->Draw(batch, std::wstring(L"エリア名: ") + selectedRule.missionName, Vector2(leftX + 86.0f * uiScale, leftY + 56.0f * uiScale), selectedStyle, 0.74f * uiScale);
	ui->Draw(batch, std::wstring(L"地形: ") + selectedRule.terrainLabel, Vector2(leftX + 86.0f * uiScale, leftY + 80.0f * uiScale), normalStyle, 0.60f * uiScale);

	ui->Draw(batch, std::wstring(L"敵傾向: ") + selectedRule.enemyTrend, Vector2(leftX + 14.0f * uiScale, leftY + 120.0f * uiScale), normalStyle, 0.58f * uiScale);
	ui->Draw(batch, std::wstring(L"戦術メモ: ") + selectedRule.tacticalMemo, Vector2(leftX + 14.0f * uiScale, leftY + 144.0f * uiScale), normalStyle, 0.56f * uiScale);

	const float enemyLoad = std::max(0.0f, std::min(1.0f, static_cast<float>(selectedRule.maxAliveCount) / 7.0f));
	const float timeLoad = std::max(0.0f, std::min(1.0f, 1.0f - (selectedRule.stageStartTimeSec / 360.0f)));
	const float relayLoad = std::max(0.0f, std::min(1.0f, static_cast<float>(selectedRule.requiredRelayCount) / 3.0f));
	ui->Draw(batch, L"任務強度", Vector2(leftX + 14.0f * uiScale, leftY + 176.0f * uiScale), titleStyle, 0.66f * uiScale);
	ui->Draw(batch, L"敵圧", Vector2(leftX + 16.0f * uiScale, leftY + 212.0f * uiScale), subStyle, 0.54f * uiScale);
	DrawSolidRect(batch, Vector2(leftX + 88.0f * uiScale, leftY + 220.0f * uiScale), Vector2(120.0f * uiScale, 6.0f * uiScale), Color(0.12f, 0.15f, 0.18f, 0.92f));
	DrawSolidRect(batch, Vector2(leftX + 88.0f * uiScale, leftY + 220.0f * uiScale), Vector2(120.0f * uiScale * enemyLoad, 6.0f * uiScale), selectedAccent);
	ui->Draw(batch, L"時間圧", Vector2(leftX + 16.0f * uiScale, leftY + 234.0f * uiScale), subStyle, 0.54f * uiScale);
	DrawSolidRect(batch, Vector2(leftX + 88.0f * uiScale, leftY + 242.0f * uiScale), Vector2(120.0f * uiScale, 6.0f * uiScale), Color(0.12f, 0.15f, 0.18f, 0.92f));
	DrawSolidRect(batch, Vector2(leftX + 88.0f * uiScale, leftY + 242.0f * uiScale), Vector2(120.0f * uiScale * timeLoad, 6.0f * uiScale), Color(selectedAccent.x * 0.92f, selectedAccent.y * 0.86f, selectedAccent.z, 0.94f));
	ui->Draw(batch, L"副目標圧", Vector2(leftX + 16.0f * uiScale, leftY + 256.0f * uiScale), subStyle, 0.54f * uiScale);
	DrawSolidRect(batch, Vector2(leftX + 88.0f * uiScale, leftY + 264.0f * uiScale), Vector2(120.0f * uiScale, 6.0f * uiScale), Color(0.12f, 0.15f, 0.18f, 0.92f));
	DrawSolidRect(batch, Vector2(leftX + 88.0f * uiScale, leftY + 264.0f * uiScale), Vector2(120.0f * uiScale * relayLoad, 6.0f * uiScale), Color(selectedAccent.x * 0.72f, selectedAccent.y * 0.92f, selectedAccent.z * 0.86f, 0.94f));

	DrawSolidRect(batch, Vector2(leftX + 14.0f * uiScale, leftY + 286.0f * uiScale), Vector2(leftW - 28.0f * uiScale, std::max(1.0f, 1.0f * uiScale)), Color(0.36f, 0.46f, 0.56f, 0.55f));
	ui->Draw(batch, L"\u30df\u30c3\u30b7\u30e7\u30f3\u6761\u4ef6", Vector2(leftX + 14.0f * uiScale, leftY + 300.0f * uiScale), titleStyle, 0.72f * uiScale);
	std::wstring missionLine = std::wstring(L"制限 ") + std::to_wstring(static_cast<int>(selectedRule.stageStartTimeSec)) + L"秒  /  スコアアタック";
	missionLine += std::wstring(L"  /  Relay ") + std::to_wstring(selectedRule.requiredRelayCount);
	ui->Draw(batch, missionLine, Vector2(leftX + 14.0f * uiScale, leftY + 326.0f * uiScale), helpStyle, 0.60f * uiScale);
	ui->Draw(batch, selectedRule.missionSummary, Vector2(leftX + 14.0f * uiScale, leftY + 348.0f * uiScale), normalStyle, 0.52f * uiScale);

	DrawSolidRect(batch, Vector2(leftX + leftW - 92.0f * uiScale, leftY + 194.0f * uiScale), Vector2(58.0f * uiScale, 84.0f * uiScale), Color(0.06f, 0.1f, 0.14f, 0.76f));
	DrawSolidRect(batch, Vector2(leftX + leftW - 92.0f * uiScale, leftY + 194.0f * uiScale), Vector2(58.0f * uiScale, 2.0f * uiScale), selectedAccent);
	ui->Draw(batch, std::to_wstring(static_cast<int>(selectedRule.stageStartTimeSec / 60.0f)), Vector2(leftX + leftW - 74.0f * uiScale, leftY + 208.0f * uiScale), selectedStyle, 0.96f * uiScale);
	ui->Draw(batch, L"TIME", Vector2(leftX + leftW - 78.0f * uiScale, leftY + 236.0f * uiScale), subStyle, 0.48f * uiScale);
	ui->Draw(batch, std::to_wstring(selectedRule.requiredRelayCount), Vector2(leftX + leftW - 74.0f * uiScale, leftY + 252.0f * uiScale), helpStyle, 0.82f * uiScale);
	ui->Draw(batch, L"RELAY", Vector2(leftX + leftW - 82.0f * uiScale, leftY + 274.0f * uiScale), subStyle, 0.42f * uiScale);

	// 右パネル: ステージカード一覧。
	DrawSolidRect(batch, Vector2(rightX, rightY), Vector2(rightW, rightH), Color(0.04f, 0.07f, 0.1f, 0.72f));
	DrawSolidRect(batch, Vector2(rightX, rightY), Vector2(rightW, std::max(1.0f, 1.5f * uiScale)), Color(0.58f, 0.72f, 0.9f, 0.7f));
	const float cardX = rightX + 12.0f * uiScale;
	const float cardW = rightW - 24.0f * uiScale;
	const float cardH = 82.0f * uiScale;
	const float cardGap = 10.0f * uiScale;
	const float listStartY = rightY + 12.0f * uiScale;
	const float selectPulse = std::sinf(m_blinkTimer * 5.2f) * 0.5f + 0.5f;

	for (int i = 0; i < 3; ++i)
	{
		const bool selected = (i == m_selectedStage);
		const Color accent = stageAccentColors[static_cast<size_t>(i)];
		const float y = listStartY + static_cast<float>(i) * (cardH + cardGap);
		const float pulseBoost = selected ? (0.08f + selectPulse * 0.10f) : 0.0f;
		const Color cardColor = selected
			? Color(0.08f + accent.x * (0.14f + pulseBoost), 0.10f + accent.y * (0.12f + pulseBoost), 0.12f + accent.z * (0.10f + pulseBoost), 0.92f)
			: Color(0.08f, 0.11f, 0.14f, 0.76f);
		const Color edgeColor = selected ? accent : Color(accent.x * 0.52f, accent.y * 0.52f, accent.z * 0.52f, 0.62f);

		DrawSolidRect(batch, Vector2(cardX, y), Vector2(cardW, cardH), cardColor);
		DrawSolidRect(batch, Vector2(cardX, y), Vector2(5.0f * uiScale, cardH), Color(edgeColor.x, edgeColor.y, edgeColor.z, selected ? (0.72f + selectPulse * 0.26f) : 0.46f));
		DrawSolidRect(batch, Vector2(cardX, y), Vector2(cardW, std::max(1.0f, 1.5f * uiScale)), edgeColor);
		DrawSolidRect(batch, Vector2(cardX, y + cardH - 1.5f), Vector2(cardW, std::max(1.0f, 1.5f * uiScale)), edgeColor);

		const Action::StageRuleDefinition& stageRule = *stageRules[static_cast<size_t>(i)];
		const wchar_t* durationLabel = L"5分耐久戦";
		if (i == 0)
		{
			durationLabel = L"1分短期戦";
		}
		else if (i == 1)
		{
			durationLabel = L"3分制圧戦";
		}
		const std::wstring stageTitle = std::wstring(durationLabel) + L"  " + stageRule.missionName;
		ui->Draw(batch, stageTitle, Vector2(cardX + 22.0f * uiScale, y + 10.0f * uiScale), selected ? selectedStyle : normalStyle, 0.84f * uiScale);
		const float summaryScale = 0.56f * uiScale;
		const float summaryMaxWidth = cardW - 118.0f * uiScale;
		const std::wstring summary = stageRule.missionSummary;
		std::wstring line1 = FitTextWidth(ui, summary, summaryScale, summaryMaxWidth);
		std::wstring remaining;
		if (!line1.empty() && line1.back() == L'…')
		{
			// 省略になった場合は2行目を出さず、1行目に集約する。
			remaining.clear();
		}
		else if (line1.size() < summary.size())
		{
			remaining = summary.substr(line1.size());
		}
		std::wstring line2;
		if (!remaining.empty())
		{
			line2 = FitTextWidth(ui, remaining, summaryScale, summaryMaxWidth);
		}
		ui->Draw(batch, line1, Vector2(cardX + 16.0f * uiScale, y + 42.0f * uiScale), selected ? selectedStyle : normalStyle, summaryScale);
		if (!line2.empty())
		{
			ui->Draw(batch, line2, Vector2(cardX + 16.0f * uiScale, y + 58.0f * uiScale), selected ? selectedStyle : normalStyle, summaryScale);
		}

		// カード右端に簡易マップ図を表示し、地形差を視覚で伝える。
		const Vector2 mapPos(cardX + cardW - 90.0f * uiScale, y + 12.0f * uiScale);
		const Vector2 mapSize(74.0f * uiScale, 56.0f * uiScale);
		DrawSolidRect(batch, mapPos, mapSize, selected ? Color(0.07f, 0.16f, 0.1f, 0.86f) : Color(0.06f, 0.08f, 0.1f, 0.78f));
		DrawSolidRect(batch, mapPos, Vector2(mapSize.x, std::max(1.0f, 1.0f * uiScale)), selected ? Color(0.54f, 0.96f, 0.64f, 0.92f) : Color(0.46f, 0.56f, 0.66f, 0.68f));

		switch (i)
		{
		case 0:
			// Stage1: 中央障害物 + 四隅湧き。
			DrawSolidRect(batch, mapPos + Vector2(30.0f * uiScale, 22.0f * uiScale), Vector2(14.0f * uiScale, 12.0f * uiScale), Color(0.8f, 0.32f, 0.24f, 0.92f));
			DrawSolidRect(batch, mapPos + Vector2(8.0f * uiScale, 8.0f * uiScale), Vector2(4.0f * uiScale, 4.0f * uiScale), Color(0.34f, 0.9f, 1.0f, 0.86f));
			DrawSolidRect(batch, mapPos + Vector2(62.0f * uiScale, 8.0f * uiScale), Vector2(4.0f * uiScale, 4.0f * uiScale), Color(0.34f, 0.9f, 1.0f, 0.86f));
			DrawSolidRect(batch, mapPos + Vector2(8.0f * uiScale, 44.0f * uiScale), Vector2(4.0f * uiScale, 4.0f * uiScale), Color(0.34f, 0.9f, 1.0f, 0.86f));
			DrawSolidRect(batch, mapPos + Vector2(62.0f * uiScale, 44.0f * uiScale), Vector2(4.0f * uiScale, 4.0f * uiScale), Color(0.34f, 0.9f, 1.0f, 0.86f));
			break;
		case 1:
			// Stage2: 防衛回廊。
			DrawSolidRect(batch, mapPos + Vector2(8.0f * uiScale, 10.0f * uiScale), Vector2(58.0f * uiScale, 5.0f * uiScale), Color(0.94f, 0.66f, 0.26f, 0.9f));
			DrawSolidRect(batch, mapPos + Vector2(8.0f * uiScale, 41.0f * uiScale), Vector2(58.0f * uiScale, 5.0f * uiScale), Color(0.94f, 0.66f, 0.26f, 0.9f));
			DrawSolidRect(batch, mapPos + Vector2(31.0f * uiScale, 20.0f * uiScale), Vector2(12.0f * uiScale, 16.0f * uiScale), Color(0.44f, 0.9f, 1.0f, 0.84f));
			break;
		default:
			// Stage3: 中央高台 + 包囲配置。
			DrawSolidRect(batch, mapPos + Vector2(26.0f * uiScale, 18.0f * uiScale), Vector2(22.0f * uiScale, 18.0f * uiScale), Color(0.44f, 1.0f, 0.84f, 0.9f));
			DrawSolidRect(batch, mapPos + Vector2(4.0f * uiScale, 4.0f * uiScale), Vector2(8.0f * uiScale, 6.0f * uiScale), Color(0.92f, 0.34f, 1.0f, 0.82f));
			DrawSolidRect(batch, mapPos + Vector2(62.0f * uiScale, 4.0f * uiScale), Vector2(8.0f * uiScale, 6.0f * uiScale), Color(0.92f, 0.34f, 1.0f, 0.82f));
			DrawSolidRect(batch, mapPos + Vector2(4.0f * uiScale, 46.0f * uiScale), Vector2(8.0f * uiScale, 6.0f * uiScale), Color(0.92f, 0.34f, 1.0f, 0.82f));
			DrawSolidRect(batch, mapPos + Vector2(62.0f * uiScale, 46.0f * uiScale), Vector2(8.0f * uiScale, 6.0f * uiScale), Color(0.92f, 0.34f, 1.0f, 0.82f));
			break;
		}
	}

	const float helpY = std::max(frameY + 8.0f * uiScale, std::min(frameY + frameH - 22.0f * uiScale, height - safeMargin - 18.0f * uiScale));
	ui->Draw(batch, L"\u4e0a\u4e0b\u30ad\u30fc / W S / A D \u3067\u9078\u629e   Enter\u3067\u51fa\u6483   Esc\u3067\u623b\u308b", Vector2(frameX + 26.0f * uiScale, helpY), helpStyle, 0.74f * uiScale);

	if (m_clickFxTimer > 0.0f)
	{
		const float progress = 1.0f - (m_clickFxTimer / kClickFxDurationSec);
		const float radius = (7.0f + progress * 40.0f) * uiScale;
		const float alpha = (1.0f - progress) * 0.86f;
		const Color fxColor(0.62f, 0.9f, 1.0f, alpha);
		for (int particle = 0; particle < 12; ++particle)
		{
			const float fp = static_cast<float>(particle);
			const float angle = (DirectX::XM_2PI / 12.0f) * fp + progress * 1.6f;
			const Vector2 dir(std::cosf(angle), std::sinf(angle));
			const Vector2 point = m_clickFxPos + dir * radius;
			const float size = (2.0f + std::fmod(fp, 3.0f)) * uiScale;
			DrawSolidRect(batch, point - Vector2(size * 0.5f, size * 0.5f), Vector2(size, size), fxColor);
		}

		const float coreRadius = (3.0f + (1.0f - progress) * 10.0f) * uiScale;
		DrawSolidRect(batch, m_clickFxPos + Vector2(-coreRadius, -uiScale), Vector2(coreRadius * 2.0f, 2.0f * uiScale), Color(0.96f, 1.0f, 0.98f, alpha * 0.92f));
		DrawSolidRect(batch, m_clickFxPos + Vector2(-uiScale, -coreRadius), Vector2(2.0f * uiScale, coreRadius * 2.0f), Color(0.96f, 1.0f, 0.98f, alpha * 0.92f));
	}
	EndSpriteLayer();
}

// 破棄可能なリソースを解放する。
void StageSelectScene::Finalize()
{
	m_uiSolidTexture.Reset();
	m_textRenderer.reset();
	m_clickFxTimer = 0.0f;
	m_clickFxPos = Vector2::Zero;
}

// SpriteBatch上へ単色矩形を描画する。
void StageSelectScene::DrawSolidRect(
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

