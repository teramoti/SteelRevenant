//------------------------//------------------------
// Contents(処理内容) ステージ床テーマの描画スタイル生成を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "StageFloorStyle.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;

namespace
{
	// 床面上に薄い矩形パネルを追加する。
	void AddPanel(
		std::vector<SceneFx::FloorDrawCommand>& outCommands,
		float width,
		float depth,
		float y,
		const DirectX::SimpleMath::Vector3& position,
		const Color& color,
		float yaw = 0.0f)
	{
		SceneFx::FloorDrawCommand command;
		command.world =
			Matrix::CreateScale(width, 0.035f, depth) *
			Matrix::CreateRotationY(yaw) *
			Matrix::CreateTranslation(position.x, y, position.z);
		command.color = color;
		outCommands.push_back(command);
	}

	// Stage1: 外縁区画テーマ。
	class OuterDeckFloorStyle final : public SceneFx::IStageFloorStyle
	{
	public:
		// 毎フレームの床パレットを返す。
		SceneFx::FloorPalette BuildFloorPalette(float floorPulse) const override
		{
			SceneFx::FloorPalette palette;
			palette.baseColor = Color(0.07f + floorPulse * 0.01f, 0.09f + floorPulse * 0.01f, 0.11f + floorPulse * 0.02f, 1.0f);
			palette.topColor = Color(0.12f, 0.14f + floorPulse * 0.015f, 0.17f + floorPulse * 0.02f, 0.96f);
			palette.panelColor = Color(0.16f, 0.18f + floorPulse * 0.02f, 0.22f + floorPulse * 0.02f, 0.88f);
			palette.seamColor = Color(0.32f, 0.78f, 0.92f, 0.32f);
			palette.accentColor = Color(0.74f, 0.86f, 0.98f, 0.88f);
			return palette;
		}

		// 毎フレームの床ライン色を返す。
		SceneFx::LanePalette BuildLanePalette(float linePulse) const override
		{
			SceneFx::LanePalette palette;
			palette.laneAColor = Color(0.30f, 0.40f + linePulse * 0.05f, 0.20f, 0.18f);
			palette.laneBColor = Color(0.30f, 0.24f + linePulse * 0.04f, 0.16f, 0.14f);
			return palette;
		}

		// 床追加ディテールの描画コマンドを構築する。
		void BuildDetailCommands(float sceneTime, std::vector<SceneFx::FloorDrawCommand>& outCommands) const override
		{
			const float pulse = std::sinf(sceneTime * 0.9f) * 0.5f + 0.5f;
			const Color frameColor(0.10f, 0.12f + pulse * 0.01f, 0.15f + pulse * 0.015f, 0.42f);
			const Color panelColor(0.16f, 0.18f + pulse * 0.015f, 0.22f + pulse * 0.02f, 0.30f);
			const Color seamColor(0.38f, 0.82f, 0.96f, 0.18f);
			AddPanel(outCommands, 40.0f, 3.0f, 0.05f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, -21.0f), frameColor);
			AddPanel(outCommands, 40.0f, 3.0f, 0.05f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 21.0f), frameColor);
			AddPanel(outCommands, 3.0f, 40.0f, 0.05f, DirectX::SimpleMath::Vector3(-21.0f, 0.0f, 0.0f), frameColor);
			AddPanel(outCommands, 3.0f, 40.0f, 0.05f, DirectX::SimpleMath::Vector3(21.0f, 0.0f, 0.0f), frameColor);
			AddPanel(outCommands, 22.0f, 8.0f, 0.055f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), panelColor);
			AddPanel(outCommands, 10.0f, 18.0f, 0.055f, DirectX::SimpleMath::Vector3(-13.5f, 0.0f, 0.0f), panelColor);
			AddPanel(outCommands, 10.0f, 18.0f, 0.055f, DirectX::SimpleMath::Vector3(13.5f, 0.0f, 0.0f), panelColor);
			AddPanel(outCommands, 20.0f, 0.22f, 0.07f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), seamColor);
			AddPanel(outCommands, 0.22f, 18.0f, 0.07f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), seamColor);
		}
	};

	// Stage2: 防衛回廊テーマ。
	class CorridorFloorStyle final : public SceneFx::IStageFloorStyle
	{
	public:
		// 毎フレームの床パレットを返す。
		SceneFx::FloorPalette BuildFloorPalette(float floorPulse) const override
		{
			SceneFx::FloorPalette palette;
			palette.baseColor = Color(0.07f + floorPulse * 0.01f, 0.08f + floorPulse * 0.01f, 0.10f + floorPulse * 0.015f, 1.0f);
			palette.topColor = Color(0.12f, 0.13f + floorPulse * 0.01f, 0.15f + floorPulse * 0.015f, 0.96f);
			palette.panelColor = Color(0.18f, 0.17f + floorPulse * 0.015f, 0.16f + floorPulse * 0.012f, 0.88f);
			palette.seamColor = Color(0.92f, 0.68f, 0.28f, 0.30f);
			palette.accentColor = Color(0.78f, 0.84f, 0.94f, 0.86f);
			return palette;
		}

		// 毎フレームの床ライン色を返す。
		SceneFx::LanePalette BuildLanePalette(float linePulse) const override
		{
			SceneFx::LanePalette palette;
			palette.laneAColor = Color(0.58f, 0.44f + linePulse * 0.05f, 0.20f, 0.18f);
			palette.laneBColor = Color(0.24f, 0.38f + linePulse * 0.05f, 0.46f, 0.14f);
			return palette;
		}

		// 床追加ディテールの描画コマンドを構築する。
		void BuildDetailCommands(float sceneTime, std::vector<SceneFx::FloorDrawCommand>& outCommands) const override
		{
			const float pulse = std::sinf(sceneTime * 1.1f) * 0.5f + 0.5f;
			const Color frameColor(0.15f, 0.14f + pulse * 0.01f, 0.13f, 0.40f);
			const Color panelColor(0.20f, 0.18f + pulse * 0.015f, 0.17f, 0.30f);
			const Color seamColor(0.92f, 0.68f, 0.28f, 0.18f);
			AddPanel(outCommands, 32.0f, 6.0f, 0.055f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), panelColor);
			AddPanel(outCommands, 28.0f, 4.0f, 0.052f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, -12.5f), panelColor);
			AddPanel(outCommands, 28.0f, 4.0f, 0.052f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 12.5f), panelColor);
			AddPanel(outCommands, 8.0f, 24.0f, 0.052f, DirectX::SimpleMath::Vector3(-14.0f, 0.0f, 0.0f), frameColor);
			AddPanel(outCommands, 8.0f, 24.0f, 0.052f, DirectX::SimpleMath::Vector3(14.0f, 0.0f, 0.0f), frameColor);
			AddPanel(outCommands, 0.24f, 28.0f, 0.072f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), seamColor);
			AddPanel(outCommands, 18.0f, 0.18f, 0.072f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, -12.5f), seamColor);
			AddPanel(outCommands, 18.0f, 0.18f, 0.072f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 12.5f), seamColor);
		}
	};

	// Stage3: 中央区テーマ。
	class CentralFloorStyle final : public SceneFx::IStageFloorStyle
	{
	public:
		// 毎フレームの床パレットを返す。
		SceneFx::FloorPalette BuildFloorPalette(float floorPulse) const override
		{
			SceneFx::FloorPalette palette;
			palette.baseColor = Color(0.06f + floorPulse * 0.01f, 0.08f + floorPulse * 0.015f, 0.11f + floorPulse * 0.02f, 1.0f);
			palette.topColor = Color(0.10f, 0.14f + floorPulse * 0.02f, 0.19f + floorPulse * 0.03f, 0.96f);
			palette.panelColor = Color(0.14f, 0.18f + floorPulse * 0.02f, 0.24f + floorPulse * 0.025f, 0.88f);
			palette.seamColor = Color(0.36f, 0.88f, 0.98f, 0.32f);
			palette.accentColor = Color(0.72f, 0.86f, 1.0f, 0.88f);
			return palette;
		}

		// 毎フレームの床ライン色を返す。
		SceneFx::LanePalette BuildLanePalette(float linePulse) const override
		{
			SceneFx::LanePalette palette;
			palette.laneAColor = Color(0.18f, 0.42f + linePulse * 0.06f, 0.60f, 0.18f);
			palette.laneBColor = Color(0.44f, 0.24f + linePulse * 0.05f, 0.30f, 0.14f);
			return palette;
		}

		// 床追加ディテールの描画コマンドを構築する。
		void BuildDetailCommands(float sceneTime, std::vector<SceneFx::FloorDrawCommand>& outCommands) const override
		{
			const float pulse = std::sinf(sceneTime * 1.0f) * 0.5f + 0.5f;
			const Color frameColor(0.10f, 0.13f + pulse * 0.012f, 0.18f + pulse * 0.018f, 0.38f);
			const Color panelColor(0.14f, 0.18f + pulse * 0.02f, 0.24f + pulse * 0.025f, 0.30f);
			const Color seamColor(0.36f, 0.88f, 0.98f, 0.18f);
			AddPanel(outCommands, 16.0f, 16.0f, 0.055f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), frameColor, DirectX::XM_PIDIV4);
			AddPanel(outCommands, 10.0f, 10.0f, 0.065f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), panelColor, DirectX::XM_PIDIV4);
			AddPanel(outCommands, 7.0f, 20.0f, 0.052f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), panelColor);
			AddPanel(outCommands, 20.0f, 7.0f, 0.052f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), panelColor);
			AddPanel(outCommands, 0.22f, 18.0f, 0.074f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), seamColor, DirectX::XM_PIDIV4);
			AddPanel(outCommands, 18.0f, 0.22f, 0.074f, DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), seamColor, DirectX::XM_PIDIV4);
		}
	};

	// 想定外ステージ番号向けフォールバック。
	class FallbackFloorStyle final : public SceneFx::IStageFloorStyle
	{
	public:
		// 毎フレームの床パレットを返す。
		SceneFx::FloorPalette BuildFloorPalette(float floorPulse) const override
		{
			SceneFx::FloorPalette palette;
			palette.baseColor = Color(0.05f + floorPulse * 0.02f, 0.08f + floorPulse * 0.03f, 0.10f + floorPulse * 0.04f, 1.0f);
			palette.topColor = Color(0.09f, 0.14f + floorPulse * 0.04f, 0.18f + floorPulse * 0.05f, 0.96f);
			palette.panelColor = Color(0.14f, 0.18f + floorPulse * 0.04f, 0.22f + floorPulse * 0.05f, 0.84f);
			palette.seamColor = Color(0.44f, 0.86f, 0.98f, 0.28f);
			palette.accentColor = Color(0.78f, 0.88f, 1.0f, 0.88f);
			return palette;
		}

		// 毎フレームの床ライン色を返す。
		SceneFx::LanePalette BuildLanePalette(float linePulse) const override
		{
			SceneFx::LanePalette palette;
			palette.laneAColor = Color(0.14f, 0.66f + linePulse * 0.25f, 0.95f, 0.72f);
			palette.laneBColor = Color(0.95f, 0.34f + linePulse * 0.28f, 0.24f, 0.66f);
			return palette;
		}

		// 床追加ディテールの描画コマンドを構築する。
		void BuildDetailCommands(float, std::vector<SceneFx::FloorDrawCommand>&) const override
		{
		}
	};
}

namespace SceneFx
{
	// ステージテーマ番号に応じた床スタイルを生成する。
	std::unique_ptr<IStageFloorStyle> StageFloorStyleFactory::Create(int stageThemeIndex)
	{
		switch (stageThemeIndex)
		{
		case 1:
			return std::make_unique<OuterDeckFloorStyle>();
		case 2:
			return std::make_unique<CorridorFloorStyle>();
		case 3:
			return std::make_unique<CentralFloorStyle>();
		default:
			return std::make_unique<FallbackFloorStyle>();
		}
	}
}

