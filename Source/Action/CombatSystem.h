//------------------------//------------------------
// Contents(処理内容) プレイヤー戦闘、敵AI、移動更新を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
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
		float enemyRangedAttackTriggerDistance;
		float enemyMeleeAttackReach;
		float enemyRangedAttackReach;
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
		float enemyRangedDamageBase;
		float enemyRangedDamagePerDanger;
		float enemyMeleeGuardDamageScale;
		float enemyRangedGuardDamageScale;
	};

	// 1フレーム分の入力スナップショット。
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

	// プレイヤーのランタイム状態。
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

		// プレイヤーの実行時状態を既定値で初期化する。
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

	// 敵FSMの主要状態。
	enum class EnemyStateType
	{
		Idle,
		Wander,
		Chase,
		Aim,
		Attack,
		Return,
		Dead
	};

	// 敵の武器種別。
	enum class EnemyWeaponType
	{
		Melee,
		Ranged
	};

	// 敵の移動役割。
	enum class EnemyMoveRole
	{
		DirectPressure,
		Flank,
		KeepDistance
	};

	// 敵のアーキタイプ定義。
	enum class EnemyArchetype
	{
		BladeRush,
		BladeFlank,
		GunHold,
		GunPressure
	};

	// 敵1体分のランタイム状態。
	struct EnemyState
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 spawnPosition;
		float yaw;
		float hp;
		float maxHp;
		EnemyStateType state;
		EnemyArchetype archetype;
		EnemyWeaponType weaponType;
		EnemyMoveRole moveRole;
		float moveSpeedScale;
		float attackRangeScale;
		float attackDamageScale;
		float attackWindupScale;
		float repathIntervalScale;
		float leashScale;
		float idealRangeMin;
		float idealRangeMax;
		float aimLeadSec;
		float projectileSpeed;
		float attackCooldownSec;
		float stateTimer;
		float repathTimer;
		float wanderPauseSec;
		float tacticalMoveTimer;
		bool firedProjectileThisFrame;
		bool hitByCurrentSwing;
		bool hasWanderTarget;
		bool hasTacticalTarget;
		DirectX::SimpleMath::Vector3 projectileSpawnPosition;
		DirectX::SimpleMath::Vector3 projectileVelocity;
		float projectileDamage;
		DirectX::SimpleMath::Vector3 wanderTarget;
		DirectX::SimpleMath::Vector3 tacticalTarget;
		std::vector<PathGrid::GridCoord> path;
		size_t pathCursor;

		// 敵の実行時状態を既定値で初期化する。
		EnemyState()
			: position(0.0f, 0.8f, 0.0f)
			, spawnPosition(0.0f, 0.8f, 0.0f)
			, yaw(0.0f)
			, hp(40.0f)
			, maxHp(40.0f)
			, state(EnemyStateType::Idle)
			, archetype(EnemyArchetype::GunPressure)
			, weaponType(EnemyWeaponType::Melee)
			, moveRole(EnemyMoveRole::DirectPressure)
			, moveSpeedScale(1.0f)
			, attackRangeScale(1.0f)
			, attackDamageScale(1.0f)
			, attackWindupScale(1.0f)
			, repathIntervalScale(1.0f)
			, leashScale(1.0f)
			, idealRangeMin(4.5f)
			, idealRangeMax(8.5f)
			, aimLeadSec(0.35f)
			, projectileSpeed(18.0f)
			, attackCooldownSec(0.65f)
			, stateTimer(0.0f)
			, repathTimer(0.0f)
			, wanderPauseSec(0.0f)
			, tacticalMoveTimer(0.0f)
			, firedProjectileThisFrame(false)
			, hitByCurrentSwing(false)
			, hasWanderTarget(false)
			, hasTacticalTarget(false)
			, projectileSpawnPosition(DirectX::SimpleMath::Vector3::Zero)
			, projectileVelocity(DirectX::SimpleMath::Vector3::Zero)
			, projectileDamage(0.0f)
			, wanderTarget(DirectX::SimpleMath::Vector3::Zero)
			, tacticalTarget(DirectX::SimpleMath::Vector3::Zero)
			, pathCursor(0)
		{
		}
	};

	// 戦闘と行動制御を集約するクラス。
	class CombatSystem
	{
	public:
		// 既定パラメータで戦闘システムを初期化する。
		CombatSystem();

		// 戦闘調整値を現在設定へ反映する。
		void SetTuning(const CombatTuning& tuning);
		// 現在の戦闘調整値を返す。
		const CombatTuning& GetTuning() const;

		// 攻撃吸着距離と判定角度を設定する。
		void SetAttackAssistConfig(float attackRange, float attackDotThreshold);

		// プレイヤーの入力と移動状態を更新する。
		void UpdatePlayer(
			PlayerState& player,
			const InputSnapshot& input,
			const DirectX::SimpleMath::Vector3& cameraForward,
			const DirectX::SimpleMath::Vector3& cameraRight,
			GameState& gameState,
			float groundHeight = 0.8f) const;

		// プレイヤー攻撃のヒット判定とダメージを解決する。
		void ResolvePlayerAttack(
			PlayerState& player,
			std::vector<EnemyState>& enemies,
			GameState& gameState) const;

		// 敵AI、移動、攻撃、状態遷移を更新する。
		void UpdateEnemies(
			std::vector<EnemyState>& enemies,
			PlayerState& player,
			const InputSnapshot& input,
			const PathGrid& grid,
			const AStarSolver& solver,
			GameState& gameState) const;

		// 原点から最も近い生存敵を返す。
		int FindNearestEnemy(
			const std::vector<EnemyState>& enemies,
			const DirectX::SimpleMath::Vector3& origin,
			float maxDistance) const;

		// 生存敵がいるかを返す。
		bool HasLivingEnemy(const std::vector<EnemyState>& enemies) const;

	private:
		CombatTuning m_tuning;
		float m_attackRange;
		float m_attackDotThreshold;
		mutable std::mt19937 m_rng;
	};
}
