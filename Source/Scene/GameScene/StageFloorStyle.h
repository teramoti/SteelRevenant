//------------------------//------------------------
// Contents(処理内容) ステージ床テーマの描画スタイル生成を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include <memory>
#include <vector>

#include <SimpleMath.h>

namespace SceneFx
{
	// 床の基準カラーセット。
	struct FloorPalette
	{
		DirectX::SimpleMath::Color baseColor;
		DirectX::SimpleMath::Color topColor;
		DirectX::SimpleMath::Color panelColor;
		DirectX::SimpleMath::Color seamColor;
		DirectX::SimpleMath::Color accentColor;
	};

	// 床ラインのカラーセット。
	struct LanePalette
	{
		DirectX::SimpleMath::Color laneAColor;
		DirectX::SimpleMath::Color laneBColor;
	};

	// 床ディテール描画用コマンド。
	struct FloorDrawCommand
	{
		DirectX::SimpleMath::Matrix world;
		DirectX::SimpleMath::Color color;
	};

	// ステージ床テーマ切替 Strategy。
	class IStageFloorStyle
	{
	public:
		// 床スタイル派生を基底経由で安全に破棄する。
		virtual ~IStageFloorStyle() = default;

		// 毎フレームの床パレットを返す。
		virtual FloorPalette BuildFloorPalette(float floorPulse) const = 0;
		// 毎フレームの床ライン色を返す。
		virtual LanePalette BuildLanePalette(float linePulse) const = 0;
		// 床追加ディテールの描画コマンドを構築する。
		virtual void BuildDetailCommands(float sceneTime, std::vector<FloorDrawCommand>& outCommands) const = 0;
	};

	// ステージ番号に応じた床テーマ Strategy を生成する。
	class StageFloorStyleFactory
	{
	public:
		// ステージテーマ番号に応じた床スタイルを生成する。
		static std::unique_ptr<IStageFloorStyle> Create(int stageThemeIndex);
	};
}

