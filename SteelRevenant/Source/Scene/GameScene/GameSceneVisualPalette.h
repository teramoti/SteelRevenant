#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <SimpleMath.h>

namespace GameSceneVisualPalette
{
    using Color = DirectX::SimpleMath::Color;

    constexpr Color skyZenithColor  { 0.02f, 0.04f, 0.12f, 1.0f };
    constexpr Color skyUpperColor   { 0.04f, 0.07f, 0.20f, 1.0f };
    constexpr Color skyMidColor     { 0.08f, 0.12f, 0.28f, 1.0f };
    constexpr Color skyLowerColor   { 0.15f, 0.18f, 0.35f, 1.0f };
    constexpr Color skyHorizonColor { 0.22f, 0.24f, 0.38f, 1.0f };

    constexpr Color floorBaseColor  { 0.18f, 0.20f, 0.22f, 1.0f };
    constexpr Color floorSeamColor  { 0.32f, 0.36f, 0.42f, 1.0f };
    constexpr Color wallColor       { 0.12f, 0.14f, 0.18f, 1.0f };
    constexpr Color wallAccentColor { 0.24f, 0.28f, 0.35f, 1.0f };
    constexpr Color pillarColor     { 0.20f, 0.22f, 0.26f, 1.0f };

    constexpr Color hitFlashColor   { 1.00f, 0.40f, 0.10f, 0.90f };
    constexpr Color guardFlashColor { 0.30f, 0.70f, 1.00f, 0.80f };
    constexpr Color lockOnRingColor { 1.00f, 0.90f, 0.10f, 1.00f };
    constexpr Color speedItemColor  { 0.10f, 1.00f, 0.50f, 1.00f };
    constexpr Color healthItemColor { 1.00f, 0.20f, 0.20f, 1.00f };

    constexpr Color playerBodyColor  { 0.28f, 0.38f, 0.62f, 1.0f }; // より落ち着いた青
    constexpr Color playerSwordColor { 0.95f, 0.92f, 0.80f, 1.0f }; // 刃は明るめ
    constexpr Color playerGuardColor { 0.18f, 0.48f, 0.92f, 1.0f }; // ガードエフェクトの色
    constexpr Color enemyRushColor   { 0.85f, 0.25f, 0.18f, 1.0f };
    constexpr Color enemyFlankColor  { 0.60f, 0.18f, 0.60f, 1.0f };
    constexpr Color enemyDeadColor   { 0.25f, 0.10f, 0.10f, 1.0f };
    constexpr Color enemyHitColor    { 1.00f, 0.60f, 0.10f, 1.0f };

    constexpr Color hpBarBackColor { 0.15f, 0.15f, 0.15f, 0.85f };
    constexpr Color hpBarFullColor { 0.20f, 0.80f, 0.30f, 1.0f };
    constexpr Color hpBarMidColor  { 0.90f, 0.70f, 0.05f, 1.0f };
    constexpr Color hpBarLowColor  { 0.90f, 0.15f, 0.10f, 1.0f };
}


