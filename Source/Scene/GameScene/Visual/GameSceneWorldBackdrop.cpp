//------------------------//------------------------
// Contents(処理内容) 本編ゲームシーンの背景・遠景描画を分離実装する。
//------------------------//------------------------
#include "../GameScene.h"

#include <cmath>

#include "GameSceneVisualPalette.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kSkyScrollSpeed = 0.08f;
}

void GameScene::DrawWorldBackdrop()
{
	const SceneFx::StageRenderPalette palette = SceneFx::BuildStageRenderPalette(m_stageThemeIndex, m_sceneTime);
	const auto StageTint = [&palette](const Color& base, float alphaScale = 1.0f) -> Color
	{
		return SceneFx::ApplyStageTint(palette, base, alphaScale);
	};
	const Color skyBaseColor = palette.skyBaseColor;
	const Color skyHazeColor = palette.skyHazeColor;
	const Color horizonGlowColor = palette.horizonGlowColor;
	const float skyPhase = m_sceneTime * 0.06f;
	const Matrix skyWorld =
		Matrix::CreateScale(180.0f) *
		Matrix::CreateRotationY(m_sceneTime * kSkyScrollSpeed) *
		Matrix::CreateTranslation(m_player.position.x, 0.0f, m_player.position.z);
	m_skyMesh->Draw(skyWorld, m_view, m_proj, StageTint(skyBaseColor));

	const Matrix upperHazeWorld =
		Matrix::CreateScale(156.0f) *
		Matrix::CreateRotationY(-skyPhase * 0.8f) *
		Matrix::CreateTranslation(m_player.position.x, 0.0f, m_player.position.z);
	m_skyMesh->Draw(upperHazeWorld, m_view, m_proj, StageTint(skyHazeColor));

	const Matrix horizonGlowWorld =
		Matrix::CreateScale(146.0f, 78.0f, 146.0f) *
		Matrix::CreateRotationY(skyPhase * 0.45f) *
		Matrix::CreateTranslation(m_player.position.x, -16.0f, m_player.position.z);
	m_skyMesh->Draw(horizonGlowWorld, m_view, m_proj, StageTint(horizonGlowColor));
	const Color silhouetteColor = StageTint(Color(0.08f, 0.10f, 0.13f, 0.78f));
	for (int band = 0; band < 5; ++band)
	{
		const float fb = static_cast<float>(band);
		const float z = m_player.position.z - 42.0f + fb * 18.0f;
		const float x = m_player.position.x - 28.0f + std::fmod(fb * 11.0f, 18.0f);
		const float width = 18.0f + fb * 4.0f;
		const float height = 2.8f + std::fmod(fb * 1.3f, 2.2f);
		const Matrix silhouetteWorld =
			Matrix::CreateScale(width, height, 1.2f) *
			Matrix::CreateTranslation(x, height * 0.5f + 0.2f, z);
		m_floorMesh->Draw(silhouetteWorld, m_view, m_proj, silhouetteColor);
	}
}
