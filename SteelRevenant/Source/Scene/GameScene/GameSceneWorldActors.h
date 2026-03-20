#pragma once
#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>
#include "../../Action/CombatSystem.h"

// Bug#2修正: guardPose の bladePitch 符号修正 (左手系で正しく前方を向く)
class GameSceneWorldActors
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void RenderPlayer(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const Action::PlayerState& player,
        bool isHitFlash, float hitFlashT);
    void RenderEnemies(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const std::vector<Action::EnemyState>& enemies,
        int lockTargetIndex);
    void RenderLockOnRing(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& targetPos,
        float ringPulse);

private:
    void DrawSword(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Matrix& bodyWorld,
        const Action::PlayerState& player);

    std::unique_ptr<DirectX::GeometricPrimitive> m_box;
    std::unique_ptr<DirectX::GeometricPrimitive> m_sphere;
    std::unique_ptr<DirectX::GeometricPrimitive> m_cylinder;
    std::unique_ptr<DirectX::GeometricPrimitive> m_torus;
    std::unique_ptr<DirectX::GeometricPrimitive> m_capsule;
};
