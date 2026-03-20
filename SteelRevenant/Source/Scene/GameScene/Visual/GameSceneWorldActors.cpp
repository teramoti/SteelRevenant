//------------------------//------------------------
// Contents(処理内容) 本編ゲームシーンのプレイヤー・敵・弾・エフェクト描画を分離実装する。
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
	constexpr float kAttackVisualDurationSec = 0.18f;
	constexpr float kWeaponSwingYawRad = 1.45f;
	constexpr float kWeaponSwingPitchRad = 1.15f;
	constexpr float kPlayerRunCycleSpeed = 7.6f;
	constexpr float kEnemyBreathSpeed = 4.8f;
	constexpr float kEnemySpawnDurationSec = 0.4f;
	constexpr float kEnemyAttackWindupVisualSec = 0.45f;
}

void GameScene::DrawWorldActors()
{
	const SceneFx::StageRenderPalette palette = SceneFx::BuildStageRenderPalette(m_stageThemeIndex, m_sceneTime);
	const auto StageTint = [&palette](const Color& base, float alphaScale = 1.0f) -> Color
	{
		return SceneFx::ApplyStageTint(palette, base, alphaScale);
	};
	const auto DrawContactShadow = [&](const Vector3& position, float scaleX, float scaleZ, float alpha)
	{
		const Matrix shadowWorld =
			Matrix::CreateScale(scaleX, 0.05f, scaleZ) *
			Matrix::CreateTranslation(position.x, 0.055f, position.z);
		m_effectOrbMesh->Draw(shadowWorld, m_view, m_proj, StageTint(Color(0.01f, 0.015f, 0.02f, alpha)));
	};
	const auto LerpFloat = [](float a, float b, float t)
	{
		const float clampedT = Utility::MathEx::Clamp(t, 0.0f, 1.0f);
		return a + (b - a) * clampedT;
	};
	const auto LerpVector = [&LerpFloat](const Vector3& a, const Vector3& b, float t)
	{
		return Vector3(
			LerpFloat(a.x, b.x, t),
			LerpFloat(a.y, b.y, t),
			LerpFloat(a.z, b.z, t));
	};

	const float attackBlend = GetAttackBlend();
	const float carryBlend = Utility::MathEx::Clamp(attackBlend, 0.0f, 1.0f);
	const float comboSign = (m_player.comboIndex % 2 == 0) ? -1.0f : 1.0f;
	const float runCycle = std::sinf(m_sceneTime * kPlayerRunCycleSpeed);
	const float legSwing = runCycle * 0.26f;
	const float armPulse = runCycle * 0.18f;
	const SceneFx::PlayerVisualProfile playerVisual = SceneFx::BuildPlayerVisualProfile(m_player, m_sceneTime);
	const Color playerColor = playerVisual.armorLight;
	DrawContactShadow(m_player.position, 0.56f, 0.78f, 0.18f);

	const Matrix playerRoot =
		Matrix::CreateRotationY(m_player.yaw + attackBlend * comboSign * 0.08f) *
		Matrix::CreateTranslation(m_player.position + Vector3(0.0f, std::sinf(m_sceneTime * 4.4f) * 0.04f, 0.0f));

	const Matrix playerBodyWorld =
		Matrix::CreateScale(0.44f, 0.62f, 0.44f) *
		Matrix::CreateRotationX(attackBlend * 0.3f) *
		Matrix::CreateTranslation(0.0f, 1.0f, attackBlend * 0.12f) *
		playerRoot;
	m_enemyMesh->Draw(playerBodyWorld, m_view, m_proj, playerColor);

	const Matrix playerHeadWorld =
		Matrix::CreateScale(0.2f) *
		Matrix::CreateTranslation(0.0f, 1.68f, 0.0f) *
		playerRoot;
	m_enemyMesh->Draw(playerHeadWorld, m_view, m_proj, playerVisual.armorLight);

	const Matrix playerFacePlateWorld =
		Matrix::CreateScale(0.12f, 0.06f, 0.03f) *
		Matrix::CreateTranslation(0.0f, 1.7f, 0.2f) *
		playerRoot;
	m_obstacleMesh->Draw(playerFacePlateWorld, m_view, m_proj, playerVisual.accentColor);
	const Matrix playerVisorWorld =
		Matrix::CreateScale(0.07f, 0.03f, 0.032f) *
		Matrix::CreateTranslation(0.0f, 1.7f, 0.215f) *
		playerRoot;
	m_obstacleMesh->Draw(playerVisorWorld, m_view, m_proj, playerVisual.emissiveColor);

	const Matrix playerBackPlateWorld =
		Matrix::CreateScale(0.08f, 0.045f, 0.025f) *
		Matrix::CreateTranslation(0.0f, 1.66f, -0.19f) *
		playerRoot;
	m_obstacleMesh->Draw(playerBackPlateWorld, m_view, m_proj, playerVisual.armorDark);

	const Matrix playerChestPlateWorld =
		Matrix::CreateScale(0.36f, 0.2f, 0.14f) *
		Matrix::CreateRotationX(attackBlend * 0.16f) *
		Matrix::CreateTranslation(0.0f, 1.08f, 0.18f) *
		playerRoot;
	const Matrix playerCoreWorld =
		Matrix::CreateScale(0.09f, 0.14f, 0.032f) *
		Matrix::CreateTranslation(0.0f, 1.08f, 0.27f) *
		playerRoot;
	const Matrix playerShoulderLeftWorld =
		Matrix::CreateScale(0.16f, 0.10f, 0.18f) *
		Matrix::CreateRotationZ(-0.24f) *
		Matrix::CreateTranslation(-0.32f, 1.28f, 0.0f) *
		playerRoot;
	const Matrix playerShoulderRightWorld =
		Matrix::CreateScale(0.16f, 0.10f, 0.18f) *
		Matrix::CreateRotationZ(0.24f) *
		Matrix::CreateTranslation(0.32f, 1.28f, 0.02f) *
		playerRoot;
	const Matrix playerHipLeftWorld =
		Matrix::CreateScale(0.1f, 0.18f, 0.08f) *
		Matrix::CreateRotationZ(-0.1f) *
		Matrix::CreateTranslation(-0.18f, 0.64f, 0.07f) *
		playerRoot;
	const Matrix playerHipRightWorld =
		Matrix::CreateScale(0.1f, 0.18f, 0.08f) *
		Matrix::CreateRotationZ(0.1f) *
		Matrix::CreateTranslation(0.18f, 0.64f, 0.07f) *
		playerRoot;
	m_obstacleMesh->Draw(playerChestPlateWorld, m_view, m_proj, playerVisual.armorDark);
	m_obstacleMesh->Draw(playerCoreWorld, m_view, m_proj, playerVisual.emissiveColor);
	m_obstacleMesh->Draw(playerShoulderLeftWorld, m_view, m_proj, playerVisual.trimColor);
	m_obstacleMesh->Draw(playerShoulderRightWorld, m_view, m_proj, playerVisual.trimColor);
	m_obstacleMesh->Draw(playerHipLeftWorld, m_view, m_proj, playerVisual.armorDark);
	m_obstacleMesh->Draw(playerHipRightWorld, m_view, m_proj, playerVisual.armorDark);

	const Matrix playerArmLeftWorld =
		Matrix::CreateScale(0.14f, 0.48f, 0.14f) *
		Matrix::CreateRotationZ(LerpFloat(-0.08f + armPulse * 0.35f, -0.2f + armPulse, carryBlend)) *
		Matrix::CreateTranslation(LerpVector(Vector3(-0.32f, 1.04f, -0.04f), Vector3(-0.4f, 1.06f, 0.0f), carryBlend)) *
		playerRoot;
	const Vector3 playerRightArmOffset = LerpVector(
		Vector3(0.24f, 1.0f, -0.10f),
		Vector3(0.34f, 1.02f, 0.06f + attackBlend * 0.10f),
		carryBlend);
	const Matrix playerArmRightWorld =
		Matrix::CreateScale(0.14f, 0.50f, 0.14f) *
		Matrix::CreateRotationX(LerpFloat(0.58f, -0.24f - attackBlend * 0.82f, carryBlend)) *
		Matrix::CreateRotationZ(LerpFloat(0.08f + armPulse * 0.25f, 0.28f - armPulse + comboSign * attackBlend * 0.42f, carryBlend)) *
		Matrix::CreateTranslation(playerRightArmOffset) *
		playerRoot;
	m_playerMesh->Draw(playerArmLeftWorld, m_view, m_proj, playerVisual.underColor);
	m_playerMesh->Draw(playerArmRightWorld, m_view, m_proj, playerVisual.underColor);

	const Matrix playerLegLeftWorld =
		Matrix::CreateScale(0.14f, 0.55f, 0.16f) *
		Matrix::CreateRotationX(legSwing) *
		Matrix::CreateTranslation(-0.16f, 0.32f, 0.0f) *
		playerRoot;
	const Matrix playerLegRightWorld =
		Matrix::CreateScale(0.14f, 0.55f, 0.16f) *
		Matrix::CreateRotationX(-legSwing) *
		Matrix::CreateTranslation(0.16f, 0.32f, 0.0f) *
		playerRoot;
	m_playerMesh->Draw(playerLegLeftWorld, m_view, m_proj, playerVisual.underColor);
	m_playerMesh->Draw(playerLegRightWorld, m_view, m_proj, playerVisual.underColor);
	const Matrix playerShinLeftWorld =
		Matrix::CreateScale(0.1f, 0.22f, 0.08f) *
		Matrix::CreateTranslation(-0.16f, 0.16f, 0.13f) *
		playerRoot;
	const Matrix playerShinRightWorld =
		Matrix::CreateScale(0.1f, 0.22f, 0.08f) *
		Matrix::CreateTranslation(0.16f, 0.16f, 0.13f) *
		playerRoot;
	m_obstacleMesh->Draw(playerShinLeftWorld, m_view, m_proj, playerVisual.armorDark);
	m_obstacleMesh->Draw(playerShinRightWorld, m_view, m_proj, playerVisual.armorDark);

	const Vector3 forward(std::sin(m_player.yaw), 0.0f, std::cos(m_player.yaw));
	const Vector3 right(forward.z, 0.0f, -forward.x);
	const float swingYaw = comboSign * (attackBlend * 2.0f - 1.0f) * kWeaponSwingYawRad;
	const float swingPitch = -0.52f + std::sinf(attackBlend * DirectX::XM_PI) * (kWeaponSwingPitchRad * 0.86f);
	const float carryYawOffset = LerpFloat(-0.48f, swingYaw, carryBlend);
	const float carryPitch = LerpFloat(0.18f, swingPitch, carryBlend);
	const float carryRoll = LerpFloat(-2.24f, -0.18f + comboSign * attackBlend * 0.10f, carryBlend);
	const Matrix bladeRotation =
		Matrix::CreateRotationZ(carryRoll) *
		Matrix::CreateRotationX(carryPitch) *
		Matrix::CreateRotationY(m_player.yaw + carryYawOffset);
	Vector3 bladeDir = Vector3::TransformNormal(Vector3::UnitY, bladeRotation);
	bladeDir.Normalize();
	const Vector3 gripAnchor = Vector3::Transform(
		LerpVector(
			Vector3(0.22f, 0.80f, -0.12f),
			Vector3(0.36f, 0.78f, 0.16f + attackBlend * 0.03f),
			carryBlend),
		playerRoot);
	const Vector3 weaponCenter =
		gripAnchor +
		bladeDir * LerpFloat(0.42f, 0.54f + attackBlend * 0.06f, carryBlend) +
		forward * LerpFloat(-0.14f, 0.04f + attackBlend * 0.10f, carryBlend) +
		right * LerpFloat(0.14f, 0.02f + comboSign * 0.03f, carryBlend);
	const Matrix weaponWorld =
		bladeRotation *
		Matrix::CreateTranslation(weaponCenter);
	const Color weaponColor = playerVisual.weaponColor;
	m_weaponMesh->Draw(weaponWorld, m_view, m_proj, weaponColor);
	const Matrix weaponSpineWorld =
		Matrix::CreateScale(0.03f, 0.94f, 0.05f) *
		bladeRotation *
		Matrix::CreateTranslation(weaponCenter + bladeDir * 0.02f);
	m_obstacleMesh->Draw(weaponSpineWorld, m_view, m_proj, Color(0.28f, 0.32f, 0.36f, 1.0f));

	const Matrix weaponHandleWorld =
		Matrix::CreateScale(0.07f, 0.24f, 0.08f) *
		bladeRotation *
		Matrix::CreateTranslation(gripAnchor - bladeDir * 0.18f);
	m_obstacleMesh->Draw(weaponHandleWorld, m_view, m_proj, Color(0.18f, 0.14f, 0.12f, 1.0f));

	const Matrix weaponGuardWorld =
		Matrix::CreateScale(0.34f, 0.035f, 0.11f) *
		bladeRotation *
		Matrix::CreateTranslation(gripAnchor + bladeDir * 0.02f);
	m_obstacleMesh->Draw(weaponGuardWorld, m_view, m_proj, playerVisual.accentColor);

	const Matrix weaponPommelWorld =
		Matrix::CreateScale(0.08f, 0.07f, 0.08f) *
		Matrix::CreateTranslation(gripAnchor - bladeDir * 0.33f);
	m_obstacleMesh->Draw(weaponPommelWorld, m_view, m_proj, Color(0.48f, 0.42f, 0.30f, 1.0f));

	const Matrix invView = m_view.Invert();
	const Vector3 cameraPos(invView._41, invView._42, invView._43);

	for (size_t i = 0; i < m_enemies.size(); ++i)
	{
		const Action::EnemyState& enemy = m_enemies[i];
		if (enemy.state == Action::EnemyStateType::Dead)
		{
			continue;
		}

		const SceneFx::EnemyVisualProfile enemyVisual =
		SceneFx::BuildEnemyVisualProfile(enemy, m_gameState.dangerLevel, i, m_sceneTime);
		const Color enemyColor = enemyVisual.armorLight;
		const float enemyBreath = std::sinf(m_sceneTime * kEnemyBreathSpeed + static_cast<float>(i) * 0.78f) * 0.07f;
		const float idleBlend = 1.0f - Utility::MathEx::Clamp(enemy.stateTimer / kEnemySpawnDurationSec, 0.0f, 1.0f);
		const float entryScale = (enemy.state == Action::EnemyStateType::Idle && enemy.stateTimer > 0.0f)
			? (0.35f + idleBlend * 0.65f)
			: 1.0f;
		const float hitStunBlend = (enemy.state == Action::EnemyStateType::Chase && enemy.stateTimer > 0.0f)
			? Utility::MathEx::Clamp(enemy.stateTimer / 0.18f, 0.0f, 1.0f)
			: 0.0f;
		const float returnLean = (enemy.state == Action::EnemyStateType::Return)
			? std::sinf(m_sceneTime * 7.0f + static_cast<float>(i) * 0.6f) * 0.12f
			: 0.0f;
		const float staggerLean = hitStunBlend * std::sinf(enemy.stateTimer * 20.0f + static_cast<float>(i) * 0.6f) * 0.24f + returnLean;
		const Vector3 enemyForward(std::sin(enemy.yaw), 0.0f, std::cos(enemy.yaw));
		const Vector3 enemyBack = -enemyForward;
		const float hitKickBack = hitStunBlend * 0.26f;
		const float hitLift = hitStunBlend * 0.07f;
		const float hitScaleXZ = 1.0f + hitStunBlend * 0.10f;
		const float hitScaleY = 1.0f - hitStunBlend * 0.15f;
		const float enemyAttackBlend = (enemy.state == Action::EnemyStateType::Attack)
			? (1.0f - Utility::MathEx::Clamp(enemy.stateTimer / kEnemyAttackWindupVisualSec, 0.0f, 1.0f))
			: 0.0f;
		Vector3 toCameraDelta = enemy.position - cameraPos;
		toCameraDelta.y = 0.0f;
		const float cameraDistanceSq = toCameraDelta.LengthSquared();
		const bool midVisualLod = cameraDistanceSq >= (20.0f * 20.0f);
		const bool lowVisualLod = cameraDistanceSq >= (28.0f * 28.0f);
		DrawContactShadow(enemy.position, 0.42f, 0.60f, lowVisualLod ? 0.07f : 0.11f);
		const Matrix enemyRoot =
			Matrix::CreateScale(entryScale * hitScaleXZ, entryScale * hitScaleY, entryScale * hitScaleXZ) *
			Matrix::CreateRotationY(enemy.yaw + staggerLean + enemyAttackBlend * 0.08f) *
			Matrix::CreateTranslation(enemy.position + enemyBack * hitKickBack + Vector3(0.0f, enemyBreath + hitLift, 0.0f));

		const Matrix enemyBodyWorld =
			Matrix::CreateScale(0.44f, 0.62f, 0.44f) *
			Matrix::CreateRotationX(enemyAttackBlend * 0.36f) *
			Matrix::CreateTranslation(0.0f, 1.0f, enemyAttackBlend * 0.15f) *
			enemyRoot;
		m_enemyMesh->Draw(enemyBodyWorld, m_view, m_proj, enemyColor);

		const Matrix enemyHeadWorld =
			Matrix::CreateScale(0.2f) *
			Matrix::CreateTranslation(0.0f, 1.68f, 0.0f) *
			enemyRoot;
		if (!lowVisualLod)
		{
			m_enemyMesh->Draw(enemyHeadWorld, m_view, m_proj, enemyVisual.armorLight);
		}

		if (!midVisualLod)
		{
			const Matrix enemyChestPlateWorld =
				Matrix::CreateScale(0.34f, 0.18f, 0.14f) *
				Matrix::CreateRotationX(enemyAttackBlend * 0.18f) *
				Matrix::CreateTranslation(0.0f, 1.08f, 0.18f) *
				enemyRoot;
			const Matrix enemyShoulderLeftWorld =
				Matrix::CreateScale(0.16f, 0.1f, 0.18f) *
				Matrix::CreateRotationZ(-0.24f) *
				Matrix::CreateTranslation(-0.32f, 1.27f, 0.0f) *
				enemyRoot;
			const Matrix enemyShoulderRightWorld =
				Matrix::CreateScale(0.16f, 0.1f, 0.18f) *
				Matrix::CreateRotationZ(0.24f) *
				Matrix::CreateTranslation(0.32f, 1.27f, 0.02f) *
				enemyRoot;
			m_obstacleMesh->Draw(enemyChestPlateWorld, m_view, m_proj, enemyVisual.armorDark);
			m_obstacleMesh->Draw(enemyShoulderLeftWorld, m_view, m_proj, enemyVisual.trimColor);
			m_obstacleMesh->Draw(enemyShoulderRightWorld, m_view, m_proj, enemyVisual.trimColor);
		}

		if (!lowVisualLod && hitStunBlend > 0.01f)
		{
			const Matrix hitCutWorld =
				Matrix::CreateScale(0.54f + hitStunBlend * 0.14f, 0.028f, 0.12f) *
				Matrix::CreateRotationZ(0.78f) *
				Matrix::CreateRotationY(enemy.yaw + 0.12f) *
				Matrix::CreateTranslation(enemy.position + Vector3(0.0f, 1.04f + hitStunBlend * 0.04f, 0.0f));
			m_obstacleMesh->Draw(hitCutWorld, m_view, m_proj, Color(0.26f, 0.02f, 0.03f, 0.24f + hitStunBlend * 0.34f));
		}

		if (!midVisualLod)
		{
			const bool rangedEnemy = (enemy.weaponType == Action::EnemyWeaponType::Ranged);
			const float enemyReadyBlend = (enemy.state == Action::EnemyStateType::Aim)
				? 0.68f
				: enemyAttackBlend;
			const float enemyArmPulse = std::sinf(m_sceneTime * 5.2f + static_cast<float>(i)) * 0.2f;
			const Matrix enemyArmLeftWorld =
				Matrix::CreateScale(0.14f, 0.48f, 0.14f) *
				Matrix::CreateRotationX(LerpFloat(0.08f, rangedEnemy ? -0.18f : 0.0f, enemyReadyBlend)) *
				Matrix::CreateRotationZ(LerpFloat(-0.02f + enemyArmPulse * 0.25f, (rangedEnemy ? -0.08f : -0.2f) + enemyArmPulse * (rangedEnemy ? 0.45f : 1.0f), enemyReadyBlend)) *
				Matrix::CreateTranslation(LerpVector(
					Vector3(-0.30f, 1.02f, -0.06f),
					Vector3(rangedEnemy ? -0.34f : -0.4f, 1.06f, rangedEnemy ? 0.14f : 0.0f),
					enemyReadyBlend)) *
				enemyRoot;
			const Matrix enemyArmRightWorld =
				Matrix::CreateScale(0.14f, 0.48f, 0.14f) *
				Matrix::CreateRotationX(LerpFloat(rangedEnemy ? 0.62f : 0.54f, rangedEnemy ? (-0.42f - enemyAttackBlend * 0.25f) : (-enemyAttackBlend * 0.9f), enemyReadyBlend)) *
				Matrix::CreateRotationZ(LerpFloat(0.04f, rangedEnemy ? 0.08f : (0.2f - enemyArmPulse), enemyReadyBlend)) *
				Matrix::CreateTranslation(LerpVector(
					Vector3(0.24f, 1.0f, -0.10f),
					Vector3(rangedEnemy ? 0.34f : 0.4f, 1.06f, rangedEnemy ? (0.18f + enemyAttackBlend * 0.14f) : (enemyAttackBlend * 0.2f)),
					enemyReadyBlend)) *
				enemyRoot;
			m_playerMesh->Draw(enemyArmLeftWorld, m_view, m_proj, enemyVisual.underColor);
			m_playerMesh->Draw(enemyArmRightWorld, m_view, m_proj, enemyVisual.underColor);

			const float enemyLegSwing = std::sinf(m_sceneTime * 5.8f + static_cast<float>(i) * 0.9f) * 0.26f;
			const Matrix enemyLegLeftWorld =
				Matrix::CreateScale(0.14f, 0.55f, 0.16f) *
				Matrix::CreateRotationX(enemyLegSwing) *
				Matrix::CreateTranslation(-0.16f, 0.32f, 0.0f) *
				enemyRoot;
			const Matrix enemyLegRightWorld =
				Matrix::CreateScale(0.14f, 0.55f, 0.16f) *
				Matrix::CreateRotationX(-enemyLegSwing) *
				Matrix::CreateTranslation(0.16f, 0.32f, 0.0f) *
				enemyRoot;
			m_playerMesh->Draw(enemyLegLeftWorld, m_view, m_proj, enemyVisual.underColor);
			m_playerMesh->Draw(enemyLegRightWorld, m_view, m_proj, enemyVisual.underColor);
			if (rangedEnemy)
			{
				const float gunPitch = LerpFloat(0.82f, -0.18f - enemyAttackBlend * 0.12f, enemyReadyBlend);
				const float gunYaw = LerpFloat(-0.52f, 0.0f, enemyReadyBlend);
				const Vector3 gunBodyOffset = LerpVector(
					Vector3(0.24f, 0.84f, -0.10f),
					Vector3(0.52f, 1.0f, 0.24f + enemyAttackBlend * 0.08f),
					enemyReadyBlend);
				const Vector3 gunBarrelOffset = LerpVector(
					Vector3(0.30f, 0.88f, 0.08f),
					Vector3(0.58f, 1.02f, 0.52f + enemyAttackBlend * 0.16f),
					enemyReadyBlend);
				const Matrix enemyGunBodyWorld =
					Matrix::CreateScale(0.18f, 0.11f, 0.42f) *
					Matrix::CreateRotationY(gunYaw) *
					Matrix::CreateRotationX(gunPitch) *
					Matrix::CreateTranslation(gunBodyOffset) *
					enemyRoot;
				const Matrix enemyGunBarrelWorld =
					Matrix::CreateScale(0.05f, 0.05f, 0.34f) *
					Matrix::CreateRotationY(gunYaw) *
					Matrix::CreateRotationX(gunPitch) *
					Matrix::CreateTranslation(gunBarrelOffset) *
					enemyRoot;
				const Matrix enemyGunCoreWorld =
					Matrix::CreateScale(0.06f, 0.04f, 0.08f) *
					Matrix::CreateTranslation(LerpVector(
						Vector3(0.22f, 0.88f, -0.12f),
						Vector3(0.48f, 1.02f, 0.18f),
						enemyReadyBlend)) *
					enemyRoot;
				m_obstacleMesh->Draw(enemyGunBodyWorld, m_view, m_proj, enemyVisual.weaponColor);
				m_obstacleMesh->Draw(enemyGunBarrelWorld, m_view, m_proj, enemyVisual.weaponColor);
				m_obstacleMesh->Draw(enemyGunCoreWorld, m_view, m_proj, enemyVisual.emissiveColor);
				if (enemyReadyBlend > 0.05f)
				{
					const Vector3 muzzlePos = Vector3::Transform(gunBarrelOffset + Vector3(0.0f, 0.0f, 0.26f), enemyRoot);
					const float chargeAlpha = 0.14f + enemyReadyBlend * 0.42f;
					const Matrix muzzleGlowWorld =
						Matrix::CreateScale(0.12f + enemyReadyBlend * 0.07f, 0.12f + enemyReadyBlend * 0.07f, 0.12f + enemyReadyBlend * 0.07f) *
						Matrix::CreateTranslation(muzzlePos);
					const Matrix muzzleTrailWorld =
						Matrix::CreateScale(0.06f, 0.025f, 0.48f + enemyReadyBlend * 0.26f) *
						Matrix::CreateRotationY(enemy.yaw) *
						Matrix::CreateRotationX(-0.16f) *
						Matrix::CreateTranslation(muzzlePos + enemyForward * (0.20f + enemyReadyBlend * 0.18f));
					m_effectOrbMesh->Draw(muzzleGlowWorld, m_view, m_proj, Color(enemyVisual.emissiveColor.x, enemyVisual.emissiveColor.y, enemyVisual.emissiveColor.z, chargeAlpha));
					m_effectTrailMesh->Draw(muzzleTrailWorld, m_view, m_proj, Color(enemyVisual.emissiveColor.x, enemyVisual.emissiveColor.y, enemyVisual.emissiveColor.z, chargeAlpha * 0.78f));
				}
			}
			else
			{
				const float bladePitch = LerpFloat(0.84f, -0.4f - enemyAttackBlend * 0.8f, enemyReadyBlend);
				const float bladeYaw = LerpFloat(-0.44f, 0.0f, enemyReadyBlend);
				const Vector3 bladeOffset = LerpVector(
					Vector3(0.22f, 0.82f, -0.10f),
					Vector3(0.58f, 0.96f, 0.18f + enemyAttackBlend * 0.18f),
					enemyReadyBlend);
				const Matrix enemyBladeWorld =
					Matrix::CreateScale(0.08f, 0.08f, 0.56f) *
					Matrix::CreateRotationY(bladeYaw) *
					Matrix::CreateRotationX(bladePitch) *
					Matrix::CreateTranslation(bladeOffset) *
					enemyRoot;
				const Matrix enemyBladeHandleWorld =
					Matrix::CreateScale(0.09f, 0.11f, 0.16f) *
					Matrix::CreateTranslation(LerpVector(
						Vector3(0.18f, 0.84f, -0.18f),
						Vector3(0.52f, 0.98f, 0.0f),
						enemyReadyBlend)) *
					enemyRoot;
				m_obstacleMesh->Draw(enemyBladeWorld, m_view, m_proj, enemyVisual.weaponColor);
				m_obstacleMesh->Draw(enemyBladeHandleWorld, m_view, m_proj, enemyVisual.trimColor);
				if (enemyReadyBlend > 0.05f)
				{
					const Matrix meleeCueWorld =
						Matrix::CreateScale(0.22f + enemyReadyBlend * 0.08f, 0.03f, 1.12f + enemyReadyBlend * 0.34f) *
						Matrix::CreateRotationZ(0.52f) *
						Matrix::CreateRotationX(-0.22f) *
						Matrix::CreateRotationY(enemy.yaw + 0.04f) *
						Matrix::CreateTranslation(enemy.position + enemyForward * (0.82f + enemyReadyBlend * 0.18f) + Vector3(0.0f, 1.02f, 0.0f));
					m_effectTrailMesh->Draw(meleeCueWorld, m_view, m_proj, Color(1.0f, 0.30f, 0.18f, 0.16f + enemyReadyBlend * 0.30f));
				}
			}
		}

		if (!lowVisualLod)
		{
			const float enemyMaxHp = std::max(1.0f, enemy.maxHp);
			const float hpRatio = Utility::MathEx::Clamp(enemy.hp / enemyMaxHp, 0.0f, 1.0f);
			const Vector3 hpAnchor = enemy.position + Vector3(0.0f, 2.08f, 0.0f);
			Vector3 toCamera = cameraPos - hpAnchor;
			toCamera.y = 0.0f;
			if (toCamera.LengthSquared() < 0.0001f)
			{
				toCamera = Vector3::UnitZ;
			}
			else
			{
				toCamera.Normalize();
			}
			const float hpYaw = std::atan2(toCamera.x, toCamera.z);
			const Matrix hpBackWorld =
				Matrix::CreateScale(0.84f, 0.05f, 0.1f) *
				Matrix::CreateRotationY(hpYaw) *
				Matrix::CreateTranslation(hpAnchor);
			const Matrix hpFillWorld =
				Matrix::CreateScale(0.82f * hpRatio, 0.036f, 0.08f) *
				Matrix::CreateTranslation((hpRatio - 1.0f) * 0.41f, 0.0f, 0.0f) *
				Matrix::CreateRotationY(hpYaw) *
				Matrix::CreateTranslation(hpAnchor + Vector3(0.0f, 0.0f, -0.002f));
			m_obstacleMesh->Draw(hpBackWorld, m_view, m_proj, Color(0.08f, 0.08f, 0.1f, 0.92f));
			m_obstacleMesh->Draw(hpFillWorld, m_view, m_proj, Color(0.24f + (1.0f - hpRatio) * 0.72f, 0.86f * hpRatio + 0.16f, 0.2f, 0.95f));
		}

		if (m_showPathDebug && !lowVisualLod && !enemy.path.empty())
		{
			for (size_t c = enemy.pathCursor; c < enemy.path.size(); ++c)
			{
				const Vector3 p = m_grid.GridToWorld(enemy.path[c], 0.05f);
				const Matrix cellWorld = Matrix::CreateScale(0.2f, 0.05f, 0.2f) * Matrix::CreateTranslation(p);
				m_debugCellMesh->Draw(cellWorld, m_view, m_proj, DirectX::Colors::Orange);
			}
		}
	}

	for (size_t projectileIndex = 0; projectileIndex < m_enemyProjectiles.size(); ++projectileIndex)
	{
		const EnemyProjectileInfo& projectile = m_enemyProjectiles[projectileIndex];
		Vector3 velocityDir = projectile.velocity;
		if (velocityDir.LengthSquared() <= 0.0001f)
		{
			velocityDir = Vector3::UnitZ;
		}
		else
		{
			velocityDir.Normalize();
		}
		const float projectileYaw = std::atan2(velocityDir.x, velocityDir.z);
		const float projectilePitch = -std::asinf(Utility::MathEx::Clamp(velocityDir.y, -1.0f, 1.0f));
		const float pulse = std::sinf(m_sceneTime * 18.0f + static_cast<float>(projectileIndex) * 0.7f) * 0.5f + 0.5f;
		const Matrix projectileWorld =
			Matrix::CreateScale(projectile.radius * (0.95f + pulse * 0.18f)) *
			Matrix::CreateTranslation(projectile.position);
		const Matrix projectileTrailWorld =
			Matrix::CreateScale(projectile.radius * 0.55f, 0.022f, 0.82f) *
			Matrix::CreateRotationY(projectileYaw) *
			Matrix::CreateRotationX(projectilePitch) *
			Matrix::CreateTranslation(projectile.position - velocityDir * 0.36f);
		m_effectOrbMesh->Draw(projectileWorld, m_view, m_proj, Color(projectile.color.x, projectile.color.y, projectile.color.z, 0.88f));
		m_effectTrailMesh->Draw(projectileTrailWorld, m_view, m_proj, Color(projectile.color.x, projectile.color.y, projectile.color.z, 0.44f));
	}

	if (m_effectTrailMesh && m_effectOrbMesh)
	{
		m_slashHitEffects.Draw(*m_effectTrailMesh, *m_effectOrbMesh, m_view, m_proj);
	}
}
