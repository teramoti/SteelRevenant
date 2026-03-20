//------------------------//------------------------
// Contents(処理内容) 5層スカイドームの初期化と描画を実装する。
//------------------------//------------------------
// Bug#1修正: skyBaseColor のみ参照していた問題を解消。
//           各層が skyZenithColor / skyUpperColor / skyMidColor /
//           skyLowerColor / skyHorizonColor を個別に参照する。
//------------------------//------------------------
#include "GameSceneWorldBackdrop.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

// 球体を 5 つ生成し、天頂〜地平線の色を割り当てる。
void GameSceneWorldBackdrop::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    // Layer0 = 天頂 (最小球・最内側)
    m_layers[0].color   = skyZenithColor;
    m_layers[0].scale   = 500.0f;
    m_layers[0].yOffset = 80.0f;

    // Layer1 = 上空
    m_layers[1].color   = skyUpperColor;
    m_layers[1].scale   = 490.0f;
    m_layers[1].yOffset = 40.0f;

    // Layer2 = 中空 (地平線寄り)
    m_layers[2].color   = skyMidColor;
    m_layers[2].scale   = 480.0f;
    m_layers[2].yOffset = 0.0f;

    // Layer3 = 低空
    m_layers[3].color   = skyLowerColor;
    m_layers[3].scale   = 470.0f;
    m_layers[3].yOffset = -40.0f;

    // Layer4 = 地平線
    m_layers[4].color   = skyHorizonColor;
    m_layers[4].scale   = 460.0f;
    m_layers[4].yOffset = -80.0f;

    for (int i = 0; i < kLayerCount; ++i)
    {
        m_layers[i].sphere = DirectX::GeometricPrimitive::CreateSphere(context, 2.0f, 8, false);
    }
    (void)device;
}

// カメラ位置を中心にドームを描画する。
void GameSceneWorldBackdrop::Render(
    ID3D11DeviceContext*    context,
    const Matrix&           view,
    const Matrix&           projection,
    const Vector3&          cameraPos)
{
    System::DrawManager::GetInstance().ApplyPrimitiveState();

    for (int i = 0; i < kLayerCount; ++i)
    {
        const SkyLayer& layer = m_layers[i];
        if (!layer.sphere) continue;

        const Matrix world = Matrix::CreateScale(layer.scale)
            * Matrix::CreateTranslation(
                cameraPos.x,
                cameraPos.y + layer.yOffset,
                cameraPos.z);

        layer.sphere->Draw(world, view, projection, layer.color);
    }
}
