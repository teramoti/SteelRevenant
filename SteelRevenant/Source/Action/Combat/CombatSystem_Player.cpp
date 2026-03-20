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
		player.attackTriggeredThisFrame = false;

		player.yaw = Utility::MathEx::WrapRadians(player.yaw + input.mouseDelta.x * m_tuning.playerYawScale);

		player.damageGraceTimer = std::max(0.0f, player.damageGraceTimer - dt);
		player.attackTimer = std::max(0.0f, player.attackTimer - dt);
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

		if (player.attackTimer <= 0.0f && player.comboTimer <= 0.0f)
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
			moveRight = Vector3(moveForward.z, 0.0f, -moveForward.x);
		}

		Vector3 moveDir = Vector3::Zero;
		if (input.moveForward) { moveDir += moveForward; }
		if (input.moveBack) { moveDir -= moveForward; }
		if (input.moveRight) { moveDir += moveRight; }
		if (input.moveLeft) { moveDir -= moveRight; }
		moveDir = Utility::MathEx::SafeNormalize(moveDir);

		if (input.attackPressed && player.attackCooldown <= 0.0f)
		{
			if (player.comboTimer > 0.0f)
			{
				player.comboIndex = std::min(3, player.comboIndex + 1);
			}
			else
			{
				player.comboIndex = 1;
			}

			player.attackTimer = m_tuning.comboAttackWindowSec;
			player.attackCooldown = m_tuning.comboCooldownSec;
			player.comboTimer = m_tuning.comboChainWindowSec;
			player.attackTriggeredThisFrame = true;
			GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::MeleeSlash, 0.82f);
		}

		player.guarding = input.guardHeld;

		const Vector3 targetVelocity = moveDir * m_tuning.walkSpeed;
		const float gain = (moveDir.LengthSquared() > 0.0001f) ? m_tuning.moveAccelGain : m_tuning.moveStopGain;
		const float blend = Utility::MathEx::Clamp(1.0f - std::exp(-gain * dt), 0.0f, 1.0f);
		player.moveVelocity += (targetVelocity - player.moveVelocity) * blend;

		player.position += player.moveVelocity * dt;
		player.position.y = playerGroundY;

		player.position.x = Utility::MathEx::Clamp(player.position.x, m_tuning.playerBoundsMin, m_tuning.playerBoundsMax);
		player.position.z = Utility::MathEx::Clamp(player.position.z, m_tuning.playerBoundsMin, m_tuning.playerBoundsMax);
	}

// プレイヤー攻撃のヒット判定とダメージを解決する。

	void CombatSystem::ResolvePlayerAttack(
		PlayerState& player,
		std::vector<EnemyState>& enemies,
		GameState& gameState) const
	{
		if (player.attackTimer <= 0.0f || player.comboIndex <= 0)
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
		for (size_t i = 0; i < enemies.size(); ++i)
		{
			EnemyState& enemy = enemies[i];
			if (enemy.state == EnemyStateType::Dead || enemy.hitByCurrentSwing)
			{
				continue;
			}

			Vector3 toEnemy = enemy.position - player.position;
			toEnemy.y = 0.0f;
			const float dist = toEnemy.Length();
			if (dist > m_attackRange || dist <= 0.001f)
			{
				// ロックオン中は「ギリ届かない」ケースだけ救済して気持ちよさを優先する。
				if (!(hasLockTarget && static_cast<int>(i) == player.lockEnemyIndex && dist <= (m_attackRange * m_tuning.lockAssistRangeMultiplier)))
				{
					continue;
				}
			}

			toEnemy /= dist;
			// ロックオン対象への攻撃は、軽く正面補正して当てやすくする（オリジナリティ枠）。
			if (hasLockTarget && static_cast<int>(i) == player.lockEnemyIndex)
			{
				forward = Utility::MathEx::SafeNormalize(Vector3(toEnemy.x, 0.0f, toEnemy.z));
			}
			const float facing = forward.Dot(toEnemy);
			if (facing < m_attackDotThreshold)
			{
				if (!(hasLockTarget && static_cast<int>(i) == player.lockEnemyIndex && facing >= (m_attackDotThreshold - m_tuning.lockAssistDotRelax)))
				{
					continue;
				}
			}

			enemy.hitByCurrentSwing = true;
			enemy.hp -= static_cast<float>(damage);
			hitAny = true;

			player.comboGauge = std::min(100.0f, player.comboGauge + m_tuning.comboGaugeHitGain);

			if (enemy.hp <= 0.0f)
			{
				enemy.hp = 0.0f;
				enemy.state = EnemyStateType::Dead;
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.hasWanderTarget = false;
				gameState.killCount += 1;
				player.comboGauge = std::min(100.0f, player.comboGauge + m_tuning.comboGaugeKillBonus);
				killedAny = true;
			}
			else
			{
				enemy.state = EnemyStateType::Chase;
				enemy.stateTimer = std::max(enemy.stateTimer, m_tuning.enemyHitStunSec);
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.hasWanderTarget = false;
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
