//------------------------//------------------------
// Contents(処理内容) プレイヤー・敵・武器・エフェクトの描画を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "../GameScene.h"

#include <algorithm>
#include <cmath>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "../../../Utility/SimpleMathEx.h"
#include "../EnemyVisualProfile.h"
#include "../PlayerVisualProfile.h"
#include "GameSceneVisualPalette.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr float kAttackVisualDurationSec    = 0.18f;
    constexpr float kWeaponSwingYawRad          = 1.45f;
    constexpr float kWeaponSwingPitchRad        = 1.15f;
    constexpr float kPlayerRunCycleSpeed        = 7.6f;
    constexpr float kEnemyBreathSpeed           = 4.8f;
    constexpr float kEnemySpawnDurationSec      = 0.4f;
    constexpr float kEnemyAttackWindupVisualSec = 0.45f;

    // 剣の色定義（冷たい金属表現）
    const Color kBladeColor       = { 0.80f, 0.84f, 0.90f, 1.0f }; // 刃身ベース
    const Color kBladeEdgeColor   = { 0.95f, 0.97f, 1.00f, 1.0f }; // 刃縁（明るい）
    const Color kBladeFullerColor = { 0.52f, 0.55f, 0.62f, 1.0f }; // 血溝（暗め）
    const Color kBladeFlatColor   = { 0.62f, 0.65f, 0.72f, 1.0f }; // 平面（やや暗め）
    const Color kGuardColor       = { 0.35f, 0.30f, 0.20f, 1.0f }; // 鍔（真鍮）
    const Color kGuardTrimColor   = { 0.55f, 0.48f, 0.30f, 1.0f }; // 鍔トリム
    const Color kGripColor        = { 0.13f, 0.09f, 0.07f, 1.0f }; // 柄（黒革）
    const Color kGripWrapColor    = { 0.20f, 0.16f, 0.12f, 1.0f }; // 柄巻き
    const Color kPommelColor      = { 0.42f, 0.36f, 0.24f, 1.0f }; // 柄頭
}

void GameScene::DrawWorldActors()
{
    const SceneFx::StageRenderPalette palette = SceneFx::BuildStageRenderPalette(m_stageThemeIndex, m_sceneTime);
    const auto StageTint = [&palette](const Color& base, float alphaScale = 1.0f) -> Color
    {
        return SceneFx::ApplyStageTint(palette, base, alphaScale);
    };
    const auto DrawShadow = [&](const Vector3& pos, float sx, float sz, float alpha)
    {
        m_effectOrbMesh->Draw(
            Matrix::CreateScale(sx, 0.05f, sz) * Matrix::CreateTranslation(pos.x, 0.055f, pos.z),
            m_view, m_proj, StageTint(Color(0.01f, 0.015f, 0.02f, alpha)));
    };
    const auto Lf = [](float a, float b, float t) -> float
    {
        return a + (b - a) * Utility::MathEx::Clamp(t, 0.0f, 1.0f);
    };
    const auto Lv = [&Lf](const Vector3& a, const Vector3& b, float t) -> Vector3
    {
        return { Lf(a.x,b.x,t), Lf(a.y,b.y,t), Lf(a.z,b.z,t) };
    };

    // =========================================================================
    // プレイヤー描画
    // =========================================================================
    const float attackBlend = GetAttackBlend();
    const float carryBlend  = Utility::MathEx::Clamp(attackBlend, 0.0f, 1.0f);
    const float comboSign   = (m_player.comboIndex % 2 == 0) ? -1.0f : 1.0f;
    const float runCycle    = std::sinf(m_sceneTime * kPlayerRunCycleSpeed);
    const float legSwing    = runCycle * 0.26f;
    const float armPulse    = runCycle * 0.18f;

    const SceneFx::PlayerVisualProfile pv = SceneFx::BuildPlayerVisualProfile(m_player, m_sceneTime);
    const Vector3 forward(std::sin(m_player.yaw), 0.0f, std::cos(m_player.yaw));
    const Vector3 right(forward.z, 0.0f, -forward.x);

    DrawShadow(m_player.position, 0.56f, 0.78f, 0.18f);

    const Matrix pRoot =
        Matrix::CreateRotationY(m_player.yaw + attackBlend * comboSign * 0.08f) *
        Matrix::CreateTranslation(m_player.position + Vector3(0.0f, std::sinf(m_sceneTime * 4.4f) * 0.04f, 0.0f));

    // 胴体
    m_enemyMesh->Draw(
        Matrix::CreateScale(0.44f, 0.62f, 0.44f) * Matrix::CreateRotationX(attackBlend * 0.3f) *
        Matrix::CreateTranslation(0.0f, 1.0f, attackBlend * 0.12f) * pRoot,
        m_view, m_proj, pv.armorLight);

    // 頭部
    m_enemyMesh->Draw(
        Matrix::CreateScale(0.20f) * Matrix::CreateTranslation(0.0f, 1.68f, 0.0f) * pRoot,
        m_view, m_proj, pv.armorLight);

    // バイザー（発光スリット）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.13f, 0.024f, 0.032f) * Matrix::CreateTranslation(0.0f, 1.695f, 0.198f) * pRoot,
        m_view, m_proj, pv.emissiveColor);

    // バイザー端キャップ
    for (int s : { -1, 1 })
    {
        m_obstacleMesh->Draw(
            Matrix::CreateScale(0.026f, 0.024f, 0.028f) *
            Matrix::CreateTranslation(s * 0.072f, 1.695f, 0.192f) * pRoot,
            m_view, m_proj, pv.emissiveColor);
    }

    // 頭頂リッジ
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.055f, 0.045f, 0.18f) * Matrix::CreateTranslation(0.0f, 1.88f, 0.0f) * pRoot,
        m_view, m_proj, pv.accentColor);

    // 胸甲
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.36f, 0.20f, 0.14f) * Matrix::CreateRotationX(attackBlend * 0.16f) *
        Matrix::CreateTranslation(0.0f, 1.08f, 0.18f) * pRoot,
        m_view, m_proj, pv.armorDark);

    // コアライン（発光）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.09f, 0.14f, 0.032f) * Matrix::CreateTranslation(0.0f, 1.08f, 0.27f) * pRoot,
        m_view, m_proj, pv.emissiveColor);

    // 肩甲（左右）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.16f, 0.10f, 0.18f) * Matrix::CreateRotationZ(-0.24f) *
        Matrix::CreateTranslation(-0.32f, 1.28f, 0.0f) * pRoot,
        m_view, m_proj, pv.trimColor);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.16f, 0.10f, 0.18f) * Matrix::CreateRotationZ(0.24f) *
        Matrix::CreateTranslation(0.32f, 1.28f, 0.02f) * pRoot,
        m_view, m_proj, pv.trimColor);

    // 腰甲（左右）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.10f, 0.18f, 0.08f) * Matrix::CreateRotationZ(-0.1f) *
        Matrix::CreateTranslation(-0.18f, 0.64f, 0.07f) * pRoot,
        m_view, m_proj, pv.armorDark);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.10f, 0.18f, 0.08f) * Matrix::CreateRotationZ(0.1f) *
        Matrix::CreateTranslation(0.18f, 0.64f, 0.07f) * pRoot,
        m_view, m_proj, pv.armorDark);

    // 脚（左右）
    m_playerMesh->Draw(
        Matrix::CreateScale(0.14f, 0.55f, 0.16f) * Matrix::CreateRotationX(legSwing) *
        Matrix::CreateTranslation(-0.16f, 0.32f, 0.0f) * pRoot,
        m_view, m_proj, pv.underColor);
    m_playerMesh->Draw(
        Matrix::CreateScale(0.14f, 0.55f, 0.16f) * Matrix::CreateRotationX(-legSwing) *
        Matrix::CreateTranslation(0.16f, 0.32f, 0.0f) * pRoot,
        m_view, m_proj, pv.underColor);

    // 脛甲（左右）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.10f, 0.22f, 0.08f) * Matrix::CreateTranslation(-0.16f, 0.16f, 0.13f) * pRoot,
        m_view, m_proj, pv.armorDark);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.10f, 0.22f, 0.08f) * Matrix::CreateTranslation(0.16f, 0.16f, 0.13f) * pRoot,
        m_view, m_proj, pv.armorDark);

    // 腕（左右）
    m_playerMesh->Draw(
        Matrix::CreateScale(0.14f, 0.48f, 0.14f) *
        Matrix::CreateRotationX(Lf(0.02f, 0.32f - attackBlend * 0.18f, carryBlend)) *
        Matrix::CreateRotationZ(Lf(-0.10f + armPulse * 0.25f, -1.02f + comboSign * attackBlend * 0.10f, carryBlend)) *
        Matrix::CreateTranslation(Lv(Vector3(-0.30f, 1.02f, -0.02f), Vector3(0.06f, 0.92f, -0.10f), carryBlend)) *
        pRoot,
        m_view, m_proj, pv.underColor);
    m_playerMesh->Draw(
        Matrix::CreateScale(0.14f, 0.50f, 0.14f) *
        Matrix::CreateRotationX(Lf(0.58f, -0.24f - attackBlend * 0.82f, carryBlend)) *
        Matrix::CreateRotationZ(Lf(0.08f + armPulse * 0.25f, 0.28f - armPulse + comboSign * attackBlend * 0.42f, carryBlend)) *
        Matrix::CreateTranslation(Lv(Vector3(0.30f, 0.94f, -0.18f), Vector3(0.30f, 1.00f, 0.04f + attackBlend * 0.08f), carryBlend)) *
        pRoot,
        m_view, m_proj, pv.underColor);

    // =========================================================================
    // 剣（ロングソード）描画
    // =========================================================================
    // 振りのイーズ: 序盤ゆっくり→中盤一気に加速→終盤ゆっくり
    // これにより「ため→鋭い振り→フォロースルー」の重さが生まれる
    const float swingEase  = attackBlend < 0.5f
        ? 4.0f * attackBlend * attackBlend * attackBlend              // 序盤: ease-in (ゆっくり)
        : 1.0f - std::powf(-2.0f * attackBlend + 2.0f, 3.0f) * 0.5f;// 終盤: ease-out (フォロースルー)
    const float swingYaw   = comboSign * (swingEase * 2.0f - 1.0f) * kWeaponSwingYawRad;
    const float swingPitch = -0.48f + std::sinf(swingEase * DirectX::XM_PI) * (kWeaponSwingPitchRad * 1.05f);
    // 待機: 剣を右肩前方に立てて構える（腰の後ろではなく正面構え）
    const float cyaw   = Lf(-0.28f, swingYaw, carryBlend);
    const float cpitch = Lf(-0.62f, swingPitch, carryBlend);  // 待機時は刃先を上に
    const float croll  = Lf(-0.18f, -0.12f + comboSign * swingEase * 0.08f, carryBlend);

    const Matrix bladeRot =
        Matrix::CreateRotationZ(croll) *
        Matrix::CreateRotationX(cpitch) *
        Matrix::CreateRotationY(m_player.yaw + cyaw);

    Vector3 bDir = Vector3::TransformNormal(Vector3::UnitY, bladeRot); bDir.Normalize();
    Vector3 bRight = Vector3::TransformNormal(Vector3::UnitX, bladeRot); bRight.Normalize();

    // 待機構え: 右腰前方でグリップ、刃先を斜め上前方に向ける
    // 振り時: 右肩から前方へ一気に振り抜く
    const Vector3 gripAnchor = Vector3::Transform(
        Lv(Vector3(0.28f, 0.88f, 0.08f), Vector3(0.24f, 0.82f, 0.14f + attackBlend * 0.04f), carryBlend),
        pRoot);
    const Vector3 wCenter =
        gripAnchor +
        bDir    * Lf(0.44f,  0.52f + swingEase * 0.06f, carryBlend) +
        forward * Lf( 0.10f, -0.04f + swingEase * 0.10f, carryBlend) +
        right   * Lf( 0.08f,  0.04f + comboSign * 0.04f, carryBlend);

    // 刃身メイン（薄い板: 0.15 x 1.32 x 0.018）
    m_weaponMesh->Draw(bladeRot * Matrix::CreateTranslation(wCenter), m_view, m_proj, kBladeColor);

    // 刃縁ハイライト（左右エッジを明るく）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.007f, 1.28f, 0.022f) * bladeRot * Matrix::CreateTranslation(wCenter + bRight * 0.075f),
        m_view, m_proj, kBladeEdgeColor);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.007f, 1.28f, 0.022f) * bladeRot * Matrix::CreateTranslation(wCenter - bRight * 0.075f),
        m_view, m_proj, kBladeEdgeColor);

    // 血溝（フラー: 刃身中央の縦溝）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.020f, 0.86f, 0.024f) * bladeRot * Matrix::CreateTranslation(wCenter - bDir * 0.12f),
        m_view, m_proj, kBladeFullerColor);

    // 刃先（3段テーパーで尖らせる）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.10f, 0.13f, 0.016f) * bladeRot * Matrix::CreateTranslation(wCenter + bDir * 0.70f),
        m_view, m_proj, kBladeFlatColor);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.046f, 0.09f, 0.012f) * bladeRot * Matrix::CreateTranslation(wCenter + bDir * 0.82f),
        m_view, m_proj, kBladeColor);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.014f, 0.065f, 0.008f) * bladeRot * Matrix::CreateTranslation(wCenter + bDir * 0.92f),
        m_view, m_proj, kBladeEdgeColor);

    // 鍔（クロスガード）: 横に広い板
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.42f, 0.038f, 0.10f) * bladeRot * Matrix::CreateTranslation(gripAnchor + bDir * 0.03f),
        m_view, m_proj, kGuardColor);
    // 鍔トリム（上面ライン）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.38f, 0.018f, 0.016f) * bladeRot * Matrix::CreateTranslation(gripAnchor + bDir * 0.03f),
        m_view, m_proj, kGuardTrimColor);
    // 鍔端キャップ（左右）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.030f, 0.054f, 0.11f) * bladeRot * Matrix::CreateTranslation(gripAnchor + bDir * 0.03f + bRight * 0.208f),
        m_view, m_proj, kGuardTrimColor);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.030f, 0.054f, 0.11f) * bladeRot * Matrix::CreateTranslation(gripAnchor + bDir * 0.03f - bRight * 0.208f),
        m_view, m_proj, kGuardTrimColor);

    // 柄（グリップ）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.050f, 0.27f, 0.064f) * bladeRot * Matrix::CreateTranslation(gripAnchor - bDir * 0.17f),
        m_view, m_proj, kGripColor);
    // 柄巻きリング（3本）
    for (int ri = 0; ri < 3; ++ri)
    {
        const float ro = -0.08f + static_cast<float>(ri) * 0.08f;
        m_obstacleMesh->Draw(
            Matrix::CreateScale(0.058f, 0.018f, 0.072f) * bladeRot *
            Matrix::CreateTranslation(gripAnchor - bDir * (0.17f - ro)),
            m_view, m_proj, kGripWrapColor);
    }

    // 柄頭（ポンメル）
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.080f, 0.070f, 0.080f) * bladeRot * Matrix::CreateTranslation(gripAnchor - bDir * 0.34f),
        m_view, m_proj, kPommelColor);
    m_obstacleMesh->Draw(
        Matrix::CreateScale(0.054f, 0.030f, 0.054f) * bladeRot * Matrix::CreateTranslation(gripAnchor - bDir * 0.39f),
        m_view, m_proj, kGuardTrimColor);

    // コンボLv3: 刃身グロー
    if (m_player.comboLevel >= 3 && attackBlend > 0.05f)
    {
        m_weaponMesh->Draw(
            Matrix::CreateScale(1.4f, 1.0f, 2.0f) * bladeRot * Matrix::CreateTranslation(wCenter),
            m_view, m_proj,
            Color(pv.emissiveColor.R(), pv.emissiveColor.G(), pv.emissiveColor.B(), attackBlend * 0.40f));
    }

    // =========================================================================
    // 敵描画
    // =========================================================================
    const Matrix invView = m_view.Invert();
    const Vector3 camPos(invView._41, invView._42, invView._43);

    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        const Action::EnemyState& en = m_enemies[i];
        if (en.state == Action::EnemyStateType::Dead) continue;

        const SceneFx::EnemyVisualProfile ev =
            SceneFx::BuildEnemyVisualProfile(en, m_gameState.dangerLevel, i, m_sceneTime);

        const float breath  = std::sinf(m_sceneTime * kEnemyBreathSpeed + static_cast<float>(i) * 0.78f) * 0.07f;
        const float idleB   = 1.0f - Utility::MathEx::Clamp(en.stateTimer / kEnemySpawnDurationSec, 0.0f, 1.0f);
        const float eScale  = (en.state == Action::EnemyStateType::Idle && en.stateTimer > 0.0f)
                            ? (0.35f + idleB * 0.65f) : 1.0f;
        const float hitB    = (en.state == Action::EnemyStateType::Chase && en.stateTimer > 0.0f)
                            ? Utility::MathEx::Clamp(en.stateTimer / 0.18f, 0.0f, 1.0f) : 0.0f;
        const float retLean = (en.state == Action::EnemyStateType::Return)
                            ? std::sinf(m_sceneTime * 7.0f + static_cast<float>(i) * 0.6f) * 0.12f : 0.0f;
        const float stagLean = hitB * std::sinf(en.stateTimer * 20.0f + static_cast<float>(i) * 0.6f) * 0.24f + retLean;
        const Vector3 eFwd(-std::sin(en.yaw), 0.0f, -std::cos(en.yaw)); // 後ろ方向
        const float eAtk = (en.state == Action::EnemyStateType::Attack)
                         ? (1.0f - Utility::MathEx::Clamp(en.stateTimer / kEnemyAttackWindupVisualSec, 0.0f, 1.0f)) : 0.0f;

        Vector3 toCam = en.position - camPos; toCam.y = 0.0f;
        const float distSq = toCam.LengthSquared();
        const bool midLod  = distSq >= (20.0f * 20.0f);
        const bool lowLod  = distSq >= (28.0f * 28.0f);

        DrawShadow(en.position, 0.42f, 0.60f, lowLod ? 0.07f : 0.11f);

        const Matrix eRoot =
            Matrix::CreateScale(eScale * (1.0f + hitB * 0.10f), eScale * (1.0f - hitB * 0.15f), eScale * (1.0f + hitB * 0.10f)) *
            Matrix::CreateRotationY(en.yaw + stagLean + eAtk * 0.08f) *
            Matrix::CreateTranslation(en.position + eFwd * hitB * 0.26f + Vector3(0.0f, breath + hitB * 0.07f, 0.0f));

        // 胴体
        m_enemyMesh->Draw(
            Matrix::CreateScale(0.44f, 0.62f, 0.44f) * Matrix::CreateRotationX(eAtk * 0.36f) *
            Matrix::CreateTranslation(0.0f, 1.0f, eAtk * 0.15f) * eRoot,
            m_view, m_proj, ev.armorLight);

        // 頭部 + 目
        if (!lowLod)
        {
            m_enemyMesh->Draw(
                Matrix::CreateScale(0.20f) * Matrix::CreateTranslation(0.0f, 1.68f, 0.0f) * eRoot,
                m_view, m_proj, ev.armorLight);
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.09f, 0.018f, 0.028f) * Matrix::CreateTranslation(0.0f, 1.695f, 0.196f) * eRoot,
                m_view, m_proj, ev.emissiveColor);
        }

        if (!midLod)
        {
            // 胸甲・肩甲
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.34f, 0.18f, 0.14f) * Matrix::CreateRotationX(eAtk * 0.18f) *
                Matrix::CreateTranslation(0.0f, 1.08f, 0.18f) * eRoot,
                m_view, m_proj, ev.armorDark);
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.16f, 0.10f, 0.18f) * Matrix::CreateRotationZ(-0.24f) *
                Matrix::CreateTranslation(-0.32f, 1.27f, 0.0f) * eRoot,
                m_view, m_proj, ev.trimColor);
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.16f, 0.10f, 0.18f) * Matrix::CreateRotationZ(0.24f) *
                Matrix::CreateTranslation(0.32f, 1.27f, 0.02f) * eRoot,
                m_view, m_proj, ev.trimColor);

            // 腕・脚アニメ
            const bool mov  = en.state == Action::EnemyStateType::Wander
                           || en.state == Action::EnemyStateType::Chase
                           || en.state == Action::EnemyStateType::Return;
            const float loco   = mov ? 1.0f : 0.0f;
            const float eArmP  = std::sinf(m_sceneTime * (4.6f + loco * 2.2f) + static_cast<float>(i)) * (0.12f + loco * 0.22f);
            const float eLegS  = std::sinf(m_sceneTime * (3.0f + loco * 4.2f) + static_cast<float>(i) * 0.9f) * (0.08f + loco * 0.24f);
            const float flankB = (en.archetype == Action::EnemyArchetype::BladeFlank) ? 0.16f : 0.0f;

            m_playerMesh->Draw(
                Matrix::CreateScale(0.14f, 0.48f, 0.14f) *
                Matrix::CreateRotationX(Lf(0.08f + loco * 0.08f, 0.10f + flankB, eAtk)) *
                Matrix::CreateRotationZ(Lf(-0.04f + eArmP * 0.65f, -0.34f - flankB + eArmP * 1.1f, eAtk)) *
                Matrix::CreateTranslation(Lv(Vector3(-0.30f, 1.02f, -0.06f), Vector3(-0.4f, 1.06f, 0.0f), eAtk)) *
                eRoot, m_view, m_proj, ev.underColor);
            m_playerMesh->Draw(
                Matrix::CreateScale(0.14f, 0.48f, 0.14f) *
                Matrix::CreateRotationX(Lf(0.54f - loco * 0.06f, -0.18f - eAtk * 0.9f, eAtk)) *
                Matrix::CreateRotationZ(Lf(0.02f - eArmP * 0.55f, 0.30f - eArmP * 1.05f, eAtk)) *
                Matrix::CreateTranslation(Lv(Vector3(0.24f, 1.0f, -0.10f), Vector3(0.4f, 1.06f, eAtk * 0.2f), eAtk)) *
                eRoot, m_view, m_proj, ev.underColor);

            m_playerMesh->Draw(
                Matrix::CreateScale(0.14f, 0.55f, 0.16f) * Matrix::CreateRotationX(eLegS) *
                Matrix::CreateTranslation(-0.16f, 0.32f, 0.0f) * eRoot,
                m_view, m_proj, ev.underColor);
            m_playerMesh->Draw(
                Matrix::CreateScale(0.14f, 0.55f, 0.16f) * Matrix::CreateRotationX(-eLegS) *
                Matrix::CreateTranslation(0.16f, 0.32f, 0.0f) * eRoot,
                m_view, m_proj, ev.underColor);

            // 敵の武器（薄い刃身で描画）
            const float ebp   = Lf(0.84f, -0.4f - eAtk * 0.8f, eAtk);
            const float eby   = Lf(-0.44f, 0.0f, eAtk);
            const Vector3 ebo = Lv(Vector3(0.22f, 0.82f, -0.10f), Vector3(0.58f, 0.96f, 0.18f + eAtk * 0.18f), eAtk);

            const Matrix eBRot =
                Matrix::CreateRotationY(eby) *
                Matrix::CreateRotationX(ebp) *
                Matrix::CreateRotationY(en.yaw);
            Vector3 ebDir = Vector3::TransformNormal(Vector3::UnitY, eBRot); ebDir.Normalize();
            Vector3 ebRight = Vector3::TransformNormal(Vector3::UnitX, eBRot); ebRight.Normalize();
            const Vector3 ebPos = Vector3::Transform(ebo, eRoot);

            // 刃身
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.11f, 0.58f, 0.014f) * eBRot * Matrix::CreateTranslation(ebPos),
                m_view, m_proj, ev.weaponColor);
            // 刃縁
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.007f, 0.56f, 0.017f) * eBRot * Matrix::CreateTranslation(ebPos + ebRight * 0.055f),
                m_view, m_proj, kBladeEdgeColor);
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.007f, 0.56f, 0.017f) * eBRot * Matrix::CreateTranslation(ebPos - ebRight * 0.055f),
                m_view, m_proj, kBladeEdgeColor);
            // 刃先
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.04f, 0.07f, 0.011f) * eBRot * Matrix::CreateTranslation(ebPos + ebDir * 0.32f),
                m_view, m_proj, kBladeEdgeColor);
            // 鍔
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.26f, 0.028f, 0.075f) * eBRot * Matrix::CreateTranslation(ebPos - ebDir * 0.30f),
                m_view, m_proj, ev.trimColor);
            // 柄
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.040f, 0.18f, 0.052f) * eBRot * Matrix::CreateTranslation(ebPos - ebDir * 0.42f),
                m_view, m_proj, kGripColor);

            // 攻撃予備動作キュー
            if (eAtk > 0.05f)
            {
                const Vector3 eFwd2(std::sin(en.yaw), 0.0f, std::cos(en.yaw));
                m_effectTrailMesh->Draw(
                    Matrix::CreateScale(0.22f + eAtk * 0.08f, 0.03f, 1.12f + eAtk * 0.34f) *
                    Matrix::CreateRotationZ(0.52f) * Matrix::CreateRotationX(-0.22f) *
                    Matrix::CreateRotationY(en.yaw + 0.04f) *
                    Matrix::CreateTranslation(en.position + eFwd2 * (0.82f + eAtk * 0.18f) + Vector3(0.0f, 1.02f, 0.0f)),
                    m_view, m_proj,
                    Color(1.0f, 0.30f, 0.18f, 0.16f + eAtk * 0.30f));
            }
        }

        // 被弾カットライン
        if (!lowLod && hitB > 0.01f)
        {
            const Vector3 eFwd2(std::sin(en.yaw), 0.0f, std::cos(en.yaw));
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.54f + hitB * 0.14f, 0.028f, 0.12f) *
                Matrix::CreateRotationZ(0.78f) *
                Matrix::CreateRotationY(en.yaw + 0.12f) *
                Matrix::CreateTranslation(en.position + Vector3(0.0f, 1.04f + hitB * 0.04f, 0.0f)),
                m_view, m_proj,
                Color(0.26f, 0.02f, 0.03f, 0.24f + hitB * 0.34f));
        }

        // HP バー（ビルボード）
        if (!lowLod)
        {
            const float hpR = Utility::MathEx::Clamp(en.hp / std::max(1.0f, en.maxHp), 0.0f, 1.0f);
            const Vector3 hpA = en.position + Vector3(0.0f, 2.08f, 0.0f);
            Vector3 tc = camPos - hpA; tc.y = 0.0f;
            if (tc.LengthSquared() < 0.0001f) tc = Vector3::UnitZ; else tc.Normalize();
            const float hpYaw = std::atan2(tc.x, tc.z);

            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.84f, 0.05f, 0.10f) * Matrix::CreateRotationY(hpYaw) * Matrix::CreateTranslation(hpA),
                m_view, m_proj, Color(0.08f, 0.08f, 0.10f, 0.92f));
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.82f * hpR, 0.036f, 0.08f) *
                Matrix::CreateTranslation((hpR - 1.0f) * 0.41f, 0.0f, 0.0f) *
                Matrix::CreateRotationY(hpYaw) * Matrix::CreateTranslation(hpA + Vector3(0.0f, 0.0f, -0.002f)),
                m_view, m_proj,
                Color(0.24f + (1.0f - hpR) * 0.72f, 0.86f * hpR + 0.16f, 0.2f, 0.95f));
        }

        // デバッグ: A* 経路
        if (m_showPathDebug && !lowLod && !en.path.empty())
        {
            for (size_t c = en.pathCursor; c < en.path.size(); ++c)
            {
                const Vector3 p = m_grid.GridToWorld(en.path[c], 0.05f);
                m_debugCellMesh->Draw(
                    Matrix::CreateScale(0.2f, 0.05f, 0.2f) * Matrix::CreateTranslation(p),
                    m_view, m_proj, DirectX::Colors::Orange);
            }
        }
    }

    // 斬撃ヒットエフェクト
    if (m_effectTrailMesh && m_effectOrbMesh)
    {
        m_slashHitEffects.Draw(*m_effectTrailMesh, *m_effectOrbMesh, m_view, m_proj);
    }
}
