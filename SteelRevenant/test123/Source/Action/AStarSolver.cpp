//------------------------//------------------------
// Contents(処理内容) PathGrid 上の4近傍A*探索を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "AStarSolver.h"

#include <queue>
#include <limits>
#include <cmath>

namespace
{
	struct OpenNode
	{
		int index;
		float fScore;
	};

	struct OpenNodeLess
	{
		// fScore が小さいノードを優先できるよう比較する。
		bool operator()(const OpenNode& a, const OpenNode& b) const
		{
			// priority_queue は最大ヒープなので逆順で比較
			return a.fScore > b.fScore;
		}
	};

	// 2ノード間の推定残コストを返す。
	float Heuristic(const Action::PathGrid::GridCoord& a, const Action::PathGrid::GridCoord& b)
	{
		// 4近傍なのでマンハッタン距離を採用
		return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
	}
}

namespace Action
{
	// A*探索でstartからgoalまでの経路を求める。
	std::vector<PathGrid::GridCoord> AStarSolver::FindPath(
		const PathGrid& grid,
		const PathGrid::GridCoord& start,
		const PathGrid::GridCoord& goal) const
	{
		std::vector<PathGrid::GridCoord> empty;

		if (!grid.IsWalkable(start) || !grid.IsWalkable(goal))
		{
			return empty;
		}

		if (start == goal)
		{
			std::vector<PathGrid::GridCoord> path;
			path.push_back(start);
			return path;
		}

		const int total = grid.GetWidth() * grid.GetHeight();
		const float kInf = (std::numeric_limits<float>::max)();

		std::vector<float> gScore(static_cast<size_t>(total), kInf);
		std::vector<int> cameFrom(static_cast<size_t>(total), -1);
		std::vector<unsigned char> closed(static_cast<size_t>(total), 0);

		const int startIdx = grid.ToIndex(start);
		const int goalIdx = grid.ToIndex(goal);

		gScore[static_cast<size_t>(startIdx)] = 0.0f;

		std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeLess> open;
		open.push(OpenNode{ startIdx, Heuristic(start, goal) });

		while (!open.empty())
		{
			const OpenNode currentNode = open.top();
			open.pop();

			if (closed[static_cast<size_t>(currentNode.index)] != 0)
			{
				continue;
			}

			closed[static_cast<size_t>(currentNode.index)] = 1;
			if (currentNode.index == goalIdx)
			{
				break;
			}

			const PathGrid::GridCoord current = grid.ToCoord(currentNode.index);
			const std::vector<PathGrid::GridCoord> neighbors = grid.GetNeighbors4(current);

			for (size_t i = 0; i < neighbors.size(); ++i)
			{
				const int nextIdx = grid.ToIndex(neighbors[i]);
				if (closed[static_cast<size_t>(nextIdx)] != 0)
				{
					continue;
				}

				const float tentativeG = gScore[static_cast<size_t>(currentNode.index)] + 1.0f;
				if (tentativeG < gScore[static_cast<size_t>(nextIdx)])
				{
					cameFrom[static_cast<size_t>(nextIdx)] = currentNode.index;
					gScore[static_cast<size_t>(nextIdx)] = tentativeG;
					const float f = tentativeG + Heuristic(neighbors[i], goal);
					open.push(OpenNode{ nextIdx, f });
				}
			}
		}

		if (cameFrom[static_cast<size_t>(goalIdx)] == -1)
		{
			return empty;
		}

		std::vector<PathGrid::GridCoord> reversePath;
		for (int idx = goalIdx; idx != -1; idx = cameFrom[static_cast<size_t>(idx)])
		{
			reversePath.push_back(grid.ToCoord(idx));
			if (idx == startIdx)
			{
				break;
			}
		}

		std::vector<PathGrid::GridCoord> path;
		path.reserve(reversePath.size());
		for (int i = static_cast<int>(reversePath.size()) - 1; i >= 0; --i)
		{
			path.push_back(reversePath[static_cast<size_t>(i)]);
		}
		return path;
	}
}


