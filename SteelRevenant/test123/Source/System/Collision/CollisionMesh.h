#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

//=============================================================================
// CollisionMesh.h
//
// 縲仙ｽｹ蜑ｲ縲・//   荳芽ｧ貞ｽ｢繝昴Μ繧ｴ繝ｳ繝｡繝・す繝･繧剃ｽｿ縺｣縺溽ｲｾ蟇・↑繧ｳ繝ｪ繧ｸ繝ｧ繝ｳ蛻､螳壹ｒ謠蝉ｾ帙☆繧九け繝ｩ繧ｹ縲・//   NavMesh 縺ｮ邁｡譏薙げ繝ｪ繝・ラ繧ｳ繝ｪ繧ｸ繝ｧ繝ｳ縺ｨ縺ｯ逡ｰ縺ｪ繧翫・//   蝨ｰ蠖｢縺ｮ蛯ｾ譁懊・谿ｵ蟾ｮ繝ｻ蜃ｹ蜃ｸ縺ｫ蟇ｾ蠢懊☆繧句慍髱｢鬮倥＆縺ｮ隗｣豎ｺ縺ｫ菴ｿ逕ｨ縺吶ｋ縲・//
// 縲蝉ｸｻ縺ｪ逕ｨ騾斐・//   - ResolveGroundHeight: 繝ｯ繝ｼ繝ｫ繝牙ｺｧ讓吶・逵滉ｸ九↓縺ゅｋ蝨ｰ髱｢鬮倥＆繧定ｿ斐☆
//   - RayCast: 繝ｬ繧､縺ｨ荳芽ｧ貞ｽ｢繝｡繝・す繝･縺ｮ莠､蟾ｮ蛻､螳・//
// 縲占ｨｭ險医Γ繝｢縲・//   繝｡繝・す繝･繝・・繧ｿ縺ｯ繝舌う繝翫Μ繝輔ぃ繧､繝ｫ縺九ｉ隱ｭ縺ｿ霎ｼ繧縲・//   繧ｲ繝ｼ繝繝励Ξ繧､荳ｭ縺ｯ螟画峩縺励↑縺・ｼ磯撕逧・さ繝ｪ繧ｸ繝ｧ繝ｳ縺ｮ縺ｿ蟇ｾ蠢懶ｼ峨・//=============================================================================

#include <vector>
#include <SimpleMath.h>

namespace System
{
    //=========================================================================
    // CollisionMesh  窶・荳芽ｧ貞ｽ｢繝昴Μ繧ｴ繝ｳ繧ｳ繝ｪ繧ｸ繝ｧ繝ｳ
    //=========================================================================
    class CollisionMesh
    {
    public:
        //---------------------------------------------------------------------
        // Triangle  窶・繧ｳ繝ｪ繧ｸ繝ｧ繝ｳ荳芽ｧ貞ｽ｢ 1 譫・        //---------------------------------------------------------------------
        struct Triangle
        {
            DirectX::SimpleMath::Vector3 v0; ///< 鬆らせ 0
            DirectX::SimpleMath::Vector3 v1; ///< 鬆らせ 1
            DirectX::SimpleMath::Vector3 v2; ///< 鬆らせ 2
            DirectX::SimpleMath::Vector3 normal; ///< 豕慕ｷ壹・繧ｯ繝医Ν・域ｭ｣隕丞喧貂医∩・・        };

    public:
        CollisionMesh() = default;

        //---------------------------------------------------------------------
        // 隱ｭ縺ｿ霎ｼ縺ｿ
        //---------------------------------------------------------------------

        /// @brief 繝舌う繝翫Μ繝輔ぃ繧､繝ｫ縺九ｉ繧ｳ繝ｪ繧ｸ繝ｧ繝ｳ繝｡繝・す繝･繧定ｪｭ縺ｿ霎ｼ繧縲・        /// @param filePath  繧ｳ繝ｪ繧ｸ繝ｧ繝ｳ繝｡繝・す繝･繝輔ぃ繧､繝ｫ繝代せ
        /// @return 隱ｭ縺ｿ霎ｼ縺ｿ謌仙粥縺ｪ繧・true
        bool LoadFromFile(const wchar_t* filePath);

        /// @brief 荳芽ｧ貞ｽ｢繝ｪ繧ｹ繝医ｒ逶ｴ謗･險ｭ螳壹☆繧具ｼ医ユ繧ｹ繝医・謇句虚驟咲ｽｮ逕ｨ・峨・        void SetTriangles(std::vector<Triangle> triangles);

        //---------------------------------------------------------------------
        // 蛻､螳・        //---------------------------------------------------------------------

        /// @brief 謖・ｮ・XZ 蠎ｧ讓吶・逵滉ｸ翫°繧我ｸ句髄縺阪↓繝ｬ繧､繧帝｣帙・縺励※蝨ｰ髱｢鬮倥＆繧定ｿ斐☆縲・        /// @param xzPosition  蛻､螳壹☆繧・XZ 蠎ｧ讓呻ｼ・ 謌仙・縺ｯ辟｡隕厄ｼ・        /// @param searchHeight 繝ｬ繧､髢句ｧ矩ｫ倥＆ (m)
        /// @return 蝨ｰ髱｢鬮倥＆ (m)縲ゅΓ繝・す繝･縺ｨ莠､蟾ｮ縺励↑縺・ｴ蜷医・ searchHeight 繧定ｿ斐☆
        float ResolveGroundHeight(
            const DirectX::SimpleMath::Vector3& xzPosition,
            float searchHeight = 100.0f) const;

        /// @brief 繝ｬ繧､縺ｨ繝｡繝・す繝･縺ｮ譛霑大ｍ莠､轤ｹ繧定ｿ斐☆縲・        /// @param rayOrigin     繝ｬ繧､蟋狗せ
        /// @param rayDirection  繝ｬ繧､譁ｹ蜷托ｼ域ｭ｣隕丞喧貂医∩・・        /// @param[out] hitPoint 莠､轤ｹ蠎ｧ讓呻ｼ井ｺ､蟾ｮ縺励↑縺・ｴ蜷医・譛ｪ螳夂ｾｩ・・        /// @param[out] hitDist  莠､轤ｹ縺ｾ縺ｧ縺ｮ霍晞屬 (m)
        /// @return 莠､蟾ｮ縺励◆蝣ｴ蜷・true
        bool RayCast(
            const DirectX::SimpleMath::Vector3& rayOrigin,
            const DirectX::SimpleMath::Vector3& rayDirection,
            DirectX::SimpleMath::Vector3& hitPoint,
            float& hitDist) const;

        //---------------------------------------------------------------------
        // Accessors
        //---------------------------------------------------------------------

        /// @brief 繝ｭ繝ｼ繝画ｸ医∩縺ｮ荳芽ｧ貞ｽ｢繝ｪ繧ｹ繝医ｒ霑斐☆縲・        const std::vector<Triangle>& GetTriangles() const { return m_triangles; }

        /// @brief 繝｡繝・す繝･縺瑚ｪｭ縺ｿ霎ｼ縺ｾ繧後※縺・ｋ縺玖ｿ斐☆縲・        bool IsLoaded() const { return !m_triangles.empty(); }

    private:
        std::vector<Triangle> m_triangles; ///< 繧ｳ繝ｪ繧ｸ繝ｧ繝ｳ荳芽ｧ貞ｽ｢繝ｪ繧ｹ繝・    };

} // namespace System

