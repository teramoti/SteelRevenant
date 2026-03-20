//------------------------//------------------------
// Contents(処理内容) PathGrid から生成する簡易ナビメッシュを宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
//-----------------------------------------------------------------------------
// NavMesh (Grid-derived coarse graph)
//-----------------------------------------------------------------------------
// 役割:
// - PathGrid から粗いナビゲーショングラフを生成する。
// - 長距離追跡時にセル単位A*より少ないノードで経路探索する。
// 注意:
// - 現段階は「自動生成された簡易NavMesh」。将来は手動ポリゴン化へ拡張する。
//-----------------------------------------------------------------------------
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>
#include "PathGrid.h"
namespace Action
{
	class NavMeshGraph
	{
	public:
		// ナビメッシュ生成状態を既定値で初期化する。
		NavMeshGraph()
			: m_width(0)
			, m_height(0)
			, m_cellSize(1.0f)
			, m_stride(3)
		{
		}
		// グリッドから簡易NavMeshを再構築する。
		void Build(const PathGrid& grid, int stride = 3)
		{
			m_width = grid.GetWidth();
			m_height = grid.GetHeight();
			m_cellSize = grid.GetCellSize();
			m_stride = std::max(2, std::min(6, stride));
			m_nodes.clear();
			for (int y = 0; y < m_height; y += m_stride)
			{
				for (int x = 0; x < m_width; x += m_stride)
				{
					const PathGrid::GridCoord c(x, y);
					if (!grid.IsWalkable(c))
					{
						continue;
					}
					NavNode node;
					node.coord = c;
					m_nodes.push_back(node);
				}
			}
			if (m_nodes.empty())
			{
				return;
			}
			const float maxDist = static_cast<float>(m_stride) * 1.75f;
			const float maxDistSq = maxDist * maxDist;
			for (size_t i = 0; i < m_nodes.size(); ++i)
			{
				for (size_t j = i + 1; j < m_nodes.size(); ++j)
				{
					const float dx = static_cast<float>(m_nodes[i].coord.x - m_nodes[j].coord.x);
					const float dy = static_cast<float>(m_nodes[i].coord.y - m_nodes[j].coord.y);
					const float distSq = dx * dx + dy * dy;
					if (distSq > maxDistSq)
					{
						continue;
					}
					if (!IsLineWalkable(grid, m_nodes[i].coord, m_nodes[j].coord))
					{
						continue;
					}
					m_nodes[i].links.push_back(static_cast<int>(j));
					m_nodes[j].links.push_back(static_cast<int>(i));
				}
			}
		}
		// グリッド設定変更がないかを判定する。
		bool IsBuiltFor(const PathGrid& grid) const
		{
			return m_width == grid.GetWidth() &&
				m_height == grid.GetHeight() &&
				std::fabs(m_cellSize - grid.GetCellSize()) < 0.0001f &&
				!m_nodes.empty();
		}
		// NavMesh上で経路探索し、セル座標列として返す。
		std::vector<PathGrid::GridCoord> FindPath(
			const PathGrid& grid,
			const PathGrid::GridCoord& start,
			const PathGrid::GridCoord& goal) const
		{
			std::vector<PathGrid::GridCoord> empty;
			if (m_nodes.empty() || !grid.IsWalkable(start) || !grid.IsWalkable(goal))
			{
				return empty;
			}
			if (start == goal)
			{
				empty.push_back(start);
				return empty;
			}
			const int startNode = FindClosestNode(grid, start);
			const int goalNode = FindClosestNode(grid, goal);
			if (startNode < 0 || goalNode < 0)
			{
				return empty;
			}
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
					return a.fScore > b.fScore;
				}
			};
			const float inf = std::numeric_limits<float>::max();
			std::vector<float> gScore(m_nodes.size(), inf);
			std::vector<int> cameFrom(m_nodes.size(), -1);
			std::vector<unsigned char> closed(m_nodes.size(), 0);
			gScore[static_cast<size_t>(startNode)] = 0.0f;
			std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeLess> open;
			open.push(OpenNode{ startNode, Heuristic(startNode, goalNode) });
			while (!open.empty())
			{
				const OpenNode current = open.top();
				open.pop();
				if (closed[static_cast<size_t>(current.index)] != 0)
				{
					continue;
				}
				closed[static_cast<size_t>(current.index)] = 1;
				if (current.index == goalNode)
				{
					break;
				}
				const NavNode& node = m_nodes[static_cast<size_t>(current.index)];
				for (size_t i = 0; i < node.links.size(); ++i)
				{
					const int next = node.links[i];
					if (closed[static_cast<size_t>(next)] != 0)
					{
						continue;
					}
					const float tentative = gScore[static_cast<size_t>(current.index)] + LinkCost(current.index, next);
					if (tentative < gScore[static_cast<size_t>(next)])
					{
						gScore[static_cast<size_t>(next)] = tentative;
						cameFrom[static_cast<size_t>(next)] = current.index;
						open.push(OpenNode{ next, tentative + Heuristic(next, goalNode) });
					}
				}
			}
			if (cameFrom[static_cast<size_t>(goalNode)] == -1)
			{
				return empty;
			}
			std::vector<PathGrid::GridCoord> reversePath;
			reversePath.push_back(goal);
			for (int idx = goalNode; idx != -1; idx = cameFrom[static_cast<size_t>(idx)])
			{
				reversePath.push_back(m_nodes[static_cast<size_t>(idx)].coord);
				if (idx == startNode)
				{
					break;
				}
			}
			reversePath.push_back(start);
			std::vector<PathGrid::GridCoord> path;
			path.reserve(reversePath.size());
			for (int i = static_cast<int>(reversePath.size()) - 1; i >= 0; --i)
			{
				if (!path.empty() && path.back() == reversePath[static_cast<size_t>(i)])
				{
					continue;
				}
				path.push_back(reversePath[static_cast<size_t>(i)]);
			}
			return path;
		}
	private:
		struct NavNode
		{
			PathGrid::GridCoord coord;
			std::vector<int> links;
		};
		// 2ノード間の推定残コストを返す。
		float Heuristic(int lhs, int rhs) const
		{
			return LinkCost(lhs, rhs);
		}
		// 2ノード間の直線距離コストを返す。
		float LinkCost(int lhs, int rhs) const
		{
			const float dx = static_cast<float>(m_nodes[static_cast<size_t>(lhs)].coord.x - m_nodes[static_cast<size_t>(rhs)].coord.x);
			const float dy = static_cast<float>(m_nodes[static_cast<size_t>(lhs)].coord.y - m_nodes[static_cast<size_t>(rhs)].coord.y);
			return std::sqrt(dx * dx + dy * dy);
		}
		// 2セル間を結ぶ線上に通行不可セルがないか判定する。
		bool IsLineWalkable(const PathGrid& grid, const PathGrid::GridCoord& from, const PathGrid::GridCoord& to) const
		{
			const int dx = to.x - from.x;
			const int dy = to.y - from.y;
			const int steps = std::max(std::abs(dx), std::abs(dy));
			if (steps <= 0)
			{
				return grid.IsWalkable(from);
			}
			for (int i = 0; i <= steps; ++i)
			{
				const float t = static_cast<float>(i) / static_cast<float>(steps);
				const int x = static_cast<int>(std::round(static_cast<float>(from.x) + static_cast<float>(dx) * t));
				const int y = static_cast<int>(std::round(static_cast<float>(from.y) + static_cast<float>(dy) * t));
				if (!grid.IsWalkable(PathGrid::GridCoord(x, y)))
				{
					return false;
				}
			}
			return true;
		}
		// 目標セルから見て最も近く、直線接続できるノードを返す。
		int FindClosestNode(const PathGrid& grid, const PathGrid::GridCoord& target) const
		{
			int best = -1;
			float bestDistSq = std::numeric_limits<float>::max();
			for (size_t i = 0; i < m_nodes.size(); ++i)
			{
				const PathGrid::GridCoord c = m_nodes[i].coord;
				const float dx = static_cast<float>(target.x - c.x);
				const float dy = static_cast<float>(target.y - c.y);
				const float distSq = dx * dx + dy * dy;
				if (distSq >= bestDistSq)
				{
					continue;
				}
				if (!IsLineWalkable(grid, target, c))
				{
					continue;
				}
				bestDistSq = distSq;
				best = static_cast<int>(i);
			}
			return best;
		}
		int m_width;
		int m_height;
		float m_cellSize;
		int m_stride;
		std::vector<NavNode> m_nodes;
	};
}
