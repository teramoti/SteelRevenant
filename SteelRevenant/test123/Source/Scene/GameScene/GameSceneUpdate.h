#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdlib>
#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

#include "../../Action/CombatSystem.h"
#include "../../Action/GameState.h"

enum class DropItemType
{
    Health
};

struct DropItem
{
    DropItemType type;
    DirectX::SimpleMath::Vector3 position;
    bool active = true;
    float bobTimer = 0.0f;
    float lifetime = 15.0f;
};

class GameSceneUpdate
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void ProcessEnemyDrops(
        const std::vector<Action::EnemyState>& enemies,
        std::vector<Action::EnemyStateType>& prevStates);
    bool TrySpawnHealthDrop(const DirectX::SimpleMath::Vector3& pos);
    void UpdateDropItems(Action::PlayerState& player, Action::GameState& gameState, float dt, std::vector<DirectX::SimpleMath::Vector3>& outPicked);
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection);
    void Reset();

private:
    void SpawnDrop(const DirectX::SimpleMath::Vector3& pos, DropItemType type);

    std::vector<DropItem> m_items;
    std::unique_ptr<DirectX::GeometricPrimitive> m_itemPrimitive;

    static constexpr float kPickupRadius = 1.5f;
    static constexpr float kHealthRestore = 25.0f;
    static constexpr float kDropChance = 0.30f;
};

