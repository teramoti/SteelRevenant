#pragma once
#include <d3d11.h>
#include <vector>
#include <memory>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>
#include "../../Action/CombatSystem.h"

struct SpeedUpItem { DirectX::SimpleMath::Vector3 position; bool active=true; float bobTimer=0.0f; };

// Bug#5修正: SetTuning を毎フレーム呼ばず取得時・効果切れ時のみ呼ぶ。
class GameSceneArenaLayer
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int stageIndex);
    void UpdateSpeedUpItems(Action::PlayerState& player, Action::CombatSystem& combatSystem, float dt);
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection);
    void Reset(int stageIndex);
    bool  IsSpeedActive()    const { return m_speedActive; }
    float GetSpeedRemaining()const { return m_speedTimer; }

private:
    std::vector<SpeedUpItem>                     m_items;
    std::unique_ptr<DirectX::GeometricPrimitive> m_itemPrimitive;
    bool  m_speedActive   = false;
    float m_speedTimer    = 0.0f;
    float m_baseWalkSpeed = 0.0f;
    static constexpr float kPickupRadius     = 1.5f;
    static constexpr float kSpeedBoostFactor = 1.6f;
    static constexpr float kSpeedDuration    = 5.0f;
};
