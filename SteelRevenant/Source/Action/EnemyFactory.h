//------------------------//------------------------
// Contents(処理内容) 危険度ごとの敵生成処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
#pragma once

#include <SimpleMath.h>

#include "CombatSystem.h"

namespace Action
{
	class EnemyFactory
	{
	public:
		// 指定危険度に応じた敵 1 体を生成する。
		static EnemyState CreateEnemy(
			const DirectX::SimpleMath::Vector3& spawnBase,
			int dangerLevel,
			int spawnSerial);
	};
}
