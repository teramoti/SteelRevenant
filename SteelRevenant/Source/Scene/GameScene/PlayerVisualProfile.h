//------------------------//------------------------
// Contents(処理内容) プレイヤーモデルの配色決定処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include <SimpleMath.h>

#include "../../Action/CombatSystem.h"

namespace SceneFx
{
	// プレイヤー1体分の描画色セット。
	struct PlayerVisualProfile
	{
		DirectX::SimpleMath::Color armorDark;
		DirectX::SimpleMath::Color armorLight;
		DirectX::SimpleMath::Color underColor;
		DirectX::SimpleMath::Color accentColor;
		DirectX::SimpleMath::Color trimColor;
		DirectX::SimpleMath::Color emissiveColor;
		DirectX::SimpleMath::Color weaponColor;
	};

	// プレイヤー状態から描画色セットを構築する。
	PlayerVisualProfile BuildPlayerVisualProfile(
		const Action::PlayerState& player,
		float sceneTime);
}

