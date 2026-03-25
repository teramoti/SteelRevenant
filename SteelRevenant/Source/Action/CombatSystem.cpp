//------------------------//------------------------
// Contents(処理内容) プレイヤー戦闘、敵AI、移動更新を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "CombatSystem.h"

#include "NavMesh.h"
#include "Combat/CombatEnemyStateUtil.h"
#include "../Utility/SimpleMathEx.h"
#include "../Utility/Sound/AudioSystem.h"

#include <algorithm>
#include <cmath>
#include <random>

using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kPlayerYawScale = 0.022f;
	constexpr float kWalkSpeed = 7.8f;
	constexpr float kMoveAccelGain = 13.0f;
	constexpr float kMoveStopGain = 19.0f;
	constexpr float kDamageGraceSec = 0.30f;
	constexpr float kPlayerBoundsMin = -21.5f;
	constexpr float kPlayerBoundsMax = 21.5f;

	constexpr float kPlayerAttackDamage1 = 18.0f;
	constexpr float kPlayerAttackDamage2 = 24.0f;
	constexpr float kPlayerAttackDamage3 = 32.0f;
	constexpr float kComboDamageBonusPerLevel = 0.15f;
	constexpr float kComboLevelGauge1 = 20.0f;
	constexpr float kComboLevelGauge2 = 50.0f;
	constexpr float kComboLevelGauge3 = 80.0f;
	constexpr float kComboMissPenalty = 6.0f;
	constexpr float kLockAssistRangeMultiplier = 1.25f;
	constexpr float kLockAssistDotRelax = 0.12f;
	constexpr float kAttackAssistRangeDefault = 2.6f;
	constexpr float kAttackAssistDotDefault = 0.30f;

	constexpr float kComboAttackWindowSec = 0.11f;
	constexpr float kComboCooldownSec = 0.14f;
	constexpr float kComboChainWindowSec = 0.38f;
	constexpr float kComboGaugeDecayPerSec = 7.5f;
	constexpr float kComboGaugeHitGain = 16.0f;
	constexpr float kComboGaugeKillBonus = 7.0f;
	constexpr float kComboAttackWindupSec = 0.06f;
	constexpr float kComboAttackActiveSec = 0.09f;
	constexpr float kComboAttackFollowThroughSec = 0.05f;
	constexpr float kComboAttackRecoverSec = 0.08f;

	constexpr float kEnemyRepathSec = 0.25f;
	constexpr float kEnemyMeleeAttackTriggerDistance = 1.9f;
	constexpr float kEnemyMeleeAttackReach = 2.0f;
	constexpr float kEnemyFlankOffset = 6.0f;
	constexpr float kEnemyAttackWindupSec = 0.42f;
	constexpr float kEnemyMoveSpeedBase = 3.9f;
	constexpr float kEnemyMoveSpeedPerDanger = 0.58f;
	constexpr float kEnemyIdleWakeupDistance = 36.0f;
	constexpr float kEnemyReturnWakeupDistance = 8.5f;
	constexpr float kEnemyLeashDistance = 28.0f;
	constexpr float kEnemyReturnArriveDistance = 0.95f;
	constexpr float kEnemyHitStunSec = 0.26f;
	constexpr float kEnemyKnockbackSpeed = 6.8f;
	constexpr float kEnemyKnockbackDamping = 8.5f;

	constexpr float kEnemyMeleeDamageBase = 12.0f;
	constexpr float kEnemyMeleeDamagePerDanger = 2.3f;
	constexpr float kEnemyMeleeGuardDamageScale = 0.35f;
	constexpr float kEnemyWanderSpeedScale = 0.55f;
	constexpr float kEnemyWanderMinDistance = 4.0f;
	constexpr int kEnemyWanderCandidateAttempts = 8;
	constexpr float kEnemyWanderPauseMinSec = 0.6f;
	constexpr float kEnemyWanderPauseMaxSec = 1.4f;

	constexpr float kAttackRangeDefault = 2.6f;
	constexpr float kAttackDotThresholdDefault = 0.30f;
	constexpr float kLaserPreferredRange = 8.8f;
	constexpr float kLaserRetreatRange = 5.4f;
	constexpr float kLaserPressureRange = 12.5f;

// 敵AIの意思決定結果。
	enum class EnemyDecisionIntent
	{
		None,
		Idle,
		Wander,
		Chase,
		Attack,
		ReturnToSpawn
	};

// BehaviorTreeへ渡す敵1体分の評価文脈。
	struct EnemyDecisionContext
	{
		Action::EnemyState& enemy;
		float distToPlayer;
		float distToSpawn;
		EnemyDecisionIntent intent;

		// 敵 1 体分の意思決定文脈を初期化する。
		EnemyDecisionContext(Action::EnemyState& targetEnemy, float playerDistance, float spawnDistance)
			: enemy(targetEnemy)
			, distToPlayer(playerDistance)
			, distToSpawn(spawnDistance)
			, intent(EnemyDecisionIntent::None)
		{
		}
	};

// XZ平面上の距離を求める。
	float FlatDistance(const Vector3& a, const Vector3& b)
	{
		Vector3 d = b - a;
		d.y = 0.0f;
		return d.Length();
	}

	Vector3 SafeNormalizeWithFallback(const Vector3& value, const Vector3& fallback)
	{
		Vector3 result = Utility::MathEx::SafeNormalize(value);
		if (result.LengthSquared() <= 0.0001f)
		{
			result = Utility::MathEx::SafeNormalize(fallback);
		}
		return (result.LengthSquared() > 0.0001f) ? result : Vector3::UnitZ;
	}

	// RNG から指定範囲の実数を生成する。
	float RandomFloat(std::mt19937& rng, float minValue, float maxValue)
	{
		std::uniform_real_distribution<float> dist(minValue, maxValue);
		return dist(rng);
	}

	// RNG から指定範囲の整数を生成する。
	int RandomInt(std::mt19937& rng, int minValue, int maxValue)
	{
		std::uniform_int_distribution<int> dist(minValue, maxValue);
		return dist(rng);
	}

// 意思決定結果をEnemyStateへ反映する。
	void ApplyEnemyIntent(Action::EnemyState& enemy, EnemyDecisionIntent intent, const Action::CombatTuning& tuning)
	{
		switch (intent)
		{
		case EnemyDecisionIntent::Idle:
			if (enemy.state != Action::EnemyStateType::Idle)
			{
				enemy.state = Action::EnemyStateType::Idle;
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.hasWanderTarget = false;
			}
			break;

		case EnemyDecisionIntent::Wander:
			if (enemy.state != Action::EnemyStateType::Wander)
			{
				enemy.state = Action::EnemyStateType::Wander;
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.hasWanderTarget = false;
			}
			break;

		case EnemyDecisionIntent::Chase:
			if (enemy.state != Action::EnemyStateType::Chase)
			{
				enemy.state = Action::EnemyStateType::Chase;
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.hasWanderTarget = false;
			}
			break;

		case EnemyDecisionIntent::Attack:
			if (enemy.state != Action::EnemyStateType::Attack)
			{
				enemy.state = Action::EnemyStateType::Attack;
				enemy.stateTimer = tuning.enemyAttackWindupSec * enemy.attackWindupScale;
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.hasWanderTarget = false;
			}
			break;

		case EnemyDecisionIntent::ReturnToSpawn:
			if (enemy.state != Action::EnemyStateType::Return)
			{
				enemy.state = Action::EnemyStateType::Return;
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.hasWanderTarget = false;
			}
			break;

		case EnemyDecisionIntent::None:
		default:
			break;
		}
	}

	// ?????????????????
	Action::PathGrid::GridCoord FindNearestWalkable(
		const Action::PathGrid& grid,
		const Action::PathGrid::GridCoord& origin)
	{
		if (grid.IsWalkable(origin))
		{
			return origin;
		}

		const int maxRadius = 6;
		for (int r = 1; r <= maxRadius; ++r)
		{
			for (int dy = -r; dy <= r; ++dy)
			{
				for (int dx = -r; dx <= r; ++dx)
				{
					if (std::abs(dx) != r && std::abs(dy) != r)
					{
						continue;
					}
					const Action::PathGrid::GridCoord candidate(origin.x + dx, origin.y + dy);
					if (grid.IsWalkable(candidate))
					{
						return candidate;
					}
				}
			}
		}

		return origin;
	}

	// NavMesh ? A* ??????????????
	std::vector<Action::PathGrid::GridCoord> BuildHybridPath(
		const Action::PathGrid& grid,
		const Action::AStarSolver& solver,
		const Action::NavMeshGraph& navMesh,
		const Vector3& startWorld,
		const Vector3& goalWorld)
	{
		Action::PathGrid::GridCoord start = grid.WorldToGrid(startWorld);
		Action::PathGrid::GridCoord goal = grid.WorldToGrid(goalWorld);
		start = FindNearestWalkable(grid, start);
		goal = FindNearestWalkable(grid, goal);

		const int manhattan = std::abs(start.x - goal.x) + std::abs(start.y - goal.y);
		if (manhattan >= 10)
		{
			const std::vector<Action::PathGrid::GridCoord> navPath = navMesh.FindPath(grid, start, goal);
			if (navPath.size() > 1)
			{
				return navPath;
			}
		}

		return solver.FindPath(grid, start, goal);
	}

	// 役割に応じた追跡目標位置を計算する。
	Vector3 BuildRoleTarget(const Action::EnemyState& enemy, const Vector3& playerPos, float playerYaw, const Action::CombatTuning& tuning)
	{
		Vector3 target = playerPos;
		const Vector3 playerRight(std::cos(playerYaw), 0.0f, -std::sin(playerYaw));

		if (enemy.isLaserEnemy)
		{
			Vector3 away = enemy.position - playerPos;
			away.y = 0.0f;
			away = SafeNormalizeWithFallback(away, playerRight);
			const float sign = (std::sin(enemy.spawnPosition.x + enemy.spawnPosition.z) >= 0.0f) ? 1.0f : -1.0f;
			target = playerPos + away * kLaserPreferredRange + playerRight * sign * 3.2f;
			return target;
		}

		if (enemy.moveRole == Action::EnemyMoveRole::Flank)
		{
			const float sign = (std::sin(enemy.spawnPosition.x + enemy.spawnPosition.z) >= 0.0f) ? 1.0f : -1.0f;
			target += playerRight * (tuning.enemyFlankOffset * sign);
		}

		return target;
	}

	// 徘徊先を歩行可能セルから抽選する。
	Vector3 PickWanderTarget(
		const Action::EnemyState& enemy,
		const Action::PathGrid& grid,
		std::mt19937& rng)
	{
		for (int attempt = 0; attempt < kEnemyWanderCandidateAttempts; ++attempt)
		{
			const int x = RandomInt(rng, 0, std::max(0, grid.GetWidth() - 1));
			const int y = RandomInt(rng, 0, std::max(0, grid.GetHeight() - 1));
			const Action::PathGrid::GridCoord candidate(x, y);
			if (!grid.IsWalkable(candidate))
			{
				continue;
			}

			const Vector3 target = grid.GridToWorld(candidate, enemy.position.y);
			if (FlatDistance(enemy.position, target) < kEnemyWanderMinDistance)
			{
				continue;
			}
			return target;
		}

		const Action::PathGrid::GridCoord fallback = FindNearestWalkable(grid, grid.WorldToGrid(enemy.spawnPosition));
		return grid.GridToWorld(fallback, enemy.position.y);
	}

	// 危険度を反映した移動速度を返す。
	float BuildEnemyMoveSpeed(const Action::CombatTuning& tuning, const Action::EnemyState& enemy, int dangerLevel)
	{
		const float dangerBoost = static_cast<float>(std::max(0, dangerLevel - 1));
		// incorporate enemy.baseMoveSpeed to allow per-enemy speed tuning
		return (tuning.enemyMoveSpeedBase + dangerBoost * tuning.enemyMoveSpeedPerDanger) * enemy.moveSpeedScale * enemy.baseMoveSpeed;
	}

// ワールド座標を目標に再経路探索する。
	void RepathToWorld(
		Action::EnemyState& enemy,
		const Action::PathGrid& grid,
		const Action::AStarSolver& solver,
		const Action::NavMeshGraph& navMesh,
		const Vector3& goalWorld)
	{
		enemy.path = BuildHybridPath(grid, solver, navMesh, enemy.position, goalWorld);
		enemy.pathCursor = (enemy.path.size() > 1) ? 1u : 0u;
	}

// 現在の経路に沿って1フレーム移動する。
	Vector3 MoveAlongPath(Action::EnemyState& enemy, const Action::PathGrid& grid, float moveSpeed, float dt)
	{
		if (!enemy.path.empty() && enemy.pathCursor < enemy.path.size())
		{
			Vector3 target = grid.GridToWorld(enemy.path[enemy.pathCursor], enemy.position.y);
			Vector3 dir = target - enemy.position;
			dir.y = 0.0f;

			if (dir.LengthSquared() <= 0.25f)
			{
				enemy.pathCursor += 1;
				return Vector3::Zero;
			}

			dir.Normalize();
			enemy.position += dir * moveSpeed * dt;
			return dir;
		}

		return Vector3::Zero;
	}
}

namespace Action
{
// 既定パラメータで戦闘システムを構築する。
		CombatSystem::CombatSystem()
			: m_tuning()
			, m_attackRange(kAttackRangeDefault)
			, m_attackDotThreshold(kAttackDotThresholdDefault)
			, m_rng(std::random_device{}())
		{
			m_tuning.playerYawScale = kPlayerYawScale;
			m_tuning.walkSpeed = kWalkSpeed;
			m_tuning.moveAccelGain = kMoveAccelGain;
			m_tuning.moveStopGain = kMoveStopGain;
			m_tuning.damageGraceSec = kDamageGraceSec;
			m_tuning.playerBoundsMin = kPlayerBoundsMin;
			m_tuning.playerBoundsMax = kPlayerBoundsMax;
			m_tuning.playerAttackDamage1 = kPlayerAttackDamage1;
			m_tuning.playerAttackDamage2 = kPlayerAttackDamage2;
			m_tuning.playerAttackDamage3 = kPlayerAttackDamage3;
			m_tuning.comboDamageBonusPerLevel = kComboDamageBonusPerLevel;
			m_tuning.comboLevelGauge1 = kComboLevelGauge1;
			m_tuning.comboLevelGauge2 = kComboLevelGauge2;
			m_tuning.comboLevelGauge3 = kComboLevelGauge3;
			m_tuning.comboMissPenalty = kComboMissPenalty;
			m_tuning.lockAssistRangeMultiplier = kLockAssistRangeMultiplier;
			m_tuning.lockAssistDotRelax = kLockAssistDotRelax;
			m_tuning.attackAssistRangeDefault = kAttackAssistRangeDefault;
			m_tuning.attackAssistDotDefault = kAttackAssistDotDefault;
			m_tuning.comboAttackWindowSec = kComboAttackWindowSec;
			m_tuning.comboCooldownSec = kComboCooldownSec;
			m_tuning.comboChainWindowSec = kComboChainWindowSec;
			m_tuning.comboGaugeDecayPerSec = kComboGaugeDecayPerSec;
			m_tuning.comboGaugeHitGain = kComboGaugeHitGain;
			m_tuning.comboGaugeKillBonus = kComboGaugeKillBonus;
			m_tuning.comboAttackWindupSec = kComboAttackWindupSec;
			m_tuning.comboAttackActiveSec = kComboAttackActiveSec;
			m_tuning.comboAttackFollowThroughSec = kComboAttackFollowThroughSec;
			m_tuning.comboAttackRecoverSec = kComboAttackRecoverSec;
			m_tuning.enemyRepathSec = kEnemyRepathSec;
			m_tuning.enemyMeleeAttackTriggerDistance = kEnemyMeleeAttackTriggerDistance;
			m_tuning.enemyMeleeAttackReach = kEnemyMeleeAttackReach;
			m_tuning.enemyFlankOffset = kEnemyFlankOffset;
			m_tuning.enemyAttackWindupSec = kEnemyAttackWindupSec;
			m_tuning.enemyMoveSpeedBase = kEnemyMoveSpeedBase;
			m_tuning.enemyMoveSpeedPerDanger = kEnemyMoveSpeedPerDanger;
			m_tuning.enemyIdleWakeupDistance = kEnemyIdleWakeupDistance;
			m_tuning.enemyReturnWakeupDistance = kEnemyReturnWakeupDistance;
			m_tuning.enemyLeashDistance = kEnemyLeashDistance;
			m_tuning.enemyReturnArriveDistance = kEnemyReturnArriveDistance;
			m_tuning.enemyHitStunSec = kEnemyHitStunSec;
			m_tuning.enemyKnockbackSpeed = kEnemyKnockbackSpeed;
			m_tuning.enemyKnockbackDamping = kEnemyKnockbackDamping;
			m_tuning.enemyMeleeDamageBase = kEnemyMeleeDamageBase;
			m_tuning.enemyMeleeDamagePerDanger = kEnemyMeleeDamagePerDanger;
			m_tuning.enemyMeleeGuardDamageScale = kEnemyMeleeGuardDamageScale;
		}

		// 戦闘調整値を現在設定へ反映する。
		void CombatSystem::SetTuning(const CombatTuning& tuning)
		{
			m_tuning = tuning;
		}

		// 現在の戦闘調整値を返す。
		const CombatTuning& CombatSystem::GetTuning() const
		{
			return m_tuning;
		}

// 攻撃吸着距離と正面判定角度を更新する。
	void CombatSystem::SetAttackAssistConfig(float attackRange, float attackDotThreshold)
	{
		m_attackRange = Utility::MathEx::Clamp(attackRange, 1.4f, 4.5f);
		m_attackDotThreshold = Utility::MathEx::Clamp(attackDotThreshold, -0.25f, 0.95f);
	}

// プレイヤーの移動、攻撃、ガード状態を更新する。
	void CombatSystem::UpdateEnemies(
		std::vector<EnemyState>& enemies,
		PlayerState& player,
		const InputSnapshot& input,
		const PathGrid& grid,
		const AStarSolver& solver,
		GameState& gameState) const
	{
		const float dt = std::max(0.0f, input.dt);

		static NavMeshGraph s_navMesh;
		if (!s_navMesh.IsBuiltFor(grid))
		{
			s_navMesh.Build(grid, 3);
		}

		std::vector<size_t> enemyRankByIndex(enemies.size(), enemies.size());
		std::vector<size_t> order;
		order.reserve(enemies.size());
		for (size_t i = 0; i < enemies.size(); ++i)
		{
			if (enemies[i].state != EnemyStateType::Dead)
			{
				order.push_back(i);
			}
		}
		std::sort(order.begin(), order.end(), [&enemies, &player](size_t lhs, size_t rhs)
		{
			return FlatDistance(enemies[lhs].position, player.position) < FlatDistance(enemies[rhs].position, player.position);
		});
		for (size_t rank = 0; rank < order.size(); ++rank)
		{
			enemyRankByIndex[order[rank]] = rank;
		}

		if (player.attackTriggeredThisFrame)
		{
			for (size_t i = 0; i < enemies.size(); ++i)
			{
				enemies[i].hitByCurrentSwing = false;
			}
		}


		for (size_t i = 0; i < enemies.size(); ++i)
		{
			EnemyState& enemy = enemies[i];
			if (enemy.state == EnemyStateType::Dead)
			{
				continue;
			}

			enemy.stateTimer = std::max(0.0f, enemy.stateTimer - dt);
			enemy.repathTimer = std::max(0.0f, enemy.repathTimer - dt);
			enemy.wanderPauseSec = std::max(0.0f, enemy.wanderPauseSec - dt);
			enemy.hitReactTimer = std::max(0.0f, enemy.hitReactTimer - dt);

			// --- レーザー敵の予兆→発射遷移 ---
			if (enemy.isLaserEnemy)
			{
				const float toPlayerDist = FlatDistance(enemy.position, player.position);
				if (enemy.laserWarningActive)
				{
					enemy.laserPreDelayTimer -= dt;
					if (enemy.laserAimDirection.LengthSquared() <= 0.0001f)
					{
						enemy.laserAimDirection = SafeNormalizeWithFallback(player.position - enemy.position, Vector3::UnitZ);
					}
					enemy.yaw = std::atan2(enemy.laserAimDirection.x, enemy.laserAimDirection.z);
					if (enemy.laserPreDelayTimer <= 0.0f)
					{
						enemy.laserWarningActive = false;
						enemy.laserFireTimer = 0.95f;
						GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::LaserFire, 1.0f);
					}
				}
				else if (enemy.laserFireTimer > 0.0f)
				{
					enemy.laserFireTimer -= dt;
					const Vector3 beamDir = SafeNormalizeWithFallback(enemy.laserAimDirection, Vector3::UnitZ);
					enemy.yaw = std::atan2(beamDir.x, beamDir.z);
					const float beamRange = 24.0f;
					const float beamRadius = 1.15f;
					if (toPlayerDist <= beamRange)
					{
						Vector3 toPlayer = player.position - enemy.position;
						toPlayer.y = 0.0f;
						const float alongBeam = beamDir.Dot(toPlayer);
						const float lateralSq = std::max(0.0f, toPlayer.LengthSquared() - alongBeam * alongBeam);
						if (alongBeam > 0.0f && alongBeam <= beamRange && lateralSq <= beamRadius * beamRadius)
						{
							if (!player.isSliding)
							{
								float damagePerSec = 2.2f * enemy.timeDamageMultiplier;
								const float drain = damagePerSec * dt;
								gameState.stageTimer = std::max(0.0f, gameState.stageTimer - drain);
								gameState.damageTaken += static_cast<int>(std::round(drain * 10.0f));
								if (gameState.stageTimer <= 0.0f)
								{
									gameState.stageTimer = 0.0f;
									gameState.timeExpired = true;
								}
								if (player.damageGraceTimer <= 0.0f)
								{
									player.damageGraceTimer = m_tuning.damageGraceSec;
									GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::PlayerHit, 0.9f);
								}
							}
						}
					}

					if (enemy.laserFireTimer <= 0.0f)
					{
						enemy.laserPreDelayTimer = 1.35f + static_cast<float>(i % 3) * 0.2f;
						enemy.laserWarningActive = false;
					}
				}
				else
				{
					enemy.laserPreDelayTimer = std::max(0.0f, enemy.laserPreDelayTimer - dt);
					if (enemy.laserPreDelayTimer <= 0.0f &&
						toPlayerDist >= kLaserRetreatRange &&
						toPlayerDist <= kLaserPressureRange)
					{
						enemy.laserWarningActive = true;
						enemy.laserPreDelayTimer = 0.68f;
						enemy.laserAimDirection = SafeNormalizeWithFallback(player.position - enemy.position, Vector3::UnitZ);
						GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::LaserCharge, 0.95f);
					}
				}
			}

			// --- 突進敵の突進行動 ---
			if (enemy.isDashingEnemy)
			{
				const float dashDistance = FlatDistance(enemy.position, player.position);
				if (enemy.dashActive)
				{
					enemy.dashTimer -= dt;
					Vector3 dashDir = SafeNormalizeWithFallback(player.position - enemy.position, Vector3::UnitZ);
					enemy.position += dashDir * 14.0f * dt;
					enemy.yaw = std::atan2(dashDir.x, dashDir.z);
					const float hitRange = 1.2f;
					if (FlatDistance(enemy.position, player.position) <= hitRange)
					{
						if (player.damageGraceTimer <= 0.0f && !player.isSliding)
						{
							const float dashDrain = 3.0f * enemy.timeDamageMultiplier;
							gameState.stageTimer = std::max(0.0f, gameState.stageTimer - dashDrain);
							gameState.damageTaken += static_cast<int>(std::round(dashDrain * 10.0f));
							player.damageGraceTimer = m_tuning.damageGraceSec;
							GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::PlayerHit, 1.0f);
						}
						enemy.dashActive = false;
						enemy.dashPreDelayTimer = 1.2f;
						GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::ChargeWarning, 0.9f);
					}
					if (enemy.dashTimer <= 0.0f)
					{
						enemy.dashActive = false;
						enemy.dashPreDelayTimer = 1.2f;
						GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::ChargeWarning, 0.9f);
					}
				}
				else
				{
					enemy.dashPreDelayTimer -= dt;
					if (enemy.dashPreDelayTimer <= 0.0f && dashDistance <= 12.0f)
					{
						enemy.dashActive = true;
						enemy.dashTimer = 0.45f;
					}
				}
			}

			if (enemy.isDashingEnemy && enemy.dashActive)
			{
				continue;
			}

			if (enemy.hitReactTimer > 0.0f)
			{
				enemy.position += enemy.knockbackVelocity * dt;
				const float knockbackBlend = Utility::MathEx::Clamp(1.0f - std::exp(-m_tuning.enemyKnockbackDamping * dt), 0.0f, 1.0f);
				enemy.knockbackVelocity += (Vector3::Zero - enemy.knockbackVelocity) * knockbackBlend;
				if (enemy.knockbackVelocity.LengthSquared() > 0.0001f)
				{
					enemy.yaw = std::atan2(enemy.knockbackVelocity.x, enemy.knockbackVelocity.z);
				}
				continue;
			}

			const float distToPlayer = FlatDistance(enemy.position, player.position);
			const float distToSpawn = FlatDistance(enemy.position, enemy.spawnPosition);
			const size_t lodRank = enemyRankByIndex[i];
			const float repathLodScale = (lodRank < 20) ? 1.0f : ((lodRank < 40) ? 2.0f : 4.0f);
			// use enemy.aggroRadius when available; fallback to tuning
			const float wakeupDistance = (enemy.aggroRadius > 0.0f) ? enemy.aggroRadius : m_tuning.enemyIdleWakeupDistance;
			// determine trigger distance from preferredRange; melee slightly larger to be aggressive
			const float triggerDistance = (enemy.preferredRange > 0.0f)
				? ((enemy.isRanged) ? enemy.preferredRange : (enemy.preferredRange * 1.05f))
				: m_tuning.enemyMeleeAttackTriggerDistance * enemy.attackRangeScale;
			const bool laserHoldingFireLane = enemy.isLaserEnemy &&
				(distToPlayer >= kLaserRetreatRange && distToPlayer <= kLaserPressureRange);

			// tuning反映版の意思決定（BTより優先）。
			EnemyDecisionContext context(enemy, distToPlayer, distToSpawn);
			context.intent = EnemyDecisionIntent::None;
			if (enemy.state == EnemyStateType::Attack && enemy.stateTimer > 0.0f)
			{
				context.intent = EnemyDecisionIntent::Attack;
			}
			else if (enemy.state == EnemyStateType::Idle && enemy.stateTimer > 0.0f)
			{
				context.intent = EnemyDecisionIntent::Idle;
			}
			else
			{
				if (enemy.state == EnemyStateType::Return &&
					distToSpawn > m_tuning.enemyReturnArriveDistance &&
					distToPlayer > m_tuning.enemyReturnWakeupDistance)
				{
					context.intent = EnemyDecisionIntent::ReturnToSpawn;
				}
				else if (distToSpawn > (m_tuning.enemyLeashDistance * enemy.leashScale) &&
					distToPlayer > m_tuning.enemyReturnWakeupDistance)
				{
					context.intent = EnemyDecisionIntent::ReturnToSpawn;
				}
				else if (enemy.isLaserEnemy && (enemy.laserWarningActive || enemy.laserFireTimer > 0.0f) && laserHoldingFireLane)
				{
					context.intent = EnemyDecisionIntent::Idle;
				}
				// use aggroRadius to decide to wake/engage
				else if (distToPlayer <= wakeupDistance ||
					enemy.state == EnemyStateType::Chase ||
					enemy.state == EnemyStateType::Attack)
				{
					// if within preferred attack distance => Attack, else Chase
					if (distToPlayer <= triggerDistance && enemy.stateTimer <= 0.0f)
					{
						context.intent = EnemyDecisionIntent::Attack;
					}
					else
					{
						context.intent = EnemyDecisionIntent::Chase;
					}
				}
				else
				{
					context.intent = EnemyDecisionIntent::Chase;
				}
			}
			ApplyEnemyIntent(enemy, context.intent, m_tuning);

			Vector3 moveDir = Vector3::Zero;
			if (enemy.state == EnemyStateType::Wander)
			{
				// Replace random wandering with direct role-based pursuit to increase pressure on player.
				Vector3 chaseTarget = BuildRoleTarget(enemy, player.position, player.yaw, m_tuning);
				Vector3 directToTarget = chaseTarget - enemy.position;
				directToTarget.y = 0.0f;
				const float moveSpeed = BuildEnemyMoveSpeed(m_tuning, enemy, gameState.dangerLevel);
				// Direct steer toward the role target (no random wander pathfinding)
				Vector3 steer = SafeNormalizeWithFallback(directToTarget, player.position - enemy.position);
				enemy.position += steer * moveSpeed * dt;
				moveDir = steer;
				// keep path state cleared to avoid periodic pathfinding overhead
				Action::CombatInternal::ClearEnemyPathState(enemy);
				enemy.repathTimer = m_tuning.enemyRepathSec * enemy.repathIntervalScale * repathLodScale;
			}
			else if (enemy.state == EnemyStateType::Chase)
			{
				Vector3 chaseTarget = BuildRoleTarget(enemy, player.position, player.yaw, m_tuning);
				Vector3 directToTarget = chaseTarget - enemy.position;
				directToTarget.y = 0.0f;
				const float moveSpeed = BuildEnemyMoveSpeed(m_tuning, enemy, gameState.dangerLevel);
				// For ranged enemies, try to maintain preferredRange (retreat/hold)
				if (enemy.isRanged || enemy.isLaserEnemy)
				{
					Vector3 toPlayer = player.position - enemy.position;
					toPlayer.y = 0.0f;
					const float dist = toPlayer.Length();
					const float hysteresis = 0.6f; // avoid jitter
					if (dist > enemy.preferredRange + hysteresis)
					{
						Vector3 dir = SafeNormalizeWithFallback(toPlayer, Vector3::UnitZ);
						enemy.position += dir * moveSpeed * dt;
						moveDir = dir;
						Action::CombatInternal::ClearEnemyPathState(enemy);
						enemy.repathTimer = m_tuning.enemyRepathSec * enemy.repathIntervalScale * repathLodScale;
					}
					else if (dist < enemy.preferredRange - hysteresis)
					{
						// move away slightly to maintain distance
						Vector3 away = SafeNormalizeWithFallback(enemy.position - player.position, Vector3::UnitZ);
						enemy.position += away * moveSpeed * 0.85f * dt;
						moveDir = away;
						Action::CombatInternal::ClearEnemyPathState(enemy);
						enemy.repathTimer = m_tuning.enemyRepathSec * enemy.repathIntervalScale * repathLodScale;
					}
					else
					{
						// hold position
						moveDir = Vector3::Zero;
					}
				}
				else
				{
					const bool useDirectSteer = directToTarget.LengthSquared() <= (enemy.isLaserEnemy ? 64.0f : 90.0f);
					if (useDirectSteer)
					{
						moveDir = SafeNormalizeWithFallback(directToTarget, player.position - enemy.position);
						enemy.position += moveDir * moveSpeed * dt;
						Action::CombatInternal::ClearEnemyPathState(enemy);
						enemy.repathTimer = m_tuning.enemyRepathSec * enemy.repathIntervalScale * repathLodScale;
					}
					else
					{
						if (enemy.repathTimer <= 0.0f || enemy.path.empty() || enemy.pathCursor >= enemy.path.size())
						{
							RepathToWorld(enemy, grid, solver, s_navMesh, chaseTarget);
							enemy.repathTimer = m_tuning.enemyRepathSec * enemy.repathIntervalScale * repathLodScale;
						}
						moveDir = MoveAlongPath(enemy, grid, moveSpeed, dt);
					}
				}
			}
			else if (enemy.state == EnemyStateType::Return)
			{
				if (distToSpawn <= m_tuning.enemyReturnArriveDistance)
				{
					enemy.state = EnemyStateType::Idle;
					enemy.stateTimer = RandomFloat(m_rng, kEnemyWanderPauseMinSec, kEnemyWanderPauseMaxSec);
					enemy.wanderPauseSec = enemy.stateTimer;
					enemy.hasWanderTarget = false;
					Action::CombatInternal::ClearEnemyPathState(enemy);
				}
				else
				{
					if (enemy.repathTimer <= 0.0f || enemy.path.empty() || enemy.pathCursor >= enemy.path.size())
					{
						RepathToWorld(enemy, grid, solver, s_navMesh, enemy.spawnPosition);
						enemy.repathTimer = m_tuning.enemyRepathSec * enemy.repathIntervalScale * repathLodScale;
					}
					moveDir = MoveAlongPath(enemy, grid, BuildEnemyMoveSpeed(m_tuning, enemy, gameState.dangerLevel) * 0.9f, dt);
				}
			}
			else if (enemy.state == EnemyStateType::Attack)
			{
				if (enemy.stateTimer <= 0.0f)
				{
					const float attackReach = m_tuning.enemyMeleeAttackReach * enemy.attackRangeScale;
					if (distToPlayer <= attackReach)
					{
						if (player.damageGraceTimer <= 0.0f && !player.isSliding)
						{
							const float dangerBoost = static_cast<float>(std::max(0, gameState.dangerLevel - 1));
							float damage = (m_tuning.enemyMeleeDamageBase + dangerBoost * m_tuning.enemyMeleeDamagePerDanger) * enemy.attackDamageScale;
							// --- 時間ダメージ倍率適用 ---
							damage *= enemy.timeDamageMultiplier;
							if (player.guarding)
							{
							damage *= m_tuning.enemyMeleeGuardDamageScale;
							}
							const float timerDrain = player.guarding
								? 0.55f
								: Utility::MathEx::Clamp(0.8f + damage * 0.055f, 0.8f, 3.2f);
							gameState.stageTimer = std::max(0.0f, gameState.stageTimer - timerDrain);
							gameState.damageTaken += static_cast<int>(std::round(damage));
							if (gameState.stageTimer <= 0.0f)
							{
								gameState.stageTimer = 0.0f;
								gameState.timeExpired = true;
							}
							player.damageGraceTimer = m_tuning.damageGraceSec;
							GameAudio::AudioSystem::GetInstance().PlaySe(
								player.guarding ? GameAudio::SeId::GuardBlock : GameAudio::SeId::PlayerHit,
								player.guarding ? 0.72f : 0.84f);
						}
					}
					enemy.state = EnemyStateType::Chase;
					enemy.stateTimer = 0.0f;
					Action::CombatInternal::ClearEnemyPathState(enemy);
					enemy.hasWanderTarget = false;
				}
			}

			if (moveDir.LengthSquared() > 0.0001f)
			{
				enemy.yaw = std::atan2(moveDir.x, moveDir.z);
			}
		}
	}

// 原点から最も近い生存敵を返す。
	int CombatSystem::FindNearestEnemy(
		const std::vector<EnemyState>& enemies,
		const Vector3& origin,
		float maxDistance) const
	{
		const float maxDistSq = std::max(0.0f, maxDistance * maxDistance);
		int bestIndex = -1;
		float bestDistSq = maxDistSq;

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			const EnemyState& enemy = enemies[i];
			if (enemy.state == EnemyStateType::Dead)
			{
				continue;
			}

			Vector3 d = enemy.position - origin;
			d.y = 0.0f;
			const float distSq = d.LengthSquared();
			if (distSq > bestDistSq)
			{
				continue;
			}

			bestDistSq = distSq;
			bestIndex = static_cast<int>(i);
		}

		return bestIndex;
	}

// 生存敵が1体でも存在するか返す。
	bool CombatSystem::HasLivingEnemy(const std::vector<EnemyState>& enemies) const
	{
		for (size_t i = 0; i < enemies.size(); ++i)
		{
			if (enemies[i].state != EnemyStateType::Dead)
			{
				return true;
			}
		}
		return false;
	}
}
