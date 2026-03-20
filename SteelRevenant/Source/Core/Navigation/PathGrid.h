#pragma once

//=============================================================================
// PathGrid.h
//
// 【役割】
//   アリーナ空間を均一グリッドに分割し、各セルの通行可否を管理するクラス。
//   AStarSolver がこのグリッドを参照して経路を探索する。
//
// 【座標系】
//   ワールド座標 (x, z) をグリッド座標 (col, row) に変換して管理する。
//   Y 軸（高さ方向）は無視し、水平面上の 2D グリッドとして扱う。
//
// 【設計メモ】
//   - セルサイズと原点はコンストラクタで固定する。
//   - 障害物配置後に BlockCell() で通行不可を設定し、
//     ゲーム中に動的に変更することは想定していない。
//   - AStarSolver と完全に分離しており、
//     グリッド管理と探索アルゴリズムの責務を独立させている。
//=============================================================================

#include <vector>
#include <SimpleMath.h>

namespace Navigation
{
    //=========================================================================
    // PathGrid
    //=========================================================================
    class PathGrid
    {
    public:
        //---------------------------------------------------------------------
        // GridCoord  ― グリッド上の 2D 座標
        //---------------------------------------------------------------------
        struct GridCoord
        {
            int col; ///< 列インデックス（X 方向）
            int row; ///< 行インデックス（Z 方向）

            /// @brief 2 つのグリッド座標が等しいか返す。
            bool operator==(const GridCoord& other) const
            {
                return col == other.col && row == other.row;
            }
        };

    public:
        /// @brief グリッドを初期化する。
        /// @param cols      列数（X 方向の分割数）
        /// @param rows      行数（Z 方向の分割数）
        /// @param cellSize  セルの一辺の長さ (m)
        /// @param origin    グリッド左下のワールド座標
        PathGrid(int cols, int rows, float cellSize,
                 const DirectX::SimpleMath::Vector3& origin);

        //---------------------------------------------------------------------
        // セル操作
        //---------------------------------------------------------------------

        /// @brief 指定セルを通行不可にする。
        /// @param coord  対象グリッド座標
        void BlockCell(const GridCoord& coord);

        /// @brief 指定セルを通行可能に戻す。
        /// @param coord  対象グリッド座標
        void UnblockCell(const GridCoord& coord);

        /// @brief 指定セルが通行可能か返す。
        /// @param coord  対象グリッド座標
        /// @return 通行可能なら true（範囲外は false）
        bool IsWalkable(const GridCoord& coord) const;

        //---------------------------------------------------------------------
        // 座標変換
        //---------------------------------------------------------------------

        /// @brief ワールド座標をグリッド座標に変換する。
        /// @param worldPos  ワールド座標（Y 成分は無視）
        /// @return 対応するグリッド座標
        GridCoord WorldToGrid(const DirectX::SimpleMath::Vector3& worldPos) const;

        /// @brief グリッド座標のセル中心ワールド座標を返す。
        /// @param coord  グリッド座標
        /// @return セル中心のワールド座標（Y = 0）
        DirectX::SimpleMath::Vector3 GridToWorld(const GridCoord& coord) const;

        //---------------------------------------------------------------------
        // 隣接セル取得
        //---------------------------------------------------------------------

        /// @brief 指定セルに隣接する通行可能なセル一覧を返す（4 方向）。
        /// @param coord  基準グリッド座標
        /// @return 通行可能な隣接セルのリスト
        std::vector<GridCoord> GetNeighbors(const GridCoord& coord) const;

        //---------------------------------------------------------------------
        // Accessors
        //---------------------------------------------------------------------

        /// @brief 列数を返す。
        int GetCols() const { return m_cols; }

        /// @brief 行数を返す。
        int GetRows() const { return m_rows; }

        /// @brief セルサイズを返す (m)。
        float GetCellSize() const { return m_cellSize; }

        /// @brief グリッド座標がグリッド範囲内か返す。
        /// @param coord  判定するグリッド座標
        bool IsInBounds(const GridCoord& coord) const;

    private:
        /// @brief グリッド座標を 1 次元配列インデックスに変換する。
        int ToIndex(const GridCoord& coord) const;

    private:
        int   m_cols;                           ///< 列数
        int   m_rows;                           ///< 行数
        float m_cellSize;                       ///< セルの一辺 (m)
        DirectX::SimpleMath::Vector3 m_origin;  ///< グリッド左下のワールド座標
        std::vector<bool> m_walkable;           ///< 各セルの通行可否フラグ (col * rows + row)
    };

} // namespace Navigation
