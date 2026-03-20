//------------------------//------------------------
// Contents(処理内容) 敵モデルの配色決定処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include <cstddef>

#include <SimpleMath.h>

#include "../../Action/CombatSystem.h"

namespace SceneFx
{
	// 敵1体分の描画色セット。
	struct EnemyVisualProfile
	{
		DirectX::SimpleMath::Color armorDark;
		DirectX::SimpleMath::Color armorLight;
		DirectX::SimpleMath::Color underColor;
		DirectX::SimpleMath::Color trimColor;
		DirectX::SimpleMath::Color emissiveColor;
		DirectX::SimpleMath::Color weaponColor;
	};

	// 敵の状態と難易度進行から描画色を算出する。
	EnemyVisualProfile BuildEnemyVisualProfile(
		const Action::EnemyState& enemy,
		int dangerLevel,
		size_t enemyIndex,
		float sceneTime);
}

