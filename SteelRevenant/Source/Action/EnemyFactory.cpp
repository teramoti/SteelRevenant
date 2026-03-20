//------------------------//------------------------
// Contents(処理内容) 危険度ごとの敵生成処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
#include "EnemyFactory.h"

#include <algorithm>

namespace Action
{
	// 危険度と通し番号から敵 1 体を生成する。
	EnemyState EnemyFactory::CreateEnemy(
		const DirectX::SimpleMath::Vector3& spawnBase,
		int dangerLevel,
		int spawnSerial)
	{
		EnemyState enemy;

		const int clampedDanger = std::max(1, std::min(10, dangerLevel));
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
		}

		enemy.maxHp = std::max(10.0f, enemy.maxHp);
		enemy.hp = enemy.maxHp;
		return enemy;
	}
}
