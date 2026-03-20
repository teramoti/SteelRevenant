//------------------------//------------------------
// Contents(処理内容) 通行判定付きグリッド地形を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "PathGrid.h"
#include "../Utility/SimpleMathEx.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace Action
{
	// 既定値でグリッドを初期化する。
	PathGrid::PathGrid()
		: m_width(0)
		, m_height(0)
		, m_cellSize(1.0f)
		, m_origin(Vector2::Zero)
	{
	}

	// グリッドサイズとセルサイズを確定し、通行情報を初期化する。
	void PathGrid::Initialize(int width, int height, float cellSize, const Vector2& origin)
	{
		m_width = std::max(1, width);
		m_height = std::max(1, height);
		m_cellSize = std::max(0.1f, cellSize);
		m_origin = origin;
		m_blocked.assign(static_cast<size_t>(m_width * m_height), 0);
	}

	// 座標が範囲内か判定する。
	bool PathGrid::IsInBounds(const GridCoord& coord) const
	{
		return coord.x >= 0 && coord.x < m_width && coord.y >= 0 && coord.y < m_height;
	}

	// 指定セルが通行可能か判定する。
	bool PathGrid::IsWalkable(const GridCoord& coord) const
	{
		if (!IsInBounds(coord))
		{
			return false;
		}
		return m_blocked[static_cast<size_t>(ToIndex(coord))] == 0;
	}

	// セルの通行可否を設定する。
	void PathGrid::SetBlocked(const GridCoord& coord, bool blocked)
	{
		if (!IsInBounds(coord))
		{
			return;
		}
		m_blocked[static_cast<size_t>(ToIndex(coord))] = blocked ? 1 : 0;
	}

	// ワールド矩形を覆うセル群の通行可否を一括設定する。
	void PathGrid::SetBlockedRect(float minX, float minZ, float maxX, float maxZ, bool blocked)
	{
		// 矩形をグリッドへ変換し、対象セルを一括設定する。
		GridCoord minC = WorldToGrid(Vector3(minX, 0.0f, minZ));
		GridCoord maxC = WorldToGrid(Vector3(maxX, 0.0f, maxZ));

		const int fromX = Utility::MathEx::Clamp(std::min(minC.x, maxC.x), 0, m_width - 1);
		const int toX = Utility::MathEx::Clamp(std::max(minC.x, maxC.x), 0, m_width - 1);
		const int fromY = Utility::MathEx::Clamp(std::min(minC.y, maxC.y), 0, m_height - 1);
		const int toY = Utility::MathEx::Clamp(std::max(minC.y, maxC.y), 0, m_height - 1);

		for (int y = fromY; y <= toY; ++y)
		{
			for (int x = fromX; x <= toX; ++x)
			{
				SetBlocked(GridCoord(x, y), blocked);
			}
		}
	}

	// ワールド座標をグリッド座標へ変換する。
	PathGrid::GridCoord PathGrid::WorldToGrid(const Vector3& worldPos) const
	{
		const float gx = (worldPos.x - m_origin.x) / m_cellSize;
		const float gy = (worldPos.z - m_origin.y) / m_cellSize;
		return GridCoord(static_cast<int>(std::floor(gx)), static_cast<int>(std::floor(gy)));
	}

	// グリッド座標の中心ワールド座標を返す。
	Vector3 PathGrid::GridToWorld(const GridCoord& coord, float y) const
	{
		const float worldX = m_origin.x + (static_cast<float>(coord.x) + 0.5f) * m_cellSize;
		const float worldZ = m_origin.y + (static_cast<float>(coord.y) + 0.5f) * m_cellSize;
		return Vector3(worldX, y, worldZ);
	}

	// 4近傍の通行可能セルを列挙する。
	std::vector<PathGrid::GridCoord> PathGrid::GetNeighbors4(const GridCoord& coord) const
	{
		static const int kOffsets[4][2] =
		{
			{ 1, 0 },
			{ -1, 0 },
			{ 0, 1 },
			{ 0, -1 }
		};

		std::vector<GridCoord> result;
		result.reserve(4);

		for (int i = 0; i < 4; ++i)
		{
			GridCoord c(coord.x + kOffsets[i][0], coord.y + kOffsets[i][1]);
			if (IsWalkable(c))
			{
				result.push_back(c);
			}
		}
		return result;
	}

	// 通行不可セル一覧を返す。
	std::vector<PathGrid::GridCoord> PathGrid::GetBlockedCells() const
	{
		std::vector<GridCoord> cells;
		for (int i = 0; i < static_cast<int>(m_blocked.size()); ++i)
		{
			if (m_blocked[static_cast<size_t>(i)] != 0)
			{
				cells.push_back(ToCoord(i));
			}
		}
		return cells;
	}

	// 2次元座標を1次元インデックスへ変換する。
	int PathGrid::ToIndex(const GridCoord& coord) const
	{
		return coord.y * m_width + coord.x;
	}

	// 1次元インデックスを2次元座標へ変換する。
	PathGrid::GridCoord PathGrid::ToCoord(int index) const
	{
		const int x = index % m_width;
		const int y = index / m_width;
		return GridCoord(x, y);
	}
}


