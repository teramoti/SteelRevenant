//------------------------//------------------------
// Contents(処理内容) CombatSystem のプレイヤー更新と近接攻撃解決を分割実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// updated by OpenAI (構成分割)
// last updated (最終更新日) 2026 / 03 / 19
//------------------------//------------------------
#include "../CombatSystem.h"
#include "CombatEnemyStateUtil.h"
#include "../../Utility/Sound/AudioSystem.h"

// Ensure any 'min'/'max' macros from Windows headers do not break std::min/std::max usage.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <algorithm>
#include <cmath>

#include "../../Utility/SimpleMathEx.h"

using DirectX::SimpleMath::Vector3;

namespace Action
{
	void CombatSystem::UpdatePlayer(
		PlayerState& player,
		const InputSnapshot& input,
		const Vector3& cameraForward,
		const Vector3& cameraRight,
		GameState& gameState,
		float groundHeight) const
	{
		const float dt = std::max(0.0f, input.dt);
		const float playerGroundY = std::max(0.8f, groundHeight);
		const float windupSec = std::max(0.0f, m_tuning.comboAttackWindupSec);
		const float activeSec = std::max(0.0f, m_tuning.comboAttackActiveSec);
		const float followThroughSec = std::max(0.0f, m_tuning.comboAttackFollowThroughSec);
		const float recoverSec = std::max(0.0f, m_tuning.comboAttackRecoverSec);
		const float attackMotionTotalSec = windupSec + activeSec + followThroughSec + recoverSec;
		player.attackTriggeredThisFrame = false;
		player.swingTrailActive = false;

		// Apply mouse look to yaw (invert delta.x if input is positive to match expected right movement)
		player.yaw = Utility::MathEx::WrapRadians(player.yaw - input.mouseDelta.x * m_tuning.playerYawScale);

		player.damageGraceTimer = std::max(0.0f, player.damageGraceTimer - dt);
		player.attackCooldown = std::max(0.0f, player.attackCooldown - dt);
		player.comboTimer = std::max(0.0f, player.comboTimer - dt);
		player.comboGauge = std::max(0.0f, player.comboGauge - dt * m_tuning.comboGaugeDecayPerSec);

		if (player.comboGauge >= m_tuning.comboLevelGauge3)
		{
			player.comboLevel = 3;
		}
		else if (player.comboGauge >= m_tuning.comboLevelGauge2)
		{
			player.comboLevel = 2;
		}
		else if (player.comboGauge >= m_tuning.comboLevelGauge1)
		{
			player.comboLevel = 1;
		}
		else
		{
			player.comboLevel = 0;
		}

		player.attackPhaseBlend = 0.0f;
		switch (player.attackPhase)
		{
		case PlayerAttackPhase::Windup:
			player.attackPhaseTimer = std::max(0.0f, player.attackPhaseTimer - dt);
			player.attackWindupTimer = player.attackPhaseTimer;
			player.attackTimer = activeSec;
			player.attackPhaseBlend = (windupSec > 0.0f)
				? 1.0f - Utility::MathEx::Clamp(player.attackPhaseTimer / windupSec, 0.0f, 1.0f)
				: 1.0f;
			if (player.attackPhaseTimer <= 0.0f)
			{
				player.attackPhase = PlayerAttackPhase::Active;
				player.attackPhaseTimer = activeSec;
				player.attackWindupTimer = 0.0f;
				player.attackTimer = activeSec;
				player.attackPhaseBlend = 0.0f;
			}
			break;

		case PlayerAttackPhase::Active:
			player.attackPhaseTimer = std::max(0.0f, player.attackPhaseTimer - dt);
			player.attackWindupTimer = 0.0f;
			player.attackTimer = player.attackPhaseTimer;
			player.attackPhaseBlend = (activeSec > 0.0f)
				? 1.0f - Utility::MathEx::Clamp(player.attackPhaseTimer / activeSec, 0.0f, 1.0f)
				: 1.0f;
			if (player.attackPhaseTimer <= 0.0f)
			{
				player.attackPhase = PlayerAttackPhase::FollowThrough;
				player.attackPhaseTimer = followThroughSec;
				player.attackTimer = 0.0f;
				player.attackPhaseBlend = 0.0f;
			}
			break;

		case PlayerAttackPhase::FollowThrough:
			player.attackPhaseTimer = std::max(0.0f, player.attackPhaseTimer - dt);
			player.attackWindupTimer = 0.0f;
			player.attackTimer = 0.0f;
			player.attackPhaseBlend = (followThroughSec > 0.0f)
				? 1.0f - Utility::MathEx::Clamp(player.attackPhaseTimer / followThroughSec, 0.0f, 1.0f)
				: 1.0f;
			if (player.attackPhaseTimer <= 0.0f)
			{
				player.attackPhase = PlayerAttackPhase::Recover;
				player.attackPhaseTimer = recoverSec;
				player.attackPhaseBlend = 0.0f;
			}
			break;

		case PlayerAttackPhase::Recover:
			player.attackPhaseTimer = std::max(0.0f, player.attackPhaseTimer - dt);
			player.attackWindupTimer = 0.0f;
			player.attackTimer = 0.0f;
			player.attackPhaseBlend = (recoverSec > 0.0f)
				? 1.0f - Utility::MathEx::Clamp(player.attackPhaseTimer / recoverSec, 0.0f, 1.0f)
				: 1.0f;
			if (player.attackPhaseTimer <= 0.0f)
			{
				player.attackPhase = PlayerAttackPhase::Idle;
				player.attackPhaseTimer = 0.0f;
				player.attackPhaseBlend = 0.0f;
			}
			break;

		case PlayerAttackPhase::Idle:
		default:
			player.attackPhase = PlayerAttackPhase::Idle;
			player.attackPhaseTimer = 0.0f;
			player.attackWindupTimer = 0.0f;
			player.attackTimer = 0.0f;
			break;
		}

		if (player.attackPhase == PlayerAttackPhase::Idle && player.comboTimer <= 0.0f)
		{
			player.comboIndex = 0;
		}

		Vector3 moveForward = Utility::MathEx::SafeNormalize(Vector3(cameraForward.x, 0.0f, cameraForward.z));
		Vector3 moveRight = Utility::MathEx::SafeNormalize(Vector3(cameraRight.x, 0.0f, cameraRight.z));
		if (moveForward.LengthSquared() <= 0.0001f)
		{
			moveForward = Vector3(std::sin(player.yaw), 0.0f, std::cos(player.yaw));
		}
		if (moveRight.LengthSquared() <= 0.0001f)
		{
			moveRight = Vector3(-moveForward.z, 0.0f, moveForward.x);
		}


		// --- スライド入力・状態遷移・移動分岐 ---
		// スライド開始条件: Shift 押下で発動
		bool slideInput = input.slideHeld;
		const float slideDuration = 0.25f;
		const float slideCooldownTime = 1.0f;
		const float slideSpeed = 12.0f;

		if (!player.isSliding && player.slideCooldown <= 0.0f && slideInput)
		{
			player.isSliding = true;
			player.slideTimer = slideDuration;
			player.slideCooldown = slideCooldownTime;
			// スライド方向は現在の移動方向
			Vector3 dir = Vector3::Zero;
			if (input.moveForward) { dir += moveForward; }
			if (input.moveBack) { dir -= moveForward; }
			if (input.moveRight) { dir += moveRight; }
			if (input.moveLeft) { dir -= moveRight; }
			player.slideDirection = Utility::MathEx::SafeNormalize(dir);
			if (player.slideDirection.LengthSquared() < 0.0001f) player.slideDirection = moveForward;
			// play slide sound once
			GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::DashSlide, 0.95f);
		}

		if (player.isSliding)
		{
			player.slideTimer -= dt;
			if (player.slideTimer <= 0.0f)
			{
				player.isSliding = false;
			}
		}
		else
		{
			player.slideCooldown = std::max(0.0f, player.slideCooldown - dt);
		}

		Vector3 moveDir = Vector3::Zero;
		if (!player.isSliding)
		{
			if (input.moveForward) { moveDir += moveForward; }
			if (input.moveBack) { moveDir -= moveForward; }
			if (input.moveRight) { moveDir += moveRight; }
			if (input.moveLeft) { moveDir -= moveRight; }
			moveDir = Utility::MathEx::SafeNormalize(moveDir);
		}

		if (input.attackPressed && player.attackCooldown <= 0.0f && player.attackPhase == PlayerAttackPhase::Idle && !player.isSliding)
		{
			if (player.comboTimer > 0.0f)
			{
				player.comboIndex = (std::min)(3, player.comboIndex + 1);
			}
			else
			{
				player.comboIndex = 1;
			}

			player.attackPhase = PlayerAttackPhase::Windup;
			player.attackPhaseBlend = 0.0f;
			player.attackPhaseTimer = windupSec;
			player.attackWindupTimer = windupSec;
			player.attackTimer = activeSec;
			player.attackCooldown = std::max(m_tuning.comboCooldownSec, attackMotionTotalSec);
			player.comboTimer = m_tuning.comboChainWindowSec;
			player.attackTriggeredThisFrame = true;
			GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::MeleeSlash, 0.82f);
		}

		player.guarding = input.guardHeld;

		Vector3 velocity = Vector3::Zero;
		if (player.isSliding)
		{
			velocity = player.slideDirection * slideSpeed;
		}
		else
		{
			const Vector3 targetVelocity = moveDir * m_tuning.walkSpeed;
			const float gain = (moveDir.LengthSquared() > 0.0001f) ? m_tuning.moveAccelGain : m_tuning.moveStopGain;
			const float blend = Utility::MathEx::Clamp(1.0f - std::exp(-gain * dt), 0.0f, 1.0f);
			velocity = player.moveVelocity + (targetVelocity - player.moveVelocity) * blend;
		}
		player.moveVelocity = velocity;

		player.position += player.moveVelocity * dt;
		player.position.y = playerGroundY;

		player.position.x = Utility::MathEx::Clamp(player.position.x, m_tuning.playerBoundsMin, m_tuning.playerBoundsMax);
		player.position.z = Utility::MathEx::Clamp(player.position.z, m_tuning.playerBoundsMin, m_tuning.playerBoundsMax);

		if (player.attackPhase == PlayerAttackPhase::Active ||
			(player.attackPhase == PlayerAttackPhase::FollowThrough && player.attackPhaseBlend < 0.5f))
		{
			player.swingTrailActive = true;
		}
	}

// プレイヤー攻撃のヒット判定とダメージを解決する。

	void CombatSystem::ResolvePlayerAttack(
		PlayerState& player,
		std::vector<EnemyState>& enemies,
		GameState& gameState) const
	{
		if (player.attackPhase != PlayerAttackPhase::Active ||
			player.attackTimer <= 0.0f ||
			player.comboIndex <= 0)
		{
			return;
		}

		Vector3 forward(std::sin(player.yaw), 0.0f, std::cos(player.yaw));
		const bool hasLockTarget =
			(player.lockEnemyIndex >= 0) &&
			(player.lockEnemyIndex < static_cast<int>(enemies.size())) &&
			(enemies[static_cast<size_t>(player.lockEnemyIndex)].state != EnemyStateType::Dead);

		float baseDamage = m_tuning.playerAttackDamage1;
		if (player.comboIndex == 2) { baseDamage = m_tuning.playerAttackDamage2; }
		if (player.comboIndex >= 3) { baseDamage = m_tuning.playerAttackDamage3; }

		float damageValue = baseDamage;
		const float comboMultiplier = 1.0f + static_cast<float>(player.comboLevel) * m_tuning.comboDamageBonusPerLevel;
		damageValue *= comboMultiplier;
		const int damage = static_cast<int>(std::round(damageValue));

		bool hitAny = false;
		bool killedAny = false;

		// Performance instrumentation
		int scanned = 0;
		int candidates = 0;
		int normalized = 0;
		int hits = 0;

		// Limit work per swing
		const int kMaxHitsPerSwing = 6; // cap simultaneous hits to avoid expensive frames

		// Precompute squared ranges to avoid sqrt for distant enemies
		const float rangeSq = m_attackRange * m_attackRange;
		const float lockAssistRange = m_attackRange * m_tuning.lockAssistRangeMultiplier;
		const float lockAssistRangeSq = lockAssistRange * lockAssistRange;

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			scanned++;
			EnemyState& enemy = enemies[i];
			if (enemy.state == EnemyStateType::Dead || enemy.hitByCurrentSwing)
			{
				continue;
			}

			Vector3 toEnemy = enemy.position - player.position;
			toEnemy.y = 0.0f;
			// Cheap axis-aligned rejection before expensive math
			const float absX = std::abs(toEnemy.x);
			const float absZ = std::abs(toEnemy.z);
			if (absX > m_attackRange || absZ > m_attackRange)
			{
				continue;
			}

			const float distSq = toEnemy.LengthSquared();

			// Quick rejection using squared distance
			if (distSq > rangeSq || distSq <= 0.000001f)
			{
				// Allow lock-assist cases even if slightly out of base range
				if (!(hasLockTarget && static_cast<int>(i) == player.lockEnemyIndex && distSq <= lockAssistRangeSq))
				{
					continue;
				}
			}

			candidates++;

			// Avoid expensive sqrt when possible: compute forward dot and test using squared values
			const float fwdDot = forward.x * toEnemy.x + forward.z * toEnemy.z;
			// if lock target, adjust forwardForCheck later; handle sign check now
			if (hasLockTarget && static_cast<int>(i) == player.lockEnemyIndex)
			{
				// use normalized direction for lock target check (rare) - fall back to previous behavior
				const float dist = std::sqrt(distSq);
				Vector3 toEnemyN = toEnemy / dist;
				Vector3 forwardForCheck = Utility::MathEx::SafeNormalize(Vector3(toEnemyN.x, 0.0f, toEnemyN.z));
				const float facing = forwardForCheck.Dot(toEnemyN);
				if (facing < m_attackDotThreshold)
				{
					if (!(hasLockTarget && static_cast<int>(i) == player.lockEnemyIndex && facing >= (m_attackDotThreshold - m_tuning.lockAssistDotRelax)))
					{
						continue;
					}
				}
				// proceed with normalized path below
				normalized++;
				// apply hit
				enemy.hitByCurrentSwing = true;
				enemy.hp -= static_cast<float>(damage);
				enemy.hitReactTimer = m_tuning.enemyHitStunSec;
				enemy.knockbackVelocity = toEnemyN * (m_tuning.enemyKnockbackSpeed + static_cast<float>(player.comboIndex - 1) * 0.8f);
				hitAny = true;
				hits++;

				player.comboGauge = (std::min)(100.0f, player.comboGauge + m_tuning.comboGaugeHitGain);

				if (enemy.hp <= 0.0f)
				{
					enemy.hp = 0.0f;
					enemy.state = EnemyStateType::Dead;
					enemy.hitReactTimer = 0.0f;
					enemy.knockbackVelocity = Vector3::Zero;
					Action::CombatInternal::ClearEnemyPathState(enemy);
					enemy.hasWanderTarget = false;
					gameState.killCount += 1;
					player.comboGauge = (std::min)(100.0f, player.comboGauge + m_tuning.comboGaugeKillBonus);
					killedAny = true;
				}
				else
				{
					enemy.state = EnemyStateType::Chase;
					enemy.stateTimer = 0.0f;
					Action::CombatInternal::ClearEnemyPathState(enemy);
					enemy.hasWanderTarget = false;
				}
			}
			else
			{
				// Non-lock target: use squared-dot test to avoid sqrt
				if (fwdDot <= 0.0f)
				{
					continue;
				}
				const float cosThresh = m_attackDotThreshold;
				const float cosThreshSq = cosThresh * cosThresh;
				if ((fwdDot * fwdDot) < (cosThreshSq * distSq))
				{
					continue;
				}

				// Normalize only when applying knockback etc.
				const float dist = std::sqrt(distSq);
				normalized++;
				Vector3 toEnemyN = toEnemy / dist;

				// Apply hit
				enemy.hitByCurrentSwing = true;
				enemy.hp -= static_cast<float>(damage);
				enemy.hitReactTimer = m_tuning.enemyHitStunSec;
				enemy.knockbackVelocity = toEnemyN * (m_tuning.enemyKnockbackSpeed + static_cast<float>(player.comboIndex - 1) * 0.8f);
				hitAny = true;
				hits++;

				player.comboGauge = (std::min)(100.0f, player.comboGauge + m_tuning.comboGaugeHitGain);

				if (enemy.hp <= 0.0f)
				{
					enemy.hp = 0.0f;
					enemy.state = EnemyStateType::Dead;
					enemy.hitReactTimer = 0.0f;
					enemy.knockbackVelocity = Vector3::Zero;
					Action::CombatInternal::ClearEnemyPathState(enemy);
					enemy.hasWanderTarget = false;
					gameState.killCount += 1;
					player.comboGauge = (std::min)(100.0f, player.comboGauge + m_tuning.comboGaugeKillBonus);
					killedAny = true;
				}
				else
				{
					enemy.state = EnemyStateType::Chase;
					enemy.stateTimer = 0.0f;
					Action::CombatInternal::ClearEnemyPathState(enemy);
					enemy.hasWanderTarget = false;
				}
			}

			// enforce per-swing hit cap
			if (hits >= kMaxHitsPerSwing)
			{
				break;
			}
		}

		if (!hitAny && player.attackTriggeredThisFrame)
		{
			player.comboGauge = std::max(0.0f, player.comboGauge - m_tuning.comboMissPenalty);
		}
		if (killedAny)
		{
			GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::EnemyDestroy, 0.90f);
		}

	}
}
