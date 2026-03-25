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

        // ソラ風を意識した濃紺ベースの人型配色に整理
        profile.armorDark = Color(
            Clamp01(0.08f + pulse * 0.01f),
            Clamp01(0.10f + pulse * 0.01f),
            Clamp01(0.16f + pulse * 0.02f + guardTint * 0.5f),
            1.0f);
        profile.armorLight = Color(
            Clamp01(0.18f + pulse * 0.02f),
            Clamp01(0.20f + pulse * 0.02f),
            Clamp01(0.30f + pulse * 0.03f + guardTint * 0.35f),
            1.0f);
        profile.underColor = Color(0.05f, 0.05f, 0.07f, 1.0f);
        profile.accentColor = Color(0.42f, 0.25f, 0.11f, 1.0f);
        profile.trimColor = Color(0.58f, 0.10f, 0.12f, 1.0f);
        profile.emissiveColor = Color(
            Clamp01(0.24f + pulse * 0.04f + guardTint * 0.4f),
            Clamp01(0.58f + pulse * 0.05f),
            Clamp01(0.96f + pulse * 0.02f),
            1.0f);
        profile.weaponColor = Color(0.36f, 0.40f, 0.48f, 1.0f);
        return profile;
    }
}


