//------------------------//------------------------
// Contents(処理内容) CombatSystem で共有する敵経路状態ユーティリティを定義する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#pragma once

#include "../CombatSystem.h"

namespace Action
{
	namespace CombatInternal
	{
		// 経路追跡に関する一時状態を破棄する。
		inline void ClearEnemyPathState(EnemyState& enemy)
		{
			enemy.path.clear();
			enemy.pathCursor = 0;
			enemy.repathTimer = 0.0f;
		}
	}
}
