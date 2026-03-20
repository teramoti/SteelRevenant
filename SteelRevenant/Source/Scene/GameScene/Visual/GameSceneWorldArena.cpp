//------------------------//------------------------
// Contents(処理内容) 本編ゲームシーンの床・障害物・ギミック描画を分離実装する。
//------------------------//------------------------
#include "../GameScene.h"

#include <cmath>
#include <vector>

#include "GameSceneVisualPalette.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kFloorPulseSpeed = 1.6f;
	constexpr float kFloorSweepSpeed = 0.42f;
}

void GameScene::DrawWorldArena()
{
	const SceneFx::StageRenderPalette palette = SceneFx::BuildStageRenderPalette(m_stageThemeIndex, m_sceneTime);
	const int stageTheme = palette.stageTheme;
	const float stagePulse = palette.stagePulse;
	const auto StageTint = [&palette](const Color& base, float alphaScale = 1.0f) -> Color
	{
		return SceneFx::ApplyStageTint(palette, base, alphaScale);
	};
	const float floorPulse = std::sinf(m_sceneTime * kFloorPulseSpeed) * 0.5f + 0.5f;
	const Matrix floorWorld =
		Matrix::CreateScale(58.0f, 0.6f, 58.0f) *
		Matrix::CreateTranslation(0.0f, -0.35f, 0.0f);

	SceneFx::FloorPalette floorPalette;
	floorPalette.baseColor = Color(0.05f + floorPulse * 0.03f, 0.10f + floorPulse * 0.05f, 0.12f + floorPulse * 0.06f, 1.0f);
	floorPalette.topColor = Color(0.09f, 0.16f + floorPulse * 0.08f, 0.2f + floorPulse * 0.06f, 0.96f);
	if (m_floorStyle)
	{
		floorPalette = m_floorStyle->BuildFloorPalette(floorPulse);
	}

	m_floorMesh->Draw(floorWorld, m_view, m_proj, StageTint(floorPalette.baseColor));

	// 床面レイヤー1: ベースパネル
	m_floorMesh->Draw(
		Matrix::CreateScale(54.0f, 0.05f, 54.0f) * Matrix::CreateTranslation(0.0f, 0.02f, 0.0f),
		m_view, m_proj, StageTint(floorPalette.topColor));

	// 床面レイヤー2: 光沢パネル
	m_obstacleMesh->Draw(
		Matrix::CreateScale(50.0f, 0.025f, 50.0f) * Matrix::CreateTranslation(0.0f, 0.055f, 0.0f),
		m_view, m_proj, StageTint(floorPalette.panelColor));

	// グリッドライン (5m間隔 縦横各11本) - 奥行き感を生む
	const Color gridColor = StageTint(Color(
		floorPalette.seamColor.R(), floorPalette.seamColor.G(),
		floorPalette.seamColor.B(), 0.72f));
	const Color gridGlowColor = StageTint(Color(
		floorPalette.accentColor.R(), floorPalette.accentColor.G(),
		floorPalette.accentColor.B(), 0.45f + floorPulse * 0.20f));
	for (int gi = -5; gi <= 5; ++gi)
	{
		const float gf = static_cast<float>(gi) * 5.0f;
		m_obstacleMesh->Draw(Matrix::CreateScale(50.0f, 0.012f, 0.07f) * Matrix::CreateTranslation(0.0f, 0.068f, gf), m_view, m_proj, gridColor);
		m_obstacleMesh->Draw(Matrix::CreateScale(0.07f, 0.012f, 50.0f) * Matrix::CreateTranslation(gf, 0.068f, 0.0f), m_view, m_proj, gridColor);
	}
	// 中央十字アクセントライン
	m_obstacleMesh->Draw(Matrix::CreateScale(50.0f, 0.016f, 0.18f) * Matrix::CreateTranslation(0.0f, 0.072f, 0.0f), m_view, m_proj, gridGlowColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.18f, 0.016f, 50.0f) * Matrix::CreateTranslation(0.0f, 0.072f, 0.0f), m_view, m_proj, gridGlowColor);
	// 外周リング（アリーナ境界）
	const Color rimColor = StageTint(Color(floorPalette.accentColor.R(), floorPalette.accentColor.G(), floorPalette.accentColor.B(), 0.55f + floorPulse * 0.25f));
	m_obstacleMesh->Draw(Matrix::CreateScale(52.0f, 0.022f, 0.22f) * Matrix::CreateTranslation(0.0f, 0.075f, -26.0f), m_view, m_proj, rimColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(52.0f, 0.022f, 0.22f) * Matrix::CreateTranslation(0.0f, 0.075f,  26.0f), m_view, m_proj, rimColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.22f, 0.022f, 52.0f) * Matrix::CreateTranslation(-26.0f, 0.075f, 0.0f), m_view, m_proj, rimColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.22f, 0.022f, 52.0f) * Matrix::CreateTranslation( 26.0f, 0.075f, 0.0f), m_view, m_proj, rimColor);

	const float sweepOffset = std::sinf(m_sceneTime * kFloorSweepSpeed) * 18.0f;
	const Color sweepColor = StageTint(Color(
		floorPalette.accentColor.R(),
		floorPalette.accentColor.G(),
		floorPalette.accentColor.B(),
		0.10f + floorPulse * 0.08f));
	const Color glossColor = StageTint(Color(0.82f, 0.94f, 1.0f, 0.06f + floorPulse * 0.05f));
	m_obstacleMesh->Draw(
		Matrix::CreateScale(10.0f, 0.015f, 52.0f) * Matrix::CreateRotationY(0.16f) * Matrix::CreateTranslation(sweepOffset, 0.082f, 0.0f),
		m_view, m_proj, sweepColor);
	m_obstacleMesh->Draw(
		Matrix::CreateScale(6.0f, 0.012f, 48.0f) * Matrix::CreateRotationY(-0.22f) * Matrix::CreateTranslation(-sweepOffset * 0.75f, 0.083f, 0.0f),
		m_view, m_proj, glossColor);
	for (int panelIndex = -3; panelIndex <= 3; ++panelIndex)
	{
		const float xf = static_cast<float>(panelIndex) * 7.5f;
		m_obstacleMesh->Draw(
			Matrix::CreateScale(4.8f, 0.010f, 22.0f) * Matrix::CreateTranslation(xf, 0.080f, 0.0f),
			m_view, m_proj,
			StageTint(Color(floorPalette.panelColor.R() + 0.02f, floorPalette.panelColor.G() + 0.02f, floorPalette.panelColor.B() + 0.03f, 0.18f)));
	}

	std::vector<SceneFx::FloorDrawCommand> floorDetailCommands;
	floorDetailCommands.reserve(128);
	if (m_floorStyle)
	{
		m_floorStyle->BuildDetailCommands(m_sceneTime, floorDetailCommands);
	}
	for (size_t commandIndex = 0; commandIndex < floorDetailCommands.size(); ++commandIndex)
	{
		const SceneFx::FloorDrawCommand& command = floorDetailCommands[commandIndex];
		m_obstacleMesh->Draw(command.world, m_view, m_proj, StageTint(command.color));
	}





	auto DrawTheme2Strips = [&]()
	{
		for (int lane = 0; lane < 3; ++lane)
		{
			const float fl = static_cast<float>(lane);
			const float z = -14.0f + fl * 14.0f;
			const Matrix stripA =
				Matrix::CreateScale(18.0f, 0.022f, 1.2f) *
				Matrix::CreateTranslation(0.0f, 0.07f, z);
			const Matrix stripB =
				Matrix::CreateScale(18.0f, 0.016f, 0.16f) *
				Matrix::CreateTranslation(0.0f, 0.085f, z);
			m_obstacleMesh->Draw(stripA, m_view, m_proj, StageTint(Color(0.24f, 0.20f, 0.18f, 0.16f)));
			m_obstacleMesh->Draw(stripB, m_view, m_proj, StageTint(Color(0.78f, 0.60f, 0.20f, 0.10f)));
		}
	};

	auto DrawTheme3Arcs = [&]()
	{
		(void)stagePulse;
	};

	auto DrawTheme2Walls = [&]()
	{
	};

	auto DrawTheme3CenterDais = [&]()
	{
		const Matrix centerDais =
			Matrix::CreateScale(8.8f, 0.08f, 8.8f) *
			Matrix::CreateTranslation(0.0f, 0.05f, 0.0f);
		const Matrix centerTrim =
			Matrix::CreateScale(6.6f, 0.03f, 6.6f) *
			Matrix::CreateTranslation(0.0f, 0.09f, 0.0f);
		m_floorMesh->Draw(centerDais, m_view, m_proj, StageTint(Color(0.14f, 0.18f, 0.28f, 0.54f)));
		m_obstacleMesh->Draw(centerTrim, m_view, m_proj, StageTint(Color(0.30f, 0.72f, 0.82f, 0.10f)));
	};

	switch (stageTheme)
	{
	case 2:
		DrawTheme2Strips();
		DrawTheme2Walls();
		break;
	case 3:
		DrawTheme3Arcs();
		DrawTheme3CenterDais();
		break;
	default:
		break;
	}

	for (size_t i = 0; i < m_obstacleWorlds.size(); ++i)
	{
		const float fi = static_cast<float>(i);
		const float pulse = std::sinf(m_sceneTime * 0.7f + fi * 0.6f) * 0.5f + 0.5f;
		const Color obstacleColor(
			Utility::MathEx::Clamp(floorPalette.panelColor.x + 0.02f + pulse * 0.02f, 0.0f, 1.0f),
			Utility::MathEx::Clamp(floorPalette.panelColor.y + 0.02f + pulse * 0.02f, 0.0f, 1.0f),
			Utility::MathEx::Clamp(floorPalette.panelColor.z + 0.02f + pulse * 0.03f, 0.0f, 1.0f),
			1.0f);
		m_obstacleMesh->Draw(m_obstacleWorlds[i], m_view, m_proj, StageTint(obstacleColor));
		const Matrix capWorld =
			Matrix::CreateScale(m_obstacleWorlds[i]._11 * 0.92f, 0.03f, m_obstacleWorlds[i]._33 * 0.92f) *
			Matrix::CreateTranslation(m_obstacleWorlds[i]._41, m_obstacleWorlds[i]._42 + m_obstacleWorlds[i]._22 * 0.5f, m_obstacleWorlds[i]._43);
		m_obstacleMesh->Draw(capWorld, m_view, m_proj, StageTint(Color(floorPalette.accentColor.x, floorPalette.accentColor.y, floorPalette.accentColor.z, 0.10f)));
	}
}
