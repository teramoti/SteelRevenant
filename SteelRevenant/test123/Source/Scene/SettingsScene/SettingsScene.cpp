#include "SettingsScene.h"

#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/UiUtil.h"
#include "../../Utility/Sound/AudioSystem.h"
#include "../Base/SceneUiSound.h"

#include <Keyboard.h>
#include <Mouse.h>
#include <algorithm>
#include <array>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

extern void ExitGame();

namespace
{
    constexpr float kSensitivityMin = 0.01f;
    constexpr float kSensitivityMax = 0.35f;
    constexpr float kSensitivityStep = 0.01f;
    constexpr float kVolumeMin = 0.0f;
    constexpr float kVolumeMax = 1.0f;
    constexpr float kVolumeStep = 0.05f;
    constexpr float kClickFxDurationSec = 0.22f;
    constexpr int kSettingControlCount = 5;
    constexpr int kBackItemIndex = kSettingControlCount;
    constexpr int kTitleItemIndex = kSettingControlCount + 1;
    constexpr int kQuitItemIndex = kSettingControlCount + 2;
    constexpr int kMenuItemCount = kSettingControlCount + 3;

    enum SettingControlIndex
    {
        MouseSensitivity = 0,
        MasterVolume,
        BgmVolume,
        SeVolume,
        UiVolume,
    };

    float ClampVolume(float value)
    {
        return std::max(kVolumeMin, (std::min)(kVolumeMax, value));
    }

    const wchar_t* GetSettingLabel(int index)
    {
        switch (index)
        {
        case MouseSensitivity:
            return L"マウス感度";
        case MasterVolume:
            return L"マスター音量";
        case BgmVolume:
            return L"BGM 音量";
        case SeVolume:
            return L"効果音音量";
        case UiVolume:
            return L"UI 音量";
        default:
            return L"";
        }
    }

    const wchar_t* GetSettingDetail(int index)
    {
        switch (index)
        {
        case MouseSensitivity:
            return L"カメラの回転速度";
        case MasterVolume:
            return L"出力全体の音量";
        case BgmVolume:
            return L"BGM の音量";
        case SeVolume:
            return L"アクション効果音の音量";
        case UiVolume:
            return L"メニュー音の音量";
        default:
            return L"";
        }
    }
}

SettingsScene::SettingsScene(SceneManager* sceneManager)
    : ActionSceneBase(sceneManager, false)
{
    m_SceneFlag = false;
}

SettingsScene::~SettingsScene()
{
    Finalize();
}

void SettingsScene::Initialize()
{
    m_selectedItem = 0;
    m_sceneTime = 0.0f;
    m_backScene = TITLE_SCENE;
    if (m_sceneManager != nullptr)
    {
        m_backScene = m_sceneManager->HasSuspendedScene()
            ? m_sceneManager->GetSuspendedSceneId()
            : m_sceneManager->GetPreviousSceneId();
    }
    m_mouseSensitivity = GameSaveData::GetInstance().GetMouseSensitivity();
    System::InputManager::GetInstance().SetMouseSensitivity(m_mouseSensitivity);
    m_audioVolume = GameSaveData::GetInstance().GetAudioVolumeSettings();
    m_clickFxTimer = 0.0f;
    m_clickFxPos = Vector2::Zero;
    m_uiSolidTexture = UiUtil::CreateSolidTexture(m_directX.GetDevice().Get());

    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    System::InputManager::GetInstance().ResetMouseDelta();
    SetSystemCursorVisible(true);
    EnsureTextRenderer();
}

void SettingsScene::Update(const DX::StepTimer& stepTimer)
{
    const float dt = static_cast<float>(stepTimer.GetElapsedSeconds());
    if (dt <= 0.0f)
    {
        return;
    }

    m_sceneTime += dt;
    m_clickFxTimer = std::max(0.0f, m_clickFxTimer - dt);

    if (m_textRenderer != nullptr)
    {
        m_textRenderer->Update(dt);
    }

    const System::InputManager& input = System::InputManager::GetInstance();
    const DirectX::Keyboard::KeyboardStateTracker* keyboardTracker = input.GetKeyboardTracker();
    const DirectX::Mouse::ButtonStateTracker* mouseTracker = input.GetMouseTracker();
    const DirectX::Mouse::State* mouseState = input.GetMouseState();
    if (keyboardTracker == nullptr || mouseTracker == nullptr || mouseState == nullptr)
    {
        return;
    }

    const Vector2 mousePoint(static_cast<float>(mouseState->x), static_cast<float>(mouseState->y));
    const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
    const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
    const float scale = std::max(0.8f, (std::min)(1.3f, (std::min)(width / 1280.0f, height / 720.0f)));
    const Vector2 panelPos(width * 0.18f, height * 0.14f);
    const Vector2 panelSize(width * 0.64f, height * 0.72f);
    const Vector2 rowSize(panelSize.x - 80.0f * scale, 44.0f * scale);
    const float rowStartY = panelPos.y + 118.0f * scale;
    const float rowStepY = 52.0f * scale;
    const Vector2 buttonSize(panelSize.x - 80.0f * scale, 48.0f * scale);
    const Vector2 backPos(panelPos.x + 40.0f * scale, panelPos.y + panelSize.y - 170.0f * scale);
    const Vector2 titlePos(panelPos.x + 40.0f * scale, panelPos.y + panelSize.y - 112.0f * scale);
    const Vector2 quitPos(panelPos.x + 40.0f * scale, panelPos.y + panelSize.y - 54.0f * scale);

    auto rowRectPos = [&](int index) -> Vector2
    {
        return Vector2(panelPos.x + 40.0f * scale, rowStartY + static_cast<float>(index) * rowStepY);
    };

    const int previousSelected = m_selectedItem;
    if (keyboardTracker->pressed.Up || keyboardTracker->pressed.W)
    {
        m_selectedItem = (m_selectedItem + kMenuItemCount - 1) % kMenuItemCount;
    }
    if (keyboardTracker->pressed.Down || keyboardTracker->pressed.S)
    {
        m_selectedItem = (m_selectedItem + 1) % kMenuItemCount;
    }

    for (int index = 0; index < kSettingControlCount; ++index)
    {
        if (UiUtil::IsInsideRect(mousePoint, rowRectPos(index), rowSize))
        {
            m_selectedItem = index;
        }
    }
    if (UiUtil::IsInsideRect(mousePoint, backPos, buttonSize))
    {
        m_selectedItem = kBackItemIndex;
    }
    if (UiUtil::IsInsideRect(mousePoint, titlePos, buttonSize))
    {
        m_selectedItem = kTitleItemIndex;
    }
    if (UiUtil::IsInsideRect(mousePoint, quitPos, buttonSize))
    {
        m_selectedItem = kQuitItemIndex;
    }

    if (m_selectedItem != previousSelected)
    {
        SceneUiSound::PlayMove();
    }

    if (m_selectedItem >= 0 && m_selectedItem < kSettingControlCount)
    {
        const float step = (m_selectedItem == MouseSensitivity) ? kSensitivityStep : kVolumeStep;
        if (keyboardTracker->pressed.Left || keyboardTracker->pressed.A)
        {
            SetSettingValue(m_selectedItem, GetSettingValue(m_selectedItem) - step);
            SceneUiSound::PlayMove();
        }
        if (keyboardTracker->pressed.Right || keyboardTracker->pressed.D)
        {
            SetSettingValue(m_selectedItem, GetSettingValue(m_selectedItem) + step);
            SceneUiSound::PlayMove();
        }
    }

    if (mouseTracker->leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
    {
        m_clickFxTimer = kClickFxDurationSec;
        m_clickFxPos = mousePoint;

        for (int index = 0; index < kSettingControlCount; ++index)
        {
            if (!UiUtil::IsInsideRect(mousePoint, rowRectPos(index), rowSize))
            {
                continue;
            }

            const float ratio = std::max(0.0f, (std::min)(1.0f, (mousePoint.x - rowRectPos(index).x - 260.0f * scale) / (rowSize.x - 320.0f * scale)));
            if (index == MouseSensitivity)
            {
                SetSettingValue(index, kSensitivityMin + (kSensitivityMax - kSensitivityMin) * ratio);
            }
            else
            {
                SetSettingValue(index, ratio);
            }
            SceneUiSound::PlayMove();
            return;
        }

        if (UiUtil::IsInsideRect(mousePoint, backPos, buttonSize))
        {
            SceneUiSound::PlayBack();
            ReturnToBackScene(m_backScene);
            return;
        }
        if (UiUtil::IsInsideRect(mousePoint, titlePos, buttonSize))
        {
            SceneUiSound::PlayConfirm();
            m_sceneManager->RequestTransition(TITLE_SCENE);
            return;
        }
        if (UiUtil::IsInsideRect(mousePoint, quitPos, buttonSize))
        {
            SceneUiSound::PlayConfirm();
            ExitGame();
            return;
        }
    }

    if (keyboardTracker->pressed.Enter || keyboardTracker->pressed.Space)
    {
        if (m_selectedItem == kBackItemIndex)
        {
            SceneUiSound::PlayBack();
            ReturnToBackScene(m_backScene);
            return;
        }
        if (m_selectedItem == kTitleItemIndex)
        {
            SceneUiSound::PlayConfirm();
            m_sceneManager->RequestTransition(TITLE_SCENE);
            return;
        }
        if (m_selectedItem == kQuitItemIndex)
        {
            SceneUiSound::PlayConfirm();
            ExitGame();
            return;
        }
    }

    if (keyboardTracker->pressed.Escape)
    {
        SceneUiSound::PlayBack();
        ReturnToBackScene(m_backScene);
    }
}

void SettingsScene::Render()
{
    System::UIShaderText* ui = EnsureTextRenderer();
    if (ui == nullptr)
    {
        return;
    }

    BeginSpriteLayer();
    DirectX::SpriteBatch* batch = System::DrawManager::GetInstance().GetSprite();
    if (batch == nullptr)
    {
        EndSpriteLayer();
        return;
    }

    const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
    const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
    const float scale = std::max(0.8f, (std::min)(1.3f, (std::min)(width / 1280.0f, height / 720.0f)));
    const Vector2 panelPos(width * 0.18f, height * 0.14f);
    const Vector2 panelSize(width * 0.64f, height * 0.72f);
    const Vector2 rowSize(panelSize.x - 80.0f * scale, 44.0f * scale);
    const float rowStartY = panelPos.y + 118.0f * scale;
    const float rowStepY = 52.0f * scale;
    const Vector2 buttonSize(panelSize.x - 80.0f * scale, 48.0f * scale);
    const Vector2 backPos(panelPos.x + 40.0f * scale, panelPos.y + panelSize.y - 170.0f * scale);
    const Vector2 titlePos(panelPos.x + 40.0f * scale, panelPos.y + panelSize.y - 112.0f * scale);
    const Vector2 quitPos(panelPos.x + 40.0f * scale, panelPos.y + panelSize.y - 54.0f * scale);

    auto rowRectPos = [&](int index) -> Vector2
    {
        return Vector2(panelPos.x + 40.0f * scale, rowStartY + static_cast<float>(index) * rowStepY);
    };

    DrawSolidRect(batch, Vector2::Zero, Vector2(width, height), Color(0.02f, 0.03f, 0.05f, 0.94f));
    DrawSolidRect(batch, panelPos, panelSize, Color(0.05f, 0.08f, 0.12f, 0.88f));
    DrawSolidRect(batch, panelPos, Vector2(panelSize.x, 2.0f * scale), Color(0.58f, 0.82f, 1.0f, 0.88f));

    System::UIShaderStyle titleStyle;
    titleStyle.baseColor = Color(0.94f, 0.98f, 1.0f, 1.0f);
    titleStyle.outlineColor = Color(0.06f, 0.08f, 0.10f, 1.0f);
    titleStyle.pulseAmount = 0.0f;
    titleStyle.pulseSpeed = 0.0f;

    System::UIShaderStyle normalStyle;
    normalStyle.baseColor = Color(0.84f, 0.90f, 0.96f, 1.0f);
    normalStyle.outlineColor = Color(0.05f, 0.06f, 0.08f, 1.0f);
    normalStyle.pulseAmount = 0.0f;

    System::UIShaderStyle selectedStyle = normalStyle;
    selectedStyle.baseColor = Color(0.68f, 0.90f, 1.0f, 1.0f);
    selectedStyle.outlineColor = Color(0.04f, 0.10f, 0.16f, 1.0f);

    ui->Draw(batch, L"設定", panelPos + Vector2(28.0f * scale, 20.0f * scale), titleStyle, 1.0f * scale);
    ui->Draw(batch, L"操作と音量を調整します", panelPos + Vector2(28.0f * scale, 54.0f * scale), normalStyle, 0.56f * scale);

    for (int index = 0; index < kSettingControlCount; ++index)
    {
        const Vector2 rowPos = rowRectPos(index);
        const bool selected = (m_selectedItem == index);
        DrawSolidRect(batch, rowPos, rowSize, selected ? Color(0.10f, 0.18f, 0.24f, 0.94f) : Color(0.07f, 0.10f, 0.14f, 0.86f));

        const float value = GetSettingValue(index);
        const float ratio = (index == MouseSensitivity)
            ? std::max(0.0f, (std::min)(1.0f, (value - kSensitivityMin) / (kSensitivityMax - kSensitivityMin)))
            : std::max(0.0f, (std::min)(1.0f, value));
        const Vector2 barPos = rowPos + Vector2(260.0f * scale, 26.0f * scale);
        const Vector2 barSize(rowSize.x - 320.0f * scale, 6.0f * scale);

        DrawSolidRect(batch, barPos, barSize, Color(0.16f, 0.18f, 0.22f, 0.95f));
        DrawSolidRect(batch, barPos, Vector2(barSize.x * ratio, barSize.y), Color(0.36f, 0.76f, 1.0f, 0.96f));
        DrawSolidRect(batch, barPos + Vector2(barSize.x * ratio - 2.0f * scale, -5.0f * scale), Vector2(4.0f * scale, 16.0f * scale), Color(0.95f, 0.98f, 1.0f, 0.96f));

        const int percentValue = static_cast<int>(std::round(value * 100.0f));
        const std::wstring valueText = UiUtil::ToWStringFixed(static_cast<float>(percentValue), 0) + L"%";

        ui->Draw(batch, GetSettingLabel(index), rowPos + Vector2(14.0f * scale, 6.0f * scale), selected ? selectedStyle : normalStyle, 0.60f * scale);
        ui->Draw(batch, GetSettingDetail(index), rowPos + Vector2(14.0f * scale, 24.0f * scale), normalStyle, 0.42f * scale);
        ui->Draw(batch, valueText, rowPos + Vector2(rowSize.x - 74.0f * scale, 10.0f * scale), selected ? selectedStyle : normalStyle, 0.58f * scale);
    }

    const auto drawButton = [&](const Vector2& pos, const wchar_t* label, const wchar_t* detail, bool selected, const Color& baseColor)
    {
        DrawSolidRect(batch, pos, buttonSize, selected ? baseColor : Color(0.08f, 0.11f, 0.14f, 0.84f));
        ui->Draw(batch, label, pos + Vector2(16.0f * scale, 10.0f * scale), selected ? selectedStyle : normalStyle, 0.68f * scale);
        ui->Draw(batch, detail, pos + Vector2(16.0f * scale, 28.0f * scale), normalStyle, 0.42f * scale);
    };

    drawButton(backPos, L"戻る", L"前の画面に戻る", m_selectedItem == kBackItemIndex, Color(0.10f, 0.18f, 0.24f, 0.92f));
    drawButton(titlePos, L"タイトルへ", L"タイトル画面に戻る", m_selectedItem == kTitleItemIndex, Color(0.12f, 0.20f, 0.28f, 0.92f));
    drawButton(quitPos, L"終了", L"アプリケーションを終了", m_selectedItem == kQuitItemIndex, Color(0.24f, 0.12f, 0.12f, 0.92f));

    if (m_clickFxTimer > 0.0f)
    {
        const float progress = 1.0f - (m_clickFxTimer / kClickFxDurationSec);
        const float radius = (6.0f + progress * 30.0f) * scale;
        const float alpha = (1.0f - progress) * 0.85f;
        for (int i = 0; i < 10; ++i)
        {
            const float angle = (DirectX::XM_2PI / 10.0f) * static_cast<float>(i) + progress * 1.8f;
            const Vector2 offset(std::cosf(angle) * radius, std::sinf(angle) * radius);
            DrawSolidRect(batch, m_clickFxPos + offset - Vector2(1.5f * scale, 1.5f * scale), Vector2(3.0f * scale, 3.0f * scale), Color(0.70f, 0.92f, 1.0f, alpha));
        }
    }

    EndSpriteLayer();
}

void SettingsScene::Finalize()
{
    m_uiSolidTexture.Reset();
    m_textRenderer.reset();
    m_sceneTime = 0.0f;
    m_clickFxTimer = 0.0f;
    m_clickFxPos = Vector2::Zero;
    // モードを相対移動に戻す
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
    System::InputManager::GetInstance().ResetMouseDelta();
}

void SettingsScene::DrawSolidRect(
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

void SettingsScene::ApplyMouseSensitivity(float value)
{
    m_mouseSensitivity = std::max(kSensitivityMin, (std::min)(kSensitivityMax, value));
    System::InputManager::GetInstance().SetMouseSensitivity(m_mouseSensitivity);
    GameSaveData::GetInstance().SetMouseSensitivity(m_mouseSensitivity);
}

float SettingsScene::GetSettingValue(int index) const
{
    switch (index)
    {
    case MouseSensitivity:
        return m_mouseSensitivity;
    case MasterVolume:
        return m_audioVolume.master;
    case BgmVolume:
        return m_audioVolume.bgm;
    case SeVolume:
        return m_audioVolume.se;
    case UiVolume:
        return m_audioVolume.ui;
    default:
        return 0.0f;
    }
}

void SettingsScene::SetSettingValue(int index, float value)
{
    switch (index)
    {
    case MouseSensitivity:
        ApplyMouseSensitivity(value);
        break;
    case MasterVolume:
        m_audioVolume.master = ClampVolume(value);
        GameAudio::AudioSystem::GetInstance().ApplyVolumeSettings(m_audioVolume);
        GameSaveData::GetInstance().SetAudioVolumeSettings(m_audioVolume);
        break;
    case BgmVolume:
        ApplyAudioBusVolume(GameAudio::AudioBus::Bgm, value);
        break;
    case SeVolume:
        ApplyAudioBusVolume(GameAudio::AudioBus::Se, value);
        break;
    case UiVolume:
        ApplyAudioBusVolume(GameAudio::AudioBus::Ui, value);
        break;
    default:
        break;
    }
}

void SettingsScene::ApplyAudioBusVolume(GameAudio::AudioBus bus, float value)
{
    const float clamped = ClampVolume(value);
    switch (bus)
    {
    case GameAudio::AudioBus::Bgm:
        m_audioVolume.bgm = clamped;
        break;
    case GameAudio::AudioBus::Se:
        m_audioVolume.se = clamped;
        break;
    case GameAudio::AudioBus::Ui:
        m_audioVolume.ui = clamped;
        break;
    default:
        break;
    }

    GameAudio::AudioSystem::GetInstance().ApplyVolumeSettings(m_audioVolume);
    GameSaveData::GetInstance().SetAudioVolumeSettings(m_audioVolume);
}

void SettingsScene::ReturnToBackScene(SceneID backScene)
{
    if (m_sceneManager != nullptr)
    {
        m_sceneManager->RequestTransition(backScene);
    }
}
