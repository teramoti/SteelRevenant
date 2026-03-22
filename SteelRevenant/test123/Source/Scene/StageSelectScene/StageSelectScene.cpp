#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "StageSelectScene.h"

#include "../../Action/BattleRuleBook.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/UiUtil.h"
#include "../Base/SceneUiSound.h"
#include "../SceneManager/SceneManager.h"
#include "../../Utility/Sound/AudioSystem.h"

#include <Mouse.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

namespace
{
    constexpr float kClickFxDurationSec = 0.22f;
    constexpr int kStageCount = 3;

    std::wstring FitTextWidth(
        System::UIShaderText* ui,
        const std::wstring& text,
        float scale,
        float maxWidth)
    {
        if (ui == nullptr || text.empty() || maxWidth <= 0.0f)
        {
            return text;
        }

        const std::wstring ellipsis = L"...";
        if (ui->Measure(text, scale).x <= maxWidth)
        {
            return text;
        }

        std::wstring trimmed = text;
        while (!trimmed.empty())
        {
            const std::wstring candidate = trimmed + ellipsis;
            if (ui->Measure(candidate, scale).x <= maxWidth)
            {
                return candidate;
            }
            trimmed.pop_back();
        }

        return ellipsis;
    }
}

StageSelectScene::StageSelectScene(SceneManager* sceneManager)
    : ActionSceneBase(sceneManager, false)
    , m_selectedStage(0)
    , m_blinkTimer(0.0f)
    , m_clickFxTimer(0.0f)
    , m_clickFxPos(Vector2::Zero)
{
    m_SceneFlag = false;
}

StageSelectScene::~StageSelectScene()
{
    Finalize();
}

void StageSelectScene::Initialize()
{
    m_selectedStage = std::clamp(GameSaveData::GetInstance().GetStageNum() - 1, 0, kStageCount - 1);
    m_blinkTimer = 0.0f;
    m_clickFxTimer = 0.0f;
    m_clickFxPos = Vector2::Zero;
    m_uiSolidTexture = UiUtil::CreateSolidTexture(m_directX.GetDevice().Get());
    EnsureTextRenderer();

    // keyboard-only stage select: hide system cursor
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
    System::InputManager::GetInstance().ResetMouseDelta();
    SetSystemCursorVisible(false);

    // Play stage select BGM
    GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::StageSelect);
}

void StageSelectScene::Update(const DX::StepTimer& stepTimer)
{
    const float dt = static_cast<float>(stepTimer.GetElapsedSeconds());
    m_blinkTimer += dt;
    m_clickFxTimer = std::max(0.0f, m_clickFxTimer - dt);

    if (m_textRenderer != nullptr)
    {
        m_textRenderer->Update(dt);
    }

    const System::InputManager& input = System::InputManager::GetInstance();
    const DirectX::Keyboard::KeyboardStateTracker* keyboard = input.GetKeyboardTracker();

    const int previousSelection = m_selectedStage;

    const auto launchSelectedStage = [this]()
    {
        GameSaveData::GetInstance().SetStageNum(m_selectedStage + 1);
        if (m_sceneManager != nullptr)
        {
            m_sceneManager->RequestTransition(GAME_SCENE);
        }
    };

    const auto returnToTitle = [this]()
    {
        if (m_sceneManager != nullptr)
        {
            m_sceneManager->RequestTransition(TITLE_SCENE);
        }
    };

    if (keyboard != nullptr)
    {
        if (keyboard->pressed.Up || keyboard->pressed.W || keyboard->pressed.A || keyboard->pressed.Left)
        {
            m_selectedStage = (m_selectedStage + kStageCount - 1) % kStageCount;
        }
        if (keyboard->pressed.Down || keyboard->pressed.S || keyboard->pressed.D || keyboard->pressed.Right)
        {
            m_selectedStage = (m_selectedStage + 1) % kStageCount;
        }
        if (keyboard->pressed.Enter || keyboard->pressed.Space)
        {
            SceneUiSound::PlayConfirm();
            launchSelectedStage();
            return;
        }
        if (keyboard->pressed.Tab || keyboard->pressed.F1)
        {
            SceneUiSound::PlayBack();
            if (m_sceneManager != nullptr)
                m_sceneManager->RequestTransition(SETTINGS_SCENE);
            return;
        }
        if (keyboard->pressed.Escape)
        {
            SceneUiSound::PlayBack();
            returnToTitle();
            return;
        }
    }

    // mouse input is intentionally ignored (keyboard-only UI). Click FX and mouse hit-testing removed.

    if (m_selectedStage != previousSelection)
    {
        SceneUiSound::PlayMove();
    }
}

void StageSelectScene::Render()
{
    System::UIShaderText* ui = EnsureTextRenderer();
    DirectX::SpriteBatch* batch = System::DrawManager::GetInstance().GetSprite();
    if (ui == nullptr || batch == nullptr)
    {
        return;
    }

    const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
    const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
    const float uiScale = std::max(0.80f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));

    const float frameX = width * 0.08f;
    const float frameY = height * 0.10f;
    const float frameW = width * 0.84f;
    const float frameH = height * 0.78f;
    const float detailW = frameW * 0.42f;
    const float detailX = frameX + 18.0f * uiScale;
    const float detailY = frameY + 56.0f * uiScale;
    const float detailH = frameH - 84.0f * uiScale;
    const float listX = frameX + detailW + 26.0f * uiScale;
    const float listY = frameY + 74.0f * uiScale;
    const float cardW = frameW - detailW - 44.0f * uiScale;
    const float cardH = 92.0f * uiScale;
    const float cardGap = 14.0f * uiScale;

    const std::array<const Action::StageRuleDefinition*, kStageCount> stageRules =
    {
        &Action::BattleRuleBook::GetInstance().GetStageRule(1),
        &Action::BattleRuleBook::GetInstance().GetStageRule(2),
        &Action::BattleRuleBook::GetInstance().GetStageRule(3)
    };

    const Action::StageRuleDefinition& selectedRule =
        *stageRules[static_cast<std::size_t>(std::clamp(m_selectedStage, 0, kStageCount - 1))];

    System::UIShaderStyle titleStyle;
    titleStyle.baseColor = Color(0.94f, 0.97f, 1.0f, 1.0f);
    titleStyle.outlineColor = Color(0.06f, 0.08f, 0.12f, 1.0f);
    titleStyle.pulseAmount = 0.0f;

    System::UIShaderStyle bodyStyle;
    bodyStyle.baseColor = Color(0.82f, 0.86f, 0.92f, 1.0f);
    bodyStyle.outlineColor = Color(0.04f, 0.05f, 0.08f, 1.0f);
    bodyStyle.pulseAmount = 0.0f;

    System::UIShaderStyle accentStyle = bodyStyle;
    accentStyle.baseColor = Color(0.70f, 0.92f, 1.0f, 1.0f);

    const float blink = 0.55f + 0.45f * std::sin(m_blinkTimer * 3.2f);
    System::UIShaderStyle helpStyle = bodyStyle;
    helpStyle.baseColor = Color(1.0f, 0.88f, 0.36f, blink);

    BeginSpriteLayer();

    for (int band = 0; band < 14; ++band)
    {
        const float t = static_cast<float>(band) / 13.0f;
        const float y = t * height;
        DrawSolidRect(
            batch,
            Vector2(0.0f, y),
            Vector2(width, height / 13.0f + 2.0f),
            Color(0.02f + t * 0.04f, 0.04f + t * 0.05f, 0.08f + t * 0.08f, 0.96f));
    }

    DrawSolidRect(batch, Vector2(frameX, frameY), Vector2(frameW, frameH), Color(0.03f, 0.05f, 0.08f, 0.84f));
    DrawSolidRect(batch, Vector2(frameX, frameY), Vector2(frameW, 2.0f), Color(0.62f, 0.80f, 1.0f, 0.92f));

    // Japanese localized headings and reorganized left panel priority
    ui->Draw(batch, L"ステージ選択", Vector2(frameX + 24.0f * uiScale, frameY + 18.0f * uiScale), titleStyle, 1.28f * uiScale);
    ui->Draw(batch, L"ミッションを選択して出撃してください。", Vector2(frameX + 260.0f * uiScale, frameY + 24.0f * uiScale), bodyStyle, 0.72f * uiScale);

    DrawSolidRect(batch, Vector2(detailX, detailY), Vector2(detailW - 18.0f * uiScale, detailH), Color(0.05f, 0.08f, 0.12f, 0.86f));
    DrawSolidRect(batch, Vector2(detailX, detailY), Vector2(detailW - 18.0f * uiScale, 1.5f), Color(0.48f, 0.64f, 0.84f, 0.84f));

    ui->Draw(batch, L"ミッション概要", Vector2(detailX + 14.0f * uiScale, detailY + 14.0f * uiScale), titleStyle, 0.90f * uiScale);

    // priority: Stage name, summary, terrain, enemy特徴, clear conditions, numeric info
    const std::wstring stageLabel = selectedRule.missionName != nullptr ? std::wstring(selectedRule.missionName) : (std::wstring(L"ステージ") + std::to_wstring(m_selectedStage + 1));
    const std::wstring missionSummary = selectedRule.missionSummary != nullptr ? selectedRule.missionSummary : L"";
    const std::wstring terrainLabel = selectedRule.terrainLabel != nullptr ? selectedRule.terrainLabel : L"";
    const std::wstring enemyTrend = selectedRule.enemyTrend != nullptr ? selectedRule.enemyTrend : L"";
    const std::wstring tacticalMemo = selectedRule.tacticalMemo != nullptr ? selectedRule.tacticalMemo : L"";

    ui->Draw(batch, stageLabel, Vector2(detailX + 14.0f * uiScale, detailY + 52.0f * uiScale), accentStyle, 1.20f * uiScale);
    ui->Draw(batch, missionSummary, Vector2(detailX + 14.0f * uiScale, detailY + 92.0f * uiScale), bodyStyle, 0.62f * uiScale);

    ui->Draw(batch, L"エリア構成", Vector2(detailX + 14.0f * uiScale, detailY + 140.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, terrainLabel, Vector2(detailX + 14.0f * uiScale, detailY + 158.0f * uiScale), bodyStyle, 0.56f * uiScale);

    ui->Draw(batch, L"敵の特徴", Vector2(detailX + 14.0f * uiScale, detailY + 196.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, enemyTrend, Vector2(detailX + 14.0f * uiScale, detailY + 214.0f * uiScale), bodyStyle, 0.58f * uiScale);

    ui->Draw(batch, L"攻略メモ", Vector2(detailX + 14.0f * uiScale, detailY + 252.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, tacticalMemo, Vector2(detailX + 14.0f * uiScale, detailY + 270.0f * uiScale), bodyStyle, 0.56f * uiScale);

    // numeric info (secondary)
    const std::wstring timeText = UiUtil::ToWStringFixed(selectedRule.stageStartTimeSec, 0) + L" 秒";
    const std::wstring relayText = std::to_wstring(selectedRule.requiredRelayCount) + L" 回";
    const std::wstring aliveText = std::to_wstring(selectedRule.maxAliveCount);
    const std::wstring waveText = std::to_wstring(selectedRule.totalWaveCount);

    ui->Draw(batch, L"制限時間", Vector2(detailX + 14.0f * uiScale, detailY + 322.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, timeText, Vector2(detailX + 130.0f * uiScale, detailY + 322.0f * uiScale), bodyStyle, 0.58f * uiScale);
    ui->Draw(batch, L"リレー目標", Vector2(detailX + 14.0f * uiScale, detailY + 348.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, relayText, Vector2(detailX + 130.0f * uiScale, detailY + 348.0f * uiScale), bodyStyle, 0.58f * uiScale);
    ui->Draw(batch, L"同時上限", Vector2(detailX + 14.0f * uiScale, detailY + 374.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, aliveText, Vector2(detailX + 130.0f * uiScale, detailY + 374.0f * uiScale), bodyStyle, 0.58f * uiScale);
    ui->Draw(batch, L"WAVE数", Vector2(detailX + 14.0f * uiScale, detailY + 400.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, waveText, Vector2(detailX + 130.0f * uiScale, detailY + 400.0f * uiScale), bodyStyle, 0.58f * uiScale);

    const std::array<Color, kStageCount> accents =
    {
        Color(0.62f, 0.84f, 1.0f, 0.96f),
        Color(1.0f, 0.72f, 0.30f, 0.96f),
        Color(0.38f, 0.96f, 0.88f, 0.96f)
    };

    for (int i = 0; i < kStageCount; ++i)
    {
        const bool selected = (i == m_selectedStage);
        const Color accent = accents[static_cast<std::size_t>(i)];
        const Vector2 cardPos(listX, listY + static_cast<float>(i) * (cardH + cardGap));
        const Vector2 cardSize(cardW, cardH);
        const float selectPulse = 0.5f + 0.5f * std::sin(m_blinkTimer * 4.8f);
        const float expand = selected ? (5.0f + selectPulse * 5.0f) * uiScale : 0.0f;
        const Vector2 drawPos = selected ? (cardPos - Vector2(expand * 0.55f, expand * 0.28f)) : cardPos;
        const Vector2 drawSize = selected ? (cardSize + Vector2(expand * 1.10f, expand * 0.56f)) : cardSize;

        // stronger selected visuals
        DrawSolidRect(
            batch,
            drawPos,
            drawSize,
            selected ? Color(0.10f, 0.14f, 0.22f, 0.94f) : Color(0.06f, 0.08f, 0.12f, 0.78f));
        DrawSolidRect(batch, drawPos, Vector2(6.0f * uiScale, drawSize.y), accent);
        if (selected)
        {
            DrawSolidRect(
                batch,
                drawPos - Vector2(8.0f * uiScale, 8.0f * uiScale),
                drawSize + Vector2(16.0f * uiScale, 16.0f * uiScale),
                DirectX::SimpleMath::Color(accent.x, accent.y, accent.z, 0.12f + selectPulse * 0.10f));
            DrawSolidRect(
                batch,
                drawPos + Vector2(drawSize.x - 6.0f * uiScale, 0.0f),
                Vector2(6.0f * uiScale, drawSize.y),
                Color(accent.x, accent.y, accent.z, 0.65f));
            DrawSolidRect(
                batch,
                drawPos + Vector2(-3.0f * uiScale, -3.0f * uiScale),
                drawSize + Vector2(6.0f * uiScale, 6.0f * uiScale),
                DirectX::SimpleMath::Color(0.22f, 0.56f, 1.0f, 0.18f + selectPulse * 0.08f));
        }
        DrawSolidRect(batch, drawPos, Vector2(drawSize.x, 1.5f), selected ? accent : Color(accent.x, accent.y, accent.z, 0.40f));

        const Action::StageRuleDefinition& stageRule = *stageRules[static_cast<std::size_t>(i)];
        const std::wstring cardTitle = std::wstring(L"ステージ") + std::to_wstring(i + 1) + L"  "
            + (stageRule.missionName != nullptr ? std::wstring(stageRule.missionName) : L"");
        const std::wstring cardSummary = stageRule.missionSummary != nullptr ? stageRule.missionSummary : L"";
        const std::wstring cardTime = UiUtil::ToWStringFixed(stageRule.stageStartTimeSec, 0) + L"秒";
        const std::wstring cardRelay = L"リレー " + std::to_wstring(stageRule.requiredRelayCount) + L"回";

        ui->Draw(batch, cardTitle, drawPos + Vector2(18.0f * uiScale, 14.0f * uiScale), selected ? accentStyle : titleStyle, (selected ? 0.92f : 0.88f) * uiScale);
        ui->Draw(
            batch,
            FitTextWidth(ui, cardSummary, 0.56f * uiScale, drawSize.x - 178.0f * uiScale),
            drawPos + Vector2(18.0f * uiScale, 46.0f * uiScale),
            bodyStyle,
            0.56f * uiScale);
        ui->Draw(batch, cardTime, drawPos + Vector2(drawSize.x - 84.0f * uiScale, 18.0f * uiScale), bodyStyle, 0.58f * uiScale);
        ui->Draw(batch, cardRelay, drawPos + Vector2(drawSize.x - 132.0f * uiScale, 50.0f * uiScale), bodyStyle, 0.52f * uiScale);
    }

    ui->Draw(
        batch,
        L"↑/↓ または W/S: 選択   Enter: 出撃   Tab/F1: 設定   Esc: タイトルへ",
        Vector2(frameX + 24.0f * uiScale, frameY + frameH - 28.0f * uiScale),
        helpStyle,
        0.72f * uiScale);

    if (m_clickFxTimer > 0.0f)
    {
        const float progress = 1.0f - (m_clickFxTimer / kClickFxDurationSec);
        const float radius = (8.0f + progress * 34.0f) * uiScale;
        const float alpha = (1.0f - progress) * 0.85f;
        const Color fxColor(0.72f, 0.96f, 1.0f, alpha);

        DrawSolidRect(
            batch,
            m_clickFxPos + Vector2(-radius, -1.0f * uiScale),
            Vector2(radius * 2.0f, 2.0f * uiScale),
            fxColor);
        DrawSolidRect(
            batch,
            m_clickFxPos + Vector2(-1.0f * uiScale, -radius),
            Vector2(2.0f * uiScale, radius * 2.0f),
            fxColor);
    }

    EndSpriteLayer();
}

void StageSelectScene::Finalize()
{
    // restore system cursor and return mouse to absolute mode
    SetSystemCursorVisible(true);
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    System::InputManager::GetInstance().ResetMouseDelta();
}

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

    batch->Draw(
        m_uiSolidTexture.Get(),
        position,
        nullptr,
        color,
        0.0f,
        Vector2::Zero,
        DirectX::XMFLOAT2(size.x, size.y));
}
