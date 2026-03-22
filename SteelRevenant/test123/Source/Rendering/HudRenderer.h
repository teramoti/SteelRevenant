#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <string>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

#include "../Action/CombatSystem.h"
#include "../Action/GameState.h"

namespace System
{
    class UIShaderText;
}

namespace Rendering
{
    class HudRenderer
    {
    public:
        struct HudContext
        {
            const Action::PlayerState* player = nullptr;
            float attackBlend = 0.0f;
            const Action::GameState* gameState = nullptr;
            float hitBloodTimer = 0.0f;
            float damageBloodTimer = 0.0f;
            float stageIntroTimer = 0.0f;
            const std::vector<Action::EnemyState>* enemies = nullptr;
            bool isPaused = false;
            int pauseSelectedIndex = 0;
            float pauseClickFxTimer = 0.0f;
            DirectX::SimpleMath::Vector2 pauseClickFxPos = DirectX::SimpleMath::Vector2::Zero;
            bool showDebugDetail = false;
            float mouseSensitivity = 0.08f;
            float attackAssistRange = 2.6f;
            float attackAssistDot = 0.30f;
        };

        enum class PauseMenuResult
        {
            None,
            Resume,
            Restart,
            Title,
            Quit
        };

        explicit HudRenderer(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> solidTexture);

        void Draw(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& context,
            float width,
            float height) const;

        PauseMenuResult UpdatePauseMenu(
            const DirectX::SimpleMath::Vector2& mousePos,
            bool mouseClicked,
            int& outIndex,
            float width,
            float height) const;

    private:
        void DrawStatusBars(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        void DrawTimer(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        void DrawObjectiveInfo(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        void DrawObjectiveBanner(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        void DrawStageIntro(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        void DrawMiniMap(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        void DrawBloodOverlay(
            DirectX::SpriteBatch* batch,
            const HudContext& ctx,
            float width, float height) const;

        void DrawPauseOverlay(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float width, float height) const;

        void DrawDebugPanel(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        void DrawSolidRect(
            DirectX::SpriteBatch* batch,
            const DirectX::SimpleMath::Vector2& pos,
            const DirectX::SimpleMath::Vector2& size,
            const DirectX::SimpleMath::Color& color) const;

        DirectX::SimpleMath::Vector2 WorldToMiniMap(
            const DirectX::SimpleMath::Vector3& worldPos,
            const DirectX::SimpleMath::Vector2& mapTopLeft,
            float mapSize) const;

        std::wstring FormatTimerMMSS(float seconds) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_solidTexture;
    };
} // namespace Rendering

