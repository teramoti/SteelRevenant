#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../IScene.h"

#include <memory>
#include <vector>
#include <string>

#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <SimpleMath.h>
#include <wrl/client.h>

class TitleScene : public IScene
{
public:
    void Initialize() override;
    void Update(const DX::StepTimer& timer) override;
    void Render() override;
    void Finalize() override;

    // demo entity used by the attract demo (public so helper functions can initialize)
    struct DemoEnemy { DirectX::SimpleMath::Vector2 pos; DirectX::SimpleMath::Vector2 vel; bool isLaser; bool isDasher; float stateTimer; float warnTimer; };

private:
    float m_timer = 0.0f;
    float m_blink = 0.0f;
    DirectX::SpriteBatch* m_spriteBatch = nullptr;
    std::unique_ptr<DirectX::SpriteFont> m_font;

    // Menu
    std::vector<std::wstring> m_menuItems = { L"出撃", L"設定", L"終了" };
    int m_selectedIndex = 0;

    // small per-menu animation offsets for smooth transition
    std::vector<float> m_menuOffsets;

    // Logo particles for simple visual flourish
    struct LogoParticle { DirectX::SimpleMath::Vector2 pos; DirectX::SimpleMath::Vector2 vel; float life; };
    std::vector<LogoParticle> m_logoParticles;

    // attract/demo mode
    float m_idleTimer = 0.0f; // seconds of inactivity
    bool m_idleAccentTriggered = false;
    float m_idleLoopSeTimer = 0.0f;
    bool m_demoMode = false;
    float m_demoTimer = 0.0f; // progress timer used during demo animation

    // demo content
    int m_demoStageIndex = 1; // 1..3
    std::vector<DemoEnemy> m_demoEnemies;
    DirectX::SimpleMath::Vector2 m_demoPlayerPos;

    // simple white texture for drawing primitives
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_whiteTex;
};

