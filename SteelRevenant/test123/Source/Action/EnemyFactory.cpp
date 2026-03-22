//------------------------//------------------------
// Contents(処理内容) 危険度ごとの敵生成処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
#include "EnemyFactory.h"

#include <algorithm>

#include "BattleRuleBook.h"

// Ensure Windows min/max macros don't break std::min/std::max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace Action
{
	// 危険度と通し番号から敵 1 体を生成する。
	EnemyState EnemyFactory::CreateEnemy(
		const DirectX::SimpleMath::Vector3& spawnBase,
		int dangerLevel,
		int spawnSerial)
	{
		EnemyState enemy;

		const int clampedDanger = std::max(1, (std::min)(10, dangerLevel));
		const float jitterX = static_cast<float>((spawnSerial % 3) - 1) * 0.7f;
		const float jitterZ = static_cast<float>((spawnSerial % 5) - 2) * 0.6f;
		enemy.position = spawnBase + DirectX::SimpleMath::Vector3(jitterX, 0.0f, jitterZ);
		enemy.spawnPosition = enemy.position;

		// hp は後段で enemy.hp = enemy.maxHp により確定するため、ここでは maxHp のみ設定する。
		enemy.maxHp = 40.0f + static_cast<float>(clampedDanger - 1) * 7.0f;
		enemy.state = EnemyStateType::Idle;
		enemy.stateTimer = 0.3f + 0.04f * static_cast<float>(spawnSerial % 6);
		enemy.repathTimer = 0.0f;
		enemy.hitByCurrentSwing = false;
		enemy.path.clear();
		enemy.pathCursor = 0;

		// Default AI params (will be tuned by CombatSystem later)
		enemy.baseMoveSpeed = 1.0f;    // multiplier applied by combat tuning
		enemy.preferredRange = 2.6f;
		enemy.aggroRadius = 18.0f;
		enemy.isRanged = false;

		const int archetypeRoll = (spawnSerial * 37 + clampedDanger * 17) % 10;
		if (archetypeRoll < 6)
		{
			enemy.archetype = EnemyArchetype::BladeRush;
			enemy.moveRole = EnemyMoveRole::DirectPressure;
			enemy.moveSpeedScale = 1.18f;
			enemy.attackRangeScale = 1.08f;
			enemy.attackDamageScale = 0.95f;
			enemy.attackWindupScale = 0.86f;
			enemy.repathIntervalScale = 0.82f;
			enemy.leashScale = 1.12f;
			enemy.maxHp *= 0.92f;
			// melee defaults - make rush feel faster and more aggressive
			enemy.baseMoveSpeed = 1.25f;   // notably faster base mobility
			enemy.preferredRange = 1.8f;   // close in to very near range
			enemy.aggroRadius = 30.0f;     // becomes aggressive from further out
			enemy.isRanged = false;
		}
		else
		{
			enemy.archetype = EnemyArchetype::BladeFlank;
			enemy.moveRole = EnemyMoveRole::Flank;
			enemy.moveSpeedScale = 1.08f;
			enemy.attackRangeScale = 1.00f;
			enemy.attackDamageScale = 1.10f;
			enemy.attackWindupScale = 0.94f;
			enemy.repathIntervalScale = 0.76f;
			enemy.leashScale = 1.08f;
			// flank defaults (slightly faster than base, prefers moderate distance)
			enemy.baseMoveSpeed = 1.05f;
			enemy.preferredRange = 2.6f;
			enemy.aggroRadius = 28.0f;
			enemy.isRanged = false;
		}

		// ステージ定義に応じてレーザー／突進敵を割り当てる（最小変更）
		const BattleRuleBook& ruleBook = BattleRuleBook::GetInstance();
		const StageRuleDefinition& stageRule = ruleBook.GetActiveRule();
		const int denom = std::max(1, stageRule.baseAliveCount);
		const int slot = spawnSerial % denom;
		enemy.timeDamageMultiplier = 1.0f;
		enemy.laserAimDirection = DirectX::SimpleMath::Vector3::Zero;
		if (slot < stageRule.laserEnemyCount)
		{
			enemy.isLaserEnemy = true;
			enemy.laserPreDelayTimer = 0.0f;
			enemy.laserFireTimer = 0.0f;
			enemy.laserWarningActive = false;
			enemy.timeDamageMultiplier = 1.65f;
			enemy.moveRole = EnemyMoveRole::Flank;
			enemy.moveSpeedScale *= 0.90f;
			enemy.attackRangeScale *= 0.80f;
			enemy.attackDamageScale *= 0.85f;
			enemy.attackWindupScale *= 1.10f;
			enemy.leashScale *= 1.35f;
			enemy.maxHp *= 0.86f;
			// laser-specific AI params - clearly ranged and holds distance
			enemy.isRanged = true;
			enemy.preferredRange = 9.0f;   // keep significantly farther
			enemy.aggroRadius = 42.0f;     // long detection radius
			enemy.baseMoveSpeed *= 0.88f;  // slightly slower base
		}
		else if (slot < stageRule.laserEnemyCount + stageRule.dashEnemyCount)
		{
			enemy.isDashingEnemy = true;
			enemy.dashPreDelayTimer = 0.8f + static_cast<float>(spawnSerial % 3) * 0.25f;
			enemy.dashTimer = 0.0f;
			enemy.dashActive = false;
			enemy.timeDamageMultiplier = 2.6f;
			enemy.moveRole = EnemyMoveRole::DirectPressure;
			enemy.moveSpeedScale *= 1.26f;
			enemy.attackDamageScale *= 1.15f;
			enemy.attackWindupScale *= 0.84f;
			enemy.leashScale *= 1.12f;
			enemy.maxHp *= 0.94f;
			// dash-specific AI params - very fast close-range threat
			enemy.isRanged = false;
			enemy.preferredRange = 1.4f;
			enemy.aggroRadius = 40.0f;
			enemy.baseMoveSpeed *= 1.45f;
		}
		else if (slot < stageRule.laserEnemyCount + stageRule.dashEnemyCount + stageRule.heavyEnemyCount)
		{
			enemy.isHeavyEnemy = true;
			enemy.timeDamageMultiplier = 1.15f;
			enemy.moveRole = EnemyMoveRole::DirectPressure;
			enemy.moveSpeedScale *= 0.74f;
			enemy.attackRangeScale *= 1.12f;
			enemy.attackDamageScale *= 1.28f;
			enemy.attackWindupScale *= 1.08f;
			enemy.repathIntervalScale *= 1.12f;
			enemy.leashScale *= 1.18f;
			enemy.maxHp *= 2.35f;
			// heavy-specific AI params - tanky but slow
			enemy.isRanged = false;
			enemy.preferredRange = 2.8f;
			enemy.aggroRadius = 32.0f;
			enemy.baseMoveSpeed *= 0.72f;
		}

		enemy.maxHp = std::max(10.0f, enemy.maxHp);
		enemy.hp = enemy.maxHp;
		return enemy;
	}
}

