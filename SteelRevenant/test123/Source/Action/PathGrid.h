//------------------------//------------------------
// Contents(処理内容) 通行判定付きグリッド地形を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// PathGrid
//-----------------------------------------------------------------------------
// A* 用の2Dグリッド地形。
// XZ平面をセルに量子化し、通行可否を保持する。
//-----------------------------------------------------------------------------

#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

namespace Action
{
	class PathGrid
	{
	public:
		struct GridCoord
		{
			int x;
			int y;

			// グリッド座標を既定値で初期化する。
			GridCoord() : x(0), y(0) {}
			// x, y を指定してグリッド座標を初期化する。
			GridCoord(int px, int py) : x(px), y(py) {}

			// 同じグリッド座標かどうかを返す。
			bool operator==(const GridCoord& rhs) const
			{
				return x == rhs.x && y == rhs.y;
			}

			// 不一致判定結果を返す。
			bool operator!=(const GridCoord& rhs) const
			{
				return !(*this == rhs);
			}
		};

	public:
		// グリッド状態を既定値で生成する。
		PathGrid();

		// 2Dグリッドのサイズ、セル幅、原点を設定して初期化する。
		void Initialize(int width, int height, float cellSize, const DirectX::SimpleMath::Vector2& origin);

		// 座標がグリッド内にあるか判定する。
		bool IsInBounds(const GridCoord& coord) const;
		// 指定セルが通行可能か判定する。
		bool IsWalkable(const GridCoord& coord) const;

		// 指定セルの通行可否を設定する。
		void SetBlocked(const GridCoord& coord, bool blocked);

		// ワールド矩形でまとめて通行不可を設定
		void SetBlockedRect(float minX, float minZ, float maxX, float maxZ, bool blocked);

		// ワールド座標(XZ)をグリッド座標へ変換する。
		GridCoord WorldToGrid(const DirectX::SimpleMath::Vector3& worldPos) const;
		// グリッド座標の中心点ワールド座標を返す。
		DirectX::SimpleMath::Vector3 GridToWorld(const GridCoord& coord, float y) const;

		// 4近傍の通行可能セルを返す。
		std::vector<GridCoord> GetNeighbors4(const GridCoord& coord) const;
		// 通行不可セルの一覧を返す。
		std::vector<GridCoord> GetBlockedCells() const;

		// グリッド幅(セル数)を返す。
		int GetWidth() const { return m_width; }
		// グリッド高さ(セル数)を返す。
		int GetHeight() const { return m_height; }
		// 1セルのワールドサイズを返す。
		float GetCellSize() const { return m_cellSize; }

		// 座標を1次元インデックスへ変換する。
		int ToIndex(const GridCoord& coord) const;
		// 1次元インデックスを座標へ変換する。
		GridCoord ToCoord(int index) const;

	private:
		int m_width;
		int m_height;
		float m_cellSize;
		DirectX::SimpleMath::Vector2 m_origin;
		std::vector<unsigned char> m_blocked;
	};
}


