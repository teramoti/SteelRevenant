#include "../GameScene.h"

#include <algorithm>
#include <cmath>

#include <DirectXColors.h>
#include <DirectXMath.h>

#include "../../../Utility/SimpleMathEx.h"
#include "../EnemyVisualProfile.h"
#include "../PlayerVisualProfile.h"
#include "GameSceneVisualPalette.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kPlayerRunCycleSpeed = 7.6f;
	constexpr float kEnemyBreathSpeed = 4.8f;
	constexpr float kEnemySpawnDurationSec = 0.4f;
	constexpr float kEnemyAttackWindupVisualSec = 0.45f;

	const Color kBladeColor = { 0.80f, 0.84f, 0.90f, 1.0f };
	const Color kBladeEdgeColor = { 0.95f, 0.97f, 1.00f, 1.0f };
	const Color kBladeFullerColor = { 0.52f, 0.55f, 0.62f, 1.0f };
	const Color kBladeFlatColor = { 0.62f, 0.65f, 0.72f, 1.0f };
	const Color kGuardColor = { 0.30f, 0.26f, 0.18f, 1.0f };
	const Color kGuardTrimColor = { 0.72f, 0.60f, 0.34f, 1.0f };
	const Color kGripColor = { 0.20f, 0.34f, 0.58f, 1.0f };
	const Color kGripWrapColor = { 0.12f, 0.18f, 0.30f, 1.0f };
	const Color kPommelColor = { 0.42f, 0.36f, 0.24f, 1.0f };
	const Color kTrailCoreColor = { 0.72f, 0.44f, 0.98f, 1.0f };
	const Color kTrailRimColor = { 0.38f, 0.94f, 1.00f, 1.0f };

	float LerpFloat(float a, float b, float t)
	{
		return a + (b - a) * Utility::MathEx::Clamp(t, 0.0f, 1.0f);
	}

	Vector3 LerpVector(const Vector3& a, const Vector3& b, float t)
	{
		return Vector3(
			LerpFloat(a.x, b.x, t),
			LerpFloat(a.y, b.y, t),
			LerpFloat(a.z, b.z, t));
	}

	float EaseInCubic(float t)
	{
		t = Utility::MathEx::Clamp(t, 0.0f, 1.0f);
		return t * t * t;
	}

	float EaseOutCubic(float t)
	{
		t = Utility::MathEx::Clamp(t, 0.0f, 1.0f);
		const float inv = 1.0f - t;
		return 1.0f - inv * inv * inv;
	}

	float BuildSwingEase(float attackBlend)
	{
		if (attackBlend < 0.5f)
		{
			return 4.0f * attackBlend * attackBlend * attackBlend;
		}
		return 1.0f - std::powf(-2.0f * attackBlend + 2.0f, 3.0f) * 0.5f;
	}

	Vector3 SafeNormalize(const Vector3& value, const Vector3& fallback)
	{
		Vector3 result = value;
		if (result.LengthSquared() <= 0.0001f)
		{
			result = fallback;
		}
		else
		{
			result.Normalize();
		}
		return result;
	}
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

	const Action::CombatTuning& tuning = m_combat.GetTuning();
	const float attackBlend = GetAttackBlend();
	const float runCycle = std::sinf(m_sceneTime * kPlayerRunCycleSpeed);
	const float legSwing = runCycle * 0.26f;
	const float attackTotalSec =
		std::max(0.0f, tuning.comboAttackWindupSec) +
		std::max(0.0f, tuning.comboAttackActiveSec) +
		std::max(0.0f, tuning.comboAttackFollowThroughSec) +
		std::max(0.0f, tuning.comboAttackRecoverSec);

	struct GreatswordPose
	{
		Vector3 gripLocal;
		float rootYawOffset;
		float rootLift;
		float rootForward;
		float bladeYaw;
		float bladePitch;
		float bladeRoll;
	};

	const auto LerpPose = [](const GreatswordPose& a, const GreatswordPose& b, float t) -> GreatswordPose
	{
		GreatswordPose pose = {};
		pose.gripLocal = LerpVector(a.gripLocal, b.gripLocal, t);
		pose.rootYawOffset = LerpFloat(a.rootYawOffset, b.rootYawOffset, t);
		pose.rootLift = LerpFloat(a.rootLift, b.rootLift, t);
		pose.rootForward = LerpFloat(a.rootForward, b.rootForward, t);
		pose.bladeYaw = LerpFloat(a.bladeYaw, b.bladeYaw, t);
		pose.bladePitch = LerpFloat(a.bladePitch, b.bladePitch, t);
		pose.bladeRoll = LerpFloat(a.bladeRoll, b.bladeRoll, t);
		return pose;
	};

	const GreatswordPose guardPose =
	{
		// gripLocal: 胸前の中段構え
		// x=0.08右寄り  y=0.88胸高さ  z=0.20前方
		Vector3(0.08f, 0.88f + std::fabs(runCycle) * 0.012f, 0.20f),
		0.02f,                         // rootYawOffset: ほぼ正面
		std::fabs(runCycle) * 0.016f,  // rootLift: 走り上下動
		0.04f,                         // rootForward: 軽く前傾
		0.06f,                         // bladeYaw: 正面向き
		0.65f,                         // bladePitch: +値=刃先が前方上方（中段構え）
		0.08f                          // bladeRoll: わずかに内側
	};

	GreatswordPose windupPose = guardPose;
	GreatswordPose activePose = guardPose;
	GreatswordPose followPose = guardPose;
	switch (std::clamp(m_player.comboIndex, 1, 3))
	{
	case 2:
		// 左から右への返し斬り
		windupPose = { Vector3(-0.28f, 0.90f, -0.04f), -0.16f, 0.02f, 0.02f,  1.08f, 0.48f, -0.24f };
		activePose = { Vector3( 0.28f, 0.84f,  0.24f),  0.16f, 0.00f, 0.12f, -0.94f, 0.46f,  0.28f };
		followPose = { Vector3( 0.18f, 0.82f,  0.22f),  0.10f,-0.01f, 0.10f, -0.60f, 0.44f,  0.16f };
		break;

	case 3:
		// 頭上から振り下ろす縦斬り
		// windup: 頭上高く持ち上げる（bladePitch=-1.5で刃が真上を向く）
		windupPose = { Vector3(0.04f, 1.30f, -0.02f),  0.00f,  0.10f, 0.02f,  0.02f, -1.50f, 0.04f };
		// active: 前方下へ叩きつける（bladePitch=+1.1で刃が前下を向く）
		activePose = { Vector3(0.00f, 0.72f,  0.32f),  0.00f, -0.05f, 0.20f,  0.02f,  1.10f, 0.04f };
		// follow: 振り切り
		followPose = { Vector3(0.00f, 0.68f,  0.30f),  0.00f, -0.05f, 0.18f,  0.02f,  0.85f, 0.04f };
		break;

	case 1:
	default:
		// 右から左への横薙ぎ
		// windup: 右肩後方に引いてため。bladeYawを負にして刃が右を向く
		windupPose = { Vector3(0.34f, 0.94f, -0.06f),  0.16f, 0.02f, 0.02f, -1.05f, 0.50f,  0.22f };
		// active: 左前方へ一気に振り抜く
		activePose = { Vector3(-0.24f, 0.84f, 0.26f), -0.18f, 0.00f, 0.14f,  0.92f, 0.48f, -0.30f };
		// follow: フォロースルー
		followPose = { Vector3(-0.16f, 0.82f, 0.24f), -0.10f,-0.01f, 0.10f,  0.62f, 0.45f, -0.16f };
		break;
	}

	const auto SampleGreatswordPose = [&](float progress) -> GreatswordPose
	{
		if (attackTotalSec <= 0.0f)
		{
			return guardPose;
		}

		const float windupEnd = Utility::MathEx::Clamp(tuning.comboAttackWindupSec / attackTotalSec, 0.0f, 1.0f);
		const float activeEnd = Utility::MathEx::Clamp((tuning.comboAttackWindupSec + tuning.comboAttackActiveSec) / attackTotalSec, 0.0f, 1.0f);
		const float followEnd = Utility::MathEx::Clamp((tuning.comboAttackWindupSec + tuning.comboAttackActiveSec + tuning.comboAttackFollowThroughSec) / attackTotalSec, 0.0f, 1.0f);
		progress = Utility::MathEx::Clamp(progress, 0.0f, 1.0f);

		if (progress <= windupEnd)
		{
			const float localT = (windupEnd > 0.0f) ? (progress / windupEnd) : 1.0f;
			return LerpPose(guardPose, windupPose, EaseInCubic(localT));
		}
		if (progress <= activeEnd)
		{
			const float denom = std::max(0.0001f, activeEnd - windupEnd);
			return LerpPose(windupPose, activePose, BuildSwingEase((progress - windupEnd) / denom));
		}
		if (progress <= followEnd)
		{
			const float denom = std::max(0.0001f, followEnd - activeEnd);
			return LerpPose(activePose, followPose, EaseOutCubic((progress - activeEnd) / denom));
		}

		const float denom = std::max(0.0001f, 1.0f - followEnd);
		return LerpPose(followPose, guardPose, EaseOutCubic((progress - followEnd) / denom));
	};

	const auto SampleBladeRotation = [&](float progress) -> Matrix
	{
		const GreatswordPose pose = SampleGreatswordPose(progress);
		return
			Matrix::CreateRotationZ(pose.bladeRoll) *
			Matrix::CreateRotationX(pose.bladePitch) *
			Matrix::CreateRotationY(m_player.yaw + pose.rootYawOffset + pose.bladeYaw);
	};

	const auto DrawLimb = [&](const Vector3& start, const Vector3& end, float radius, const Color& limbColor)
	{
		Vector3 dir = end - start;
		const float length = dir.Length();
		if (length <= 0.001f)
		{
			return;
		}
		dir /= length;
		const float yaw = std::atan2(dir.x, dir.z);
		const float pitch = -std::atan2(dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z));
		const Vector3 center = start + dir * (length * 0.5f);
		m_playerMesh->Draw(
			Matrix::CreateScale(radius, length * 0.5f, radius) *
			Matrix::CreateRotationX(pitch) *
			Matrix::CreateRotationY(yaw) *
			Matrix::CreateTranslation(center),
			m_view, m_proj, limbColor);
	};

	const bool attackInProgress = m_player.attackPhase != Action::PlayerAttackPhase::Idle;
	const GreatswordPose currentPose = attackInProgress ? SampleGreatswordPose(attackBlend) : guardPose;

	const SceneFx::PlayerVisualProfile pv = SceneFx::BuildPlayerVisualProfile(m_player, m_sceneTime);
	const Vector3 forward(std::sin(m_player.yaw), 0.0f, std::cos(m_player.yaw));

	DrawShadow(m_player.position, 0.56f, 0.82f, 0.18f);

	const Matrix pRoot =
		Matrix::CreateRotationY(m_player.yaw + currentPose.rootYawOffset) *
		Matrix::CreateTranslation(
			m_player.position +
			forward * currentPose.rootForward +
			Vector3(0.0f, std::sinf(m_sceneTime * 4.4f) * 0.04f + currentPose.rootLift, 0.0f));

	m_enemyMesh->Draw(
		Matrix::CreateScale(0.46f, 0.64f, 0.44f) * Matrix::CreateRotationX(attackBlend * 0.10f) *
		Matrix::CreateTranslation(0.0f, 1.0f, 0.08f) * pRoot,
		m_view, m_proj, pv.armorLight);
	m_enemyMesh->Draw(
		Matrix::CreateScale(0.20f) * Matrix::CreateTranslation(0.0f, 1.70f, 0.0f) * pRoot,
		m_view, m_proj, pv.armorLight);
	m_obstacleMesh->Draw(
		Matrix::CreateScale(0.13f, 0.024f, 0.032f) * Matrix::CreateTranslation(0.0f, 1.715f, 0.198f) * pRoot,
		m_view, m_proj, pv.emissiveColor);
	m_obstacleMesh->Draw(
		Matrix::CreateScale(0.055f, 0.045f, 0.18f) * Matrix::CreateTranslation(0.0f, 1.89f, 0.0f) * pRoot,
		m_view, m_proj, pv.accentColor);
	m_obstacleMesh->Draw(
		Matrix::CreateScale(0.36f, 0.20f, 0.14f) * Matrix::CreateTranslation(0.0f, 1.08f, 0.18f) * pRoot,
		m_view, m_proj, pv.armorDark);
	for (int side : { -1, 1 })
	{
		m_obstacleMesh->Draw(
			Matrix::CreateScale(0.18f, 0.10f, 0.18f) * Matrix::CreateRotationZ(0.20f * static_cast<float>(side)) *
			Matrix::CreateTranslation(0.32f * static_cast<float>(side), 1.28f, 0.02f) * pRoot,
			m_view, m_proj, pv.trimColor);
		m_obstacleMesh->Draw(
			Matrix::CreateScale(0.10f, 0.18f, 0.08f) * Matrix::CreateRotationZ(0.10f * static_cast<float>(side)) *
			Matrix::CreateTranslation(0.18f * static_cast<float>(side), 0.64f, 0.07f) * pRoot,
			m_view, m_proj, pv.armorDark);
		m_playerMesh->Draw(
			Matrix::CreateScale(0.14f, 0.55f, 0.16f) * Matrix::CreateRotationX((side < 0) ? legSwing : -legSwing) *
			Matrix::CreateTranslation(0.16f * static_cast<float>(side), 0.32f, 0.0f) * pRoot,
			m_view, m_proj, pv.underColor);
		m_obstacleMesh->Draw(
			Matrix::CreateScale(0.10f, 0.22f, 0.08f) * Matrix::CreateTranslation(0.16f * static_cast<float>(side), 0.16f, 0.13f) * pRoot,
			m_view, m_proj, pv.armorDark);
	}

	const Matrix bladeRot = SampleBladeRotation(attackInProgress ? attackBlend : 0.0f);
	Vector3 bladeDir = SafeNormalize(Vector3::TransformNormal(Vector3::UnitY, bladeRot), Vector3::UnitY);
	Vector3 bladeRight = SafeNormalize(Vector3::TransformNormal(Vector3::UnitX, bladeRot), Vector3::UnitX);
	const Vector3 rearGripWorld = Vector3::Transform(currentPose.gripLocal, pRoot);
	const Vector3 frontGripWorld = rearGripWorld + bladeDir * 0.18f;
	const Vector3 guardCenter = rearGripWorld + bladeDir * 0.26f;
	const Vector3 weaponCenter = rearGripWorld + bladeDir * 0.90f;
	const Vector3 leftShoulderWorld = Vector3::Transform(Vector3(-0.24f, 1.20f, 0.04f), pRoot);
	const Vector3 rightShoulderWorld = Vector3::Transform(Vector3(0.24f, 1.20f, 0.04f), pRoot);

	DrawLimb(leftShoulderWorld, frontGripWorld, 0.070f, pv.underColor);
	DrawLimb(rightShoulderWorld, rearGripWorld, 0.074f, pv.underColor);
	m_enemyMesh->Draw(Matrix::CreateScale(0.055f) * Matrix::CreateTranslation(frontGripWorld), m_view, m_proj, pv.trimColor);
	m_enemyMesh->Draw(Matrix::CreateScale(0.060f) * Matrix::CreateTranslation(rearGripWorld), m_view, m_proj, pv.trimColor);

	m_weaponMesh->Draw(bladeRot * Matrix::CreateTranslation(weaponCenter), m_view, m_proj, kBladeColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.008f, 1.52f, 0.020f) * bladeRot * Matrix::CreateTranslation(weaponCenter + bladeRight * 0.072f), m_view, m_proj, kBladeEdgeColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.008f, 1.52f, 0.020f) * bladeRot * Matrix::CreateTranslation(weaponCenter - bladeRight * 0.072f), m_view, m_proj, kBladeEdgeColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.022f, 1.10f, 0.022f) * bladeRot * Matrix::CreateTranslation(rearGripWorld + bladeDir * 0.98f), m_view, m_proj, kBladeFullerColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.10f, 0.18f, 0.014f) * bladeRot * Matrix::CreateTranslation(rearGripWorld + bladeDir * 1.42f), m_view, m_proj, kBladeFlatColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.044f, 0.16f, 0.010f) * bladeRot * Matrix::CreateTranslation(rearGripWorld + bladeDir * 1.58f), m_view, m_proj, kBladeColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.014f, 0.10f, 0.008f) * bladeRot * Matrix::CreateTranslation(rearGripWorld + bladeDir * 1.70f), m_view, m_proj, kBladeEdgeColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.20f, 0.036f, 0.10f) * Matrix::CreateRotationZ(0.10f) * bladeRot * Matrix::CreateTranslation(guardCenter + bladeRight * 0.12f), m_view, m_proj, kGuardColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.20f, 0.036f, 0.10f) * Matrix::CreateRotationZ(-0.10f) * bladeRot * Matrix::CreateTranslation(guardCenter - bladeRight * 0.12f), m_view, m_proj, kGuardColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.34f, 0.016f, 0.016f) * bladeRot * Matrix::CreateTranslation(guardCenter), m_view, m_proj, kGuardTrimColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.046f, 0.34f, 0.060f) * bladeRot * Matrix::CreateTranslation(rearGripWorld + bladeDir * 0.09f), m_view, m_proj, kGripColor);
	for (int wrapIndex = 0; wrapIndex < 5; ++wrapIndex)
	{
		const float offset = -0.12f + static_cast<float>(wrapIndex) * 0.08f;
		m_obstacleMesh->Draw(
			Matrix::CreateScale(0.054f, 0.016f, 0.066f) * bladeRot * Matrix::CreateTranslation(rearGripWorld + bladeDir * (0.09f - offset)),
			m_view, m_proj, kGripWrapColor);
	}
	m_obstacleMesh->Draw(Matrix::CreateScale(0.082f, 0.074f, 0.082f) * bladeRot * Matrix::CreateTranslation(rearGripWorld - bladeDir * 0.10f), m_view, m_proj, kPommelColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.054f, 0.032f, 0.054f) * bladeRot * Matrix::CreateTranslation(rearGripWorld - bladeDir * 0.16f), m_view, m_proj, kGuardTrimColor);

	m_player.swingTrailBase = rearGripWorld + bladeDir * 1.00f;
	m_player.swingTrailTip = rearGripWorld + bladeDir * 1.68f;
	if (m_player.swingTrailActive && attackInProgress)
	{
		Vector3 previousMid = m_player.swingTrailBase;
		Vector3 previousTip = m_player.swingTrailTip;
		for (int sampleIndex = 1; sampleIndex < 8; ++sampleIndex)
		{
			const float sampleAlpha = 1.0f - static_cast<float>(sampleIndex) / 8.0f;
			const float sampleT = Utility::MathEx::Clamp(attackBlend - sampleAlpha * 0.12f, 0.0f, 1.0f);
			const GreatswordPose samplePose = SampleGreatswordPose(sampleT);
			const Matrix sampleRoot =
				Matrix::CreateRotationY(m_player.yaw + samplePose.rootYawOffset) *
				Matrix::CreateTranslation(
					m_player.position +
					forward * samplePose.rootForward +
					Vector3(0.0f, std::sinf(m_sceneTime * 4.4f) * 0.04f + samplePose.rootLift, 0.0f));
			const Matrix sampleRot = SampleBladeRotation(sampleT);
			const Vector3 sampleDir = SafeNormalize(Vector3::TransformNormal(Vector3::UnitY, sampleRot), bladeDir);
			const Vector3 sampleGrip = Vector3::Transform(samplePose.gripLocal, sampleRoot);
			const Vector3 midPos = sampleGrip + sampleDir * 1.00f;
			const Vector3 tipPos = sampleGrip + sampleDir * 1.68f;
			const Vector3 currentCenter = (midPos + tipPos) * 0.5f;
			const Vector3 previousCenter = (previousMid + previousTip) * 0.5f;
			const Vector3 span = tipPos - midPos;
			const Vector3 segment = currentCenter - previousCenter;
			const float segmentLength = segment.Length();
			const float spanLength = span.Length();
			if (segmentLength > 0.04f && spanLength > 0.10f)
			{
				const Vector3 segmentCenter = previousCenter + segment * 0.5f;
				const float segmentYaw = std::atan2(segment.x, segment.z);
				const float segmentPitch = -std::atan2(segment.y, std::sqrt(segment.x * segment.x + segment.z * segment.z));
				m_effectTrailMesh->Draw(
					Matrix::CreateScale(spanLength * 0.52f, 0.020f, segmentLength) *
					Matrix::CreateRotationZ(0.18f) *
					Matrix::CreateRotationX(segmentPitch) *
					Matrix::CreateRotationY(segmentYaw) *
					Matrix::CreateTranslation(segmentCenter),
					m_view, m_proj,
					Color(kTrailCoreColor.R(), kTrailCoreColor.G(), kTrailCoreColor.B(), 0.12f + sampleAlpha * 0.20f));
				m_effectTrailMesh->Draw(
					Matrix::CreateScale(spanLength * 0.68f, 0.010f, segmentLength * 0.92f) *
					Matrix::CreateRotationZ(0.18f) *
					Matrix::CreateRotationX(segmentPitch) *
					Matrix::CreateRotationY(segmentYaw) *
					Matrix::CreateTranslation(segmentCenter),
					m_view, m_proj,
					Color(kTrailRimColor.R(), kTrailRimColor.G(), kTrailRimColor.B(), 0.10f + sampleAlpha * 0.16f));
			}
			previousMid = midPos;
			previousTip = tipPos;
		}
	}

	if (m_player.comboLevel >= 3 && attackInProgress)
	{
		m_weaponMesh->Draw(
			Matrix::CreateScale(1.18f, 1.0f, 1.6f) * bladeRot * Matrix::CreateTranslation(weaponCenter),
			m_view, m_proj,
			Color(pv.emissiveColor.R(), pv.emissiveColor.G(), pv.emissiveColor.B(), 0.18f + attackBlend * 0.22f));
	}

	const Matrix invView = m_view.Invert();
	const Vector3 camPos(invView._41, invView._42, invView._43);

	for (size_t i = 0; i < m_enemies.size(); ++i)
	{
		const Action::EnemyState& en = m_enemies[i];
		if (en.state == Action::EnemyStateType::Dead)
		{
			continue;
		}

		const SceneFx::EnemyVisualProfile ev = SceneFx::BuildEnemyVisualProfile(en, m_gameState.dangerLevel, i, m_sceneTime);
		const float breath = std::sinf(m_sceneTime * kEnemyBreathSpeed + static_cast<float>(i) * 0.78f) * 0.07f;
		const float spawnBlend = 1.0f - Utility::MathEx::Clamp(en.stateTimer / kEnemySpawnDurationSec, 0.0f, 1.0f);
		const float eScale = (en.state == Action::EnemyStateType::Idle && en.stateTimer > 0.0f) ? (0.35f + spawnBlend * 0.65f) : 1.0f;
		const float hitBlend = Utility::MathEx::Clamp(en.hitReactTimer / 0.24f, 0.0f, 1.0f);
		const float attackBlendEnemy = (en.state == Action::EnemyStateType::Attack)
			? (1.0f - Utility::MathEx::Clamp(en.stateTimer / kEnemyAttackWindupVisualSec, 0.0f, 1.0f))
			: 0.0f;

		Vector3 toCam = en.position - camPos;
		toCam.y = 0.0f;
		const float distSq = toCam.LengthSquared();
		const bool midLod = distSq >= 20.0f * 20.0f;
		const bool lowLod = distSq >= 28.0f * 28.0f;

		DrawShadow(en.position, 0.42f, 0.60f, lowLod ? 0.07f : 0.11f);

		Vector3 knockbackDir = en.knockbackVelocity;
		knockbackDir.y = 0.0f;
		if (knockbackDir.LengthSquared() > 0.0001f)
		{
			knockbackDir.Normalize();
		}

		const Matrix eRoot =
			Matrix::CreateScale(eScale * (1.0f + hitBlend * 0.10f), eScale * (1.0f - hitBlend * 0.15f), eScale * (1.0f + hitBlend * 0.10f)) *
			Matrix::CreateRotationY(en.yaw + std::sinf(en.hitReactTimer * 28.0f + static_cast<float>(i)) * hitBlend * 0.34f + attackBlendEnemy * 0.08f) *
			Matrix::CreateTranslation(en.position + knockbackDir * hitBlend * 0.34f + Vector3(0.0f, breath + hitBlend * 0.07f, 0.0f));

		m_enemyMesh->Draw(
			Matrix::CreateScale(0.44f, 0.62f, 0.44f) * Matrix::CreateRotationX(attackBlendEnemy * 0.36f) *
			Matrix::CreateTranslation(0.0f, 1.0f, attackBlendEnemy * 0.15f) * eRoot,
			m_view, m_proj, ev.armorLight);

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
			const bool moving =
				en.state == Action::EnemyStateType::Wander ||
				en.state == Action::EnemyStateType::Chase ||
				en.state == Action::EnemyStateType::Return;
			const float locomotion = moving ? 1.0f : 0.0f;
			const float eArmPulse = std::sinf(m_sceneTime * (4.6f + locomotion * 2.2f) + static_cast<float>(i)) * (0.12f + locomotion * 0.22f);
			const float eLegSwing = std::sinf(m_sceneTime * (3.0f + locomotion * 4.2f) + static_cast<float>(i) * 0.9f) * (0.08f + locomotion * 0.24f);

			m_obstacleMesh->Draw(Matrix::CreateScale(0.34f, 0.18f, 0.14f) * Matrix::CreateRotationX(attackBlendEnemy * 0.18f) * Matrix::CreateTranslation(0.0f, 1.08f, 0.18f) * eRoot, m_view, m_proj, ev.armorDark);
			m_obstacleMesh->Draw(Matrix::CreateScale(0.16f, 0.10f, 0.18f) * Matrix::CreateRotationZ(-0.24f) * Matrix::CreateTranslation(-0.32f, 1.27f, 0.0f) * eRoot, m_view, m_proj, ev.trimColor);
			m_obstacleMesh->Draw(Matrix::CreateScale(0.16f, 0.10f, 0.18f) * Matrix::CreateRotationZ(0.24f) * Matrix::CreateTranslation(0.32f, 1.27f, 0.02f) * eRoot, m_view, m_proj, ev.trimColor);
			m_playerMesh->Draw(Matrix::CreateScale(0.14f, 0.48f, 0.14f) * Matrix::CreateRotationX(0.10f) * Matrix::CreateRotationZ(-0.04f + eArmPulse * 0.65f - attackBlendEnemy * 0.30f) * Matrix::CreateTranslation(-0.30f, 1.02f, -0.06f) * eRoot, m_view, m_proj, ev.underColor);
			m_playerMesh->Draw(Matrix::CreateScale(0.14f, 0.48f, 0.14f) * Matrix::CreateRotationX(0.54f - attackBlendEnemy * 0.80f) * Matrix::CreateRotationZ(0.02f - eArmPulse * 0.55f + attackBlendEnemy * 0.28f) * Matrix::CreateTranslation(0.24f, 1.0f, -0.10f + attackBlendEnemy * 0.20f) * eRoot, m_view, m_proj, ev.underColor);
			m_playerMesh->Draw(Matrix::CreateScale(0.14f, 0.55f, 0.16f) * Matrix::CreateRotationX(eLegSwing) * Matrix::CreateTranslation(-0.16f, 0.32f, 0.0f) * eRoot, m_view, m_proj, ev.underColor);
			m_playerMesh->Draw(Matrix::CreateScale(0.14f, 0.55f, 0.16f) * Matrix::CreateRotationX(-eLegSwing) * Matrix::CreateTranslation(0.16f, 0.32f, 0.0f) * eRoot, m_view, m_proj, ev.underColor);

			const Matrix enemyBladeRot =
				Matrix::CreateRotationY(-0.44f + attackBlendEnemy * 0.44f) *
				Matrix::CreateRotationX(0.84f - attackBlendEnemy * 1.20f) *
				Matrix::CreateRotationY(en.yaw);
			Vector3 enemyBladeDir = Vector3::TransformNormal(Vector3::UnitY, enemyBladeRot);
			Vector3 enemyBladeRight = Vector3::TransformNormal(Vector3::UnitX, enemyBladeRot);
			enemyBladeDir.Normalize();
			enemyBladeRight.Normalize();
			const Vector3 enemyBladePos = Vector3::Transform(Vector3(0.22f, 0.82f, -0.10f + attackBlendEnemy * 0.28f), eRoot);

			m_obstacleMesh->Draw(Matrix::CreateScale(0.11f, 0.58f, 0.014f) * enemyBladeRot * Matrix::CreateTranslation(enemyBladePos), m_view, m_proj, ev.weaponColor);
			m_obstacleMesh->Draw(Matrix::CreateScale(0.007f, 0.56f, 0.017f) * enemyBladeRot * Matrix::CreateTranslation(enemyBladePos + enemyBladeRight * 0.055f), m_view, m_proj, kBladeEdgeColor);
			m_obstacleMesh->Draw(Matrix::CreateScale(0.007f, 0.56f, 0.017f) * enemyBladeRot * Matrix::CreateTranslation(enemyBladePos - enemyBladeRight * 0.055f), m_view, m_proj, kBladeEdgeColor);
			m_obstacleMesh->Draw(Matrix::CreateScale(0.26f, 0.028f, 0.075f) * enemyBladeRot * Matrix::CreateTranslation(enemyBladePos - enemyBladeDir * 0.30f), m_view, m_proj, ev.trimColor);
			m_obstacleMesh->Draw(Matrix::CreateScale(0.040f, 0.18f, 0.052f) * enemyBladeRot * Matrix::CreateTranslation(enemyBladePos - enemyBladeDir * 0.42f), m_view, m_proj, kGripColor);
		}

		if (!lowLod && hitBlend > 0.01f)
		{
			m_obstacleMesh->Draw(
				Matrix::CreateScale(0.54f + hitBlend * 0.14f, 0.028f, 0.12f) *
				Matrix::CreateRotationZ(0.78f) *
				Matrix::CreateRotationY(en.yaw + 0.12f) *
				Matrix::CreateTranslation(en.position + Vector3(0.0f, 1.04f + hitBlend * 0.04f, 0.0f)),
				m_view, m_proj,
				Color(0.26f, 0.02f, 0.03f, 0.24f + hitBlend * 0.34f));
		}

		if (!lowLod)
		{
			const float hpRatio = Utility::MathEx::Clamp(en.hp / std::max(1.0f, en.maxHp), 0.0f, 1.0f);
			const Vector3 hpAnchor = en.position + Vector3(0.0f, 2.08f, 0.0f);
			Vector3 hpToCam = camPos - hpAnchor;
			hpToCam.y = 0.0f;
			if (hpToCam.LengthSquared() < 0.0001f)
			{
				hpToCam = Vector3::UnitZ;
			}
			else
			{
				hpToCam.Normalize();
			}
			const float hpYaw = std::atan2(hpToCam.x, hpToCam.z);

			m_obstacleMesh->Draw(
				Matrix::CreateScale(0.84f, 0.05f, 0.10f) * Matrix::CreateRotationY(hpYaw) * Matrix::CreateTranslation(hpAnchor),
				m_view, m_proj, Color(0.08f, 0.08f, 0.10f, 0.92f));
			m_obstacleMesh->Draw(
				Matrix::CreateScale(0.82f * hpRatio, 0.036f, 0.08f) *
				Matrix::CreateTranslation((hpRatio - 1.0f) * 0.41f, 0.0f, 0.0f) *
				Matrix::CreateRotationY(hpYaw) *
				Matrix::CreateTranslation(hpAnchor + Vector3(0.0f, 0.0f, -0.002f)),
				m_view, m_proj,
				Color(0.24f + (1.0f - hpRatio) * 0.72f, 0.86f * hpRatio + 0.16f, 0.20f, 0.95f));
		}

		if (m_showPathDebug && !lowLod && !en.path.empty())
		{
			for (size_t pathIndex = en.pathCursor; pathIndex < en.path.size(); ++pathIndex)
			{
				const Vector3 point = m_grid.GridToWorld(en.path[pathIndex], 0.05f);
				m_debugCellMesh->Draw(
					Matrix::CreateScale(0.2f, 0.05f, 0.2f) * Matrix::CreateTranslation(point),
					m_view, m_proj, DirectX::Colors::Orange);
			}
		}
	}

	if (m_effectTrailMesh && m_effectOrbMesh)
	{
		m_slashHitEffects.Draw(*m_effectTrailMesh, *m_effectOrbMesh, m_view, m_proj);
	}
}
