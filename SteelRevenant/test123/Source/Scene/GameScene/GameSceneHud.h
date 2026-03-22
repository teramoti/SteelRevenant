#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d11.h>
#include <string>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <wrl/client.h>

#include "../../Action/CombatSystem.h"
#include "../../Action/GameState.h"
#include "../../Action/SurvivalDirector.h"

class GameSceneHud
{
public:
    void Initialize(
        ID3D11Device* device,
        ID3D11DeviceContext* context,
        DirectX::SpriteBatch* spriteBatch,
        DirectX::SpriteFont* font);

    void Update(float dt);

    void Render(
        const Action::GameState& gameState,
        const Action::PlayerState& player,
        const Action::SurvivalDirector& director,
        const std::vector<DirectX::SimpleMath::Vector3>& spawnPoints,
        const std::vector<DirectX::SimpleMath::Vector4>& wallRects,
        const std::vector<Action::EnemyState>& enemies,
        bool speedActive,
        float speedRemaining,
        int screenW,
        int screenH);

    void RenderPauseMenu(int selectedIndex, int screenW, int screenH);

    void TriggerWaveBanner(int waveNumber, int totalWaves);
    void TriggerScreenShake(float intensity);
    DirectX::SimpleMath::Vector2 GetShakeOffset() const { return m_shakeOffset; }
    void TriggerComboPopup(int comboIndex, int comboLevel);
    void TriggerKillPulse(int totalKills);

private:
    void DrawBar(
        float x,
        float y,
        float w,
        float h,
        float ratio,
        DirectX::SimpleMath::Color fill,
        DirectX::SimpleMath::Color back);
    void DrawText(
        const std::wstring& text,
        float x,
        float y,
        DirectX::SimpleMath::Color color,
        float scale = 1.0f);

    DirectX::SpriteBatch* m_spriteBatch = nullptr;
    DirectX::SpriteFont* m_font = nullptr;

    float m_shakeTimer = 0.0f;
    float m_shakeIntensity = 0.0f;
    DirectX::SimpleMath::Vector2 m_shakeOffset;

    float m_bannerTimer = 0.0f;
    int m_bannerWave = 0;
    int m_bannerTotal = 0;

    float m_comboPopTimer = 0.0f;
    int m_comboPopIndex = 0;
    int m_comboPopLevel = 0;
    float m_killPulseTimer = 0.0f;
    int m_killPulseKills = 0;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_whiteTex;

    // previous frame stage timer to detect threshold crossing for warnings
    float m_prevStageTimer = 0.0f;
    int m_timeWarnState = 0; // 0=normal,1=yellow,2=red
};

