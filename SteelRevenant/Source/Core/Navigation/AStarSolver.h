#pragma once

//=============================================================================
// AStarSolver.h
//
// 【役割】
//   グリッド上で A* アルゴリズムによる最短経路を探索するクラス。
//   敵 AI が障害物を回避しながらプレイヤーへ近づく経路を生成するために使う。
//
// 【アルゴリズム概要】
//   A* (A-Star) は f(n) = g(n) + h(n) を最小化するノードを優先探索する。
//     g(n) : スタートからノード n までの実コスト
//     h(n) : ノード n からゴールまでのヒューリスティックコスト（マンハッタン距離）
//   最適解を保証しながら、Dijkstra 法よりも探索ノード数を大幅に削減できる。
//
// 【設計メモ】
//   - PathGrid と完全に分離しており、グリッド管理の詳細を知らない。
//   - Solve() は毎回独立した探索を行い、内部にキャッシュを持たない。
//     経路のキャッシュと再利用は EnemyState.path / pathCursor が担う。
//   - 探索上限（maxNodes）を設けることで、
//     広大なグリッドでのフレームドロップを防ぐ。
//=============================================================================

#include <vector>
#include "PathGrid.h"

namespace Navigation
{
    //=========================================================================
    // AStarSolver
    //=========================================================================
    class AStarSolver
    {
    public:
        /// @brief ソルバーを既定パラメータで構築する。
        AStarSolver();

        //---------------------------------------------------------------------
        // 経路探索
        //---------------------------------------------------------------------

        /// @brief スタートからゴールまでの最短経路をグリッド上で探索する。
        /// @param grid      ナビゲーション用グリッド（通行可否を参照）
        /// @param start     探索開始グリッド座標
        /// @param goal      探索目標グリッド座標
        /// @param maxNodes  探索するノードの上限（0 = 無制限）
        /// @return 経路上のグリッド座標リスト（start を含まず goal を含む）
        ///         経路が見つからない場合は空リストを返す
        std::vector<PathGrid::GridCoord> Solve(
            const PathGrid&           grid,
            const PathGrid::GridCoord& start,
            const PathGrid::GridCoord& goal,
            int maxNodes = 512) const;

    private:
        //---------------------------------------------------------------------
        // 内部ヘルパー
        //---------------------------------------------------------------------

        /// @brief マンハッタン距離でヒューリスティックコストを算出する。
        /// @param a  グリッド座標 A
        /// @param b  グリッド座標 B
        /// @return マンハッタン距離
        static int Heuristic(
            const PathGrid::GridCoord& a,
            const PathGrid::GridCoord& b);

        /// @brief ゴールから親を遡って経路リストを復元する。
        /// @param cameFrom  各ノードの親座標マップ
        /// @param current   ゴールのグリッド座標
        /// @return start の次のセルから goal を含む経路リスト
        static std::vector<PathGrid::GridCoord> ReconstructPath(
            const std::vector<std::pair<PathGrid::GridCoord, PathGrid::GridCoord>>& cameFrom,
            const PathGrid::GridCoord& current);
    };

} // namespace Navigation
