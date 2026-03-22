#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

#include "../../Action/CombatSystem.h"

struct SpeedUpItem
{
    DirectX::SimpleMath::Vector3 position;
    bool active = true;
    float bobTimer = 0.0f;
    float lifetime = 10.0f;
};

class GameSceneArenaLayer
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int stageIndex);
    void UpdateSpeedUpItems(
        Action::PlayerState& player,
        Action::CombatSystem& combatSystem,
        float dt,
        std::vector<DirectX::SimpleMath::Vector3>& outPicked);
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection);
    void Reset(int stageIndex);
    bool TrySpawnSpeedDrop(const DirectX::SimpleMath::Vector3& enemyPosition);

    bool IsSpeedActive() const { return m_speedActive; }
    float GetSpeedRemaining() const { return m_speedTimer; }

private:
    std::vector<SpeedUpItem> m_items;
    std::unique_ptr<DirectX::GeometricPrimitive> m_itemPrimitive;
    bool m_speedActive = false;
    float m_speedTimer = 0.0f;
    float m_baseWalkSpeed = 0.0f;

    static constexpr float kPickupRadius = 1.5f;
    static constexpr float kSpeedBoostFactor = 1.6f;
    static constexpr float kSpeedDuration = 5.0f;
    static constexpr float kDropLifetime = 10.0f;
    static constexpr float kDropChance = 0.16f;
};

