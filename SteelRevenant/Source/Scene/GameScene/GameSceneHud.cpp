//------------------------//------------------------
// Contents(処理内容) HUDの更新・描画を実装する。
// 追加機能: スクリーンシェイク・ウェーブバナー・コンボポップアップ
// Bug#4修正: 中継地点・ビーコン残存参照を含まない実装。
//------------------------//------------------------
#include "GameSceneHud.h"
#include "GameSceneVisualPalette.h"
#include <wrl/client.h>
#include <cmath>
#include <random>

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Color;

namespace
{
    static std::mt19937 s_rng(42);

    Color HpColor(float ratio)
    {
        if (ratio > 0.5f) return Color::Lerp(hpBarMidColor, hpBarFullColor, (ratio - 0.5f) * 2.0f);
        return Color::Lerp(hpBarLowColor,  hpBarMidColor,  ratio * 2.0f);
    }
}

void GameSceneHud::Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
    DirectX::SpriteBatch* spriteBatch, DirectX::SpriteFont* font)
{
    m_spriteBatch = spriteBatch;
    m_font        = font;

    // 1x1 白テクスチャを生成 (バー描画用)
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = desc.Height = 1;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage     = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    constexpr uint32_t white = 0xFFFFFFFF;
    D3D11_SUBRESOURCE_DATA initData = { &white, sizeof(uint32_t), 0 };
    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    if (SUCCEEDED(device->CreateTexture2D(&desc, &initData, tex.GetAddressOf())))
        device->CreateShaderResourceView(tex.Get(), nullptr, m_whiteTex.GetAddressOf());

    (void)context;
}

void GameSceneHud::Update(float dt)
{
    // スクリーンシェイク減衰
    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        const float t = std::max(0.0f, m_shakeTimer);
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        m_shakeOffset = Vector2(dist(s_rng), dist(s_rng)) * m_shakeIntensity * t;
    }
    else { m_shakeOffset = Vector2::Zero; }

    // ウェーブバナー
    if (m_bannerTimer > 0.0f) m_bannerTimer -= dt;

    // コンボポップアップ
    if (m_comboPopTimer > 0.0f) m_comboPopTimer -= dt;
}

void GameSceneHud::TriggerScreenShake(float intensity)
{
    m_shakeIntensity = intensity;
    m_shakeTimer     = intensity * 0.4f;
}

void GameSceneHud::TriggerWaveBanner(int waveNumber, int totalWaves)
{
    m_bannerWave  = waveNumber;
    m_bannerTotal = totalWaves;
    m_bannerTimer = 2.5f;
}

void GameSceneHud::TriggerComboPopup(int comboIndex, int comboLevel)
{
    m_comboPopIndex = comboIndex;
    m_comboPopLevel = comboLevel;
    m_comboPopTimer = 0.6f;
}

void GameSceneHud::DrawBar(float x, float y, float w, float h,
    float ratio, Color fill, Color back)
{
    if (!m_spriteBatch || !m_whiteTex) return;
    const RECT backRect = { static_cast<LONG>(x), static_cast<LONG>(y),
        static_cast<LONG>(x+w), static_cast<LONG>(y+h) };
    m_spriteBatch->Draw(m_whiteTex.Get(), backRect, back);
    const RECT fillRect = { static_cast<LONG>(x), static_cast<LONG>(y),
        static_cast<LONG>(x + w * std::max(0.0f, std::min(1.0f, ratio))), static_cast<LONG>(y+h) };
    m_spriteBatch->Draw(m_whiteTex.Get(), fillRect, fill);
}

void GameSceneHud::DrawText(const std::wstring& text, float x, float y, Color color, float scale)
{
    if (!m_font || !m_spriteBatch) return;
    m_font->DrawString(m_spriteBatch, text.c_str(),
        DirectX::XMFLOAT2(x, y), color, 0.0f,
        DirectX::XMFLOAT2(0, 0), scale);
}

void GameSceneHud::Render(
    const Action::GameState&        gs,
    const Action::PlayerState&      player,
    const Action::SurvivalDirector& director,
    bool                            speedActive,
    float                           speedRemaining,
    int screenW, int screenH)
{
    if (!m_spriteBatch) return;

    const float W = static_cast<float>(screenW);
    const float H = static_cast<float>(screenH);

    //--------------------------------------------------------------
    // 残り時間
    //--------------------------------------------------------------
    {
        const int sec  = static_cast<int>(gs.stageTimer);
        const int min  = sec / 60;
        const int srem = sec % 60;
        wchar_t buf[32];
        swprintf_s(buf, L"%02d:%02d", min, srem);
        const Color timeCol = (gs.stageTimer < 30.0f)
            ? Color(1.0f, 0.3f, 0.3f, 1.0f) : Color(1.0f, 1.0f, 1.0f, 1.0f);
        DrawText(buf, W * 0.5f - 40.0f, 16.0f, timeCol, 1.4f);
    }

    //--------------------------------------------------------------
    // Kill 数 / 危険度
    //--------------------------------------------------------------
    {
        wchar_t buf[64];
        swprintf_s(buf, L"KILL:%d  DNG:%d", gs.killCount, gs.dangerLevel);
        DrawText(buf, 16.0f, 16.0f, Color(0.85f, 0.85f, 0.85f, 1.0f), 0.9f);
    }

    //--------------------------------------------------------------
    // Wave 表示
    //--------------------------------------------------------------
    {
        wchar_t buf[64];
        if (director.IsWaveBreak())
            swprintf_s(buf, L"WAVE %d/%d  BREAK", director.GetCurrentWave(), director.GetTotalWaveCount());
        else if (director.IsCompleted())
            swprintf_s(buf, L"ALL WAVES CLEAR!");
        else
            swprintf_s(buf, L"WAVE %d/%d", director.GetCurrentWave(), director.GetTotalWaveCount());
        DrawText(buf, W - 220.0f, 16.0f, Color(0.9f, 0.85f, 0.5f, 1.0f), 0.9f);
    }

    //--------------------------------------------------------------
    // HP バー (タイマーを HP 代わりに表示)
    //--------------------------------------------------------------
    {
        constexpr float kBarW = 200.0f, kBarH = 14.0f;
        const float bx = 16.0f, by = H - 52.0f;
        const float maxT  = 300.0f; // BattleRuleBook Stage1 基準
        const float ratio = gs.stageTimer / maxT;
        DrawBar(bx, by, kBarW, kBarH, ratio, HpColor(ratio), hpBarBackColor);
        DrawText(L"TIME HP", bx, by - 18.0f, Color(0.7f,0.7f,0.7f,1.0f), 0.75f);
    }

    //--------------------------------------------------------------
    // コンボゲージ
    //--------------------------------------------------------------
    {
        constexpr float kBarW = 160.0f, kBarH = 8.0f;
        const float bx = 16.0f, by = H - 28.0f;
        const float ratio = player.comboGauge / 100.0f;
        const Color gaugeCol = (player.comboLevel >= 3) ? Color(1.0f,0.6f,0.1f,1.0f)
            : (player.comboLevel >= 2) ? Color(0.9f,0.9f,0.1f,1.0f)
            : (player.comboLevel >= 1) ? Color(0.3f,0.9f,0.4f,1.0f)
            : Color(0.4f,0.5f,0.8f,1.0f);
        DrawBar(bx, by, kBarW, kBarH, ratio, gaugeCol, hpBarBackColor);
        if (player.comboLevel > 0)
        {
            wchar_t buf[16];
            swprintf_s(buf, L"Lv%d", player.comboLevel);
            DrawText(buf, bx + kBarW + 6.0f, by - 2.0f, gaugeCol, 0.75f);
        }
    }

    //--------------------------------------------------------------
    // 速度UPアイコン
    //--------------------------------------------------------------
    if (speedActive)
    {
        wchar_t buf[32];
        swprintf_s(buf, L"SPEED UP  %.1fs", speedRemaining);
        DrawText(buf, 16.0f, H - 76.0f, Color(0.1f,1.0f,0.5f,1.0f), 0.9f);
    }

    //--------------------------------------------------------------
    // ウェーブバナー演出 (追加機能)
    //--------------------------------------------------------------
    if (m_bannerTimer > 0.0f)
    {
        const float alpha = std::min(1.0f, m_bannerTimer < 0.5f ? m_bannerTimer * 2.0f : 1.0f);
        wchar_t buf[64];
        swprintf_s(buf, L"--- WAVE %d / %d ---", m_bannerWave, m_bannerTotal);
        DrawText(buf, W * 0.5f - 120.0f, H * 0.38f,
            Color(1.0f, 0.9f, 0.3f, alpha), 1.3f);
    }

    //--------------------------------------------------------------
    // コンボポップアップ (追加機能)
    //--------------------------------------------------------------
    if (m_comboPopTimer > 0.0f)
    {
        const float alpha = m_comboPopTimer / 0.6f;
        const float rise  = (0.6f - m_comboPopTimer) * 60.0f;
        wchar_t buf[32];
        const wchar_t* label = (m_comboPopIndex >= 3) ? L"COMBO x3!!" : (m_comboPopIndex == 2) ? L"COMBO x2!" : L"HIT!";
        swprintf_s(buf, L"%s", label);
        DrawText(buf, W * 0.5f + 60.0f, H * 0.55f - rise,
            Color(1.0f, 0.8f, 0.1f, alpha), 1.0f + (1.0f - alpha) * 0.5f);
    }

    //--------------------------------------------------------------
    // スコア
    //--------------------------------------------------------------
    {
        wchar_t buf[32];
        swprintf_s(buf, L"SCORE %06d", gs.score);
        DrawText(buf, W - 220.0f, H - 36.0f, Color(0.9f,0.9f,0.9f,1.0f), 0.85f);
    }
}
