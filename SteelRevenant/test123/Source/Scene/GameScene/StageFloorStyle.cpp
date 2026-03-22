#ifndef NOMINMAX
#define NOMINMAX
#endif
//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 繧ｹ繝・・繧ｸ蛻･縺ｮ蠎翫せ繧ｿ繧､繝ｫ繧貞ｮ溯｣・☆繧九・// Bug#3菫ｮ豁｣: CorridorFloorStyle 縺ｮ seamColor 繧・//           Color(0.92f, 0.68f, 0.28f) 繧ｪ繝ｬ繝ｳ繧ｸ
//           竊・Color(0.32f, 0.36f, 0.42f) 髱堤・ 縺ｫ螟画峩縲・//------------------------
#include "StageFloorStyle.h"
#include "GameSceneVisualPalette.h"
using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;

//--------------------------------------------------------------------
// PlatformFloorStyle (Stage1)
//--------------------------------------------------------------------
Color PlatformFloorStyle::GetBaseColor() const { return floorBaseColor; }

std::vector<FloorDrawCommand> PlatformFloorStyle::BuildDetailCommands() const
{
    std::vector<FloorDrawCommand> cmds;
    constexpr float kSz = 6.0f, kTh = 0.04f, kStep = 8.0f;
    constexpr int   kH  = 3;
    for (int iz = -kH; iz <= kH; ++iz)
        for (int ix = -kH; ix <= kH; ++ix)
        {
            FloorDrawCommand c;
            c.world = Matrix::CreateScale(kSz, kTh, kSz)
                * Matrix::CreateTranslation(static_cast<float>(ix)*kStep, 0.01f, static_cast<float>(iz)*kStep);
            c.color = floorSeamColor;
            cmds.push_back(c);
        }
    return cmds;
}

//--------------------------------------------------------------------
// CorridorFloorStyle (Stage2) 笏 Bug#3 菫ｮ豁｣貂医∩
//--------------------------------------------------------------------
Color CorridorFloorStyle::GetBaseColor() const { return Color(0.14f, 0.16f, 0.18f, 1.0f); }

std::vector<FloorDrawCommand> CorridorFloorStyle::BuildDetailCommands() const
{
    std::vector<FloorDrawCommand> cmds;
    // Bug#3菫ｮ豁｣: 繧ｪ繝ｬ繝ｳ繧ｸ(0.92,0.68,0.28) 竊・髱堤・(0.32,0.36,0.42)
    const Color seamColor(0.32f, 0.36f, 0.42f, 1.0f);
    constexpr float kW = 0.4f, kLen = 50.0f, kTh = 0.03f, kStep = 5.0f;
    constexpr int   kN = 5;
    for (int i = -kN; i <= kN; ++i)
    {
        FloorDrawCommand c;
        c.world = Matrix::CreateScale(kW, kTh, kLen)
            * Matrix::CreateTranslation(static_cast<float>(i)*kStep, 0.01f, 0.0f);
        c.color = seamColor; cmds.push_back(c);
        c.world = Matrix::CreateScale(kLen, kTh, kW)
            * Matrix::CreateTranslation(0.0f, 0.01f, static_cast<float>(i)*kStep);
        c.color = seamColor; cmds.push_back(c);
    }
    return cmds;
}

//--------------------------------------------------------------------
// CoreFloorStyle (Stage3)
//--------------------------------------------------------------------
Color CoreFloorStyle::GetBaseColor() const { return Color(0.10f, 0.12f, 0.16f, 1.0f); }

std::vector<FloorDrawCommand> CoreFloorStyle::BuildDetailCommands() const
{
    std::vector<FloorDrawCommand> cmds;
    constexpr int   kRings = 5, kSegs = 24;
    constexpr float kR0 = 4.0f, kRS = 4.5f, kRW = 0.35f, kTh = 0.03f;
    const float angle = DirectX::XM_2PI / static_cast<float>(kSegs);
    for (int r = 0; r < kRings; ++r)
    {
        const float radius = kR0 + static_cast<float>(r) * kRS;
        for (int s = 0; s < kSegs; ++s)
        {
            const float a0 = static_cast<float>(s)*angle, a1 = static_cast<float>(s+1)*angle;
            const float mx = std::cos((a0+a1)*0.5f)*radius, mz = std::sin((a0+a1)*0.5f)*radius;
            FloorDrawCommand c;
            c.world = Matrix::CreateScale(radius*angle*0.9f, kTh, kRW)
                * Matrix::CreateRotationY(-(a0+a1)*0.5f+DirectX::XM_PIDIV2)
                * Matrix::CreateTranslation(mx, 0.01f, mz);
            const float b = 0.30f + static_cast<float>(r)*0.06f;
            c.color = Color(b*0.7f, b*0.8f, b, 1.0f);
            cmds.push_back(c);
        }
    }
    return cmds;
}

std::unique_ptr<IFloorStyle> CreateFloorStyle(int stageIndex)
{
    switch (stageIndex)
    {
    case 2:  return std::make_unique<CorridorFloorStyle>();
    case 3:  return std::make_unique<CoreFloorStyle>();
    default: return std::make_unique<PlatformFloorStyle>();
    }
}

