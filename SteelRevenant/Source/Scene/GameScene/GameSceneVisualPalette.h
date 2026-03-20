#pragma once
#include <SimpleMath.h>

// ゲームシーン全体で使用する色パレット定義。
// Bug#1修正: skyZenithColor / skyMidColor 等を全層分定義し、各層から個別参照できるようにする。
namespace GameSceneVisualPalette
{
    using Color = DirectX::SimpleMath::Color;

    // スカイドーム 5 層グラデーション
    constexpr Color skyZenithColor  { 0.02f, 0.04f, 0.12f, 1.0f };
    constexpr Color skyUpperColor   { 0.04f, 0.07f, 0.20f, 1.0f };
    constexpr Color skyMidColor     { 0.08f, 0.12f, 0.28f, 1.0f };
    constexpr Color skyLowerColor   { 0.15f, 0.18f, 0.35f, 1.0f };
    constexpr Color skyHorizonColor { 0.22f, 0.24f, 0.38f, 1.0f };

    // フロア・アリーナ
    constexpr Color floorBaseColor  { 0.18f, 0.20f, 0.22f, 1.0f };
    constexpr Color floorSeamColor  { 0.32f, 0.36f, 0.42f, 1.0f }; // Bug#3修正: 青灰
    constexpr Color wallColor       { 0.12f, 0.14f, 0.18f, 1.0f };
    constexpr Color wallAccentColor { 0.24f, 0.28f, 0.35f, 1.0f };
    constexpr Color pillarColor     { 0.20f, 0.22f, 0.26f, 1.0f };

    // エフェクト
    constexpr Color hitFlashColor   { 1.0f,  0.4f,  0.1f,  0.9f };
    constexpr Color guardFlashColor { 0.3f,  0.7f,  1.0f,  0.8f };
    constexpr Color lockOnRingColor { 1.0f,  0.9f,  0.1f,  1.0f };
    constexpr Color speedItemColor  { 0.1f,  1.0f,  0.5f,  1.0f };
    constexpr Color healthItemColor { 1.0f,  0.2f,  0.2f,  1.0f };

    // プレイヤー・敵
    constexpr Color playerBodyColor  { 0.55f, 0.65f, 0.80f, 1.0f };
    constexpr Color playerSwordColor { 0.85f, 0.88f, 0.95f, 1.0f };
    constexpr Color playerGuardColor { 0.30f, 0.60f, 1.00f, 1.0f };
    constexpr Color enemyRushColor   { 0.75f, 0.20f, 0.15f, 1.0f };
    constexpr Color enemyFlankColor  { 0.55f, 0.15f, 0.55f, 1.0f };
    constexpr Color enemyDeadColor   { 0.25f, 0.10f, 0.10f, 1.0f };
    constexpr Color enemyHitColor    { 1.00f, 0.60f, 0.10f, 1.0f };

    // HP バー
    constexpr Color hpBarBackColor  { 0.15f, 0.15f, 0.15f, 0.85f };
    constexpr Color hpBarFullColor  { 0.20f, 0.80f, 0.30f, 1.0f };
    constexpr Color hpBarMidColor   { 0.90f, 0.70f, 0.05f, 1.0f };
    constexpr Color hpBarLowColor   { 0.90f, 0.15f, 0.10f, 1.0f };
}
