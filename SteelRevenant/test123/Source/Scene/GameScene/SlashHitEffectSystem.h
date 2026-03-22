#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <array>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>

namespace SceneFx
{
    class SlashHitEffectSystem
    {
    public:
        void Reset();
        void Spawn(
            const DirectX::SimpleMath::Vector3& position,
            float baseYaw,
            const DirectX::SimpleMath::Color& tint,
            bool killBurst = false);
        void SpawnKill(
            const DirectX::SimpleMath::Vector3& position,
            float baseYaw,
            const DirectX::SimpleMath::Color& tint);
        void Update(float dt);
        void Draw(
            DirectX::GeometricPrimitive& streakMesh,
            DirectX::GeometricPrimitive& particleMesh,
            const DirectX::SimpleMath::Matrix& view,
            const DirectX::SimpleMath::Matrix& proj) const;

    private:
        struct SlashBurst
        {
            DirectX::SimpleMath::Vector3 position;
            float ageSec;
            float lifetimeSec;
            float yaw;
            float seed;
            DirectX::SimpleMath::Color tint;
            bool killBurst;
            bool active;
        };

        static constexpr size_t kMaxBursts = 8;
        std::array<SlashBurst, kMaxBursts> m_bursts{};
    };
}

