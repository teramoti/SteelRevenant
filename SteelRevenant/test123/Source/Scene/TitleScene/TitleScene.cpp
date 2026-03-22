#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "TitleScene.h"

#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/InputManager.h"
#include "../../Utility/DirectX11.h"
#include "../../Utility/Sound/AudioSystem.h"
#include "../SceneManager/SceneManager.h"
#include "../../GameSystem/UIConfig.h"

#include <Keyboard.h>
#include <Mouse.h>
#include <SimpleMath.h>

#include <array>
#include <cstdint>
#include <cmath>
#include <windows.h>
#include <d3d11.h>

extern SceneManager* g_sceneManager;

namespace
{
    void SetCursorVisible(bool visible)
    {
        if (visible)
        {
            while (::ShowCursor(TRUE) < 0) {}
            ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
        }
        else
        {
            while (::ShowCursor(FALSE) >= 0) {}
        }
    }

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

void TitleScene::Initialize()
{
    m_timer = 0.0f;
    m_blink = 0.0f;
    m_idleTimer = 0.0f;
    m_idleAccentTriggered = false;
    m_idleLoopSeTimer = 0.0f;
    m_demoMode = false;
    m_demoTimer = 0.0f;
    m_demoStageIndex = 1;
    m_spriteBatch = System::DrawManager::GetInstance().GetSprite();
    if (!m_font)
    {
        m_font = LoadTitleFont();
    }

    // keyboard-only title: hide cursor and don't rely on mouse coordinates
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
    System::InputManager::GetInstance().ResetMouseDelta();
    SetCursorVisible(false);
    m_selectedIndex = 0;

    // menu items (Japanese)
    m_menuItems.clear();
    m_menuItems.push_back(L"出撃");
    m_menuItems.push_back(L"設定");
    m_menuItems.push_back(L"終了");

    // initialize menu offsets
    m_menuOffsets.assign(m_menuItems.size(), 0.0f);

    // logo particles empty
    m_logoParticles.clear();

    // play title BGM if available
    GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::Title);

    // create 1x1 white texture for simple primitive drawing
    auto device = DirectX11::Get().GetDevice().Get();
    if (device)
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = 1; desc.Height = 1; desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1; desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        constexpr std::uint32_t white = 0xFFFFFFFFu;
        D3D11_SUBRESOURCE_DATA initData = { &white, sizeof(std::uint32_t), 0 };
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        if (SUCCEEDED(device->CreateTexture2D(&desc, &initData, tex.GetAddressOf())))
        {
            device->CreateShaderResourceView(tex.Get(), nullptr, m_whiteTex.GetAddressOf());
        }
    }
}

static void InitDemoEntitiesForStage(int stage, std::vector<TitleScene::DemoEnemy>& out, DirectX::SimpleMath::Vector2 playerPos)
{
    out.clear();
    if (stage == 1)
    {
        // Stage1: mostly melee enemies moving slowly around
        for (int i = 0; i < 6; ++i)
        {
            TitleScene::DemoEnemy e;
            float a = i * 1.0472f; // distribute
            e.pos = playerPos + DirectX::SimpleMath::Vector2(std::cos(a) * (6.0f + i), std::sin(a) * (4.0f + i));
            e.vel = DirectX::SimpleMath::Vector2(-std::sin(a), std::cos(a)) * 0.3f;
            e.isLaser = false; e.isDasher = false; e.stateTimer = 0.0f; e.warnTimer = 0.0f;
            out.push_back(e);
        }
    }
    else if (stage == 2)
    {
        // Stage2: mix in laser enemies with warning indicators
        for (int i = 0; i < 4; ++i)
        {
            TitleScene::DemoEnemy e;
            float a = i * 1.57f;
            e.pos = playerPos + DirectX::SimpleMath::Vector2(std::cos(a) * 10.0f, std::sin(a) * 7.0f);
            e.vel = DirectX::SimpleMath::Vector2(std::sin(a) * 0.2f, -std::cos(a) * 0.15f);
            e.isLaser = (i % 2 == 0);
            e.isDasher = false; e.stateTimer = 0.0f; e.warnTimer = static_cast<float>(i) * 0.4f;
            out.push_back(e);
        }
        // add some melee
        for (int i = 0; i < 3; ++i)
        {
            TitleScene::DemoEnemy e;
            float a = i * 2.0f;
            e.pos = playerPos + DirectX::SimpleMath::Vector2(std::cos(a) * 5.0f, std::sin(a) * 5.5f);
            e.vel = DirectX::SimpleMath::Vector2(-std::sin(a) * 0.25f, std::cos(a) * 0.2f);
            e.isLaser = false; e.isDasher = false; e.stateTimer = 0.0f; e.warnTimer = 0.0f;
            out.push_back(e);
        }
    }
    else // stage 3
    {
        // Stage3: lasers + dashers
        for (int i = 0; i < 3; ++i)
        {
            TitleScene::DemoEnemy e;
            float a = i * 2.09f;
            e.pos = playerPos + DirectX::SimpleMath::Vector2(std::cos(a) * 11.0f, std::sin(a) * 9.0f);
            e.vel = DirectX::SimpleMath::Vector2(std::sin(a) * 0.25f, -std::cos(a) * 0.2f);
            e.isLaser = (i == 0);
            e.isDasher = (i == 1);
            e.stateTimer = 0.0f; e.warnTimer = static_cast<float>(i) * 0.5f;
            out.push_back(e);
        }
        // some melee
        for (int i = 0; i < 4; ++i)
        {
            TitleScene::DemoEnemy e;
            float a = i * 1.57f;
            e.pos = playerPos + DirectX::SimpleMath::Vector2(std::cos(a) * 4.0f, std::sin(a) * 3.5f);
            e.vel = DirectX::SimpleMath::Vector2(-std::sin(a) * 0.3f, std::cos(a) * 0.18f);
            e.isLaser = false; e.isDasher = false; e.stateTimer = 0.0f; e.warnTimer = 0.0f;
            out.push_back(e);
        }
    }
}

void TitleScene::Update(const DX::StepTimer& timer)
{
    const float dt = static_cast<float>(timer.GetElapsedSeconds());
    m_timer += dt;
    m_blink += dt;

    const System::InputManager& input = System::InputManager::GetInstance();

    bool anyInput = false;
    // keyboard navigation only
    if (input.IsKeyPressed(DirectX::Keyboard::Up)) { m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_menuItems.size())) % static_cast<int>(m_menuItems.size()); anyInput = true; }
    if (input.IsKeyPressed(DirectX::Keyboard::Down)) { m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_menuItems.size()); anyInput = true; }

    // also support W/S
    if (input.IsKeyPressed(DirectX::Keyboard::W)) { m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_menuItems.size())) % static_cast<int>(m_menuItems.size()); anyInput = true; }
    if (input.IsKeyPressed(DirectX::Keyboard::S)) { m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_menuItems.size()); anyInput = true; }

    // toggle UI variant (F2)
    if (input.IsKeyPressed(DirectX::Keyboard::F2))
    {
        SystemUI::ToggleVariant();
    }

    if (input.IsKeyPressed(DirectX::Keyboard::Escape)) { // connect to existing back/quit behavior
        extern void ExitGame();
        ExitGame();
        return;
    }

    if (anyInput)
    {
        m_idleTimer = 0.0f;
        m_idleAccentTriggered = false;
        m_idleLoopSeTimer = 0.0f;
        if (m_demoMode)
        {
            m_demoMode = false; // exit demo on keyboard input
            GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::Title);
        }
    }
    else
    {
        m_idleTimer += dt;
    }

    if (!m_demoMode)
    {
        if (m_idleTimer >= 3.0f && !m_idleAccentTriggered)
        {
            GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::UiSelect, 0.22f);
            m_idleAccentTriggered = true;
            m_idleLoopSeTimer = 0.0f;
        }
        if (m_idleTimer >= 5.0f)
        {
            m_idleLoopSeTimer += dt;
            if (m_idleLoopSeTimer >= 3.4f)
            {
                GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::UiMove, 0.16f);
                m_idleLoopSeTimer = 0.0f;
            }
        }
    }

    // simple menu animation: smooth offsets toward selected
    for (size_t i = 0; i < m_menuOffsets.size(); ++i)
    {
        const float target = (static_cast<int>(i) == m_selectedIndex) ? -6.0f : 0.0f;
        m_menuOffsets[i] += (target - m_menuOffsets[i]) * std::min(1.0f, 10.0f * dt);
    }

    // enter demo mode after 18 seconds of inactivity
    if (!m_demoMode && m_idleTimer >= 18.0f)
    {
        m_demoMode = true;
        m_demoTimer = 0.0f;
        m_demoStageIndex = 1;
        // initialize demo player and enemies
        m_demoPlayerPos = DirectX::SimpleMath::Vector2(0.0f, 0.0f);
        InitDemoEntitiesForStage(m_demoStageIndex, m_demoEnemies, m_demoPlayerPos);
        // play stage BGM for demo (cycle Stage1)
        GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::DefenseCorridor);
    }

    if (m_demoMode)
    {
        m_demoTimer += dt;
        // cycle demo stage every 10 seconds
        if (m_demoTimer > 10.0f)
        {
            m_demoTimer = 0.0f;
            m_demoStageIndex = (m_demoStageIndex % 3) + 1;
            InitDemoEntitiesForStage(m_demoStageIndex, m_demoEnemies, m_demoPlayerPos);
            // switch BGM
            if (m_demoStageIndex == 1) GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::DefenseCorridor);
            else GameAudio::AudioSystem::GetInstance().PlayBgm(GameAudio::BgmId::CoreSector);
        }

        // update demo entities
        for (auto& e : m_demoEnemies)
        {
            // warning/fire timers for laser enemies
            if (e.isLaser)
            {
                e.warnTimer -= dt;
                if (e.warnTimer <= 0.0f)
                {
                    e.stateTimer = 0.7f; // fire time
                    e.warnTimer = 6.0f; // next cycle
                }
            }

            // dashers: periodically charge then dash across
            if (e.isDasher)
            {
                e.stateTimer -= dt;
                if (e.stateTimer <= 0.0f)
                {
                    // start charge
                    e.warnTimer = 0.8f;
                    e.stateTimer = 3.0f; // cooldown
                }
                if (e.warnTimer > 0.0f)
                {
                    e.warnTimer -= dt;
                    if (e.warnTimer <= 0.0f)
                    {
                        // dash: set velocity toward player briefly
                        DirectX::SimpleMath::Vector2 dir = m_demoPlayerPos - e.pos;
                        float len = std::max(0.001f, std::sqrt(dir.x*dir.x + dir.y*dir.y));
                        e.vel = dir / len * 8.0f;
                    }
                }
            }

            // simple wandering movement
            e.pos += e.vel * dt;
            // keep within radius
            const float radius = 16.0f;
            if (e.pos.x*e.pos.x + e.pos.y*e.pos.y > radius*radius)
            {
                // reflect
                e.vel = -e.vel * 0.6f;
                e.pos += e.vel * dt;
            }
        }

        return; // demo doesn't process menu input
    }

    // confirm selection (keyboard only)
    if (input.IsKeyPressed(DirectX::Keyboard::Enter) || input.IsKeyPressed(DirectX::Keyboard::Space))
    {
        if (g_sceneManager != nullptr)
        {
            const std::wstring& sel = m_menuItems[m_selectedIndex];
            if (sel == L"出撃")
            {
                g_sceneManager->RequestTransition(STAGE_SELECT_SCENE);
            }
            else if (sel == L"設定")
            {
                g_sceneManager->RequestTransition(SETTINGS_SCENE);
            }
            else if (sel == L"終了")
            {
                extern void ExitGame();
                ExitGame();
            }
        }
    }
}

void TitleScene::Render()
{
    m_spriteBatch = System::DrawManager::GetInstance().GetSprite();
    if (!m_font || m_spriteBatch == nullptr)
    {
        return;
    }

    const float width = static_cast<float>(DirectX11::Get().GetWidth());
    const float height = static_cast<float>(DirectX11::Get().GetHeight());
    const float idleIntro = (std::max)(0.0f, (std::min)(1.0f, (m_idleTimer - 3.0f) / 2.0f));
    const float idleLoop = (!m_demoMode && m_idleTimer >= 5.0f)
        ? (0.5f + 0.5f * std::sin((m_idleTimer - 5.0f) * 1.8f))
        : 0.0f;

    System::DrawManager::GetInstance().Begin();

    // simple animated background: moving grid dots/lines
    if (m_whiteTex)
    {
        const int cols = 24;
        const int rows = 12;
        const float spacingX = width / static_cast<float>(cols);
        const float spacingY = height / static_cast<float>(rows);
        const float offset = std::fmod(m_timer * (30.0f + idleIntro * 18.0f), spacingX);
        for (int y = 0; y <= rows; ++y)
        {
            const float yy = y * spacingY + std::sin(m_timer * (0.7f + idleIntro * 0.25f) + y * 0.7f) * (6.0f + idleIntro * 5.0f);
            const RECT r = { 0, static_cast<LONG>(yy), static_cast<LONG>(width), static_cast<LONG>(yy + 1.0f) };
            m_spriteBatch->Draw(m_whiteTex.Get(), r, DirectX::SimpleMath::Color(0.06f,0.06f,0.08f,0.24f + idleIntro * 0.12f));
        }
        for (int x = 0; x <= cols; ++x)
        {
            const float xx = x * spacingX + std::cos(m_timer * (0.6f + idleIntro * 0.18f) + x * 0.8f) * (8.0f + idleIntro * 7.0f) + offset * 0.5f;
            const RECT r = { static_cast<LONG>(xx), 0, static_cast<LONG>(xx + 1.0f), static_cast<LONG>(height) };
            m_spriteBatch->Draw(m_whiteTex.Get(), r, DirectX::SimpleMath::Color(0.06f,0.06f,0.08f,0.18f + idleIntro * 0.10f));
        }

        // subtle moving dots
        for (int i = 0; i < 40; ++i)
        {
            const float ax = (static_cast<float>(i) / 40.0f) * width + std::sin(m_timer * (0.2f + (i % 7) * 0.03f)) * 36.0f;
            const float ay = std::fmod(i * 73.0f + m_timer * (12.0f + idleIntro * 10.0f), height);
            const RECT d = { static_cast<LONG>(ax), static_cast<LONG>(ay), static_cast<LONG>(ax + 2.0f), static_cast<LONG>(ay + 2.0f) };
            m_spriteBatch->Draw(m_whiteTex.Get(), d, DirectX::SimpleMath::Color(0.18f,0.22f,0.28f,0.55f));
        }

        if (!m_demoMode)
        {
            const float sweepOffset = std::fmod(m_timer * (42.0f + idleLoop * 16.0f), width + 240.0f) - 120.0f;
            for (int i = 0; i < 2; ++i)
            {
                const float bandX = sweepOffset - i * (width * 0.55f);
                const RECT bandRect =
                {
                    static_cast<LONG>(bandX),
                    0,
                    static_cast<LONG>(bandX + 84.0f + idleIntro * 42.0f),
                    static_cast<LONG>(height)
                };
                m_spriteBatch->Draw(
                    m_whiteTex.Get(),
                    bandRect,
                    DirectX::SimpleMath::Color(0.14f, 0.22f, 0.34f, 0.04f + idleIntro * 0.03f + idleLoop * 0.05f));
            }
        }
    }

    // animated logo (normal or demo)
    float logoScale = 1.0f + 0.06f * std::sin(m_timer * 1.6f) + idleIntro * 0.05f + idleLoop * 0.04f;
    float logoX = width * 0.5f - 110.0f + std::sin(m_timer * 0.55f) * idleLoop * 10.0f;
    float logoY = height * 0.28f + std::sin(m_timer * (0.9f + idleIntro * 0.2f)) * idleIntro * 6.0f;
    if (m_demoMode)
    {
        // demo: more pronounced zoom and slow horizontal pan
        logoScale = 1.0f + 0.12f * std::sin(m_demoTimer * 0.9f);
        logoX += std::sin(m_demoTimer * 0.4f) * 40.0f;
        logoY += std::cos(m_demoTimer * 0.25f) * 10.0f;
    }

    // spawn some small particles near logo for visual flourish
    if (m_logoParticles.size() < 32 && (std::fmod(m_timer, 0.12f) < 0.02f))
    {
        TitleScene::LogoParticle p;
        p.pos = DirectX::SimpleMath::Vector2(logoX + 60.0f + (std::rand() % 40 - 20), logoY + 12.0f + (std::rand() % 20 - 10));
        p.vel = DirectX::SimpleMath::Vector2((std::rand() % 100 - 50) * 0.0025f, -0.02f - (std::rand() % 10) * 0.002f);
        p.life = 0.6f + (std::rand() % 40) * 0.01f;
        m_logoParticles.push_back(p);
    }

    for (auto it = m_logoParticles.begin(); it != m_logoParticles.end();) {
        it->pos += it->vel * (1.0f/60.0f);
        it->life -= 1.0f/60.0f;
        if (it->life <= 0.0f) it = m_logoParticles.erase(it);
        else ++it;
    }

    // draw logo text
    if (!m_demoMode)
    {
        m_font->DrawString(
            m_spriteBatch,
            L"SteelRevenant",
            DirectX::XMFLOAT2(logoX - 4.0f, logoY - 2.0f),
            DirectX::SimpleMath::Color(0.34f, 0.62f, 1.0f, 0.08f + idleIntro * 0.10f + idleLoop * 0.10f),
            0.0f,
            DirectX::XMFLOAT2(0, 0),
            1.68f * logoScale);
    }
    m_font->DrawString(m_spriteBatch, L"SteelRevenant", DirectX::XMFLOAT2(logoX, logoY), DirectX::SimpleMath::Color(0.98f,0.98f,1.0f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), 1.6f * logoScale);

    // render logo particles
    for (const auto& p : m_logoParticles)
    {
        if (m_whiteTex) m_spriteBatch->Draw(m_whiteTex.Get(), DirectX::XMFLOAT2(p.pos.x, p.pos.y), nullptr, DirectX::SimpleMath::Color(1.0f,0.88f,0.6f, p.life * 0.9f), 0.0f, DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT2(4.0f,4.0f));
    }

    // subtitle
    m_font->DrawString(
        m_spriteBatch,
        L"制限時間内に敵の群れを切り崩せ",
        DirectX::XMFLOAT2(width * 0.5f - 182.0f, height * 0.36f + idleIntro * 4.0f),
        DirectX::SimpleMath::Color(0.8f, 0.85f, 0.9f, 0.84f + idleLoop * 0.16f),
        0.0f,
        DirectX::XMFLOAT2(0, 0),
        0.9f + idleIntro * 0.03f);

    if (m_demoMode)
    {
        // draw simple stage overhead preview (centered)
        const float mapW = 520.0f;
        const float mapH = 320.0f;
        const float mapX = width*0.5f - mapW*0.5f;
        const float mapY = height*0.52f - mapH*0.5f;
        if (m_whiteTex) m_spriteBatch->Draw(m_whiteTex.Get(), DirectX::XMFLOAT2(mapX, mapY), nullptr, DirectX::SimpleMath::Color(0.06f,0.06f,0.08f,0.95f), 0.0f, DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT2(mapW, mapH));

        // draw player center
        const DirectX::XMFLOAT2 center(mapX + mapW*0.5f, mapY + mapH*0.5f);
        const float px = center.x; const float py = center.y;
        if (m_whiteTex) m_spriteBatch->Draw(m_whiteTex.Get(), DirectX::XMFLOAT2(px-6.0f, py-6.0f), nullptr, DirectX::SimpleMath::Color(0.9f,0.9f,1.0f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT2(12.0f,12.0f));

        // draw demo enemies
        for (const auto& e : m_demoEnemies)
        {
            const float sx = center.x + e.pos.x * 8.0f; // scale world->map
            const float sy = center.y + e.pos.y * 8.0f;
            DirectX::SimpleMath::Color col = e.isLaser ? DirectX::SimpleMath::Color(1.0f,0.6f,0.12f,0.95f) : DirectX::SimpleMath::Color(1.0f,0.18f,0.18f,0.95f);
            if (m_whiteTex) m_spriteBatch->Draw(m_whiteTex.Get(), DirectX::XMFLOAT2(sx-5.0f, sy-5.0f), nullptr, col, 0.0f, DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT2(10.0f,10.0f));

            // laser warning/beam
            if (e.isLaser && e.stateTimer > 0.0f)
            {
                // draw beam from enemy to player
                const float dx = px - sx; const float dy = py - sy;
                const float len = std::sqrt(dx*dx + dy*dy);
                const float angle = std::atan2(dy, dx);
                if (m_whiteTex && len > 8.0f)
                {
                    const DirectX::XMFLOAT2 posLine(sx + dx*0.5f, sy + dy*0.5f);
                    m_spriteBatch->Draw(m_whiteTex.Get(), posLine, nullptr, DirectX::SimpleMath::Color(1.0f,0.24f,0.12f,0.8f), -angle + DirectX::XM_PIDIV2, DirectX::XMFLOAT2(0.5f,0.5f), DirectX::XMFLOAT2(4.0f, len));
                }
            }

            // dashed indicator for dashers: show warning circle when warnTimer>0
            if (e.isDasher && e.warnTimer > 0.0f)
            {
                if (m_whiteTex) m_spriteBatch->Draw(m_whiteTex.Get(), DirectX::XMFLOAT2(sx-12.0f, sy-12.0f), nullptr, DirectX::SimpleMath::Color(1.0f,0.9f,0.2f,0.4f), 0.0f, DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT2(24.0f,24.0f));
            }
        }

        const float previewAlpha = 0.55f + 0.45f * std::sin(m_demoTimer * 2.2f);
        m_font->DrawString(m_spriteBatch, L"PRESS ANY KEY", DirectX::XMFLOAT2(width*0.5f - 118.0f, height * 0.88f), DirectX::SimpleMath::Color(1.0f,0.9f,0.6f,previewAlpha), 0.0f, DirectX::XMFLOAT2(0,0), 0.82f);
    }

    // menu (hide during demo)
    if (!m_demoMode)
    {
        const float startX = width * 0.5f - 90.0f;
        float y = height * 0.50f;
        for (int i = 0; i < static_cast<int>(m_menuItems.size()); ++i)
        {
            const bool selected = (i == m_selectedIndex);
            const float tw = 180.0f;
            const float th = 40.0f;
            // button background
            if (m_whiteTex)
            {
                const DirectX::SimpleMath::Color bg = selected ? DirectX::SimpleMath::Color(0.14f,0.22f,0.36f,0.95f) : DirectX::SimpleMath::Color(0.06f,0.08f,0.12f,0.86f);
                const RECT bgRect = { static_cast<LONG>(startX), static_cast<LONG>(y + m_menuOffsets[i]), static_cast<LONG>(startX + tw), static_cast<LONG>(y + th + m_menuOffsets[i]) };
                m_spriteBatch->Draw(m_whiteTex.Get(), bgRect, bg);
                // highlight border when selected
                if (selected)
                {
                    const RECT borderRect = { static_cast<LONG>(startX - 3), static_cast<LONG>(y - 3 + m_menuOffsets[i]), static_cast<LONG>(startX + tw + 3), static_cast<LONG>(y + th + 3 + m_menuOffsets[i]) };
                    m_spriteBatch->Draw(m_whiteTex.Get(), borderRect, DirectX::SimpleMath::Color(0.22f,0.56f,1.0f,0.22f));
                }
            }

            const auto color = selected ? DirectX::SimpleMath::Color(0.98f,0.98f,1.0f,1.0f) : DirectX::SimpleMath::Color(0.85f,0.85f,0.85f,1.0f);
            const float scale = selected ? 1.08f : 0.96f;
            // increase scale slightly for Premium UI variant
            const float uiScaleMultiplier = (SystemUI::GetVariant() == SystemUI::Variant::Premium) ? 1.12f : 1.0f;
            m_font->DrawString(m_spriteBatch, m_menuItems[i].c_str(), DirectX::XMFLOAT2(startX + 16.0f, y + 6.0f + m_menuOffsets[i]), color, 0.0f, DirectX::XMFLOAT2(0,0), scale * uiScaleMultiplier);
            y += th + 12.0f;
        }

        const float promptAlpha = 0.28f + 0.72f * (0.5f + 0.5f * std::sin(m_blink * (4.6f + idleIntro * 0.8f)));
        m_font->DrawString(
            m_spriteBatch,
            L"PRESS ANY KEY",
            DirectX::XMFLOAT2(width * 0.5f - 132.0f, height * 0.78f + idleLoop * 4.0f),
            DirectX::SimpleMath::Color(1.0f, 0.90f, 0.58f, promptAlpha),
            0.0f,
            DirectX::XMFLOAT2(0,0),
            0.88f + idleLoop * 0.04f);
        m_font->DrawString(
            m_spriteBatch,
            L"W / S で選択   Enter / Space で決定",
            DirectX::XMFLOAT2(width * 0.5f - 164.0f, height * 0.83f),
            DirectX::SimpleMath::Color(0.72f, 0.76f, 0.82f, 0.78f),
            0.0f,
            DirectX::XMFLOAT2(0, 0),
            0.58f);
    }

    // footer
    m_font->DrawString(m_spriteBatch, L"WASD: 移動  MOUSE: 視点  L-CLICK: 攻撃  R-CLICK: ガード", DirectX::XMFLOAT2(20.0f, height - 36.0f), DirectX::SimpleMath::Color(0.6f,0.6f,0.6f,1.0f), 0.0f, DirectX::XMFLOAT2(0,0), 0.7f);

    System::DrawManager::GetInstance().End();
}

void TitleScene::Finalize()
{
    // restore system cursor visibility and set mouse mode to absolute so other UI can show cursor if needed
    SetCursorVisible(true);
    m_font.reset();
    m_spriteBatch = nullptr;
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    System::InputManager::GetInstance().ResetMouseDelta();
}
