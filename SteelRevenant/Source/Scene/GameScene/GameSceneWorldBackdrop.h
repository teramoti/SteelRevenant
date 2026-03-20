#pragma once
#include <d3d11.h>
#include <memory>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

// Bug#1修正: 5層それぞれが異なる sky*Color を参照する。
class GameSceneWorldBackdrop
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPos);

private:
    struct SkyLayer
    {
        std::unique_ptr<DirectX::GeometricPrimitive> sphere;
        DirectX::SimpleMath::Color color;
        float scale, yOffset;
    };
    static constexpr int kLayerCount = 5;
    SkyLayer m_layers[kLayerCount];
};
