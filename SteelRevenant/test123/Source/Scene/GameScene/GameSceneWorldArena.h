#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

#include "StageFloorStyle.h"
#include "../../Action/PathGrid.h"

class GameSceneWorldArena
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int stageIndex);
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection);
    void BuildPathGrid(Action::PathGrid& grid) const;

    const std::vector<DirectX::SimpleMath::Vector3>& GetSpawnPoints() const
    {
        return m_spawnPoints;
    }

    const std::vector<DirectX::SimpleMath::Vector4> GetWallRects() const;

private:
    void BuildStage1();
    void BuildStage2();
    void BuildStage3();

    struct WallData
    {
        DirectX::SimpleMath::Matrix world;
        DirectX::SimpleMath::Color color;
        float minX;
        float minZ;
        float maxX;
        float maxZ;
    };

    std::unique_ptr<DirectX::GeometricPrimitive> m_floorPrimitive;
    std::unique_ptr<DirectX::GeometricPrimitive> m_boxPrimitive;
    std::unique_ptr<IFloorStyle> m_floorStyle;
    std::vector<FloorDrawCommand> m_floorDetailCmds;
    std::vector<WallData> m_walls;
    std::vector<DirectX::SimpleMath::Vector3> m_spawnPoints;
    float m_floorHalfExt = 28.0f;
};


