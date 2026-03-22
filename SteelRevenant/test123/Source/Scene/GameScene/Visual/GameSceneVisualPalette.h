#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <cmath>
#include <SimpleMath.h>

#include "../../../Utility/SimpleMathEx.h"

namespace SceneFx
{
    using Color = DirectX::SimpleMath::Color;

    struct StageRenderPalette
    {
        int stageTheme = 1;
        float stagePulse = 0.0f;
        Color stageTintMul = Color(1.0f, 1.0f, 1.0f, 1.0f);

        Color skyZenithColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        Color skyMidColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        Color skyBaseColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        Color skyHazeColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        Color horizonGlowColor = Color(0.0f, 0.0f, 0.0f, 1.0f);

        Color moonColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        Color beamColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        Color cloudColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        Color accentColor = Color(0.0f, 0.0f, 0.0f, 1.0f);

        bool showNightElements = false;
        bool showLightning = false;
        float lightningFlicker = 0.0f;
    };

    inline StageRenderPalette BuildStageRenderPalette(int stageThemeIndex, float sceneTime)
    {
        StageRenderPalette p;
        p.stageTheme = std::max(1, (std::min)(3, stageThemeIndex));
        p.stagePulse = std::sin(sceneTime * 0.7f) * 0.5f + 0.5f;

        switch (p.stageTheme)
        {
        case 1:
        default:
            p.stageTintMul = Color(1.00f, 0.97f, 1.04f, 1.0f);
            p.skyZenithColor = Color(0.04f, 0.04f, 0.10f, 1.0f);
            p.skyMidColor = Color(0.08f, 0.10f, 0.20f, 0.92f);
            p.skyBaseColor = Color(0.12f, 0.14f, 0.26f, 0.72f);
            p.skyHazeColor = Color(0.20f, 0.18f, 0.28f, 0.45f);
            p.horizonGlowColor = Color(0.58f, 0.32f, 0.12f, 0.35f);
            p.moonColor = Color(0.95f, 0.98f, 1.00f, 0.88f);
            p.beamColor = Color(0.40f, 0.50f, 0.80f, 0.12f);
            p.cloudColor = Color(0.14f, 0.16f, 0.24f, 0.28f);
            p.accentColor = Color(0.28f, 0.42f, 0.92f, 1.0f);
            p.showNightElements = true;
            break;

        case 2:
            p.stageTintMul = Color(1.06f, 0.95f, 0.88f, 1.0f);
            p.skyZenithColor = Color(0.08f, 0.05f, 0.04f, 1.0f);
            p.skyMidColor = Color(0.18f, 0.10f, 0.06f, 0.90f);
            p.skyBaseColor = Color(0.32f, 0.16f, 0.06f, 0.75f);
            p.skyHazeColor = Color(0.52f, 0.26f, 0.08f, 0.50f);
            p.horizonGlowColor = Color(1.00f, 0.48f, 0.08f, 0.60f);
            p.moonColor = Color(1.00f, 0.72f, 0.28f, 0.65f);
            p.beamColor = Color(0.90f, 0.55f, 0.15f, 0.15f);
            p.cloudColor = Color(0.28f, 0.16f, 0.08f, 0.45f);
            p.accentColor = Color(1.00f, 0.45f, 0.08f, 1.0f);
            break;

        case 3:
            p.stageTintMul = Color(0.92f, 0.96f, 1.08f, 1.0f);
            p.skyZenithColor = Color(0.06f, 0.10f, 0.18f, 1.0f);
            p.skyMidColor = Color(0.14f, 0.20f, 0.30f, 0.88f);
            p.skyBaseColor = Color(0.26f, 0.34f, 0.44f, 0.70f);
            p.skyHazeColor = Color(0.44f, 0.52f, 0.62f, 0.50f);
            p.horizonGlowColor = Color(0.62f, 0.72f, 0.84f, 0.40f);
            p.moonColor = Color(0.80f, 0.88f, 1.00f, 0.45f);
            p.beamColor = Color(0.70f, 0.82f, 1.00f, 0.18f);
            p.cloudColor = Color(0.52f, 0.60f, 0.70f, 0.55f);
            p.accentColor = Color(0.20f, 0.70f, 1.00f, 1.0f);
            p.showNightElements = true;
            {
                const float lt = std::fmod(sceneTime * 0.37f, 1.0f);
                p.showLightning = (lt < 0.04f) || (lt > 0.42f && lt < 0.45f);
                p.lightningFlicker = p.showLightning ? ((1.0f - lt / 0.04f) * 0.7f) : 0.0f;
            }
            break;
        }

        return p;
    }

    inline Color ApplyStageTint(const StageRenderPalette& palette, const Color& base, float alphaScale = 1.0f)
    {
        const float lf = palette.lightningFlicker;
        return Color(
            Utility::MathEx::Clamp(base.x * palette.stageTintMul.x + lf * 0.18f, 0.0f, 1.0f),
            Utility::MathEx::Clamp(base.y * palette.stageTintMul.y + lf * 0.20f, 0.0f, 1.0f),
            Utility::MathEx::Clamp(base.z * palette.stageTintMul.z + lf * 0.22f, 0.0f, 1.0f),
            Utility::MathEx::Clamp(base.w * alphaScale, 0.0f, 1.0f));
    }

    struct FloorPalette
    {
        Color baseColor = Color(0.10f, 0.12f, 0.14f, 1.0f);
        Color topColor = Color(0.14f, 0.18f, 0.20f, 1.0f);
        Color panelColor = Color(0.16f, 0.22f, 0.26f, 0.85f);
        Color seamColor = Color(0.32f, 0.36f, 0.42f, 1.0f);
        Color accentColor = Color(0.20f, 0.55f, 0.90f, 1.0f);
    };
}

