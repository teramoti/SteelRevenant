#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <SimpleMath.h>

#include "../../Action/CombatSystem.h"

namespace SceneFx
{
    struct PlayerVisualProfile
    {
        DirectX::SimpleMath::Color armorDark;
        DirectX::SimpleMath::Color armorLight;
        DirectX::SimpleMath::Color underColor;
        DirectX::SimpleMath::Color accentColor;
        DirectX::SimpleMath::Color trimColor;
        DirectX::SimpleMath::Color emissiveColor;
        DirectX::SimpleMath::Color weaponColor;
    };

    PlayerVisualProfile BuildPlayerVisualProfile(const Action::PlayerState& player, float sceneTime);
}

