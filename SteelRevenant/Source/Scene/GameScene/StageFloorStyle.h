#pragma once
#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

struct FloorDrawCommand
{
    DirectX::SimpleMath::Matrix world;
    DirectX::SimpleMath::Color  color;
};

class IFloorStyle
{
public:
    virtual ~IFloorStyle() = default;
    virtual std::vector<FloorDrawCommand> BuildDetailCommands() const = 0;
    virtual DirectX::SimpleMath::Color    GetBaseColor()        const = 0;
};

// Stage1: 鋼板床
class PlatformFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color    GetBaseColor()        const override;
};

// Stage2: 青灰タイル床 (Bug#3修正: seamColor をオレンジから青灰に変更済み)
class CorridorFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color    GetBaseColor()        const override;
};

// Stage3: 円形紋様床
class CoreFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color    GetBaseColor()        const override;
};

std::unique_ptr<IFloorStyle> CreateFloorStyle(int stageIndex);
