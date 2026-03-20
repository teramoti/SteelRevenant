#pragma once
#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>
#include "StageFloorStyle.h"
#include "../../Action/PathGrid.h"

// アリーナのジオメトリ（床・壁・柱）を管理し描画するクラス。
// Bug#4修正: 削除済みの中継地点・ハザード・ビーコン参照を除去済み。
class GameSceneWorldArena
{
public:
    GameSceneWorldArena() = default;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int stageIndex);

    void Render(
        ID3D11DeviceContext*                context,
        const DirectX::SimpleMath::Matrix&  view,
        const DirectX::SimpleMath::Matrix&  projection);

    // 衝突グリッドを PathGrid に反映する。
    void BuildPathGrid(Action::PathGrid& grid) const;

    // スポーンポイント一覧を返す。
    const std::vector<DirectX::SimpleMath::Vector3>& GetSpawnPoints() const { return m_spawnPoints; }

private:
    void BuildStage1();
    void BuildStage2();
    void BuildStage3();

    // 壁ボックス 1 件分のデータ。
    struct WallData
    {
        DirectX::SimpleMath::Matrix world;
        DirectX::SimpleMath::Color  color;
        // PathGrid 衝突用の AABB (XZ)
        float minX, minZ, maxX, maxZ;
    };

    std::unique_ptr<DirectX::GeometricPrimitive>  m_floorPrimitive;
    std::unique_ptr<DirectX::GeometricPrimitive>  m_boxPrimitive;
    std::unique_ptr<IFloorStyle>                  m_floorStyle;
    std::vector<FloorDrawCommand>                 m_floorDetailCmds;
    std::vector<WallData>                         m_walls;
    std::vector<DirectX::SimpleMath::Vector3>     m_spawnPoints;

    int   m_stageIndex   = 1;
    float m_floorHalfExt = 28.0f;
};
