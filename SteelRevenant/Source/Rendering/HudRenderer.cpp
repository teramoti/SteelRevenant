#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "HudRenderer.h"

#include "../GameSystem/UIShaderText.h"

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <utility>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace Rendering
{
    namespace
    {
        constexpr float kMiniMapWorldRadius = 32.0f;

        float Clamp01(float value)
        {
            return std::max(0.0f, (std::min)(1.0f, value));
        }

        Color HpColor(float ratio)
        {
            if (ratio > 0.5f)
            {
                return Color::Lerp(
                    Color(0.90f, 0.70f, 0.05f, 1.0f),
                    Color(0.20f, 0.80f, 0.30f, 1.0f),
                    (ratio - 0.5f) * 2.0f);
            }

            return Color::Lerp(
                Color(0.90f, 0.15f, 0.10f, 1.0f),
                Color(0.90f, 0.70f, 0.05f, 1.0f),
                ratio * 2.0f);
        }
    }

    HudRenderer::HudRenderer(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> solidTexture)
        : m_solidTexture(std::move(solidTexture))
    {
    }

    void HudRenderer::Draw(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& context,
        float width,
        float height) const
    {
        if (!batch)
        {
            return;
        }

        const float uiScale = std::max(1.0f, height / 600.0f);

        if (context.isPaused)
        {
            DrawPauseOverlay(batch, uiText, context, width, height);
            return;
        }

        DrawTimer(batch, uiText, context, uiScale, width, height);
        DrawObjectiveInfo(batch, uiText, context, uiScale, width, height);
        DrawObjectiveBanner(batch, uiText, context, uiScale, width, height);
        DrawStageIntro(batch, uiText, context, uiScale, width, height);
        DrawBloodOverlay(batch, context, width, height);

        if (context.showDebugDetail)
        {
            DrawDebugPanel(batch, uiText, context, uiScale, width, height);
        }
    }

    void HudRenderer::DrawStatusBars(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float uiScale,
        float width,
        float height) const
    {
        (void)width;

        if (!ctx.player)
        {
            return;
        }

        const float barWidth = 220.0f * uiScale;
        const float hpHeight = 14.0f * uiScale;
        const float comboHeight = 8.0f * uiScale;
        const float baseX = 20.0f * uiScale;
        const float hpY = height - 56.0f * uiScale;
        const float comboY = height - 30.0f * uiScale;

        const float hpRatio = Clamp01(ctx.player->hp / 100.0f);
        DrawSolidRect(batch, { baseX, hpY }, { barWidth, hpHeight }, Color(0.10f, 0.10f, 0.10f, 0.82f));
        DrawSolidRect(batch, { baseX, hpY }, { barWidth * hpRatio, hpHeight }, HpColor(hpRatio));

        const float comboRatio = Clamp01(ctx.player->comboGauge / 100.0f);
        Color comboColor(0.40f, 0.50f, 0.80f, 1.0f);
        if (ctx.player->comboLevel >= 3)
        {
            comboColor = Color(1.00f, 0.60f, 0.10f, 1.0f);
        }
        else if (ctx.player->comboLevel >= 2)
        {
            comboColor = Color(0.90f, 0.90f, 0.10f, 1.0f);
        }
        else if (ctx.player->comboLevel >= 1)
        {
            comboColor = Color(0.30f, 0.90f, 0.40f, 1.0f);
        }

        DrawSolidRect(batch, { baseX, comboY }, { barWidth, comboHeight }, Color(0.10f, 0.10f, 0.10f, 0.75f));
        DrawSolidRect(batch, { baseX, comboY }, { barWidth * comboRatio, comboHeight }, comboColor);

        if (!uiText)
        {
            return;
        }

        System::UIShaderStyle style;
        style.baseColor = Color(0.70f, 0.70f, 0.70f, 1.0f);
        uiText->Draw(batch, L"HP", { baseX, hpY - 18.0f * uiScale }, style, 0.72f * uiScale);

        style.baseColor = comboColor;
        uiText->Draw(batch, L"COMBO", { baseX, comboY - 18.0f * uiScale }, style, 0.72f * uiScale);

        if (ctx.player->comboLevel > 0)
        {
            wchar_t buffer[16];
            swprintf_s(buffer, L"Lv%d", ctx.player->comboLevel);
            uiText->Draw(batch, buffer, { baseX + barWidth + 6.0f * uiScale, comboY - 4.0f * uiScale }, style, 0.75f * uiScale);
        }
    }

    void HudRenderer::DrawTimer(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float uiScale,
        float width,
        float height) const
    {
        (void)height;

        if (!ctx.gameState || !uiText)
        {
            return;
        }

        const std::wstring timerText = FormatTimerMMSS(ctx.gameState->stageTimer);
        const Color color = ctx.gameState->stageTimer < 30.0f
            ? Color(1.00f, 0.40f, 0.20f, 1.0f)
            : Color(0.90f, 0.90f, 0.90f, 1.0f);

        System::UIShaderStyle style;
        style.baseColor = color;
        style.outlineColor = Color(0.0f, 0.0f, 0.0f, 0.85f);

        const Vector2 size = uiText->Measure(timerText, 1.4f * uiScale);
        uiText->Draw(batch, timerText, { width * 0.5f - size.x * 0.5f, 14.0f * uiScale }, style, 1.4f * uiScale);
    }

    void HudRenderer::DrawObjectiveInfo(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float uiScale,
        float width,
        float height) const
    {
        (void)height;

        if (!ctx.gameState || !uiText)
        {
            return;
        }

        System::UIShaderStyle style;
        style.baseColor = Color(0.85f, 0.85f, 0.85f, 1.0f);
        const float scale = 0.88f * uiScale;

        wchar_t buffer[64];
        swprintf_s(buffer, L"KILLS: %d", ctx.gameState->killCount);
        uiText->Draw(batch, buffer, { 20.0f * uiScale, 14.0f * uiScale }, style, scale);

        (void)width;
    }

    void HudRenderer::DrawObjectiveBanner(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float uiScale,
        float width,
        float height) const
    {
        (void)batch;
        (void)ctx;
        (void)height;

        if (!uiText)
        {
            return;
        }

        System::UIShaderStyle style;
        style.baseColor = Color(0.92f, 0.84f, 0.45f, 0.95f);

        const std::wstring text = L"SURVIVE UNTIL TIME UP";
        const Vector2 size = uiText->Measure(text, 0.78f * uiScale);
        uiText->Draw(batch, text, { width * 0.5f - size.x * 0.5f, 56.0f * uiScale }, style, 0.78f * uiScale);
    }

    void HudRenderer::DrawStageIntro(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float uiScale,
        float width,
        float height) const
    {
        if (ctx.stageIntroTimer <= 0.0f || !uiText)
        {
            return;
        }

        const float alpha = Clamp01(ctx.stageIntroTimer < 0.5f ? ctx.stageIntroTimer * 2.0f : 1.0f);

        System::UIShaderStyle style;
        style.baseColor = Color(1.0f, 0.9f, 0.3f, alpha);

        const std::wstring text = L"--- BATTLE START ---";
        const Vector2 size = uiText->Measure(text, 1.3f * uiScale);
        uiText->Draw(batch, text, { width * 0.5f - size.x * 0.5f, height * 0.38f }, style, 1.3f * uiScale);
    }

    void HudRenderer::DrawMiniMap(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float uiScale,
        float width,
        float height) const
    {
        (void)uiText;

        if (!ctx.player || !ctx.enemies)
        {
            return;
        }

        const float mapSize = 90.0f * uiScale;
        const float margin = 20.0f * uiScale;
        const Vector2 mapTopLeft(width - mapSize - margin, height - mapSize - margin - 46.0f * uiScale);

        DrawSolidRect(batch, mapTopLeft, { mapSize, mapSize }, Color(0.0f, 0.0f, 0.0f, 0.55f));

        const Vector2 playerDot = WorldToMiniMap(ctx.player->position, mapTopLeft, mapSize);
        DrawSolidRect(batch, playerDot - Vector2(3.0f, 3.0f) * uiScale, Vector2(6.0f, 6.0f) * uiScale, Color(1.0f, 1.0f, 1.0f, 1.0f));

        for (const auto& enemy : *ctx.enemies)
        {
            if (enemy.state == Action::EnemyStateType::Dead)
            {
                continue;
            }

            const Vector2 dot = WorldToMiniMap(enemy.position, mapTopLeft, mapSize);
            DrawSolidRect(batch, dot - Vector2(2.0f, 2.0f) * uiScale, Vector2(4.0f, 4.0f) * uiScale, Color(1.0f, 0.2f, 0.2f, 0.85f));
        }
    }

    void HudRenderer::DrawBloodOverlay(
        DirectX::SpriteBatch* batch,
        const HudContext& ctx,
        float width,
        float height) const
    {
        const float timer = std::max(ctx.hitBloodTimer, ctx.damageBloodTimer);
        if (timer <= 0.0f)
        {
            return;
        }

        DrawSolidRect(batch, { 0.0f, 0.0f }, { width, height }, Color(0.55f, 0.03f, 0.03f, timer * 0.35f));
    }

    void HudRenderer::DrawPauseOverlay(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float width,
        float height) const
    {
        DrawSolidRect(batch, { 0.0f, 0.0f }, { width, height }, Color(0.0f, 0.0f, 0.0f, 0.62f));

        if (!uiText)
        {
            return;
        }

        static const wchar_t* labels[] = {
            L"RESUME",
            L"RESTART",
            L"TITLE",
            L"QUIT"
        };

        const float centerY = height * 0.3f;
        const float lineHeight = 44.0f;

        for (int index = 0; index < 4; ++index)
        {
            const bool selected = index == ctx.pauseSelectedIndex;
            System::UIShaderStyle style;
            style.baseColor = selected
                ? Color(0.25f, 0.55f, 1.0f, 0.90f)
                : Color(0.75f, 0.75f, 0.75f, 1.0f);

            const float scale = selected ? 1.2f : 1.0f;
            const Vector2 size = uiText->Measure(labels[index], scale);
            uiText->Draw(
                batch,
                labels[index],
                { width * 0.5f - size.x * 0.5f, centerY + lineHeight * static_cast<float>(index) },
                style,
                scale);
        }
    }

    void HudRenderer::DrawDebugPanel(
        DirectX::SpriteBatch* batch,
        System::UIShaderText* uiText,
        const HudContext& ctx,
        float uiScale,
        float width,
        float height) const
    {
        (void)batch;
        (void)height;

        if (!uiText)
        {
            return;
        }

        System::UIShaderStyle style;
        style.baseColor = Color(0.5f, 1.0f, 0.5f, 0.85f);

        const float scale = 0.70f * uiScale;
        const float lineHeight = 18.0f * uiScale;
        float y = 60.0f * uiScale;

        wchar_t buffer[128];
        swprintf_s(buffer, L"Sensitivity: %.2f", ctx.mouseSensitivity);
        uiText->Draw(batch, buffer, { width - 220.0f * uiScale, y }, style, scale);
        y += lineHeight;

        swprintf_s(buffer, L"AtkRange:    %.2f", ctx.attackAssistRange);
        uiText->Draw(batch, buffer, { width - 220.0f * uiScale, y }, style, scale);
        y += lineHeight;

        swprintf_s(buffer, L"AtkDot:      %.2f", ctx.attackAssistDot);
        uiText->Draw(batch, buffer, { width - 220.0f * uiScale, y }, style, scale);
    }

    HudRenderer::PauseMenuResult HudRenderer::UpdatePauseMenu(
        const Vector2& mousePos,
        bool mouseClicked,
        int& outIndex,
        float width,
        float height) const
    {
        const float centerY = height * 0.3f;
        const float lineHeight = 44.0f;
        const float itemHeight = 36.0f;
        const float itemWidth = 200.0f;

        outIndex = -1;
        for (int index = 0; index < 4; ++index)
        {
            const float itemY = centerY + lineHeight * static_cast<float>(index);
            const float itemX = width * 0.5f - itemWidth * 0.5f;

            if (mousePos.x < itemX || mousePos.x > itemX + itemWidth ||
                mousePos.y < itemY || mousePos.y > itemY + itemHeight)
            {
                continue;
            }

            outIndex = index;
            if (!mouseClicked)
            {
                break;
            }

            switch (index)
            {
            case 0:
                return PauseMenuResult::Resume;
            case 1:
                return PauseMenuResult::Restart;
            case 2:
                return PauseMenuResult::Title;
            case 3:
                return PauseMenuResult::Quit;
            default:
                break;
            }
        }

        return PauseMenuResult::None;
    }

    void HudRenderer::DrawSolidRect(
        DirectX::SpriteBatch* batch,
        const Vector2& pos,
        const Vector2& size,
        const Color& color) const
    {
        if (!batch || !m_solidTexture)
        {
            return;
        }

        RECT rect = {
            static_cast<LONG>(pos.x),
            static_cast<LONG>(pos.y),
            static_cast<LONG>(pos.x + size.x),
            static_cast<LONG>(pos.y + size.y)
        };
        batch->Draw(m_solidTexture.Get(), rect, color);
    }

    Vector2 HudRenderer::WorldToMiniMap(
        const Vector3& worldPos,
        const Vector2& mapTopLeft,
        float mapSize) const
    {
        const float nx = (worldPos.x / kMiniMapWorldRadius) * 0.5f + 0.5f;
        const float ny = (worldPos.z / kMiniMapWorldRadius) * 0.5f + 0.5f;

        return Vector2(
            mapTopLeft.x + Clamp01(nx) * mapSize,
            mapTopLeft.y + Clamp01(ny) * mapSize);
    }

    std::wstring HudRenderer::FormatTimerMMSS(float seconds) const
    {
        const int totalSeconds = static_cast<int>(std::max(0.0f, seconds));
        const int minutes = totalSeconds / 60;
        const int remainingSeconds = totalSeconds % 60;

        wchar_t buffer[16];
        swprintf_s(buffer, L"%02d:%02d", minutes, remainingSeconds);
        return buffer;
    }
} // namespace Rendering


