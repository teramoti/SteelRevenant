#pragma once

//=============================================================================
// NavMesh.h
//
// 【役割】
//   障害物情報を PathGrid に反映するユーティリティ群を提供する。
//   ワールド空間の AABB（軸平行バウンディングボックス）障害物を
//   グリッドセル単位にラスタライズして通行不可セルとして登録する。
//
// 【設計メモ】
//   NavMesh という名称だが、実装は簡易的なグリッドラスタライズである。
//   本格的な NavMesh（ポリゴンメッシュによるナビゲーション領域定義）では
//   なく、プロトタイプに適した軽量版として位置づける。
//   将来的に Recast & Detour などの NavMesh ライブラリへの差し替えを
//   容易にするため、呼び出し側との間に NavMesh 層を挟む設計にしている。
//
// 【使い方】
//   NavMesh::RasterizeObstacles(grid, obstacleWorldMatrices, obstacleHalfSize);
//=============================================================================

#include <vector>
#include <SimpleMath.h>
#include "PathGrid.h"

namespace Navigation
{
    //=========================================================================
    // NavMesh  ― グリッドへの障害物ラスタライズユーティリティ
    //=========================================================================
    class NavMesh
    {
    public:
        /// @brief 障害物のワールド行列リストから通行不可セルをグリッドに登録する。
        /// @param grid             書き込み先グリッド
        /// @param obstacleWorlds   各障害物のワールド変換行列リスト
        /// @param obstacleHalfExt  障害物の半辺長 (m)（正方形 AABB を想定）
        static void RasterizeObstacles(
            PathGrid& grid,
            const std::vector<DirectX::SimpleMath::Matrix>& obstacleWorlds,
            float obstacleHalfExt);

        /// @brief グリッドのすべてのセルを通行可能状態にリセットする。
        /// @param grid  リセット対象グリッド
        static void ClearAll(PathGrid& grid);

    private:
        NavMesh() = delete; ///< 全メソッドが static のためインスタンス化を禁止する
    };

} // namespace Navigation
