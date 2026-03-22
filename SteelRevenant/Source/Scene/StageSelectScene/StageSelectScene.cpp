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
#include "../../Utility/DirectX11.h"
#include "../GameScene/GameSceneWorldBackdrop.h"
#include "../GameScene/GameSceneWorldArena.h"
#include <GeometricPrimitive.h>

#include <Mouse.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Matrix;

namespace
{
    constexpr float kClickFxDurationSec = 0.22f;
    constexpr int kStageCount = 3;


    struct StageSelectFront3DResources
    {
        GameSceneWorldBackdrop backdrop;
        std::array<std::unique_ptr<GameSceneWorldArena>, 3> arenas;
        std::unique_ptr<DirectX::GeometricPrimitive> sphere;
        std::unique_ptr<DirectX::GeometricPrimitive> cylinder;
        std::unique_ptr<DirectX::GeometricPrimitive> torus;
        bool initialized = false;

        void Ensure(ID3D11Device* device, ID3D11DeviceContext* context)
        {
            if (initialized || device == nullptr || context == nullptr)
            {
                return;
            }
            backdrop.Initialize(device, context);
            for (int i = 0; i < 3; ++i)
            {
                arenas[i] = std::make_unique<GameSceneWorldArena>();
                arenas[i]->Initialize(device, context, i + 1);
            }
            sphere = DirectX::GeometricPrimitive::CreateSphere(context, 0.42f, 12);
            cylinder = DirectX::GeometricPrimitive::CreateCylinder(context, 1.0f, 0.10f, 12);
            torus = DirectX::GeometricPrimitive::CreateTorus(context, 1.2f, 0.08f, 24);
            initialized = true;
            (void)device;
        }
    };

    StageSelectFront3DResources& GetStageSelectFront3D()
    {
        static StageSelectFront3DResources resources;
        return resources;
    }

    void RenderStageSelectFront3D(int stageIndex, float blinkTimer, float width, float height)
    {
        auto* device = DirectX11::Get().GetDevice().Get();
        auto* context = DirectX11::Get().GetContext().Get();
        if (device == nullptr || context == nullptr)
        {
            return;
        }

        auto& resources = GetStageSelectFront3D();
        resources.Ensure(device, context);
        const int stage = std::clamp(stageIndex, 1, 3);
        const float aspect = (height > 1.0f) ? (width / height) : (16.0f / 9.0f);
        const Matrix projection = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(50.0f), aspect, 0.1f, 900.0f);
        const float camYaw = 0.45f + blinkTimer * 0.18f + stage * 0.22f;
        const float camDist = 18.5f;
        const Vector3 eye(std::sin(camYaw) * camDist, 8.0f + std::sin(blinkTimer * 0.35f) * 0.5f, std::cos(camYaw) * camDist);
        const Vector3 target(0.0f, 1.0f, 0.0f);
        const Matrix view = Matrix::CreateLookAt(eye, target, Vector3::Up);

        resources.backdrop.Render(context, view, projection, eye);
        if (resources.arenas[stage - 1])
        {
            resources.arenas[stage - 1]->Render(context, view, projection);
        }

        if (resources.cylinder && resources.sphere && resources.torus)
        {
            const Color stageAccent = (stage == 1) ? Color(0.54f, 0.88f, 1.0f, 1.0f)
                : ((stage == 2) ? Color(1.0f, 0.74f, 0.30f, 1.0f) : Color(0.42f, 0.96f, 0.88f, 1.0f));
            const float pulse = 0.5f + 0.5f * std::sin(blinkTimer * 3.4f);
            resources.cylinder->Draw(Matrix::CreateScale(0.16f, 1.9f, 0.16f) * Matrix::CreateTranslation(0.0f, 1.9f, 0.0f), view, projection, Color(stageAccent.x * 0.55f, stageAccent.y * 0.55f, stageAccent.z * 0.55f, 1.0f));
            resources.sphere->Draw(Matrix::CreateScale(0.95f + pulse * 0.22f) * Matrix::CreateTranslation(0.0f, 4.2f, 0.0f), view, projection, stageAccent);
            resources.torus->Draw(Matrix::CreateScale(1.4f + pulse * 0.22f, 1.0f, 1.4f + pulse * 0.22f) * Matrix::CreateRotationX(DirectX::XM_PIDIV2) * Matrix::CreateTranslation(0.0f, 0.22f, 0.0f), view, projection, Color(stageAccent.x, stageAccent.y, stageAccent.z, 0.82f));

            const auto& spawnPoints = resources.arenas[stage - 1]->GetSpawnPoints();
            const size_t previewCount = std::min<size_t>(4, spawnPoints.size());
            for (size_t i = 0; i < previewCount; ++i)
            {
                const Vector3 pos = spawnPoints[i];
                resources.sphere->Draw(Matrix::CreateScale(0.52f) * Matrix::CreateTranslation(pos.x, pos.y + 0.7f, pos.z), view, projection, Color(1.0f, 0.22f, 0.24f, 1.0f));
                resources.torus->Draw(Matrix::CreateScale(0.85f + 0.1f * i, 1.0f, 0.85f + 0.1f * i) * Matrix::CreateRotationX(DirectX::XM_PIDIV2) * Matrix::CreateTranslation(pos.x, 0.12f, pos.z), view, projection, Color(stageAccent.x, stageAccent.y, stageAccent.z, 0.46f));
            }
        }
    }

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

    // enable mouse for stage select: show cursor and allow absolute mouse
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    System::InputManager::GetInstance().ResetMouseDelta();
    SetSystemCursorVisible(true);

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

    // mouse input: allow hover and click on stage cards
    const DirectX::Mouse::State* ms = input.GetMouseState();
    if (ms)
    {
        const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
        const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
        const float uiScale = std::max(0.80f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));
        const float frameX = width * 0.08f;
        const float frameY = height * 0.10f;
        const float frameW = width * 0.84f;
        const float detailW = frameW * 0.42f;
        const float listX = frameX + detailW + 26.0f * uiScale;
        const float listY = frameY + 74.0f * uiScale;
        const float cardW = frameW - detailW - 44.0f * uiScale;
        const float cardH = 92.0f * uiScale;
        const float cardGap = 14.0f * uiScale;

        for (int i = 0; i < kStageCount; ++i)
        {
            const float y = listY + static_cast<float>(i) * (cardH + cardGap);
            if (ms->x >= listX && ms->x <= listX + cardW && ms->y >= y && ms->y <= y + cardH)
            {
                if (m_selectedStage != i) { m_selectedStage = i; SceneUiSound::PlayMove(); }
                if (input.IsMouseButtonPressed(0))
                {
                    SceneUiSound::PlayConfirm();
                    launchSelectedStage();
                    return;
                }
            }
        }
    }

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

    RenderStageSelectFront3D(m_selectedStage + 1, m_blinkTimer, width, height);

    BeginSpriteLayer();

    for (int band = 0; band < 14; ++band)
    {
        const float t = static_cast<float>(band) / 13.0f;
        const float y = t * height;
        DrawSolidRect(
            batch,
            Vector2(0.0f, y),
            Vector2(width, height / 13.0f + 2.0f),
            Color(0.02f + t * 0.04f, 0.04f + t * 0.05f, 0.08f + t * 0.08f, 0.18f));
    }

    DrawSolidRect(batch, Vector2(frameX, frameY), Vector2(frameW, frameH), Color(0.03f, 0.05f, 0.08f, 0.60f));
    DrawSolidRect(batch, Vector2(frameX, frameY), Vector2(frameW, 2.0f), Color(0.62f, 0.80f, 1.0f, 0.92f));

    // Japanese localized headings and reorganized left panel priority
    ui->Draw(batch, L"ステージ選択", Vector2(frameX + 24.0f * uiScale, frameY + 18.0f * uiScale), titleStyle, 1.28f * uiScale);
    ui->Draw(batch, L"ミッションを選択して出撃してください。", Vector2(frameX + 260.0f * uiScale, frameY + 24.0f * uiScale), bodyStyle, 0.72f * uiScale);

    DrawSolidRect(batch, Vector2(detailX, detailY), Vector2(detailW - 18.0f * uiScale, detailH), Color(0.05f, 0.08f, 0.12f, 0.70f));
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

    // 数値情報
    const std::wstring timeText = UiUtil::ToWStringFixed(selectedRule.stageStartTimeSec, 0) + L" 秒";
    const std::wstring aliveText = std::to_wstring(selectedRule.maxAliveCount);
    const std::wstring waveText = std::to_wstring(selectedRule.totalWaveCount);

    ui->Draw(batch, L"制限時間", Vector2(detailX + 14.0f * uiScale, detailY + 322.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, timeText, Vector2(detailX + 130.0f * uiScale, detailY + 322.0f * uiScale), bodyStyle, 0.58f * uiScale);
    ui->Draw(batch, L"同時上限", Vector2(detailX + 14.0f * uiScale, detailY + 348.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, aliveText, Vector2(detailX + 130.0f * uiScale, detailY + 348.0f * uiScale), bodyStyle, 0.58f * uiScale);
    ui->Draw(batch, L"WAVE数", Vector2(detailX + 14.0f * uiScale, detailY + 374.0f * uiScale), accentStyle, 0.58f * uiScale);
    ui->Draw(batch, waveText, Vector2(detailX + 130.0f * uiScale, detailY + 374.0f * uiScale), bodyStyle, 0.58f * uiScale);

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
            selected ? Color(0.10f, 0.14f, 0.22f, 0.78f) : Color(0.06f, 0.08f, 0.12f, 0.56f));
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

        ui->Draw(batch, cardTitle, drawPos + Vector2(18.0f * uiScale, 14.0f * uiScale), selected ? accentStyle : titleStyle, (selected ? 0.92f : 0.88f) * uiScale);
        ui->Draw(
            batch,
            FitTextWidth(ui, cardSummary, 0.56f * uiScale, drawSize.x - 178.0f * uiScale),
            drawPos + Vector2(18.0f * uiScale, 46.0f * uiScale),
            bodyStyle,
            0.56f * uiScale);
        ui->Draw(batch, cardTime, drawPos + Vector2(drawSize.x - 84.0f * uiScale, 18.0f * uiScale), bodyStyle, 0.58f * uiScale);
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
