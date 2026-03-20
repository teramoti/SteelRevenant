//------------------------//------------------------
// Contents(処理内容) 敵モデルの配色決定処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "EnemyVisualProfile.h"

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

// アーキタイプごとの発光色を返す。
	Color GetArchetypeEmissive(Action::EnemyArchetype archetype)
	{
		switch (archetype)
		{
		case Action::EnemyArchetype::BladeRush:
			return Color(0.96f, 0.38f, 0.24f, 1.0f);
		case Action::EnemyArchetype::BladeFlank:
			return Color(0.88f, 0.30f, 0.62f, 1.0f);
		case Action::EnemyArchetype::GunHold:
			return Color(0.34f, 0.78f, 1.0f, 1.0f);
		case Action::EnemyArchetype::GunPressure:
		default:
			return Color(1.0f, 0.82f, 0.26f, 1.0f);
		}
	}
}

namespace SceneFx
{
// 敵1体ぶんの描画色セットを構築する。
	EnemyVisualProfile BuildEnemyVisualProfile(
		const Action::EnemyState& enemy,
		int dangerLevel,
		size_t enemyIndex,
		float sceneTime)
	{
		EnemyVisualProfile profile;

		Color emissive = GetArchetypeEmissive(enemy.archetype);

		const float dangerTint = Utility::MathEx::Clamp(0.96f + static_cast<float>(dangerLevel) * 0.035f, 0.96f, 1.14f);

		const float seed =
			static_cast<float>(enemyIndex) * 0.71f +
			enemy.spawnPosition.x * 0.047f +
			enemy.spawnPosition.z * 0.031f;
		const float jitterR = std::sinf(seed * 2.63f) * 0.03f;
		const float jitterG = std::sinf(seed * 3.11f + 1.2f) * 0.025f;
		const float jitterB = std::sinf(seed * 2.27f + 2.3f) * 0.03f;

		profile.armorDark = Color(
			Clamp01((0.10f + jitterR) * dangerTint),
			Clamp01((0.11f + jitterG) * dangerTint),
			Clamp01((0.14f + jitterB) * dangerTint),
			1.0f);
		profile.armorLight = Color(
			Clamp01((0.18f + jitterR) * dangerTint),
			Clamp01((0.21f + jitterG) * dangerTint),
			Clamp01((0.26f + jitterB) * dangerTint),
			1.0f);
		profile.underColor = Color(0.08f, 0.09f, 0.11f, 1.0f);
		profile.trimColor = Color(0.14f, 0.16f, 0.20f, 1.0f);
		profile.emissiveColor = emissive;
		profile.weaponColor = Color(0.28f, 0.30f, 0.34f, 1.0f);

		if (enemy.state == Action::EnemyStateType::Attack)
		{
			const float pulse = std::sinf(sceneTime * 11.0f + static_cast<float>(enemyIndex) * 0.4f) * 0.5f + 0.5f;
			profile.emissiveColor.x = Clamp01(profile.emissiveColor.x + 0.06f + pulse * 0.08f);
			profile.emissiveColor.y = Clamp01(profile.emissiveColor.y + pulse * 0.03f);
		}
		else if (enemy.state == Action::EnemyStateType::Return)
		{
			profile.armorLight.z = Clamp01(profile.armorLight.z + 0.05f);
		}

		if (enemy.hitByCurrentSwing)
		{
			profile.armorLight = Color(0.96f, 0.20f, 0.16f, 1.0f);
			profile.emissiveColor = Color(1.0f, 0.42f, 0.28f, 1.0f);
		}
		return profile;
	}
}

