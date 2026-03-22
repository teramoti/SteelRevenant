#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstddef>

#include <SimpleMath.h>

#include "../../Action/CombatSystem.h"

namespace SceneFx
{
    struct EnemyVisualProfile
    {
        DirectX::SimpleMath::Color armorDark;
        DirectX::SimpleMath::Color armorLight;
        DirectX::SimpleMath::Color underColor;
        DirectX::SimpleMath::Color trimColor;
        DirectX::SimpleMath::Color emissiveColor;
        DirectX::SimpleMath::Color weaponColor;
    };

    EnemyVisualProfile BuildEnemyVisualProfile(
        const Action::EnemyState& enemy,
        int dangerLevel,
        size_t enemyIndex,
        float sceneTime);
}

