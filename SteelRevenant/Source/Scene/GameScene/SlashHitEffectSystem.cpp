#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "SlashHitEffectSystem.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace SceneFx
{

    void SlashHitEffectSystem::Reset()
    {
        for (SlashBurst& burst : m_bursts)
        {
            burst = {};
            burst.active = false;
        }
    }

    void SlashHitEffectSystem::Spawn(const Vector3& position, float baseYaw, const Color& tint, bool killBurst)
    {
        size_t slotIndex = kMaxBursts;
        for (size_t i = 0; i < m_bursts.size(); ++i)
        {
            if (!m_bursts[i].active)
            {
                slotIndex = i;
                break;
            }
        }

        if (slotIndex == kMaxBursts)
        {
            float oldestAge = -1.0f;
            for (size_t i = 0; i < m_bursts.size(); ++i)
            {
                if (m_bursts[i].ageSec > oldestAge)
                {
                    oldestAge = m_bursts[i].ageSec;
                    slotIndex = i;
                }
            }
        }

        SlashBurst& burst = m_bursts[slotIndex];
        burst = {};
        burst.position = position + Vector3(0.0f, killBurst ? 1.20f : 1.00f, 0.0f);
        burst.ageSec = 0.0f;
        burst.lifetimeSec = killBurst ? 0.18f : 0.10f;
        burst.yaw = baseYaw;
        burst.seed = baseYaw * 1.618f + position.x * 0.37f + position.z * 0.23f;
        burst.tint = tint;
        burst.killBurst = killBurst;
        burst.active = true;
    }

    void SlashHitEffectSystem::SpawnKill(const Vector3& position, float baseYaw, const Color& tint)
    {
        Spawn(position, baseYaw, tint, true);
    }

    void SlashHitEffectSystem::Update(float dt)
    {
        for (SlashBurst& burst : m_bursts)
        {
            if (!burst.active)
            {
                continue;
            }

            burst.ageSec += dt;
            if (burst.ageSec >= burst.lifetimeSec)
            {
                burst.active = false;
            }
        }
    }

    void SlashHitEffectSystem::Draw(
        DirectX::GeometricPrimitive& streakMesh,
        DirectX::GeometricPrimitive& particleMesh,
        const Matrix& view,
        const Matrix& proj) const
    {
        for (const SlashBurst& burst : m_bursts)
        {
            if (!burst.active)
            {
                continue;
            }
            const float safeLifetime = std::max(0.001f, burst.lifetimeSec);
            const float lifeT = std::clamp(burst.ageSec / safeLifetime, 0.0f, 1.0f);
            const float fade = 1.0f - lifeT;

            const Vector3 forward(std::sin(burst.yaw), 0.0f, std::cos(burst.yaw));
            const Vector3 right(forward.z, 0.0f, -forward.x);
            const Vector3 origin = burst.position + forward * 0.08f + Vector3(0.0f, 0.04f, 0.0f);

            const float slashLen = burst.killBurst
                ? (1.48f - lifeT * 0.18f)
                : (1.12f - lifeT * 0.10f);

            const float slashWidth = burst.killBurst
                ? (0.18f + fade * 0.05f)
                : (0.13f + fade * 0.03f);

            const Color slashColor(
                std::clamp(burst.tint.R() * 1.05f, 0.0f, 1.0f),
                std::clamp(burst.tint.G() * 1.05f, 0.0f, 1.0f),
                std::clamp(burst.tint.B() * 1.05f, 0.0f, 1.0f),
                burst.killBurst ? (0.30f + fade * 0.40f) : (0.24f + fade * 0.32f));

            const Color flashColor(
                std::clamp(burst.tint.R() * 1.18f + 0.12f, 0.0f, 1.0f),
                std::clamp(burst.tint.G() * 1.18f + 0.12f, 0.0f, 1.0f),
                std::clamp(burst.tint.B() * 1.18f + 0.12f, 0.0f, 1.0f),
                burst.killBurst ? (0.22f + fade * 0.34f) : (0.18f + fade * 0.24f));

            const Matrix slashWorld =
                Matrix::CreateScale(slashWidth, 0.026f, slashLen) *
                Matrix::CreateRotationZ(0.58f) *
                Matrix::CreateRotationX(-0.16f) *
                Matrix::CreateRotationY(burst.yaw) *
                Matrix::CreateTranslation(origin);
            streakMesh.Draw(slashWorld, view, proj, slashColor);

            const float flashScale = burst.killBurst
                ? (0.28f + fade * 0.12f)
                : (0.20f + fade * 0.08f);
            const Matrix flashWorld =
                Matrix::CreateScale(flashScale, flashScale * 0.52f, flashScale) *
                Matrix::CreateTranslation(origin + right * 0.04f);
            particleMesh.Draw(flashWorld, view, proj, flashColor);

            if (burst.killBurst)
            {
                const float ringScale = 0.36f + lifeT * 0.70f;
                const Matrix ringWorld =
                    Matrix::CreateScale(ringScale, 0.026f, ringScale) *
                    Matrix::CreateTranslation(origin + Vector3(0.0f, 0.03f, 0.0f));
                particleMesh.Draw(
                    ringWorld,
                    view,
                    proj,
                    Color(
                        std::clamp(burst.tint.R() * 1.08f, 0.0f, 1.0f),
                        std::clamp(burst.tint.G() * 1.08f, 0.0f, 1.0f),
                        std::clamp(burst.tint.B() * 1.08f, 0.0f, 1.0f),
                        0.10f + fade * 0.18f));
            }
        }
    }
}