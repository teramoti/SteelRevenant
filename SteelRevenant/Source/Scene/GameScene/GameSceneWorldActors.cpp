//------------------------//------------------------
// Contents(処理内容) プレイヤー・敵・ロックオンリングのプリミティブ描画を実装する。
//------------------------//------------------------
// Bug#2修正: DirectX 左手座標系では RotationX の正値が「後方へ傾く」方向。
//           中段構えを作るには bladePitch を負値 (-0.55f) にする必要があった。
//           修正前: bladePitch = +0.55f → 剣が後方を向く
//           修正後: bladePitch = -0.55f → 剣が正面前方を向く
//------------------------//------------------------
#include "GameSceneWorldActors.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"

#include <cmath>

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Quaternion;

void GameSceneWorldActors::Initialize(
    ID3D11Device*        device,
    ID3D11DeviceContext* context)
{
    m_capsule  = DirectX::GeometricPrimitive::CreateCylinder(context, 1.6f, 0.35f, 12);
    m_box      = DirectX::GeometricPrimitive::CreateBox(context, {0.32f, 1.6f, 0.32f});
    m_sphere   = DirectX::GeometricPrimitive::CreateSphere(context, 0.36f, 10);
    m_cylinder = DirectX::GeometricPrimitive::CreateCylinder(context, 1.0f, 0.08f, 8);
    m_torus    = DirectX::GeometricPrimitive::CreateTorus(context, 1.6f, 0.08f, 24);
    (void)device;
}

// プレイヤー本体と剣を描画する。
void GameSceneWorldActors::RenderPlayer(
    ID3D11DeviceContext*  context,
    const Matrix&         view,
    const Matrix&         projection,
    const Action::PlayerState& player,
    bool                  isHitFlash,
    float                 hitFlashT)
{
    System::DrawManager::GetInstance().ApplyPrimitiveState();

    const float yaw = player.yaw;
    const Matrix rotY = Matrix::CreateRotationY(yaw);

    // 胴体
    const Matrix bodyWorld = Matrix::CreateScale(1.0f, 1.0f, 1.0f)
        * rotY
        * Matrix::CreateTranslation(player.position);

    const Color bodyCol = isHitFlash
        ? Color::Lerp(playerBodyColor, hitFlashColor, hitFlashT)
        : playerBodyColor;

    if (m_box)     m_box->Draw(bodyWorld, view, projection, bodyCol);

    // 頭
    const Matrix headWorld = Matrix::CreateScale(1.0f, 1.0f, 1.0f)
        * Matrix::CreateTranslation(0.0f, 1.1f, 0.0f)
        * rotY
        * Matrix::CreateTranslation(player.position);
    if (m_sphere)  m_sphere->Draw(headWorld, view, projection, bodyCol);

    // ガード発光
    if (player.guarding && m_capsule)
    {
        const Matrix guardWorld = Matrix::CreateScale(1.4f, 1.4f, 1.4f)
            * rotY
            * Matrix::CreateTranslation(player.position);
        System::DrawManager::GetInstance().ApplyAlphaBlendState();
        m_capsule->Draw(guardWorld, view, projection,
            Color(playerGuardColor.R(), playerGuardColor.G(), playerGuardColor.B(), 0.35f));
        System::DrawManager::GetInstance().ApplyPrimitiveState();
    }

    DrawSword(context, view, projection, bodyWorld, player);
}

// 剣を中段構えで描画する。
// Bug#2修正: bladePitch = -0.55f (負値が前方向き)
void GameSceneWorldActors::DrawSword(
    ID3D11DeviceContext*        context,
    const Matrix&               view,
    const Matrix&               projection,
    const Matrix&               bodyWorld,
    const Action::PlayerState&  player)
{
    if (!m_cylinder) return;

    // -----------------------------------------------------------
    // 攻撃フェーズによるスウィングアニメーション
    // -----------------------------------------------------------
    float swingPitch = 0.0f;
    float swingRoll  = 0.0f;

    switch (player.attackPhase)
    {
    case Action::PlayerAttackPhase::Windup:
        swingPitch = -0.55f + player.attackPhaseBlend * (-1.2f - (-0.55f));
        swingRoll  =  0.15f + player.attackPhaseBlend * ( 0.40f -   0.15f);
        break;
    case Action::PlayerAttackPhase::Active:
        swingPitch = -1.2f  + player.attackPhaseBlend * ( 0.30f -  (-1.2f));
        swingRoll  =  0.40f + player.attackPhaseBlend * (-0.10f -    0.40f);
        break;
    case Action::PlayerAttackPhase::FollowThrough:
        swingPitch =  0.30f + player.attackPhaseBlend * (-0.20f -    0.30f);
        swingRoll  = -0.10f;
        break;
    case Action::PlayerAttackPhase::Recover:
        swingPitch = -0.20f + player.attackPhaseBlend * (-0.55f -  (-0.20f));
        swingRoll  = -0.10f + player.attackPhaseBlend * ( 0.15f -  (-0.10f));
        break;
    default:
        // -----------------------------------------------------------
        // guardPose: 中段構え
        // Bug#2修正: bladePitch を -0.55f (左手系で前方)
        //           修正前の +0.55f は後方を向いていた
        // -----------------------------------------------------------
        swingPitch = -0.55f;
        swingRoll  =  0.15f;
        break;
    }

    // 剣の変換: 体ローカルで右肩前方に配置してからスウィング回転を掛ける
    const Matrix swordLocal =
        Matrix::CreateRotationX(swingPitch)
        * Matrix::CreateRotationZ(swingRoll)
        * Matrix::CreateTranslation(0.35f, 0.5f, 0.55f);   // 右手前方

    const Matrix swordWorld = swordLocal * bodyWorld;

    const Color swordCol = (player.attackPhase == Action::PlayerAttackPhase::Active)
        ? Color(1.0f, 0.95f, 0.6f, 1.0f)   // 攻撃中: 黄白発光
        : playerSwordColor;

    m_cylinder->Draw(swordWorld, view, projection, swordCol);
    (void)context;
}

// 敵一覧を描画する。
void GameSceneWorldActors::RenderEnemies(
    ID3D11DeviceContext*                    context,
    const Matrix&                           view,
    const Matrix&                           projection,
    const std::vector<Action::EnemyState>&  enemies,
    int                                     lockTargetIndex)
{
    if (!m_box || !m_sphere) return;

    System::DrawManager::GetInstance().ApplyPrimitiveState();

    for (size_t i = 0; i < enemies.size(); ++i)
    {
        const Action::EnemyState& e = enemies[i];

        if (e.state == Action::EnemyStateType::Dead)
        {
            // 死亡エフェクト: 低く潰れる
            const Matrix deathWorld = Matrix::CreateScale(1.0f, 0.15f, 1.0f)
                * Matrix::CreateTranslation(e.position.x, 0.1f, e.position.z);
            m_box->Draw(deathWorld, view, projection, enemyDeadColor);
            continue;
        }

        const Color baseCol = (e.archetype == Action::EnemyArchetype::BladeRush)
            ? enemyRushColor
            : enemyFlankColor;

        const Color drawCol = (e.hitReactTimer > 0.0f)
            ? Color::Lerp(baseCol, enemyHitColor,
                std::min(1.0f, e.hitReactTimer / 0.26f))
            : baseCol;

        const Matrix rotY    = Matrix::CreateRotationY(e.yaw);
        const Matrix bodyW   = rotY * Matrix::CreateTranslation(e.position);
        const Matrix headW   = Matrix::CreateTranslation(0.0f, 1.1f, 0.0f) * bodyW;

        m_box->Draw(bodyW, view, projection, drawCol);
        m_sphere->Draw(headW, view, projection, drawCol);

        // ロックオン対象: 頭上に小さい球
        if (static_cast<int>(i) == lockTargetIndex && m_sphere)
        {
            const Matrix indicatorW = Matrix::CreateScale(0.25f)
                * Matrix::CreateTranslation(e.position.x, e.position.y + 2.0f, e.position.z);
            System::DrawManager::GetInstance().ApplyAlphaBlendState();
            m_sphere->Draw(indicatorW, view, projection, lockOnRingColor);
            System::DrawManager::GetInstance().ApplyPrimitiveState();
        }
    }
    (void)context;
}

// ロックオンリング描画（トーラスで目標を囲む）。
void GameSceneWorldActors::RenderLockOnRing(
    ID3D11DeviceContext*  context,
    const Matrix&         view,
    const Matrix&         projection,
    const Vector3&        targetPos,
    float                 ringPulse)
{
    if (!m_torus) return;

    const float scale = 1.0f + 0.12f * std::sin(ringPulse * 6.28f);
    const Matrix ringWorld =
        Matrix::CreateScale(scale)
        * Matrix::CreateRotationX(DirectX::XM_PIDIV2)
        * Matrix::CreateTranslation(targetPos.x, targetPos.y + 0.8f, targetPos.z);

    System::DrawManager::GetInstance().ApplyAlphaBlendState();
    m_torus->Draw(ringWorld, view, projection,
        Color(lockOnRingColor.R(), lockOnRingColor.G(), lockOnRingColor.B(), 0.75f));
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    (void)context;
}
