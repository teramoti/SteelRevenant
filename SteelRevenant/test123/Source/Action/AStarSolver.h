//------------------------//------------------------
// Contents(処理内容) PathGrid 上の4近傍A*探索を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// AStarSolver
//-----------------------------------------------------------------------------
// PathGrid 上での4近傍A*探索。
// 速度より可読性・デバッグしやすさを優先した実装。
//-----------------------------------------------------------------------------

#include <vector>
#include "PathGrid.h"

namespace Action
{
	class AStarSolver
	{
	public:
		// 4近傍A*探索を実行し、start->goalの経路を返す。
		// 到達不可の場合は空配列を返す。
		std::vector<PathGrid::GridCoord> FindPath(
			const PathGrid& grid,
			const PathGrid::GridCoord& start,
			const PathGrid::GridCoord& goal) const;
	};
}


