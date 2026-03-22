#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GameSceneHud.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/UIConfig.h"

#include "../../Utility/Sound/AudioSystem.h"
#include "../../Utility/SimpleMathEx.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <wrl/client.h>
#include <chrono>

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

namespace
{
    std::mt19937 s_rng(42);

    Color HpColor(float ratio)
    {
        if (ratio > 0.5f)
        {
            return Color::Lerp(hpBarMidColor, hpBarFullColor, (ratio - 0.5f) * 2.0f);
        }
        return Color::Lerp(hpBarLowColor, hpBarMidColor, ratio * 2.0f);
    }
}

void GameSceneHud::Update(float dt)
{
    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        const float t = std::max(0.0f, m_shakeTimer);
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        m_shakeOffset = Vector2(dist(s_rng), dist(s_rng)) * m_shakeIntensity * t;
    }
    else
    {
        m_shakeOffset = Vector2::Zero;
    }

    if (m_bannerTimer > 0.0f)
    {
        m_bannerTimer -= dt;
    }

    if (m_comboPopTimer > 0.0f)
    {
        m_comboPopTimer -= dt;
    }

    if (m_killPulseTimer > 0.0f)
    {
        m_killPulseTimer -= dt;
    }
}

void GameSceneHud::DrawBar(
    float x,
    float y,
    float w,
    float h,
    float ratio,
    Color fill,
    Color back)
{
    if (!m_spriteBatch || !m_whiteTex)
    {
        return;
    }

    const RECT backRect = {
        static_cast<LONG>(x),
        static_cast<LONG>(y),
        static_cast<LONG>(x + w),
        static_cast<LONG>(y + h)
    };
    m_spriteBatch->Draw(m_whiteTex.Get(), backRect, back);

    const float clampedRatio = std::max(0.0f, (std::min)(1.0f, ratio));
    const RECT fillRect = {
        static_cast<LONG>(x),
        static_cast<LONG>(y),
        static_cast<LONG>(x + w * clampedRatio),
        static_cast<LONG>(y + h)
    };
    m_spriteBatch->Draw(m_whiteTex.Get(), fillRect, fill);
}

void GameSceneHud::Render(
    const Action::GameState& gs,
    const Action::PlayerState& player,
    const Action::SurvivalDirector& director,
    const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints,
    const std::vector<DirectX::SimpleMath::Vector4>& wallRects,
    const std::vector<Action::EnemyState>& enemies,
    bool speedActive,
    float speedRemaining,
    int screenW,
    int screenH)
{
    if (!m_spriteBatch)
    {
        return;
    }

    (void)speedActive;
    (void)speedRemaining;

    const float width = static_cast<float>(screenW);
    const float height = static_cast<float>(screenH);
    const float variantMul = (SystemUI::GetVariant() == SystemUI::Variant::Premium) ? 1.18f : 1.0f;
    const float uiScale = std::max(1.0f, height / 720.0f) * variantMul;

    const auto DrawRect = [&](float x, float y, float w, float h, const Color& color)
    {
        if (!m_whiteTex)
        {
            return;
        }

        const RECT rect = {
            static_cast<LONG>(x),
            static_cast<LONG>(y),
            static_cast<LONG>(x + w),
            static_cast<LONG>(y + h)
        };
        m_spriteBatch->Draw(m_whiteTex.Get(), rect, color);
    };

    const float yellowThresh = 60.0f;
    const float redThresh = 30.0f;
    const float blinkThresh = 12.0f;
    Color timeColor(0.96f, 0.96f, 0.96f, 1.0f);
    if (gs.stageTimer <= redThresh)
    {
        const float t = Utility::MathEx::Clamp(gs.stageTimer / redThresh, 0.0f, 1.0f);
        timeColor = Color(1.0f, 0.28f + 0.60f * t, 0.12f + 0.14f * t, 1.0f);
    }
    else if (gs.stageTimer <= yellowThresh)
    {
        const float t = Utility::MathEx::Clamp((gs.stageTimer - redThresh) / (yellowThresh - redThresh), 0.0f, 1.0f);
        timeColor = Color(1.0f, 0.68f + 0.28f * t, 0.18f + 0.68f * t, 1.0f);
    }
    if (gs.stageTimer <= blinkThresh)
    {
        using namespace std::chrono;
        const auto nowMs = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
        const float pulse = 0.55f + 0.45f * std::sin(static_cast<float>(nowMs) * 0.010f);
        timeColor = Color(timeColor.R(), timeColor.G(), timeColor.B(), pulse);
    }

    if (m_prevStageTimer > yellowThresh && gs.stageTimer <= yellowThresh)
    {
        GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::WarningAlert, 0.9f);
        m_timeWarnState = 1;
    }
    if (m_prevStageTimer > redThresh && gs.stageTimer <= redThresh)
    {
        GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::WarningAlert, 1.0f);
        m_timeWarnState = 2;
    }
    m_prevStageTimer = gs.stageTimer;

    const float centerPanelW = 228.0f * uiScale;
    const float centerPanelH = 62.0f * uiScale;
    const float centerPanelX = width * 0.5f - centerPanelW * 0.5f;
    const float centerPanelY = 16.0f * uiScale;
    DrawRect(centerPanelX, centerPanelY, centerPanelW, centerPanelH, Color(0.03f, 0.04f, 0.06f, 0.46f));
    DrawRect(centerPanelX + 2.0f, centerPanelY + 2.0f, centerPanelW - 4.0f, centerPanelH - 4.0f, Color(0.07f, 0.09f, 0.12f, 0.40f));

    wchar_t timeBuf[32];
    const int totalSec = static_cast<int>(gs.stageTimer);
    swprintf_s(timeBuf, L"%02d:%02d", totalSec / 60, totalSec % 60);
    const Vector2 timeSize = m_font ? Vector2(m_font->MeasureString(timeBuf)) : Vector2::Zero;
    DrawText(L"TIME", centerPanelX + 14.0f * uiScale, centerPanelY + 10.0f * uiScale, Color(0.72f, 0.76f, 0.82f, 0.95f), 0.68f * uiScale);
    DrawText(timeBuf, width * 0.5f - timeSize.x * 0.5f * 0.98f * uiScale, centerPanelY + 24.0f * uiScale, timeColor, 0.98f * uiScale);

    const float leftPanelX = 20.0f * uiScale;
    const float leftPanelY = 20.0f * uiScale;
    const float leftPanelW = 182.0f * uiScale;
    const float leftPanelH = 64.0f * uiScale;
    const float killPulse = Utility::MathEx::Clamp(m_killPulseTimer / 0.5f, 0.0f, 1.0f);
    DrawRect(leftPanelX, leftPanelY, leftPanelW, leftPanelH, Color(0.03f + killPulse * 0.08f, 0.04f + killPulse * 0.03f, 0.05f, 0.46f));
    DrawText(L"KILLS", leftPanelX + 14.0f * uiScale, leftPanelY + 10.0f * uiScale, Color(0.74f, 0.78f, 0.84f, 0.95f), 0.64f * uiScale);
    wchar_t killBuf[32];
    swprintf_s(killBuf, L"%04d", gs.killCount);
    DrawText(killBuf, leftPanelX + 14.0f * uiScale, leftPanelY + 28.0f * uiScale, Color(1.0f, 0.90f, 0.46f, 1.0f), (0.92f + killPulse * 0.08f) * uiScale);

    const float rightPanelW = 196.0f * uiScale;
    const float rightPanelH = 58.0f * uiScale;
    const float rightPanelX = width - rightPanelW - 20.0f * uiScale;
    const float rightPanelY = 20.0f * uiScale;
    DrawRect(rightPanelX, rightPanelY, rightPanelW, rightPanelH, Color(0.03f, 0.04f, 0.06f, 0.46f));
    DrawText(L"WAVE", rightPanelX + 14.0f * uiScale, rightPanelY + 10.0f * uiScale, Color(0.74f, 0.78f, 0.84f, 0.95f), 0.60f * uiScale);
    wchar_t waveBuf[32];
    swprintf_s(waveBuf, L"%d / %d", director.GetCurrentWave(), director.GetTotalWaveCount());
    DrawText(waveBuf, rightPanelX + 14.0f * uiScale, rightPanelY + 28.0f * uiScale, Color(0.98f, 0.86f, 0.40f, 1.0f), 0.84f * uiScale);

    const float mapPanelSize = 144.0f * uiScale;
    const float mapPanelX = width - mapPanelSize - 18.0f * uiScale;
    const float mapPanelY = height - mapPanelSize - 18.0f * uiScale;
    const float mapInset = 10.0f * uiScale;
    const float mapInnerX = mapPanelX + mapInset;
    const float mapInnerY = mapPanelY + mapInset;
    const float mapInnerSize = mapPanelSize - mapInset * 2.0f;
    DrawRect(mapPanelX, mapPanelY, mapPanelSize, mapPanelSize, Color(0.03f, 0.04f, 0.06f, 0.42f));
    DrawRect(mapInnerX, mapInnerY, mapInnerSize, mapInnerSize, Color(0.07f, 0.10f, 0.14f, 0.34f));

    float arenaHalfExtent = 20.0f;
    for (const DirectX::SimpleMath::Vector4& wall : wallRects)
    {
        arenaHalfExtent = (std::max)(arenaHalfExtent, (std::max)(std::abs(wall.x), std::abs(wall.y)));
        arenaHalfExtent = (std::max)(arenaHalfExtent, (std::max)(std::abs(wall.z), std::abs(wall.w)));
    }
    for (const DirectX::SimpleMath::Vector3& spawnPoint : spawnPoints)
    {
        arenaHalfExtent = (std::max)(arenaHalfExtent, (std::max)(std::abs(spawnPoint.x), std::abs(spawnPoint.z)));
    }
    arenaHalfExtent = (std::max)(arenaHalfExtent, (std::max)(std::abs(player.position.x), std::abs(player.position.z)));
    arenaHalfExtent += 2.0f;

    const auto ToMinimap = [&](float worldX, float worldZ)
    {
        const float nx = Utility::MathEx::Clamp(worldX / arenaHalfExtent, -1.0f, 1.0f);
        const float nz = Utility::MathEx::Clamp(worldZ / arenaHalfExtent, -1.0f, 1.0f);
        return Vector2(
            mapInnerX + mapInnerSize * 0.5f + nx * mapInnerSize * 0.45f,
            mapInnerY + mapInnerSize * 0.5f - nz * mapInnerSize * 0.45f);
    };

    for (const DirectX::SimpleMath::Vector4& wall : wallRects)
    {
        const Vector2 minPos = ToMinimap(wall.x, wall.w);
        const Vector2 maxPos = ToMinimap(wall.z, wall.y);
        const float rectX = (std::min)(minPos.x, maxPos.x);
        const float rectY = (std::min)(minPos.y, maxPos.y);
        const float rectW = (std::max)(2.0f * uiScale, std::abs(maxPos.x - minPos.x));
        const float rectH = (std::max)(2.0f * uiScale, std::abs(maxPos.y - minPos.y));
        DrawRect(rectX, rectY, rectW, rectH, Color(0.30f, 0.40f, 0.52f, 0.65f));
    }

    for (const DirectX::SimpleMath::Vector3& spawnPoint : spawnPoints)
    {
        const Vector2 point = ToMinimap(spawnPoint.x, spawnPoint.z);
        DrawRect(point.x - 1.0f * uiScale, point.y - 1.0f * uiScale, 2.0f * uiScale, 2.0f * uiScale, Color(0.44f, 0.62f, 0.82f, 0.30f));
    }

    int visibleEnemyCount = 0;
    for (const Action::EnemyState& enemy : enemies)
    {
        if (enemy.state == Action::EnemyStateType::Dead || enemy.hp <= 0.0f)
        {
            continue;
        }

        const Vector2 point = ToMinimap(enemy.position.x, enemy.position.z);
        const float size =
            enemy.isHeavyEnemy ? 4.6f * uiScale :
            enemy.isLaserEnemy ? 4.0f * uiScale :
                                 3.2f * uiScale;
        const Color color =
            enemy.isHeavyEnemy ? Color(0.60f, 0.94f, 0.84f, 0.95f) :
            enemy.isLaserEnemy ? Color(1.0f, 0.66f, 0.20f, 0.95f) :
                                 Color(1.0f, 0.26f, 0.26f, 0.88f);
        DrawRect(point.x - size * 0.5f, point.y - size * 0.5f, size, size, color);

        ++visibleEnemyCount;
        if (visibleEnemyCount >= 40)
        {
            break;
        }
    }

    const Vector2 playerPoint = ToMinimap(player.position.x, player.position.z);
    const float playerDot = 5.6f * uiScale;
    DrawRect(playerPoint.x - playerDot * 0.5f, playerPoint.y - playerDot * 0.5f, playerDot, playerDot, Color(0.96f, 0.98f, 1.0f, 1.0f));
    const Vector2 forward(std::sin(player.yaw), -std::cos(player.yaw));
    DrawRect(
        playerPoint.x + forward.x * 5.0f * uiScale - 1.1f * uiScale,
        playerPoint.y + forward.y * 5.0f * uiScale - 1.1f * uiScale,
        2.2f * uiScale,
        2.2f * uiScale,
        Color(0.50f, 0.82f, 1.0f, 1.0f));

    if (m_bannerTimer > 0.0f)
    {
        const float alpha = (std::min)(1.0f, m_bannerTimer < 0.45f ? (m_bannerTimer / 0.45f) : 1.0f);
        wchar_t bannerBuf[64];
        swprintf_s(bannerBuf, L"WAVE %d / %d", m_bannerWave, m_bannerTotal);
        const Vector2 bannerSize = m_font ? Vector2(m_font->MeasureString(bannerBuf)) : Vector2::Zero;
        DrawText(
            bannerBuf,
            width * 0.5f - bannerSize.x * 0.5f * 1.08f * uiScale,
            height * 0.20f,
            Color(1.0f, 0.88f, 0.36f, alpha),
            1.08f * uiScale);
    }

    if (m_comboPopTimer > 0.0f)
    {
        const float alpha = Utility::MathEx::Clamp(m_comboPopTimer / 0.6f, 0.0f, 1.0f);
        const float rise = (0.6f - m_comboPopTimer) * 42.0f;
        const wchar_t* label =
            (m_comboPopIndex >= 3) ? L"CHAIN x3" :
            (m_comboPopIndex == 2) ? L"CHAIN x2" :
                                     L"HIT";
        DrawText(
            label,
            width * 0.5f + 92.0f * uiScale,
            height * 0.56f - rise,
            Color(1.0f, 0.84f, 0.22f, alpha),
            (0.74f + (1.0f - alpha) * 0.20f) * uiScale);
    }

}

void GameSceneHud::Initialize(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    DirectX::SpriteBatch* spriteBatch,
    DirectX::SpriteFont* font)
{
    m_spriteBatch = spriteBatch;
    m_font = font;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = 1;
    desc.Height = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    constexpr std::uint32_t white = 0xFFFFFFFFu;
    D3D11_SUBRESOURCE_DATA initData = { &white, sizeof(std::uint32_t), 0 };
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (SUCCEEDED(device->CreateTexture2D(&desc, &initData, texture.GetAddressOf())))
    {
        device->CreateShaderResourceView(texture.Get(), nullptr, m_whiteTex.GetAddressOf());
    }

    (void)context;
}

void GameSceneHud::TriggerScreenShake(float intensity)
{
    m_shakeIntensity = intensity;
    m_shakeTimer = intensity * 0.4f;
}

void GameSceneHud::TriggerWaveBanner(int waveNumber, int totalWaves)
{
    m_bannerWave = waveNumber;
    m_bannerTotal = totalWaves;
    m_bannerTimer = 2.5f;
}

void GameSceneHud::TriggerComboPopup(int comboIndex, int comboLevel)
{
    m_comboPopIndex = comboIndex;
    m_comboPopLevel = comboLevel;
    m_comboPopTimer = 0.6f;
}

void GameSceneHud::TriggerKillPulse(int totalKills)
{
    m_killPulseKills = totalKills;
    m_killPulseTimer = 0.5f;
}

void GameSceneHud::DrawText(const std::wstring& text, float x, float y, Color color, float scale)
{
    if (!m_font || !m_spriteBatch)
    {
        return;
    }

    m_font->DrawString(
        m_spriteBatch,
        text.c_str(),
        DirectX::XMFLOAT2(x, y),
        color,
        0.0f,
        DirectX::XMFLOAT2(0.0f, 0.0f),
        scale);
}

void GameSceneHud::RenderPauseMenu(int selectedIndex, int screenW, int screenH)
{
    if (!m_spriteBatch || !m_whiteTex || !m_font) return;
    const float width = static_cast<float>(screenW);
    const float height = static_cast<float>(screenH);

    // overlay
    const RECT fullRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    m_spriteBatch->Draw(m_whiteTex.Get(), fullRect, DirectX::SimpleMath::Color(0.02f, 0.02f, 0.02f, 0.6f));

    // modal box
    const float boxW = 420.0f; const float boxH = 220.0f;
    const float bx = width * 0.5f - boxW * 0.5f;
    const float by = height * 0.5f - boxH * 0.5f;
    const RECT boxRect = { static_cast<LONG>(bx), static_cast<LONG>(by), static_cast<LONG>(bx + boxW), static_cast<LONG>(by + boxH) };
    m_spriteBatch->Draw(m_whiteTex.Get(), boxRect, DirectX::SimpleMath::Color(0.06f, 0.08f, 0.12f, 0.92f));

    // title
    m_font->DrawString(m_spriteBatch, L"一時停止", DirectX::XMFLOAT2(bx + 24.0f, by + 18.0f), DirectX::SimpleMath::Color(0.95f,0.95f,0.95f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), 1.4f);

    // menu options
    const std::wstring items[] = { L"ゲームに戻る", L"タイトルへ戻る", L"設定", L"終了" };
    for (int i = 0; i < 4; ++i)
    {
        const float iy = by + 68.0f + i * 36.0f;
        const bool sel = (i == selectedIndex);
        const auto bg = sel ? DirectX::SimpleMath::Color(0.16f,0.24f,0.36f,0.95f) : DirectX::SimpleMath::Color(0.07f,0.09f,0.12f,0.88f);
        const RECT itRect = { static_cast<LONG>(bx + 16.0f), static_cast<LONG>(iy - 6.0f), static_cast<LONG>(bx + boxW - 16.0f), static_cast<LONG>(iy + 28.0f) };
        m_spriteBatch->Draw(m_whiteTex.Get(), itRect, bg);
        const auto color = sel ? DirectX::SimpleMath::Color(0.98f,0.98f,1.0f,1.0f) : DirectX::SimpleMath::Color(0.86f,0.86f,0.86f,1.0f);
        m_font->DrawString(m_spriteBatch, items[i].c_str(), DirectX::XMFLOAT2(bx + 28.0f, iy), color, 0.0f, DirectX::XMFLOAT2(0,0), sel ? 1.05f : 0.95f);
    }
}
