//------------------------//------------------------
// Contents(処理内容) ステージ別の床スタイルを実装する。
//------------------------//------------------------
// Bug#3修正: CorridorFloorStyle の seamColor をオレンジ
//           Color(0.92f, 0.68f, 0.28f) から青灰 Color(0.32f, 0.36f, 0.42f) へ変更。
//------------------------//------------------------
#include "StageFloorStyle.h"
#include "GameSceneVisualPalette.h"

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;

//====================================================================
// PlatformFloorStyle (Stage1)
//====================================================================
Color PlatformFloorStyle::GetBaseColor() const
{
    return floorBaseColor;
}

std::vector<FloorDrawCommand> PlatformFloorStyle::BuildDetailCommands() const
{
    std::vector<FloorDrawCommand> cmds;

    // グリッド状の鋼板継ぎ目パネル (4x4 配置)
    constexpr float kPanelSize  = 6.0f;
    constexpr float kPanelThick = 0.04f;
    constexpr float kGridStep   = 8.0f;
    constexpr int   kGridHalf   = 3;

    for (int iz = -kGridHalf; iz <= kGridHalf; ++iz)
    {
        for (int ix = -kGridHalf; ix <= kGridHalf; ++ix)
        {
            FloorDrawCommand cmd;
            cmd.world = Matrix::CreateScale(kPanelSize, kPanelThick, kPanelSize)
                * Matrix::CreateTranslation(
                    static_cast<float>(ix) * kGridStep,
                    0.01f,
                    static_cast<float>(iz) * kGridStep);
            cmd.color = floorSeamColor;
            cmds.push_back(cmd);
        }
    }
    return cmds;
}

//====================================================================
// CorridorFloorStyle (Stage2)
// Bug#3: seamColor を青灰に修正
//====================================================================
Color CorridorFloorStyle::GetBaseColor() const
{
    // 回廊はやや暗め
    return Color(0.14f, 0.16f, 0.18f, 1.0f);
}

std::vector<FloorDrawCommand> CorridorFloorStyle::BuildDetailCommands() const
{
    std::vector<FloorDrawCommand> cmds;

    // Bug#3修正: オレンジ(0.92, 0.68, 0.28) → 青灰(0.32, 0.36, 0.42)
    const Color seamColor = Color(0.32f, 0.36f, 0.42f, 1.0f);

    // 縦方向の継ぎ目ライン
    constexpr float kLineWidth  = 0.4f;
    constexpr float kLineLength = 50.0f;
    constexpr float kLineThick  = 0.03f;
    constexpr float kStep       = 5.0f;
    constexpr int   kLines      = 5;

    for (int i = -kLines; i <= kLines; ++i)
    {
        // 縦ライン
        FloorDrawCommand c;
        c.world = Matrix::CreateScale(kLineWidth, kLineThick, kLineLength)
            * Matrix::CreateTranslation(static_cast<float>(i) * kStep, 0.01f, 0.0f);
        c.color = seamColor;
        cmds.push_back(c);

        // 横ライン
        c.world = Matrix::CreateScale(kLineLength, kLineThick, kLineWidth)
            * Matrix::CreateTranslation(0.0f, 0.01f, static_cast<float>(i) * kStep);
        c.color = seamColor;
        cmds.push_back(c);
    }
    return cmds;
}

//====================================================================
// CoreFloorStyle (Stage3)
//====================================================================
Color CoreFloorStyle::GetBaseColor() const
{
    return Color(0.10f, 0.12f, 0.16f, 1.0f);
}

std::vector<FloorDrawCommand> CoreFloorStyle::BuildDetailCommands() const
{
    std::vector<FloorDrawCommand> cmds;

    // 同心円状の紋様 (5 リング)
    constexpr int   kRings      = 5;
    constexpr float kRingStartR = 4.0f;
    constexpr float kRingStep   = 4.5f;
    constexpr float kRingWidth  = 0.35f;
    constexpr float kRingThick  = 0.03f;
    constexpr int   kSegments   = 24;

    for (int ring = 0; ring < kRings; ++ring)
    {
        const float radius = kRingStartR + static_cast<float>(ring) * kRingStep;
        const float angle  = DirectX::XM_2PI / static_cast<float>(kSegments);

        for (int seg = 0; seg < kSegments; ++seg)
        {
            const float a0 = static_cast<float>(seg)     * angle;
            const float a1 = static_cast<float>(seg + 1) * angle;
            const float mx = std::cos((a0 + a1) * 0.5f) * radius;
            const float mz = std::sin((a0 + a1) * 0.5f) * radius;
            const float arcLen = radius * angle;

            FloorDrawCommand c;
            c.world = Matrix::CreateScale(arcLen * 0.9f, kRingThick, kRingWidth)
                * Matrix::CreateRotationY(-(a0 + a1) * 0.5f + DirectX::XM_PIDIV2)
                * Matrix::CreateTranslation(mx, 0.01f, mz);

            // リングごとに明度を変える
            const float bright = 0.30f + static_cast<float>(ring) * 0.06f;
            c.color = Color(bright * 0.7f, bright * 0.8f, bright, 1.0f);
            cmds.push_back(c);
        }
    }
    return cmds;
}

//====================================================================
// ファクトリ関数
//====================================================================
std::unique_ptr<IFloorStyle> CreateFloorStyle(int stageIndex)
{
    switch (stageIndex)
    {
    case 1:  return std::make_unique<PlatformFloorStyle>();
    case 2:  return std::make_unique<CorridorFloorStyle>();
    case 3:  return std::make_unique<CoreFloorStyle>();
    default: return std::make_unique<PlatformFloorStyle>();
    }
}
