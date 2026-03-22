#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "EnemyVisualProfile.h"

#include <cmath>

#include "../../Utility/SimpleMathEx.h"

using DirectX::SimpleMath::Color;

namespace
{
    float Clamp01(float value)
    {
        return Utility::MathEx::Clamp(value, 0.0f, 1.0f);
    }

    Color GetArchetypeEmissive(Action::EnemyArchetype archetype)
    {
        switch (archetype)
        {
        case Action::EnemyArchetype::BladeRush:
            return Color(0.96f, 0.38f, 0.24f, 1.0f);
        case Action::EnemyArchetype::BladeFlank:
        default:
            return Color(1.0f, 0.82f, 0.26f, 1.0f);
        }
    }

    Color GetArchetypeArmorTint(Action::EnemyArchetype archetype)
    {
        switch (archetype)
        {
        case Action::EnemyArchetype::BladeRush:
            return Color(0.64f, 0.22f, 0.16f, 1.0f);
        case Action::EnemyArchetype::BladeFlank:
        default:
            return Color(0.50f, 0.30f, 0.12f, 1.0f);
        }
    }
}

namespace SceneFx
{
    EnemyVisualProfile BuildEnemyVisualProfile(
        const Action::EnemyState& enemy,
        int dangerLevel,
        size_t enemyIndex,
        float sceneTime)
    {
        EnemyVisualProfile profile;

        Color emissive = GetArchetypeEmissive(enemy.archetype);
        Color armorTint = GetArchetypeArmorTint(enemy.archetype);
        if (enemy.isLaserEnemy)
        {
            emissive = Color(0.18f, 0.95f, 1.00f, 1.0f);
            armorTint = Color(0.16f, 0.24f, 0.40f, 1.0f);
        }
        else if (enemy.isDashingEnemy)
        {
            emissive = Color(1.00f, 0.42f, 0.12f, 1.0f);
            armorTint = Color(0.58f, 0.18f, 0.10f, 1.0f);
        }
        else if (enemy.isHeavyEnemy)
        {
            emissive = Color(0.62f, 0.92f, 0.72f, 1.0f);
            armorTint = Color(0.38f, 0.42f, 0.46f, 1.0f);
        }
        const float dangerTint = Utility::MathEx::Clamp(
            0.96f + static_cast<float>(dangerLevel) * 0.035f,
            0.96f,
            1.14f);

        const float seed =
            static_cast<float>(enemyIndex) * 0.71f +
            enemy.spawnPosition.x * 0.047f +
            enemy.spawnPosition.z * 0.031f;
        const float jitterR = static_cast<float>(std::sin(seed * 2.63f)) * 0.03f;
        const float jitterG = static_cast<float>(std::sin(seed * 3.11f + 1.2f)) * 0.025f;
        const float jitterB = static_cast<float>(std::sin(seed * 2.27f + 2.3f)) * 0.03f;

        profile.armorDark = Color(
            Clamp01((armorTint.x * 0.34f + jitterR) * dangerTint),
            Clamp01((armorTint.y * 0.40f + jitterG) * dangerTint),
            Clamp01((armorTint.z * 0.46f + jitterB) * dangerTint),
            1.0f);
        profile.armorLight = Color(
            Clamp01((armorTint.x * 0.78f + 0.05f + jitterR) * dangerTint),
            Clamp01((armorTint.y * 0.78f + 0.06f + jitterG) * dangerTint),
            Clamp01((armorTint.z * 0.78f + 0.07f + jitterB) * dangerTint),
            1.0f);
        profile.underColor = Color(0.06f, 0.05f, 0.07f, 1.0f);
        profile.trimColor = Color(
            Clamp01(armorTint.x * 0.44f),
            Clamp01(armorTint.y * 0.44f),
            Clamp01(armorTint.z * 0.44f),
            1.0f);
        profile.emissiveColor = emissive;
        profile.weaponColor = enemy.isHeavyEnemy
            ? Color(0.24f, 0.26f, 0.30f, 1.0f)
            : Color(0.20f, 0.19f, 0.22f, 1.0f);

        if (enemy.state == Action::EnemyStateType::Attack)
        {
            const float pulse =
                static_cast<float>(std::sin(sceneTime * 11.0f + static_cast<float>(enemyIndex) * 0.4f)) * 0.5f + 0.5f;
            profile.emissiveColor.x = Clamp01(profile.emissiveColor.x + 0.06f + pulse * 0.08f);
            profile.emissiveColor.y = Clamp01(profile.emissiveColor.y + pulse * 0.03f);
        }
        else if (enemy.state == Action::EnemyStateType::Return)
        {
            profile.armorLight.z = Clamp01(profile.armorLight.z + 0.05f);
        }

        if (enemy.isLaserEnemy)
        {
            if (enemy.laserWarningActive || enemy.laserFireTimer > 0.0f)
            {
                const float beamPulse = static_cast<float>(std::sin(sceneTime * 18.0f + static_cast<float>(enemyIndex))) * 0.5f + 0.5f;
                profile.emissiveColor = Color(
                    Clamp01(0.32f + beamPulse * 0.40f),
                    Clamp01(0.92f + beamPulse * 0.06f),
                    1.0f,
                    1.0f);
                profile.trimColor = Color(0.26f, 0.84f, 1.0f, 1.0f);
            }
        }
        else if (enemy.isDashingEnemy)
        {
            if (enemy.dashActive || enemy.dashPreDelayTimer <= 0.25f)
            {
                profile.emissiveColor = Color(1.0f, 0.70f, 0.16f, 1.0f);
                profile.trimColor = Color(0.90f, 0.32f, 0.14f, 1.0f);
            }
        }
        else if (enemy.isHeavyEnemy)
        {
            profile.armorDark = Color(
                Clamp01(profile.armorDark.x + 0.04f),
                Clamp01(profile.armorDark.y + 0.04f),
                Clamp01(profile.armorDark.z + 0.05f),
                1.0f);
            profile.armorLight = Color(
                Clamp01(profile.armorLight.x + 0.06f),
                Clamp01(profile.armorLight.y + 0.06f),
                Clamp01(profile.armorLight.z + 0.07f),
                1.0f);
        }

        if (enemy.hitByCurrentSwing)
        {
            profile.armorLight = Color(0.96f, 0.20f, 0.16f, 1.0f);
            profile.emissiveColor = Color(1.0f, 0.42f, 0.28f, 1.0f);
        }

        return profile;
    }
}

