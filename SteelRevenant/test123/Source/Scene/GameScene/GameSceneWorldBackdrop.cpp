//------------------------//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 5螻､繧ｹ繧ｫ繧､繝峨・繝縺ｮ蛻晄悄蛹悶→謠冗判繧貞ｮ溯｣・☆繧九・// Bug#1菫ｮ豁｣: 蜷・ｱ､縺・skyBaseColor 縺ｮ縺ｿ蜿ら・縺励※縺・◆蝠城｡後ｒ隗｣豸医・//           Layer0=skyZenithColor, Layer1=skyUpperColor, Layer2=skyMidColor,
//           Layer3=skyLowerColor, Layer4=skyHorizonColor 繧貞句挨蜿ら・縺吶ｋ縲・//------------------------//------------------------
#include "GameSceneWorldBackdrop.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void GameSceneWorldBackdrop::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    // Bug#1菫ｮ豁｣: 蜷・ｱ､縺ｫ逡ｰ縺ｪ繧玖牡繧貞牡繧雁ｽ薙※繧・    const DirectX::SimpleMath::Color colors[kLayerCount] =
    { skyZenithColor, skyUpperColor, skyMidColor, skyLowerColor, skyHorizonColor };
    const float scales[kLayerCount]   = { 500.0f, 490.0f, 480.0f, 470.0f, 460.0f };
    const float offsets[kLayerCount]  = {  80.0f,  40.0f,   0.0f, -40.0f, -80.0f };

    for (int i = 0; i < kLayerCount; ++i)
    {
        m_layers[i].color   = colors[i];
        m_layers[i].scale   = scales[i];
        m_layers[i].yOffset = offsets[i];
        m_layers[i].sphere  = DirectX::GeometricPrimitive::CreateSphere(context, 2.0f, 8, false);
    }
    (void)device;
}

void GameSceneWorldBackdrop::Render(
    ID3D11DeviceContext* context,
    const Matrix& view, const Matrix& projection,
    const Vector3& cameraPos)
{
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    for (int i = 0; i < kLayerCount; ++i)
    {
        if (!m_layers[i].sphere) continue;
        const Matrix world = Matrix::CreateScale(m_layers[i].scale)
            * Matrix::CreateTranslation(cameraPos.x, cameraPos.y + m_layers[i].yOffset, cameraPos.z);
        m_layers[i].sphere->Draw(world, view, projection, m_layers[i].color);
    }
    (void)context;
}

