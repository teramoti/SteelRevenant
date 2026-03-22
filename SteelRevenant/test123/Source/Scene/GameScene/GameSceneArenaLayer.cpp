#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GameSceneArenaLayer.h"
#include "GameSceneVisualPalette.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "../../GameSystem/DrawManager.h"

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void GameSceneArenaLayer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int stageIndex)
{
    m_itemPrimitive = DirectX::GeometricPrimitive::CreateOctahedron(context, 0.7f);
    Reset(stageIndex);
    (void)device;
}

void GameSceneArenaLayer::Reset(int stageIndex)
{
    m_items.clear();
    m_speedActive = false;
    m_speedTimer = 0.0f;
    m_baseWalkSpeed = 0.0f;
    (void)stageIndex;
}

void GameSceneArenaLayer::UpdateSpeedUpItems(
    Action::PlayerState& player,
    Action::CombatSystem& combatSystem,
    float dt,
    std::vector<DirectX::SimpleMath::Vector3>& outPicked)
{
    for (SpeedUpItem& item : m_items)
    {
        if (!item.active)
        {
            continue;
        }

        item.bobTimer += dt;
        item.lifetime -= dt;
        if (item.lifetime <= 0.0f)
        {
            item.active = false;
            item.lifetime = 0.0f;
        }
    }

    if (m_speedActive)
    {
        m_speedTimer -= dt;
        if (m_speedTimer <= 0.0f)
        {
            m_speedTimer = 0.0f;
            m_speedActive = false;
            Action::CombatTuning tuning = combatSystem.GetTuning();
            tuning.walkSpeed = m_baseWalkSpeed;
            combatSystem.SetTuning(tuning);
        }
        m_items.erase(
            std::remove_if(
                m_items.begin(),
                m_items.end(),
                [](const SpeedUpItem& item)
                {
                    return !item.active && item.lifetime <= 0.0f;
                }),
            m_items.end());
        return;
    }

    for (SpeedUpItem& item : m_items)
    {
        if (!item.active)
        {
            continue;
        }

        const float dx = player.position.x - item.position.x;
        const float dz = player.position.z - item.position.z;
        if (dx * dx + dz * dz > kPickupRadius * kPickupRadius)
        {
            continue;
        }

        item.active = false;
        item.lifetime = 0.0f;
        if (!m_speedActive)
        {
            m_baseWalkSpeed = combatSystem.GetTuning().walkSpeed;
        }
        m_speedActive = true;
        m_speedTimer = kSpeedDuration;

        Action::CombatTuning tuning = combatSystem.GetTuning();
        tuning.walkSpeed = m_baseWalkSpeed * kSpeedBoostFactor;
        combatSystem.SetTuning(tuning);
        outPicked.push_back(item.position);
        break;
    }

    m_items.erase(
        std::remove_if(
            m_items.begin(),
            m_items.end(),
            [](const SpeedUpItem& item)
            {
                return !item.active && item.lifetime <= 0.0f;
            }),
        m_items.end());
}

bool GameSceneArenaLayer::TrySpawnSpeedDrop(const Vector3& enemyPosition)
{
    if ((std::rand() % 100) >= static_cast<int>(kDropChance * 100.0f))
    {
        return false;
    }

    const int serial = static_cast<int>(m_items.size());
    const float offsetX = static_cast<float>((serial % 3) - 1) * 0.45f;
    const float offsetZ = static_cast<float>(((serial + 1) % 3) - 1) * 0.45f;
    SpeedUpItem item;
    item.position = enemyPosition + Vector3(offsetX, 0.0f, offsetZ);
    item.active = true;
    item.bobTimer = static_cast<float>(serial) * 0.27f;
    item.lifetime = kDropLifetime;
    m_items.push_back(item);
    return true;
}

void GameSceneArenaLayer::Render(ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj)
{
    if (!m_itemPrimitive)
    {
        return;
    }

    System::DrawManager::GetInstance().ApplyAlphaBlendState();
    for (const SpeedUpItem& item : m_items)
    {
        if (!item.active)
        {
            continue;
        }

        const float bob = 0.2f * std::sin(item.bobTimer * 2.5f);
        const float spin = item.bobTimer * 1.8f;
        const float pulse = 0.85f + 0.15f * std::sin(item.bobTimer * 4.0f);
        const float alpha = (item.lifetime < 2.0f) ? (item.lifetime / 2.0f) : 0.90f;
        const Matrix world =
            Matrix::CreateScale(pulse) *
            Matrix::CreateRotationY(spin) *
            Matrix::CreateTranslation(item.position.x, item.position.y + bob, item.position.z);

        m_itemPrimitive->Draw(
            world,
            view,
            proj,
            DirectX::SimpleMath::Color(speedItemColor.R(), speedItemColor.G(), speedItemColor.B(), alpha));
    }
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    (void)context;
}

