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
		// Shift 系の入力を追加: スライド発動用
		bool slideHeld;

		InputSnapshot()
			: dt(0.0f)
			, mouseDelta(0, 0)
			, moveForward(false)
			, moveBack(false)
			, moveLeft(false)
			, moveRight(false)
			, attackPressed(false)
			, guardHeld(false)
			, lockTogglePressed(false)
			, slideHeld(false)
		{
		}
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
			// スライド関連
			bool isSliding;
			float slideTimer;
			float slideCooldown;
			DirectX::SimpleMath::Vector3 slideDirection;
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
		,
		isSliding(false)
		,
		slideTimer(0.0f)
		,
		slideCooldown(0.0f)
		,
		slideDirection(DirectX::SimpleMath::Vector3::Zero)
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

		// レーザー敵・突進敵・時間ダメージ用追加メンバ
		bool isLaserEnemy;
		float laserPreDelayTimer;
		float laserFireTimer;
		bool laserWarningActive;
		DirectX::SimpleMath::Vector3 laserAimDirection;
		bool isDashingEnemy;
		float dashPreDelayTimer;
		float dashTimer;
		bool dashActive;
		bool isHeavyEnemy;
		float timeDamageMultiplier;

		// New AI parameters set by EnemyFactory
		float baseMoveSpeed;    // multiplier or base used by CombatSystem
		float preferredRange;   // preferred engagement distance (meters)
		float aggroRadius;      // radius to become aggressive toward player
		bool isRanged;          // true for ranged/laser enemies

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
			, isLaserEnemy(false)
			, laserPreDelayTimer(0.0f)
			, laserFireTimer(0.0f)
			, laserWarningActive(false)
			, laserAimDirection(DirectX::SimpleMath::Vector3::Zero)
			, isDashingEnemy(false)
			, dashPreDelayTimer(0.0f)
			, dashTimer(0.0f)
			, dashActive(false)
			, isHeavyEnemy(false)
			, timeDamageMultiplier(1.0f)
			, baseMoveSpeed(1.0f)
			, preferredRange(2.6f)
			, aggroRadius(18.0f)
			, isRanged(false)
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
