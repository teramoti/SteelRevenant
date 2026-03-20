#pragma once

#include <cstddef>
#include <random>
#include <vector>

#include <SimpleMath.h>

#include "AStarSolver.h"
#include "GameState.h"
#include "PathGrid.h"

namespace Action
{
	// 戦闘バランス調整値をひとまとめに保持する。
	struct CombatTuning
	{
		float playerYawScale;
		float walkSpeed;
		float moveAccelGain;
		float moveStopGain;
		float damageGraceSec;
		float playerBoundsMin;
		float playerBoundsMax;

		float playerAttackDamage1;
		float playerAttackDamage2;
		float playerAttackDamage3;
		float comboDamageBonusPerLevel;
		float comboLevelGauge1;
		float comboLevelGauge2;
		float comboLevelGauge3;
		float comboMissPenalty;
		float lockAssistRangeMultiplier;
		float lockAssistDotRelax;
		float attackAssistRangeDefault;
		float attackAssistDotDefault;

		float comboAttackWindowSec;
		float comboCooldownSec;
		float comboChainWindowSec;
		float comboGaugeDecayPerSec;
		float comboGaugeHitGain;
		float comboGaugeKillBonus;
		float comboAttackWindupSec;
		float comboAttackActiveSec;
		float comboAttackFollowThroughSec;
		float comboAttackRecoverSec;

		float enemyRepathSec;
		float enemyMeleeAttackTriggerDistance;
		float enemyMeleeAttackReach;
		float enemyFlankOffset;
		float enemyAttackWindupSec;
		float enemyMoveSpeedBase;
		float enemyMoveSpeedPerDanger;
		float enemyIdleWakeupDistance;
		float enemyReturnWakeupDistance;
		float enemyLeashDistance;
		float enemyReturnArriveDistance;
		float enemyHitStunSec;
		float enemyKnockbackSpeed;
		float enemyKnockbackDamping;

		float enemyMeleeDamageBase;
		float enemyMeleeDamagePerDanger;
		float enemyMeleeGuardDamageScale;
	};

	// 1 フレーム分の入力をゲームループから切り出したスナップショット。
	struct InputSnapshot
	{
		float dt;
		DirectX::SimpleMath::Vector2 mouseDelta;
		bool moveForward;
		bool moveBack;
		bool moveLeft;
		bool moveRight;
		bool attackPressed;
		bool guardHeld;
		bool lockTogglePressed;
	};

	enum class PlayerAttackPhase
	{
		Idle,
		Windup,
		Active,
		FollowThrough,
		Recover
	};

	// プレイヤー戦闘状態を保持する。
	struct PlayerState
	{
		DirectX::SimpleMath::Vector3 position;
		float yaw;
		float hp;
		bool guarding;
		float damageGraceTimer;
		DirectX::SimpleMath::Vector3 moveVelocity;
		float attackTimer;
		float attackWindupTimer;
		float attackCooldown;
		float attackPhaseTimer;
		float comboTimer;
		int comboIndex;
		float comboGauge;
		int comboLevel;
		int lockEnemyIndex;
		PlayerAttackPhase attackPhase;
		float attackPhaseBlend;
		bool attackTriggeredThisFrame;
		DirectX::SimpleMath::Vector3 swingTrailBase;
		DirectX::SimpleMath::Vector3 swingTrailTip;
		bool swingTrailActive;

		PlayerState()
			: position(0.0f, 0.8f, 0.0f)
			, yaw(0.0f)
			, hp(100.0f)
			, guarding(false)
			, damageGraceTimer(0.0f)
			, moveVelocity(DirectX::SimpleMath::Vector3::Zero)
			, attackTimer(0.0f)
			, attackWindupTimer(0.0f)
			, attackCooldown(0.0f)
			, attackPhaseTimer(0.0f)
			, comboTimer(0.0f)
			, comboIndex(0)
			, comboGauge(0.0f)
			, comboLevel(0)
			, lockEnemyIndex(-1)
			, attackPhase(PlayerAttackPhase::Idle)
			, attackPhaseBlend(0.0f)
			, attackTriggeredThisFrame(false)
			, swingTrailBase(DirectX::SimpleMath::Vector3::Zero)
			, swingTrailTip(DirectX::SimpleMath::Vector3::Zero)
			, swingTrailActive(false)
		{
		}
	};

	enum class EnemyStateType
	{
		Idle,
		Wander,
		Chase,
		Attack,
		Return,
		Dead
	};

	enum class EnemyMoveRole
	{
		DirectPressure,
		Flank
	};

	enum class EnemyArchetype
	{
		BladeRush,
		BladeFlank
	};

	// 敵 1 体分の戦闘状態を保持する。
	struct EnemyState
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 spawnPosition;
		float yaw;
		float hp;
		float maxHp;
		EnemyStateType state;
		EnemyArchetype archetype;
		EnemyMoveRole moveRole;
		float moveSpeedScale;
		float attackRangeScale;
		float attackDamageScale;
		float attackWindupScale;
		float repathIntervalScale;
		float leashScale;
		float stateTimer;
		float repathTimer;
		float wanderPauseSec;
		float hitReactTimer;
		bool hitByCurrentSwing;
		bool hasWanderTarget;
		DirectX::SimpleMath::Vector3 wanderTarget;
		DirectX::SimpleMath::Vector3 knockbackVelocity;
		std::vector<PathGrid::GridCoord> path;
		size_t pathCursor;

		EnemyState()
			: position(0.0f, 0.8f, 0.0f)
			, spawnPosition(0.0f, 0.8f, 0.0f)
			, yaw(0.0f)
			, hp(40.0f)
			, maxHp(40.0f)
			, state(EnemyStateType::Idle)
			, archetype(EnemyArchetype::BladeRush)
			, moveRole(EnemyMoveRole::DirectPressure)
			, moveSpeedScale(1.0f)
			, attackRangeScale(1.0f)
			, attackDamageScale(1.0f)
			, attackWindupScale(1.0f)
			, repathIntervalScale(1.0f)
			, leashScale(1.0f)
			, stateTimer(0.0f)
			, repathTimer(0.0f)
			, wanderPauseSec(0.0f)
			, hitReactTimer(0.0f)
			, hitByCurrentSwing(false)
			, hasWanderTarget(false)
			, wanderTarget(DirectX::SimpleMath::Vector3::Zero)
			, knockbackVelocity(DirectX::SimpleMath::Vector3::Zero)
			, pathCursor(0)
		{
		}
	};

	// プレイヤーと敵の更新を担当する戦闘モジュール。
	class CombatSystem
	{
	public:
		CombatSystem();

		void SetTuning(const CombatTuning& tuning);
		const CombatTuning& GetTuning() const;

		void SetAttackAssistConfig(float attackRange, float attackDotThreshold);

		void UpdatePlayer(
			PlayerState& player,
			const InputSnapshot& input,
			const DirectX::SimpleMath::Vector3& cameraForward,
			const DirectX::SimpleMath::Vector3& cameraRight,
			GameState& gameState,
			float groundHeight = 0.8f) const;

		void ResolvePlayerAttack(
			PlayerState& player,
			std::vector<EnemyState>& enemies,
			GameState& gameState) const;

		void UpdateEnemies(
			std::vector<EnemyState>& enemies,
			PlayerState& player,
			const InputSnapshot& input,
			const PathGrid& grid,
			const AStarSolver& solver,
			GameState& gameState) const;

		int FindNearestEnemy(
			const std::vector<EnemyState>& enemies,
			const DirectX::SimpleMath::Vector3& origin,
			float maxDistance) const;

		bool HasLivingEnemy(const std::vector<EnemyState>& enemies) const;

	private:
		CombatTuning m_tuning;
		float m_attackRange;
		float m_attackDotThreshold;
		mutable std::mt19937 m_rng;
	};
}
