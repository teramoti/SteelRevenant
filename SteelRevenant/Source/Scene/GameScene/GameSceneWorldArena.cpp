//------------------------//------------------------
// Contents(処理内容) アリーナの床・壁・スポーンポイントを実装する。
//------------------------//------------------------
// Bug#4修正: 中継地点・ハザード・ビーコンへの参照を完全除去。
//           アリーナはステージ純粋な戦闘空間として機能する。
//------------------------//------------------------
#include "GameSceneWorldArena.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Color;

// ステージに応じてアリーナを構築する。
void GameSceneWorldArena::Initialize(
    ID3D11Device*         device,
    ID3D11DeviceContext*  context,
    int                   stageIndex)
{
    m_stageIndex = stageIndex;
    m_floorStyle = CreateFloorStyle(stageIndex);

    m_floorPrimitive = DirectX::GeometricPrimitive::CreateBox(context, { 2.0f, 0.2f, 2.0f });
    m_boxPrimitive   = DirectX::GeometricPrimitive::CreateBox(context, { 1.0f, 1.0f, 1.0f });

    m_floorDetailCmds = m_floorStyle->BuildDetailCommands();

    switch (stageIndex)
    {
    case 2:  BuildStage2(); break;
    case 3:  BuildStage3(); break;
    default: BuildStage1(); break;
    }
    (void)device;
}

// Stage1: 外縁区画 - 正方形フロア＋外周壁＋4柱。
void GameSceneWorldArena::BuildStage1()
{
    m_floorHalfExt = 26.0f;
    const float W = 28.0f;
    const float H = 3.5f;
    const float T = 1.0f;

    auto addWall = [&](float cx, float cz, float sx, float sz)
    {
        WallData w;
        w.world = Matrix::CreateScale(sx, H, sz)
            * Matrix::CreateTranslation(cx, H * 0.5f, cz);
        w.color = wallColor;
        w.minX  = cx - sx * 0.5f; w.maxX = cx + sx * 0.5f;
        w.minZ  = cz - sz * 0.5f; w.maxZ = cz + sz * 0.5f;
        m_walls.push_back(w);
    };

    // 外周 4 壁
    addWall(  0.0f,  W, W*2.0f, T);
    addWall(  0.0f, -W, W*2.0f, T);
    addWall(  W, 0.0f,  T, W*2.0f);
    addWall( -W, 0.0f,  T, W*2.0f);

    // 四隅の柱
    for (int s = -1; s <= 1; s += 2)
        for (int t = -1; t <= 1; t += 2)
        {
            WallData p;
            const float px = static_cast<float>(s) * 20.0f;
            const float pz = static_cast<float>(t) * 20.0f;
            p.world = Matrix::CreateScale(2.0f, H * 2.0f, 2.0f)
                * Matrix::CreateTranslation(px, H, pz);
            p.color = pillarColor;
            p.minX = px - 1.0f; p.maxX = px + 1.0f;
            p.minZ = pz - 1.0f; p.maxZ = pz + 1.0f;
            m_walls.push_back(p);
        }

    // スポーンポイント (外周 8 点)
    const float sp = 22.0f;
    const std::initializer_list<Vector3> pts =
    {
        { sp, 0.8f, 0.0f}, {-sp, 0.8f, 0.0f},
        {0.0f, 0.8f,  sp}, {0.0f, 0.8f, -sp},
        { sp, 0.8f,  sp}, {-sp, 0.8f,  sp},
        { sp, 0.8f, -sp}, {-sp, 0.8f, -sp},
    };
    m_spawnPoints.assign(pts.begin(), pts.end());
}

// Stage2: 防衛回廊 - 中央通路＋側面障壁。
void GameSceneWorldArena::BuildStage2()
{
    m_floorHalfExt = 26.0f;
    const float W  = 28.0f;
    const float H  = 3.5f;
    const float T  = 1.0f;

    auto addWall = [&](float cx, float cz, float sx, float sz)
    {
        WallData w;
        w.world = Matrix::CreateScale(sx, H, sz)
            * Matrix::CreateTranslation(cx, H * 0.5f, cz);
        w.color = wallColor;
        w.minX = cx - sx * 0.5f; w.maxX = cx + sx * 0.5f;
        w.minZ = cz - sz * 0.5f; w.maxZ = cz + sz * 0.5f;
        m_walls.push_back(w);
    };

    // 外周
    addWall(0.0f,  W, W*2.0f, T);
    addWall(0.0f, -W, W*2.0f, T);
    addWall( W, 0.0f, T, W*2.0f);
    addWall(-W, 0.0f, T, W*2.0f);

    // 側面障壁 (3ペア)
    for (int side = -1; side <= 1; side += 2)
    {
        for (int row = -1; row <= 1; ++row)
        {
            const float bx = static_cast<float>(side) * 14.0f;
            const float bz = static_cast<float>(row)  * 10.0f;
            WallData b;
            b.world = Matrix::CreateScale(3.0f, H, 2.0f)
                * Matrix::CreateTranslation(bx, H * 0.5f, bz);
            b.color = wallAccentColor;
            b.minX = bx - 1.5f; b.maxX = bx + 1.5f;
            b.minZ = bz - 1.0f; b.maxZ = bz + 1.0f;
            m_walls.push_back(b);
        }
    }

    // スポーンポイント (奥端)
    const float sp = 24.0f;
    m_spawnPoints =
    {
        {sp, 0.8f, 0.0f}, {-sp, 0.8f, 0.0f},
        {sp, 0.8f,  8.0f}, {sp, 0.8f, -8.0f},
        {-sp, 0.8f, 8.0f}, {-sp, 0.8f, -8.0f},
        {0.0f, 0.8f,  sp}, {0.0f, 0.8f, -sp},
    };
}

// Stage3: 中央区中枢 - 円形コア＋四隅障害。
void GameSceneWorldArena::BuildStage3()
{
    m_floorHalfExt = 26.0f;
    const float W  = 28.0f;
    const float H  = 3.5f;
    const float T  = 1.0f;

    auto addWall = [&](float cx, float cz, float sx, float sz)
    {
        WallData w;
        w.world = Matrix::CreateScale(sx, H, sz)
            * Matrix::CreateTranslation(cx, H * 0.5f, cz);
        w.color = wallColor;
        w.minX = cx - sx * 0.5f; w.maxX = cx + sx * 0.5f;
        w.minZ = cz - sz * 0.5f; w.maxZ = cz + sz * 0.5f;
        m_walls.push_back(w);
    };

    addWall(0.0f,  W, W*2.0f, T);
    addWall(0.0f, -W, W*2.0f, T);
    addWall( W, 0.0f, T, W*2.0f);
    addWall(-W, 0.0f, T, W*2.0f);

    // 四隅 L 字障害
    for (int s = -1; s <= 1; s += 2)
        for (int t = -1; t <= 1; t += 2)
        {
            const float ox = static_cast<float>(s) * 18.0f;
            const float oz = static_cast<float>(t) * 18.0f;

            WallData wa;
            wa.world = Matrix::CreateScale(6.0f, H, 1.5f)
                * Matrix::CreateTranslation(ox, H * 0.5f, oz);
            wa.color = wallAccentColor;
            wa.minX = ox - 3.0f; wa.maxX = ox + 3.0f;
            wa.minZ = oz - 0.75f; wa.maxZ = oz + 0.75f;
            m_walls.push_back(wa);

            WallData wb;
            wb.world = Matrix::CreateScale(1.5f, H, 6.0f)
                * Matrix::CreateTranslation(ox, H * 0.5f, oz);
            wb.color = wallAccentColor;
            wb.minX = ox - 0.75f; wb.maxX = ox + 0.75f;
            wb.minZ = oz - 3.0f;  wb.maxZ = oz + 3.0f;
            m_walls.push_back(wb);
        }

    const float sp = 24.0f;
    m_spawnPoints =
    {
        {sp, 0.8f,  sp}, {-sp, 0.8f,  sp},
        {sp, 0.8f, -sp}, {-sp, 0.8f, -sp},
        {sp, 0.8f, 0.0f}, {-sp, 0.8f, 0.0f},
        {0.0f, 0.8f, sp}, {0.0f, 0.8f, -sp},
    };
}

// 壁の AABB を PathGrid に反映する。
void GameSceneWorldArena::BuildPathGrid(Action::PathGrid& grid) const
{
    for (const WallData& w : m_walls)
    {
        grid.SetBlockedRect(w.minX, w.minZ, w.maxX, w.maxZ, true);
    }
}

// 床・床ディテール・壁を描画する。
void GameSceneWorldArena::Render(
    ID3D11DeviceContext*  context,
    const Matrix&         view,
    const Matrix&         projection)
{
    System::DrawManager::GetInstance().ApplyPrimitiveState();

    // メインフロア
    const float ext = m_floorHalfExt;
    const Matrix floorWorld = Matrix::CreateScale(ext * 2.0f, 0.2f, ext * 2.0f);
    if (m_floorPrimitive)
        m_floorPrimitive->Draw(floorWorld, view, projection, m_floorStyle->GetBaseColor());

    // 床ディテール (継ぎ目・紋様)
    if (m_boxPrimitive)
    {
        for (const FloorDrawCommand& cmd : m_floorDetailCmds)
            m_boxPrimitive->Draw(cmd.world, view, projection, cmd.color);
    }

    // 壁・柱
    if (m_boxPrimitive)
    {
        for (const WallData& w : m_walls)
            m_boxPrimitive->Draw(w.world, view, projection, w.color);
    }
}
