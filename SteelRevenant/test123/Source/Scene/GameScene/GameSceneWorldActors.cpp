//------------------------//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 繝励Ξ繧､繝､繝ｼ繝ｻ謨ｵ繝ｻ繝ｭ繝・け繧ｪ繝ｳ繝ｪ繝ｳ繧ｰ縺ｮ謠冗判繧貞ｮ溯｣・☆繧九・// Bug#2菫ｮ豁｣: DirectX 蟾ｦ謇句ｺｧ讓咏ｳｻ縺ｧ縺ｯ RotationX 縺ｮ豁｣蛟､縺悟ｾ梧婿縺ｸ蛯ｾ縺上・//           荳ｭ谿ｵ讒九∴縺ｫ縺ｯ bladePitch = -0.55f (雋蛟､) 縺梧ｭ｣縺励＞縲・//           菫ｮ豁｣蜑・ +0.55f 竊・蜑｣縺悟ｾ梧婿繧貞髄縺・//           菫ｮ豁｣蠕・ -0.55f 竊・蜑｣縺悟燕譁ｹ(豁｣髱｢)繧貞髄縺・//------------------------//------------------------
#include "GameSceneWorldActors.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"
#include <cmath>

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Color;

void GameSceneWorldActors::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_box      = DirectX::GeometricPrimitive::CreateBox(context, {0.32f,1.6f,0.32f});
    m_sphere   = DirectX::GeometricPrimitive::CreateSphere(context, 0.36f, 10);
    m_cylinder = DirectX::GeometricPrimitive::CreateCylinder(context, 1.0f, 0.08f, 8);
    m_torus    = DirectX::GeometricPrimitive::CreateTorus(context, 1.6f, 0.08f, 24);
    m_capsule  = DirectX::GeometricPrimitive::CreateCylinder(context, 1.6f, 0.35f, 12);
    (void)device;
}

void GameSceneWorldActors::RenderPlayer(
    ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj,
    const Action::PlayerState& player, bool isHitFlash, float hitFlashT)
{
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    const Matrix rotY    = Matrix::CreateRotationY(player.yaw);
    const Matrix bodyW   = rotY * Matrix::CreateTranslation(player.position);
    const Color  bodyCol = isHitFlash
        ? Color::Lerp(playerBodyColor, hitFlashColor, hitFlashT) : playerBodyColor;

    if (m_box)    m_box->Draw(bodyW, view, proj, bodyCol);
    if (m_sphere) m_sphere->Draw(Matrix::CreateTranslation(0,1.1f,0)*bodyW, view, proj, bodyCol);

    if (player.guarding && m_capsule)
    {
        System::DrawManager::GetInstance().ApplyAlphaBlendState();
        m_capsule->Draw(Matrix::CreateScale(1.4f)*bodyW, view, proj,
            Color(playerGuardColor.R(),playerGuardColor.G(),playerGuardColor.B(),0.35f));
        System::DrawManager::GetInstance().ApplyPrimitiveState();
    }
    DrawSword(context, view, proj, bodyW, player);
}

void GameSceneWorldActors::DrawSword(
    ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj,
    const Matrix& bodyWorld, const Action::PlayerState& player)
{
    if (!m_cylinder) return;

    // Bug#2菫ｮ豁｣: 荳ｭ谿ｵ讒九∴ = bladePitch -0.55f (蟾ｦ謇句ｺｧ讓咏ｳｻ縺ｧ蜑肴婿蜷代″)
    float swingPitch = -0.55f, swingRoll = 0.15f;

    switch (player.attackPhase)
    {
    case Action::PlayerAttackPhase::Windup:
        swingPitch = -0.55f + player.attackPhaseBlend * (-1.2f + 0.55f);
        swingRoll  =  0.15f + player.attackPhaseBlend *  0.25f; break;
    case Action::PlayerAttackPhase::Active:
        swingPitch = -1.2f  + player.attackPhaseBlend *  1.5f;
        swingRoll  =  0.40f + player.attackPhaseBlend * -0.50f; break;
    case Action::PlayerAttackPhase::FollowThrough:
        swingPitch =  0.30f + player.attackPhaseBlend * -0.50f;
        swingRoll  = -0.10f; break;
    case Action::PlayerAttackPhase::Recover:
        swingPitch = -0.20f + player.attackPhaseBlend * -0.35f;
        swingRoll  = -0.10f + player.attackPhaseBlend *  0.25f; break;
    default: break;
    }

    const Matrix swordLocal =
        Matrix::CreateRotationX(swingPitch)
        * Matrix::CreateRotationZ(swingRoll)
        * Matrix::CreateTranslation(0.35f, 0.5f, 0.55f);

    const Color swordCol = (player.attackPhase == Action::PlayerAttackPhase::Active)
        ? Color(1.0f, 0.95f, 0.6f, 1.0f) : playerSwordColor;

    m_cylinder->Draw(swordLocal * bodyWorld, view, proj, swordCol);
    (void)context;
}

void GameSceneWorldActors::RenderEnemies(
    ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj,
    const std::vector<Action::EnemyState>& enemies, int lockIdx)
{
    if (!m_box || !m_sphere) return;
    System::DrawManager::GetInstance().ApplyPrimitiveState();

    for (size_t i = 0; i < enemies.size(); ++i)
    {
        const Action::EnemyState& e = enemies[i];
        if (e.state == Action::EnemyStateType::Dead)
        {
            m_box->Draw(Matrix::CreateScale(1,0.15f,1)*Matrix::CreateTranslation(e.position.x,0.1f,e.position.z), view, proj, enemyDeadColor);
            continue;
        }
        const Color base = (e.archetype == Action::EnemyArchetype::BladeRush) ? enemyRushColor : enemyFlankColor;
        const Color col  = (e.hitReactTimer > 0.0f)
            ? Color::Lerp(base, enemyHitColor, std::min(1.0f, e.hitReactTimer/0.26f)) : base;

        const Matrix bW = Matrix::CreateRotationY(e.yaw)*Matrix::CreateTranslation(e.position);
        m_box->Draw(bW, view, proj, col);
        m_sphere->Draw(Matrix::CreateTranslation(0,1.1f,0)*bW, view, proj, col);

        if (static_cast<int>(i) == lockIdx && m_sphere)
        {
            System::DrawManager::GetInstance().ApplyAlphaBlendState();
            m_sphere->Draw(Matrix::CreateScale(0.25f)*Matrix::CreateTranslation(e.position.x,e.position.y+2,e.position.z), view, proj, lockOnRingColor);
            System::DrawManager::GetInstance().ApplyPrimitiveState();
        }
    }
    (void)context;
}

void GameSceneWorldActors::RenderLockOnRing(
    ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj,
    const Vector3& pos, float pulse)
{
    if (!m_torus) return;
    const float sc = 1.0f + 0.12f * std::sin(pulse * 6.28f);
    const Matrix w = Matrix::CreateScale(sc)
        * Matrix::CreateRotationX(DirectX::XM_PIDIV2)
        * Matrix::CreateTranslation(pos.x, pos.y+0.8f, pos.z);
    System::DrawManager::GetInstance().ApplyAlphaBlendState();
    m_torus->Draw(w, view, proj, Color(lockOnRingColor.R(),lockOnRingColor.G(),lockOnRingColor.B(),0.75f));
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    (void)context;
}

