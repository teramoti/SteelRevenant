#pragma once

#include <algorithm>
#include <cmath>

#include <SimpleMath.h>

#include "../../../Utility/SimpleMathEx.h"

namespace SceneFx
{
	struct StageRenderPalette
	{
		int stageTheme;
		float stagePulse;
		DirectX::SimpleMath::Color stageTintMul;
		DirectX::SimpleMath::Color skyBaseColor;
		DirectX::SimpleMath::Color skyHazeColor;
		DirectX::SimpleMath::Color horizonGlowColor;
		DirectX::SimpleMath::Color moonColor;
		DirectX::SimpleMath::Color beamColor;
		DirectX::SimpleMath::Color cloudColor;
		bool showNightElements;
	};

	inline StageRenderPalette BuildStageRenderPalette(int stageThemeIndex, float sceneTime)
	{
		using DirectX::SimpleMath::Color;
		StageRenderPalette palette{};
		palette.stageTheme = std::max(1, std::min(3, stageThemeIndex));
		palette.stagePulse = std::sinf(sceneTime * 0.7f) * 0.5f + 0.5f;
		palette.stageTintMul = Color(1.0f, 1.0f, 1.0f, 1.0f);
		palette.skyBaseColor = Color(0.24f, 0.32f, 0.42f, 1.0f);
		palette.skyHazeColor = Color(0.34f, 0.42f, 0.52f, 0.34f);
		palette.horizonGlowColor = Color(0.46f, 0.48f, 0.52f, 0.18f);
		palette.moonColor = Color(0.98f, 0.98f, 0.98f, 0.0f);
		palette.beamColor = Color(0.7f, 0.86f, 1.0f, 0.08f);
		palette.cloudColor = Color(0.74f, 0.80f, 0.86f, 0.10f);

		switch (palette.stageTheme)
		{
		case 2:
			palette.stageTintMul = Color(1.03f, 0.98f, 0.94f, 1.0f);
			palette.skyBaseColor = Color(0.26f, 0.28f, 0.32f, 1.0f);
			palette.skyHazeColor = Color(0.38f, 0.38f, 0.40f, 0.30f);
			palette.horizonGlowColor = Color(0.52f, 0.42f, 0.30f, 0.18f);
			palette.moonColor = Color(0.98f, 0.98f, 0.98f, 0.0f);
			palette.beamColor = Color(0.75f, 0.88f, 1.0f, 0.08f);
			palette.cloudColor = Color(0.78f, 0.76f, 0.74f, 0.09f);
			break;
		case 3:
			palette.stageTintMul = Color(0.94f, 1.0f, 1.08f, 1.0f);
			palette.skyBaseColor = Color(0.20f, 0.28f, 0.38f, 1.0f);
			palette.skyHazeColor = Color(0.30f, 0.40f, 0.50f, 0.30f);
			palette.horizonGlowColor = Color(0.34f, 0.54f, 0.62f, 0.16f);
			palette.moonColor = Color(0.98f, 0.98f, 0.98f, 0.0f);
			palette.beamColor = Color(0.7f, 0.86f, 1.0f, 0.08f);
			palette.cloudColor = Color(0.68f, 0.78f, 0.90f, 0.10f);
			break;
		default:
			break;
		}

		palette.showNightElements = false;
		return palette;
	}

	inline DirectX::SimpleMath::Color ApplyStageTint(
		const StageRenderPalette& palette,
		const DirectX::SimpleMath::Color& base,
		float alphaScale = 1.0f)
	{
		using DirectX::SimpleMath::Color;
		return Color(
			Utility::MathEx::Clamp(base.x * palette.stageTintMul.x, 0.0f, 1.0f),
			Utility::MathEx::Clamp(base.y * palette.stageTintMul.y, 0.0f, 1.0f),
			Utility::MathEx::Clamp(base.z * palette.stageTintMul.z, 0.0f, 1.0f),
			Utility::MathEx::Clamp(base.w * alphaScale, 0.0f, 1.0f));
	}
}
