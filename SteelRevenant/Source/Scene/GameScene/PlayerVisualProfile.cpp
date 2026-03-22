#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "PlayerVisualProfile.h"

#include <cmath>

#include "../../Utility/SimpleMathEx.h"

using DirectX::SimpleMath::Color;

namespace
{
    float Clamp01(float value)
    {
        return Utility::MathEx::Clamp(value, 0.0f, 1.0f);
    }
}

namespace SceneFx
{
    PlayerVisualProfile BuildPlayerVisualProfile(const Action::PlayerState& player, float sceneTime)
    {
        PlayerVisualProfile profile;

        const float pulse = std::sin(sceneTime * 2.4f) * 0.5f + 0.5f;
        const float guardTint = player.guarding ? 0.18f : 0.0f; // ガード時の強めの色付け

        // ベースアーマー色はやや彩度を下げて、ハイライトで強調
        profile.armorDark = Color(
            Clamp01(0.15f + pulse * 0.02f),
            Clamp01(0.20f + pulse * 0.02f),
            Clamp01(0.45f + pulse * 0.03f + guardTint),
            1.0f);
        profile.armorLight = Color(
            Clamp01(0.28f + pulse * 0.03f),
            Clamp01(0.40f + pulse * 0.03f),
            Clamp01(0.70f + pulse * 0.04f + guardTint),
            1.0f);
        profile.underColor = Color(0.04f, 0.06f, 0.10f, 1.0f);
        profile.accentColor = Color(0.72f, 0.86f, 0.98f, 1.0f);
        profile.trimColor = Color(0.10f, 0.14f, 0.22f, 1.0f);
        profile.emissiveColor = Color(
            Clamp01(0.25f + pulse * 0.08f + guardTint),
            Clamp01(0.80f + pulse * 0.06f),
            1.0f,
            1.0f);
        profile.weaponColor = Color(0.36f, 0.40f, 0.48f, 1.0f);
        return profile;
    }
}


