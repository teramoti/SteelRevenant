#pragma once
#include <d3d11.h>
#include <memory>
#include <vector>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

// 床描画コマンド 1 件分。
struct FloorDrawCommand
{
    DirectX::SimpleMath::Matrix    world;
    DirectX::SimpleMath::Color     color;
};

// ステージ番号ごとの床スタイルを生成するファクトリ基底。
class IFloorStyle
{
public:
    virtual ~IFloorStyle() = default;

    // 床描画コマンドリストを構築する。
    virtual std::vector<FloorDrawCommand> BuildDetailCommands() const = 0;

    // メインフロアの色。
    virtual DirectX::SimpleMath::Color GetBaseColor() const = 0;
};

// Stage1: 外縁区画 ─ 暗い鋼板床。
class PlatformFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color    GetBaseColor()        const override;
};

// Stage2: 防衛回廊 ─ 青灰タイル床。
// Bug#3修正: seamColor を 0.92/0.68/0.28 (オレンジ) から青灰へ変更。
class CorridorFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color    GetBaseColor()        const override;
};

// Stage3: 中央区中枢 ─ 円形紋様床。
class CoreFloorStyle : public IFloorStyle
{
public:
    std::vector<FloorDrawCommand> BuildDetailCommands() const override;
    DirectX::SimpleMath::Color    GetBaseColor()        const override;
};

// ステージ番号から IFloorStyle を生成するファクトリ関数。
std::unique_ptr<IFloorStyle> CreateFloorStyle(int stageIndex);
