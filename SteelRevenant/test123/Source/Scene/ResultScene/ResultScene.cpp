#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "ResultScene.h"

#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../Utility/DirectX11.h"
#include "../SceneManager/SceneManager.h"
#include "../../Action/BattleRuleBook.h"
#include "../../Utility/Sound/AudioSystem.h"

#include <Keyboard.h>
#include <Mouse.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>

extern SceneManager* g_sceneManager;

Action::GameState ResultScene::s_lastResult;
int ResultScene::s_stageIndex = 1;
int ResultScene::s_reachedWave = 1;

namespace
{
    std::unique_ptr<DirectX::SpriteFont> LoadTitleFont()
    {
        static const std::array<const wchar_t*, 9> kFontCandidates =
        {
            L"JapaneseUI_18.spritefont",
            L"../JapaneseUI_18.spritefont",
            L"../../JapaneseUI_18.spritefont",
            L"Source/JapaneseUI_18.spritefont",
            L"SegoeUI_18.spritefont",
            L"../SegoeUI_18.spritefont",
            L"../../SegoeUI_18.spritefont",
            L"SegoeUI_18.spritefont",
            nullptr
        };

        for (const wchar_t* candidate : kFontCandidates)
        {
            if (!candidate) break;
            try { auto font = std::make_unique<DirectX::SpriteFont>(DirectX11::Get().GetDevice().Get(), candidate); font->SetDefaultCharacter(L'?'); return font; }
            catch (...) {}
        }
        return {};
    }
}

void ResultScene::Initialize()
{
    m_timer = 0.0f;
    m_exitTimer = -1.0f;
    m_selectedIndex = 0;
    m_spriteBatch = System::DrawManager::GetInstance().GetSprite();
    if (!m_font)
    {
        auto f = LoadTitleFont();
        if (f) m_font = f.release();
    }
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    DirectX::Mouse::Get().SetVisible(true);
    System::InputManager::GetInstance().ResetMouseDelta();

    auto device = DirectX11::Get().GetDevice().Get();
    if (device)
    {
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
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        if (SUCCEEDED(device->CreateTexture2D(&desc, &initData, tex.GetAddressOf())))
        {
            device->CreateShaderResourceView(tex.Get(), nullptr, m_whiteTex.GetAddressOf());
        }
    }

    // play result music depending on clear/fail
    const bool timeUpResult = s_lastResult.timeExpired && !s_lastResult.stageCleared;
    if (s_lastResult.stageCleared || timeUpResult)
    {
        GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::ResultClear);
    }
    else
    {
        GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::ResultFail);
    }
}

void ResultScene::Update(const DX::StepTimer& timer)
{
    const float dt = static_cast<float>(timer.GetElapsedSeconds());
    m_timer += dt;
    const System::InputManager& input = System::InputManager::GetInstance();

    if (input.IsKeyPressed(DirectX::Keyboard::Left) || input.IsKeyPressed(DirectX::Keyboard::Up))
    {
        m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_menuItems.size())) % static_cast<int>(m_menuItems.size());
    }
    if (input.IsKeyPressed(DirectX::Keyboard::Right) || input.IsKeyPressed(DirectX::Keyboard::Down))
    {
        m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_menuItems.size());
    }

    const DirectX::Mouse::State* ms = input.GetMouseState();
    if (ms)
    {
        const float width = static_cast<float>(DirectX11::Get().GetWidth());
        const float height = static_cast<float>(DirectX11::Get().GetHeight());
        const float bw = 220.0f;
        const float bh = 52.0f;
        const float gap = 18.0f;
        const float totalW = bw * static_cast<float>(m_menuItems.size()) + gap * static_cast<float>(m_menuItems.size() - 1);
        const float bx = width * 0.5f - totalW * 0.5f;
        const float by = height * 0.76f;
        for (int i = 0; i < static_cast<int>(m_menuItems.size()); ++i)
        {
            const float ix = bx + i * (bw + gap);
            if (ms->x >= ix && ms->x <= ix + bw && ms->y >= by && ms->y <= by + bh)
            {
                m_selectedIndex = i;
                if (input.IsMouseButtonPressed(0))
                {
                    break;
                }
            }
        }
    }

    if (input.IsKeyPressed(DirectX::Keyboard::Enter) || input.IsKeyPressed(DirectX::Keyboard::Space) || (ms && input.IsMouseButtonPressed(0)))
    {
        if (g_sceneManager != nullptr)
        {
            if (m_selectedIndex == 0)
            {
                GameSaveData::GetInstance().SetStageNum(s_stageIndex);
                if (g_sceneManager) g_sceneManager->RequestTransition(GAME_SCENE);
            }
            else
            {
                if (g_sceneManager) g_sceneManager->RequestTransition(TITLE_SCENE);
            }
        }
    }
}

void ResultScene::Render()
{
    if (m_font == nullptr || m_spriteBatch == nullptr)
    {
        return;
    }

    const float width = static_cast<float>(DirectX11::Get().GetWidth());
    const float height = static_cast<float>(DirectX11::Get().GetHeight());

    System::DrawManager::GetInstance().Begin();

    const auto DrawRect = [&](float x, float y, float w, float h, const DirectX::SimpleMath::Color& color)
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

    const bool timeUpResult = s_lastResult.timeExpired && !s_lastResult.stageCleared;
    const bool cleared = s_lastResult.stageCleared || timeUpResult;
    const wchar_t* title = timeUpResult ? L"タイムアップ" : (cleared ? L"ステージクリア" : L"ステージ失敗");
    const auto titleColor = cleared ? DirectX::SimpleMath::Color(0.9f,0.8f,0.2f,1.0f) : DirectX::SimpleMath::Color(0.9f,0.28f,0.28f,1.0f);

    // Stage name from BattleRuleBook
    const Action::StageRuleDefinition& rule = Action::BattleRuleBook::GetInstance().GetStageRule(s_stageIndex);
    const wchar_t* stageName = rule.missionName ? rule.missionName : L"ステージ";

    DrawRect(0.0f, 0.0f, width, height, DirectX::SimpleMath::Color(0.02f, 0.03f, 0.05f, 0.96f));
    DrawRect(width * 0.18f, height * 0.14f, width * 0.64f, height * 0.66f, DirectX::SimpleMath::Color(0.05f, 0.07f, 0.10f, 0.88f));
    DrawRect(width * 0.18f, height * 0.14f, width * 0.64f, 3.0f, cleared ? DirectX::SimpleMath::Color(0.96f, 0.84f, 0.32f, 0.96f) : DirectX::SimpleMath::Color(0.96f, 0.34f, 0.30f, 0.96f));

    const float stageStart = rule.stageStartTimeSec > 0.0f ? rule.stageStartTimeSec : 1.0f;
    const float survivalRatio = std::max(0.0f, (std::min)(1.0f, s_lastResult.survivalTimeSec / stageStart));
    const float killTarget = static_cast<float>(std::max(24, rule.baseAliveCount * rule.totalWaveCount * 2));
    const float killRatio = std::max(0.0f, (std::min)(1.0f, static_cast<float>(s_lastResult.killCount) / killTarget));
    const float ratingScore = killRatio * 0.75f + survivalRatio * 0.25f;
    const wchar_t* rankLabel = (ratingScore >= 0.72f) ? L"A" : ((ratingScore >= 0.42f) ? L"B" : L"C");

    m_font->DrawString(m_spriteBatch, stageName, DirectX::XMFLOAT2(width * 0.5f - 120.0f, height * 0.19f), DirectX::SimpleMath::Color(0.95f,0.95f,0.95f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), 1.08f);
    m_font->DrawString(m_spriteBatch, title, DirectX::XMFLOAT2(width * 0.5f - 128.0f, height * 0.27f), titleColor, 0.0f, DirectX::XMFLOAT2(0,0), 1.54f);
    m_font->DrawString(m_spriteBatch, L"リザルト", DirectX::XMFLOAT2(width * 0.5f - 52.0f, height * 0.225f), DirectX::SimpleMath::Color(0.70f,0.78f,0.88f,0.92f), 0.0f, DirectX::XMFLOAT2(0,0), 0.68f);

    DrawRect(width * 0.24f, height * 0.40f, width * 0.18f, height * 0.16f, DirectX::SimpleMath::Color(0.08f, 0.10f, 0.14f, 0.90f));
    DrawRect(width * 0.41f, height * 0.40f, width * 0.18f, height * 0.16f, DirectX::SimpleMath::Color(0.08f, 0.10f, 0.14f, 0.90f));
    DrawRect(width * 0.58f, height * 0.40f, width * 0.18f, height * 0.16f, DirectX::SimpleMath::Color(0.08f, 0.10f, 0.14f, 0.90f));

    wchar_t buf[128];
    m_font->DrawString(m_spriteBatch, L"撃破数", DirectX::XMFLOAT2(width * 0.29f - 28.0f, height * 0.43f), DirectX::SimpleMath::Color(0.74f,0.80f,0.88f,0.94f), 0.0f, DirectX::XMFLOAT2(0,0), 0.72f);
    swprintf_s(buf, L"%d", s_lastResult.killCount);
    m_font->DrawString(m_spriteBatch, buf, DirectX::XMFLOAT2(width * 0.29f - 18.0f, height * 0.48f), DirectX::SimpleMath::Color(1.0f,0.90f,0.46f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), 1.28f);

    m_font->DrawString(m_spriteBatch, L"生存時間", DirectX::XMFLOAT2(width * 0.45f - 40.0f, height * 0.43f), DirectX::SimpleMath::Color(0.74f,0.80f,0.88f,0.94f), 0.0f, DirectX::XMFLOAT2(0,0), 0.72f);
    swprintf_s(buf, L"%.1f 秒", s_lastResult.survivalTimeSec);
    m_font->DrawString(m_spriteBatch, buf, DirectX::XMFLOAT2(width * 0.45f - 36.0f, height * 0.48f), DirectX::SimpleMath::Color(0.92f,0.96f,1.0f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), 1.20f);

    m_font->DrawString(m_spriteBatch, L"評価", DirectX::XMFLOAT2(width * 0.63f - 14.0f, height * 0.43f), DirectX::SimpleMath::Color(0.74f,0.80f,0.88f,0.94f), 0.0f, DirectX::XMFLOAT2(0,0), 0.72f);
    m_font->DrawString(m_spriteBatch, rankLabel, DirectX::XMFLOAT2(width * 0.63f - 10.0f, height * 0.47f), DirectX::SimpleMath::Color(1.0f,0.84f,0.24f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), 1.48f);

    const float bw = 220.0f;
    const float bh = 52.0f;
    const float gap = 18.0f;
    const float totalW = bw * static_cast<float>(m_menuItems.size()) + gap * static_cast<float>(m_menuItems.size() - 1);
    const float bx = width * 0.5f - totalW * 0.5f;
    const float by = height * 0.76f;

    for (int i = 0; i < static_cast<int>(m_menuItems.size()); ++i)
    {
        const float ix = bx + i * (bw + gap);
        const bool sel = (i == m_selectedIndex);
        DrawRect(ix, by, bw, bh, sel ? DirectX::SimpleMath::Color(0.16f,0.24f,0.36f,0.96f) : DirectX::SimpleMath::Color(0.08f,0.10f,0.14f,0.90f));
        const auto color = sel ? DirectX::SimpleMath::Color(0.98f,0.98f,1.0f,1.0f) : DirectX::SimpleMath::Color(0.90f,0.90f,0.92f,1.0f);
        m_font->DrawString(m_spriteBatch, m_menuItems[i].c_str(), DirectX::XMFLOAT2(ix + 48.0f, by + 12.0f), color, 0.0f, DirectX::XMFLOAT2(0,0), 0.94f);
    }

    System::DrawManager::GetInstance().End();
}

void ResultScene::Finalize()
{
    m_whiteTex.Reset();
}
