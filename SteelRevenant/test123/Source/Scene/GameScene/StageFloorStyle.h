#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Visual/GameSceneVisualPalette.h"

#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

struct FloorDrawCommand
{
    DirectX::SimpleMath::Matrix world;
    DirectX::SimpleMath::Color color;
};

class IFloorStyle
{
public:
    virtual ~IFloorStyle() = default;

    virtual std::vector<FloorDrawCommand> BuildDetailCommands() const = 0;

    virtual void BuildDetailCommands(float sceneTime, std::vector<FloorDrawCommand>& out) const
    {
        out = BuildDetailCommands();
        (void)sceneTime;
    }

    virtual SceneFx::FloorPalette BuildFloorPalette(float floorPulse) const
    {
        SceneFx::FloorPalette palette;
        palette.baseColor = GetBaseColor();
        (void)floorPulse;
        return palette;
    }

    virtual DirectX::SimpleMath::Color GetBaseColor() const = 0;
};

class PlatformFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color GetBaseColor() const override;
};

class CorridorFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color GetBaseColor() const override;
};

class CoreFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color GetBaseColor() const override;
};

std::unique_ptr<IFloorStyle> CreateFloorStyle(int stageIndex);

