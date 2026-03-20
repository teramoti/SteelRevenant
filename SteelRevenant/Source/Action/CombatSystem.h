#pragma once

//--------------------------------------------------------------------------------------
// File: CombatSystem.h
//
// プレイヤー戦闘、敵AI、移動更新で共有するデータ型と公開APIを定義する。
// - プレイヤー入力と状態
// - 近接専用の敵状態
// - 戦闘更新に使う調整値と CombatSystem 本体
//--------------------------------------------------------------------------------------

#include <cstddef>
#include <random>
#include <vector>

#include <SimpleMath.h>

#include "AStarSolver.h"
#include "GameState.h"
#include "PathGrid.h"

namespace Action
{
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

		float enemyMeleeDamageBase;
		float enemyMeleeDamagePerDanger;
		float enemyMeleeGuardDamageScale;
	};

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

	struct PlayerState
	{
		DirectX::SimpleMath::Vector3 position;
		float yaw;
		float hp;
		bool guarding;
		float damageGraceTimer;
		DirectX::SimpleMath::Vector3 moveVelocity;
		float attackTimer;
		float attackCooldown;
		float comboTimer;
		int comboIndex;
		float comboGauge;
		int comboLevel;
		int lockEnemyIndex;
		bool attackTriggeredThisFrame;

		PlayerState()
			: position(0.0f, 0.8f, 0.0f)
			, yaw(0.0f)
			, hp(100.0f)
			, guarding(false)
			, damageGraceTimer(0.0f)
			, moveVelocity(DirectX::SimpleMath::Vector3::Zero)
			, attackTimer(0.0f)
			, attackCooldown(0.0f)
			, comboTimer(0.0f)
			, comboIndex(0)
			, comboGauge(0.0f)
			, comboLevel(0)
			, lockEnemyIndex(-1)
			, attackTriggeredThisFrame(false)
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
		bool hitByCurrentSwing;
		bool hasWanderTarget;
		DirectX::SimpleMath::Vector3 wanderTarget;
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
			, hitByCurrentSwing(false)
			, hasWanderTarget(false)
			, wanderTarget(DirectX::SimpleMath::Vector3::Zero)
			, pathCursor(0)
		{
		}
	};

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
