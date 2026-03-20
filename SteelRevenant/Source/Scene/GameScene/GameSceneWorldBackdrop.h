#pragma once
#include <d3d11.h>
#include <memory>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

// スカイドーム描画を担当するクラス。
// Bug#1修正: 5層それぞれが GameSceneVisualPalette の異なる色を参照する。
class GameSceneWorldBackdrop
{
public:
    GameSceneWorldBackdrop() = default;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    void Render(
        ID3D11DeviceContext*                    context,
        const DirectX::SimpleMath::Matrix&      view,
        const DirectX::SimpleMath::Matrix&      projection,
        const DirectX::SimpleMath::Vector3&     cameraPos);

private:
    // 1層分の描画データ。
    struct SkyLayer
    {
        std::unique_ptr<DirectX::GeometricPrimitive> sphere;
        DirectX::SimpleMath::Color                   color;
        float                                        scale;
        float                                        yOffset;
    };

    static constexpr int kLayerCount = 5;
    SkyLayer m_layers[kLayerCount];
};
