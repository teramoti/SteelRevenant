#pragma once

//=============================================================================
// GameSceneVisualPalette.h
//
// 【役割】
//   ステージテーマごとの空・背景・床の色定義を管理する。
//   BuildStageRenderPalette() が毎フレーム呼ばれ、
//   sceneTime によるアニメーションカラーを返す。
//
// 【ステージテーマ】
//   1: 廃都市夜 ─ 深青紫の夜空、地平線にオレンジの残光
//   2: 灼熱工業 ─ 煤煙の橙赤空、溶鉱炉グロー
//   3: 極地嵐   ─ 青白い吹雪空、稲妻フリッカー
//=============================================================================

#include <algorithm>
#include <cmath>
#include <SimpleMath.h>
#include "../../../Utility/SimpleMathEx.h"

namespace SceneFx
{
    struct StageRenderPalette
    {
        int   stageTheme;
        float stagePulse;
        DirectX::SimpleMath::Color stageTintMul;

        // 空（4層グラデーション）
        DirectX::SimpleMath::Color skyZenithColor;   ///< 天頂（最も暗い・深い色）
        DirectX::SimpleMath::Color skyMidColor;      ///< 中天
        DirectX::SimpleMath::Color skyBaseColor;     ///< 中層ヘイズ
        DirectX::SimpleMath::Color skyHazeColor;     ///< 地平線付近のヘイズ
        DirectX::SimpleMath::Color horizonGlowColor; ///< 地平線グロー（最も明るい）

        // 演出要素
        DirectX::SimpleMath::Color moonColor;        ///< 月・太陽の色
        DirectX::SimpleMath::Color beamColor;        ///< 光条の色
        DirectX::SimpleMath::Color cloudColor;       ///< 雲の色
        DirectX::SimpleMath::Color accentColor;      ///< 床アクセントライン色

        // フラグ
        bool showNightElements;  ///< 月・星を表示するか
        bool showLightning;      ///< 稲妻フリッカーを適用するか
        float lightningFlicker;  ///< 稲妻の現在輝度 (0-1)
    };

    inline StageRenderPalette BuildStageRenderPalette(int stageThemeIndex, float sceneTime)
    {
        using DirectX::SimpleMath::Color;
        StageRenderPalette p{};
        p.stageTheme  = std::max(1, std::min(3, stageThemeIndex));
        p.stagePulse  = std::sinf(sceneTime * 0.7f) * 0.5f + 0.5f;
        p.lightningFlicker = 0.0f;
        p.showLightning    = false;

        switch (p.stageTheme)
        {
        //---------------------------------------------------------------------
        // テーマ1: 廃都市夜
        // 深い青紫の夜空 + 地平線に都市の残光（オレンジ）
        //---------------------------------------------------------------------
        case 1:
        default:
            p.stageTintMul    = Color(1.0f, 0.97f, 1.04f, 1.0f);
            p.skyZenithColor  = Color(0.04f, 0.04f, 0.10f, 1.0f); // 天頂: 深夜青
            p.skyMidColor     = Color(0.08f, 0.10f, 0.20f, 0.92f); // 中天: 夜の青
            p.skyBaseColor    = Color(0.12f, 0.14f, 0.26f, 0.72f); // 中層
            p.skyHazeColor    = Color(0.20f, 0.18f, 0.28f, 0.45f); // 都市光混じり
            p.horizonGlowColor= Color(0.58f, 0.32f, 0.12f, 0.35f); // 地平線オレンジ残光
            p.moonColor       = Color(0.95f, 0.98f, 1.00f, 0.88f); // 月: 青白
            p.beamColor       = Color(0.40f, 0.50f, 0.80f, 0.12f); // 光条: 青紫
            p.cloudColor      = Color(0.14f, 0.16f, 0.24f, 0.28f); // 雲: 暗い
            p.accentColor     = Color(0.28f, 0.42f, 0.92f, 1.0f);  // 床: ネオンブルー
            p.showNightElements = true;
            break;

        //---------------------------------------------------------------------
        // テーマ2: 灼熱工業
        // 煤煙と炎の橙赤空 + 低い地平線グロー
        //---------------------------------------------------------------------
        case 2:
            p.stageTintMul    = Color(1.06f, 0.95f, 0.88f, 1.0f);
            p.skyZenithColor  = Color(0.08f, 0.05f, 0.04f, 1.0f); // 天頂: 煤で真っ暗
            p.skyMidColor     = Color(0.18f, 0.10f, 0.06f, 0.90f); // 中天: 暗い赤茶
            p.skyBaseColor    = Color(0.32f, 0.16f, 0.06f, 0.75f); // 中層: 煙霧の橙
            p.skyHazeColor    = Color(0.52f, 0.26f, 0.08f, 0.50f); // 煤煙オレンジ
            p.horizonGlowColor= Color(1.00f, 0.48f, 0.08f, 0.60f); // 溶鉱炉グロー
            p.moonColor       = Color(1.00f, 0.72f, 0.28f, 0.65f); // 太陽: 煙越しの橙
            p.beamColor       = Color(0.90f, 0.55f, 0.15f, 0.15f); // 光条: 煤煙に散乱
            p.cloudColor      = Color(0.28f, 0.16f, 0.08f, 0.45f); // 煙雲: 暗い橙茶
            p.accentColor     = Color(1.00f, 0.45f, 0.08f, 1.0f);  // 床: 溶岩オレンジ
            p.showNightElements = false;
            break;

        //---------------------------------------------------------------------
        // テーマ3: 極地嵐
        // 青白い吹雪空 + 稲妻フリッカー
        //---------------------------------------------------------------------
        case 3:
            p.stageTintMul    = Color(0.92f, 0.96f, 1.08f, 1.0f);
            p.skyZenithColor  = Color(0.06f, 0.10f, 0.18f, 1.0f); // 天頂: 嵐の深青
            p.skyMidColor     = Color(0.14f, 0.20f, 0.30f, 0.88f); // 中天
            p.skyBaseColor    = Color(0.26f, 0.34f, 0.44f, 0.70f); // 吹雪
            p.skyHazeColor    = Color(0.44f, 0.52f, 0.62f, 0.50f); // 雪霧
            p.horizonGlowColor= Color(0.62f, 0.72f, 0.84f, 0.40f); // 稲光の反射
            p.moonColor       = Color(0.80f, 0.88f, 1.00f, 0.45f); // 月: 雲越し
            p.beamColor       = Color(0.70f, 0.82f, 1.00f, 0.18f); // 稲妻光条
            p.cloudColor      = Color(0.52f, 0.60f, 0.70f, 0.55f); // 嵐雲: 厚い
            p.accentColor     = Color(0.20f, 0.70f, 1.00f, 1.0f);  // 床: 氷ブルー
            p.showNightElements = true;
            // 稲妻: ランダムなフリッカー
            {
                const float lt = std::fmod(sceneTime * 0.37f, 1.0f);
                p.showLightning   = (lt < 0.04f || (lt > 0.42f && lt < 0.45f));
                p.lightningFlicker = p.showLightning
                    ? (1.0f - lt / 0.04f) * 0.7f
                    : 0.0f;
            }
            break;
        }

        return p;
    }

    inline DirectX::SimpleMath::Color ApplyStageTint(
        const StageRenderPalette& palette,
        const DirectX::SimpleMath::Color& base,
        float alphaScale = 1.0f)
    {
        using DirectX::SimpleMath::Color;
        // 稲妻フリッカー: 全体を一瞬白く
        const float lf = palette.lightningFlicker;
        return Color(
            Utility::MathEx::Clamp(base.x * palette.stageTintMul.x + lf * 0.18f, 0.0f, 1.0f),
            Utility::MathEx::Clamp(base.y * palette.stageTintMul.y + lf * 0.20f, 0.0f, 1.0f),
            Utility::MathEx::Clamp(base.z * palette.stageTintMul.z + lf * 0.22f, 0.0f, 1.0f),
            Utility::MathEx::Clamp(base.w * alphaScale, 0.0f, 1.0f));
    }
}
