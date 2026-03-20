#pragma once

//=============================================================================
// CollisionMesh.h
//
// 【役割】
//   三角形ポリゴンメッシュを使った精密なコリジョン判定を提供するクラス。
//   NavMesh の簡易グリッドコリジョンとは異なり、
//   地形の傾斜・段差・凹凸に対応する地面高さの解決に使用する。
//
// 【主な用途】
//   - ResolveGroundHeight: ワールド座標の真下にある地面高さを返す
//   - RayCast: レイと三角形メッシュの交差判定
//
// 【設計メモ】
//   メッシュデータはバイナリファイルから読み込む。
//   ゲームプレイ中は変更しない（静的コリジョンのみ対応）。
//=============================================================================

#include <vector>
#include <SimpleMath.h>

namespace System
{
    //=========================================================================
    // CollisionMesh  ― 三角形ポリゴンコリジョン
    //=========================================================================
    class CollisionMesh
    {
    public:
        //---------------------------------------------------------------------
        // Triangle  ― コリジョン三角形 1 枚
        //---------------------------------------------------------------------
        struct Triangle
        {
            DirectX::SimpleMath::Vector3 v0; ///< 頂点 0
            DirectX::SimpleMath::Vector3 v1; ///< 頂点 1
            DirectX::SimpleMath::Vector3 v2; ///< 頂点 2
            DirectX::SimpleMath::Vector3 normal; ///< 法線ベクトル（正規化済み）
        };

    public:
        CollisionMesh() = default;

        //---------------------------------------------------------------------
        // 読み込み
        //---------------------------------------------------------------------

        /// @brief バイナリファイルからコリジョンメッシュを読み込む。
        /// @param filePath  コリジョンメッシュファイルパス
        /// @return 読み込み成功なら true
        bool LoadFromFile(const wchar_t* filePath);

        /// @brief 三角形リストを直接設定する（テスト・手動配置用）。
        void SetTriangles(std::vector<Triangle> triangles);

        //---------------------------------------------------------------------
        // 判定
        //---------------------------------------------------------------------

        /// @brief 指定 XZ 座標の真上から下向きにレイを飛ばして地面高さを返す。
        /// @param xzPosition  判定する XZ 座標（Y 成分は無視）
        /// @param searchHeight レイ開始高さ (m)
        /// @return 地面高さ (m)。メッシュと交差しない場合は searchHeight を返す
        float ResolveGroundHeight(
            const DirectX::SimpleMath::Vector3& xzPosition,
            float searchHeight = 100.0f) const;

        /// @brief レイとメッシュの最近傍交点を返す。
        /// @param rayOrigin     レイ始点
        /// @param rayDirection  レイ方向（正規化済み）
        /// @param[out] hitPoint 交点座標（交差しない場合は未定義）
        /// @param[out] hitDist  交点までの距離 (m)
        /// @return 交差した場合 true
        bool RayCast(
            const DirectX::SimpleMath::Vector3& rayOrigin,
            const DirectX::SimpleMath::Vector3& rayDirection,
            DirectX::SimpleMath::Vector3& hitPoint,
            float& hitDist) const;

        //---------------------------------------------------------------------
        // Accessors
        //---------------------------------------------------------------------

        /// @brief ロード済みの三角形リストを返す。
        const std::vector<Triangle>& GetTriangles() const { return m_triangles; }

        /// @brief メッシュが読み込まれているか返す。
        bool IsLoaded() const { return !m_triangles.empty(); }

    private:
        std::vector<Triangle> m_triangles; ///< コリジョン三角形リスト
    };

} // namespace System
