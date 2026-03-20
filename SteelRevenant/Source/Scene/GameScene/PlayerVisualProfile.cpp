//------------------------//------------------------
// Contents(処理内容) プレイヤーモデルの配色決定処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "PlayerVisualProfile.h"

#include <cmath>

#include "../../Utility/SimpleMathEx.h"

using DirectX::SimpleMath::Color;

namespace
{
// 色成分を 0.0-1.0 に収める。
	float Clamp01(float value)
	{
		return Utility::MathEx::Clamp(value, 0.0f, 1.0f);
	}
}

namespace SceneFx
{
// プレイヤーの描画色セットを構築する。
	PlayerVisualProfile BuildPlayerVisualProfile(
		const Action::PlayerState& player,
		float sceneTime)
	{
		PlayerVisualProfile profile;

		const float pulse = std::sinf(sceneTime * 2.4f) * 0.5f + 0.5f;
		const float guardTint = player.guarding ? 0.06f : 0.0f;
		profile.armorDark = Color(
			Clamp01(0.10f + pulse * 0.01f),
			Clamp01(0.14f + pulse * 0.015f),
			Clamp01(0.20f + pulse * 0.02f + guardTint),
			1.0f);
		profile.armorLight = Color(
			Clamp01(0.22f + pulse * 0.02f),
			Clamp01(0.30f + pulse * 0.02f),
			Clamp01(0.40f + pulse * 0.03f + guardTint),
			1.0f);
		profile.underColor = Color(0.08f, 0.10f, 0.14f, 1.0f);
		profile.accentColor = Color(0.78f, 0.84f, 0.92f, 1.0f);
		profile.trimColor = Color(0.18f, 0.22f, 0.28f, 1.0f);
		profile.emissiveColor = Color(
			Clamp01(0.34f + pulse * 0.06f),
			Clamp01(0.82f + pulse * 0.04f),
			Clamp01(1.0f),
			1.0f);
		profile.weaponColor = Color(0.44f, 0.48f, 0.54f, 1.0f);
		return profile;
	}
}

