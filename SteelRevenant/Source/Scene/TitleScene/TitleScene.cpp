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
#include "../GameScene/GameSceneWorldBackdrop.h"
#include "../GameScene/GameSceneWorldArena.h"
#include <GeometricPrimitive.h>

#include <Keyboard.h>
#include <Mouse.h>
#include <SimpleMath.h>

#include <array>
#include <cstdint>
#include <cmath>
#include <windows.h>
#include <d3d11.h>

extern SceneManager* g_sceneManager;

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Color;

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


    struct TitleFront3DResources
    {
        GameSceneWorldBackdrop backdrop;
        std::array<std::unique_ptr<GameSceneWorldArena>, 3> arenas;
        std::unique_ptr<DirectX::GeometricPrimitive> box;
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
            box = DirectX::GeometricPrimitive::CreateBox(context, { 0.36f, 1.70f, 0.36f });
            sphere = DirectX::GeometricPrimitive::CreateSphere(context, 0.34f, 12);
            cylinder = DirectX::GeometricPrimitive::CreateCylinder(context, 1.0f, 0.08f, 12);
            torus = DirectX::GeometricPrimitive::CreateTorus(context, 1.0f, 0.06f, 24);
            initialized = true;
            (void)device;
        }

        void Reset()
        {
            for (auto& arena : arenas) arena.reset();
            box.reset();
            sphere.reset();
            cylinder.reset();
            torus.reset();
            initialized = false;
        }
    };

    TitleFront3DResources& GetTitleFront3D()
    {
        static TitleFront3DResources resources;
        return resources;
    }

    Matrix BuildFrontProjection(float width, float height)
    {
        const float aspect = (height > 1.0f) ? (width / height) : (16.0f / 9.0f);
        return Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(50.0f), aspect, 0.1f, 900.0f);
    }

    void DrawFrontHero(
        TitleFront3DResources& resources,
        const Matrix& view,
        const Matrix& projection,
        const Vector3& position,
        float yaw,
        float slashPulse,
        const Color& accent)
    {
        if (!resources.box || !resources.sphere || !resources.cylinder || !resources.torus)
        {
            return;
        }

        const Matrix bodyWorld = Matrix::CreateRotationY(yaw) * Matrix::CreateTranslation(position.x, position.y + 0.95f, position.z);
        resources.box->Draw(bodyWorld, view, projection, Color(0.78f, 0.84f, 0.92f, 1.0f));
        resources.sphere->Draw(Matrix::CreateScale(0.42f) * Matrix::CreateTranslation(position.x, position.y + 1.95f, position.z), view, projection, Color(0.92f, 0.96f, 1.0f, 1.0f));

        const Matrix swordWorld = Matrix::CreateScale(0.08f, 0.92f, 0.08f)
            * Matrix::CreateRotationX(DirectX::XMConvertToRadians(90.0f))
            * Matrix::CreateRotationZ(-0.72f + slashPulse * 0.36f)
            * Matrix::CreateTranslation(position.x + std::sin(yaw) * 0.65f, position.y + 1.28f, position.z + std::cos(yaw) * 0.65f);
        resources.cylinder->Draw(swordWorld, view, projection, Color(0.92f, 0.96f, 1.0f, 1.0f));

        const float ringScale = 0.95f + slashPulse * 0.55f;
        resources.torus->Draw(
            Matrix::CreateScale(ringScale, 1.0f, ringScale)
            * Matrix::CreateRotationX(DirectX::XM_PIDIV2)
            * Matrix::CreateTranslation(position.x + std::sin(yaw) * 0.95f, position.y + 1.18f, position.z + std::cos(yaw) * 0.95f),
            view,
            projection,
            Color(accent.x, accent.y, accent.z, 0.82f));
    }

    void DrawFrontEnemy(
        TitleFront3DResources& resources,
        const Matrix& view,
        const Matrix& projection,
        const Vector3& position,
        float timeSeed,
        bool isLaser,
        bool isDasher)
    {
        if (!resources.sphere || !resources.box || !resources.torus)
        {
            return;
        }

        const Color enemyColor = isLaser ? Color(1.0f, 0.56f, 0.18f, 1.0f)
            : (isDasher ? Color(0.44f, 0.96f, 0.88f, 1.0f) : Color(1.0f, 0.22f, 0.24f, 1.0f));

        resources.sphere->Draw(Matrix::CreateScale(0.55f) * Matrix::CreateTranslation(position.x, position.y + 0.9f, position.z), view, projection, enemyColor);
        resources.box->Draw(Matrix::CreateScale(0.22f, 1.0f + std::sin(timeSeed * 1.4f) * 0.08f, 0.22f) * Matrix::CreateTranslation(position.x, position.y + 0.45f, position.z), view, projection, Color(enemyColor.x * 0.55f, enemyColor.y * 0.55f, enemyColor.z * 0.55f, 1.0f));

        if (isDasher)
        {
            const float warnScale = 0.85f + 0.18f * (0.5f + 0.5f * std::sin(timeSeed * 3.8f));
            resources.torus->Draw(
                Matrix::CreateScale(warnScale, 1.0f, warnScale) * Matrix::CreateRotationX(DirectX::XM_PIDIV2) * Matrix::CreateTranslation(position.x, position.y + 0.15f, position.z),
                view,
                projection,
                Color(1.0f, 0.88f, 0.30f, 0.72f));
        }
    }

    void RenderTitleFront3D(
        float width,
        float height,
        bool demoMode,
        float baseTimer,
        float demoTimer,
        int stageIndex,
        const DirectX::SimpleMath::Vector2& playerPos,
        const std::vector<TitleScene::DemoEnemy>& enemies)
    {
        auto* device = DirectX11::Get().GetDevice().Get();
        auto* context = DirectX11::Get().GetContext().Get();
        if (device == nullptr || context == nullptr)
        {
            return;
        }

        TitleFront3DResources& resources = GetTitleFront3D();
        resources.Ensure(device, context);
        if (!resources.initialized)
        {
            return;
        }

        const int clampedStage = std::clamp(stageIndex, 1, 3);
        const float orbitT = demoMode ? demoTimer : baseTimer;
        const Vector3 target(0.0f, 1.25f, 0.0f);
        const float camYaw = (demoMode ? 0.55f : 0.92f) + orbitT * (demoMode ? 0.38f : 0.12f);
        const float camDist = demoMode ? 18.0f : 21.0f;
        const float camHeight = demoMode ? (7.0f + std::sin(orbitT * 0.35f) * 0.9f) : (8.6f + std::sin(orbitT * 0.18f) * 0.5f);
        const Vector3 eye(std::sin(camYaw) * camDist, camHeight, std::cos(camYaw) * camDist);

        const Matrix view = Matrix::CreateLookAt(eye, target, Vector3::Up);
        const Matrix projection = BuildFrontProjection(width, height);

        resources.backdrop.Render(context, view, projection, eye);
        if (resources.arenas[clampedStage - 1])
        {
            resources.arenas[clampedStage - 1]->Render(context, view, projection);
        }

        const Vector3 heroPos(playerPos.x, 0.0f, playerPos.y);
        const float slashPulse = 0.5f + 0.5f * std::sin((demoMode ? demoTimer : baseTimer) * (demoMode ? 5.8f : 2.2f));
        DrawFrontHero(resources, view, projection, heroPos, 0.85f + std::sin(orbitT * 0.9f) * 0.32f, slashPulse, demoMode ? Color(0.54f, 0.92f, 1.0f, 1.0f) : Color(0.34f, 0.72f, 1.0f, 1.0f));

        if (demoMode)
        {
            for (size_t i = 0; i < enemies.size(); ++i)
            {
                const auto& enemy = enemies[i];
                const Vector3 enemyPos(enemy.pos.x, 0.0f, enemy.pos.y);
                DrawFrontEnemy(resources, view, projection, enemyPos, orbitT + static_cast<float>(i), enemy.isLaser, enemy.isDasher);
                if (enemy.isLaser && enemy.stateTimer > 0.0f && resources.cylinder)
                {
                    Vector3 toHero = heroPos + Vector3(0.0f, 0.95f, 0.0f) - (enemyPos + Vector3(0.0f, 0.9f, 0.0f));
                    const float len = toHero.Length();
                    if (len > 0.25f)
                    {
                        toHero.Normalize();
                        const Vector3 mid = enemyPos + Vector3(0.0f, 0.9f, 0.0f) + toHero * (len * 0.5f);
                        const float yaw = std::atan2(toHero.x, toHero.z);
                        const float pitch = -std::asin(std::clamp(toHero.y, -1.0f, 1.0f));
                        resources.cylinder->Draw(
                            Matrix::CreateScale(0.05f, len * 0.5f, 0.05f)
                            * Matrix::CreateRotationZ(DirectX::XM_PIDIV2)
                            * Matrix::CreateRotationX(pitch)
                            * Matrix::CreateRotationY(yaw)
                            * Matrix::CreateTranslation(mid),
                            view,
                            projection,
                            Color(1.0f, 0.32f, 0.18f, 0.92f));
                    }
                }
            }
        }
        else
        {
            const std::array<Vector3, 4> previewEnemies =
            {
                Vector3(-5.5f, 0.0f, 6.0f),
                Vector3(6.8f, 0.0f, 4.0f),
                Vector3(-7.0f, 0.0f, -5.5f),
                Vector3(5.8f, 0.0f, -6.2f)
            };
            for (size_t i = 0; i < previewEnemies.size(); ++i)
            {
                const bool laser = (clampedStage >= 2) && (i == 1 || i == 3);
                const bool dasher = (clampedStage == 3) && (i == 2);
                DrawFrontEnemy(resources, view, projection, previewEnemies[i], orbitT + static_cast<float>(i) * 0.35f, laser, dasher);
            }
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

    // マウスでも操作できるようにする（絶対モード、有効なシステムカーソル）
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    System::InputManager::GetInstance().ResetMouseDelta();
    SetCursorVisible(true);
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
    // keyboard navigation
    if (input.IsKeyPressed(DirectX::Keyboard::Up)) { m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_menuItems.size())) % static_cast<int>(m_menuItems.size()); anyInput = true; }
    if (input.IsKeyPressed(DirectX::Keyboard::Down)) { m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_menuItems.size()); anyInput = true; }

    // also support W/S
    if (input.IsKeyPressed(DirectX::Keyboard::W)) { m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_menuItems.size())) % static_cast<int>(m_menuItems.size()); anyInput = true; }
    if (input.IsKeyPressed(DirectX::Keyboard::S)) { m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_menuItems.size()); anyInput = true; }

    // マウス操作: ホバーで選択、左クリックで決定
    const DirectX::Mouse::State* ms = input.GetMouseState();
    if (ms)
    {
        const float width = static_cast<float>(DirectX11::Get().GetWidth());
        const float height = static_cast<float>(DirectX11::Get().GetHeight());
        const float menuX = 78.0f;
        const float menuY = height * 0.56f;
        const float menuW = 320.0f;
        const float menuH = 56.0f;
        for (int i = 0; i < static_cast<int>(m_menuItems.size()); ++i)
        {
            const float y = menuY + i * (menuH + 16.0f);
            if (ms->x >= menuX && ms->x <= menuX + menuW && ms->y >= y && ms->y <= y + menuH)
            {
                if (m_selectedIndex != i) { m_selectedIndex = i; GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::UiMove, 0.16f); }
                anyInput = true;
                if (input.IsMouseButtonPressed(0))
                {
                    // emulate Enter
                    if (g_sceneManager != nullptr)
                    {
                        const std::wstring& sel = m_menuItems[m_selectedIndex];
                        if (sel == L"出撃") { g_sceneManager->RequestTransition(STAGE_SELECT_SCENE); }
                        else if (sel == L"設定") { g_sceneManager->RequestTransition(SETTINGS_SCENE); }
                        else if (sel == L"終了") { extern void ExitGame(); ExitGame(); }
                    }
                }
            }
        }

        // クリックや移動を入力と見なす
        if (ms->x != 0 || ms->y != 0 || ms->leftButton || ms->rightButton) anyInput = true;
    }

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
    if (m_font == nullptr || m_spriteBatch == nullptr)
    {
        return;
    }

    const float width = static_cast<float>(DirectX11::Get().GetWidth());
    const float height = static_cast<float>(DirectX11::Get().GetHeight());

    const int frontStage = m_demoMode ? m_demoStageIndex : std::clamp(m_selectedIndex + 1, 1, 3);
    const DirectX::SimpleMath::Vector2 frontPlayerPos = m_demoMode ? m_demoPlayerPos : DirectX::SimpleMath::Vector2(0.0f, 0.0f);
    RenderTitleFront3D(width, height, m_demoMode, m_timer, m_demoTimer, frontStage, frontPlayerPos, m_demoEnemies);

    System::DrawManager::GetInstance().Begin();

    const auto drawRect = [&](float x, float y, float w, float h, const Color& color)
    {
        if (!m_whiteTex) return;
        const RECT rect = { static_cast<LONG>(x), static_cast<LONG>(y), static_cast<LONG>(x + w), static_cast<LONG>(y + h) };
        m_spriteBatch->Draw(m_whiteTex.Get(), rect, color);
    };

    drawRect(0.0f, 0.0f, width, height, Color(0.02f, 0.03f, 0.05f, m_demoMode ? 0.20f : 0.16f));
    drawRect(0.0f, 0.0f, width, 120.0f, Color(0.01f, 0.02f, 0.03f, 0.45f));
    drawRect(0.0f, height - 140.0f, width, 140.0f, Color(0.01f, 0.02f, 0.03f, 0.50f));

    const float titlePanelX = 72.0f;
    const float titlePanelY = 78.0f;
    const float titlePanelW = width * 0.40f;
    const float titlePanelH = m_demoMode ? 170.0f : 210.0f;
    drawRect(titlePanelX, titlePanelY, titlePanelW, titlePanelH, Color(0.03f, 0.06f, 0.10f, 0.62f));
    drawRect(titlePanelX, titlePanelY, titlePanelW, 3.0f, Color(0.50f, 0.82f, 1.0f, 0.96f));

    m_font->DrawString(m_spriteBatch, L"SteelRevenant", DirectX::XMFLOAT2(titlePanelX + 22.0f, titlePanelY + 16.0f), Color(0.96f, 0.98f, 1.0f, 1.0f), 0.0f, DirectX::XMFLOAT2(0, 0), 1.58f);
    m_font->DrawString(m_spriteBatch, L"近接主体の3Dサバイバルアクション", DirectX::XMFLOAT2(titlePanelX + 24.0f, titlePanelY + 72.0f), Color(0.64f, 0.88f, 1.0f, 0.92f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.64f);
    m_font->DrawString(m_spriteBatch, L"斬り込み、守り、押し返す。短時間で密度の高い近接戦を捌く。", DirectX::XMFLOAT2(titlePanelX + 24.0f, titlePanelY + 104.0f), Color(0.82f, 0.88f, 0.94f, 0.94f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.70f);

    if (m_demoMode)
    {
        const float pulse = 0.50f + 0.50f * std::sin(m_demoTimer * 2.6f);
        drawRect(width * 0.60f, 84.0f, 280.0f, 110.0f, Color(0.04f, 0.08f, 0.12f, 0.68f));
        drawRect(width * 0.60f, 84.0f, 280.0f, 3.0f, Color(1.0f, 0.78f, 0.28f, 0.94f));
        m_font->DrawString(m_spriteBatch, L"デモ再生", DirectX::XMFLOAT2(width * 0.60f + 18.0f, 104.0f), Color(1.0f, 0.84f, 0.42f, 0.96f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.88f);
        const std::wstring stageText = std::wstring(L"ステージ ") + std::to_wstring(frontStage) + (frontStage == 1 ? L"  外縁区画" : (frontStage == 2 ? L"  防衛回廊" : L"  中枢区画"));
        m_font->DrawString(m_spriteBatch, stageText.c_str(), DirectX::XMFLOAT2(width * 0.60f + 18.0f, 138.0f), Color(0.86f, 0.92f, 1.0f, 0.94f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.68f);
        m_font->DrawString(m_spriteBatch, L"キーを押すと戻る", DirectX::XMFLOAT2(width * 0.60f + 18.0f, 164.0f), Color(1.0f, 0.92f, 0.62f, 0.48f + pulse * 0.52f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.72f);
    }
    else
    {
        const float menuX = 78.0f;
        const float menuY = height * 0.56f;
        const float menuW = 320.0f;
        const float menuH = 56.0f;
        for (int i = 0; i < static_cast<int>(m_menuItems.size()); ++i)
        {
            const bool selected = (i == m_selectedIndex);
            const float y = menuY + i * (menuH + 16.0f);
            drawRect(menuX, y + m_menuOffsets[i], menuW, menuH, selected ? Color(0.10f, 0.18f, 0.30f, 0.84f) : Color(0.04f, 0.06f, 0.10f, 0.64f));
            if (selected)
            {
                drawRect(menuX, y + m_menuOffsets[i], 6.0f, menuH, Color(0.54f, 0.88f, 1.0f, 0.98f));
                drawRect(menuX + menuW - 6.0f, y + m_menuOffsets[i], 6.0f, menuH, Color(0.54f, 0.88f, 1.0f, 0.82f));
            }
            m_font->DrawString(m_spriteBatch, m_menuItems[i].c_str(), DirectX::XMFLOAT2(menuX + 22.0f, y + 11.0f + m_menuOffsets[i]), selected ? Color(0.98f, 0.98f, 1.0f, 1.0f) : Color(0.86f, 0.90f, 0.96f, 1.0f), 0.0f, DirectX::XMFLOAT2(0, 0), selected ? 1.04f : 0.94f);
        }

        drawRect(width * 0.64f, height * 0.62f, 310.0f, 132.0f, Color(0.03f, 0.05f, 0.08f, 0.56f));
        drawRect(width * 0.64f, height * 0.62f, 310.0f, 3.0f, Color(0.32f, 0.72f, 1.0f, 0.88f));
        m_font->DrawString(m_spriteBatch, L"戦術概要", DirectX::XMFLOAT2(width * 0.64f + 18.0f, height * 0.62f + 16.0f), Color(0.70f, 0.92f, 1.0f, 0.96f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.72f);
        m_font->DrawString(m_spriteBatch, L"- 近接主体の3Dアクション", DirectX::XMFLOAT2(width * 0.64f + 22.0f, height * 0.62f + 48.0f), Color(0.88f, 0.92f, 0.96f, 0.94f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.62f);
        m_font->DrawString(m_spriteBatch, L"- ステージ毎に敵構成と圧力が変化", DirectX::XMFLOAT2(width * 0.64f + 22.0f, height * 0.62f + 72.0f), Color(0.88f, 0.92f, 0.96f, 0.94f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.62f);
        m_font->DrawString(m_spriteBatch, L"- 放置時は3D デモに移行", DirectX::XMFLOAT2(width * 0.64f + 22.0f, height * 0.62f + 96.0f), Color(0.88f, 0.92f, 0.96f, 0.94f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.62f);

        const float promptAlpha = 0.26f + 0.74f * (0.5f + 0.5f * std::sin(m_blink * 4.2f));
        m_font->DrawString(m_spriteBatch, L"キーまたはクリックで操作", DirectX::XMFLOAT2(width * 0.50f - 122.0f, height * 0.86f), Color(1.0f, 0.90f, 0.58f, promptAlpha), 0.0f, DirectX::XMFLOAT2(0, 0), 0.92f);
    }

    m_font->DrawString(m_spriteBatch, L"W / S で選択   Enter / Space で決定   18秒放置で3D DEMO", DirectX::XMFLOAT2(32.0f, height - 44.0f), Color(0.78f, 0.82f, 0.88f, 0.92f), 0.0f, DirectX::XMFLOAT2(0, 0), 0.66f);
    System::DrawManager::GetInstance().End();
}


void TitleScene::Finalize()
{
    GetTitleFront3D().Reset();
    // restore system cursor visibility and set mouse mode to absolute so other UI can show cursor if needed
    SetCursorVisible(true);
    m_font.reset();
    m_spriteBatch = nullptr;
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
    System::InputManager::GetInstance().ResetMouseDelta();
}
