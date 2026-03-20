//------------------------//------------------------
// Contents(処理内容) 拠点制圧と危険地帯の更新処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "GameScene.h"

#include "../../Action/BattleRuleBook.h"
#include "../../Utility/SimpleMathEx.h"
#include "../../Utility/Sound/AudioSystem.h"

#include <algorithm>
#include <cmath>
#include <string>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kObjectiveBannerDurationSec = 2.8f;
}

// 中継地点とハザード領域を配置する。
void GameScene::SetupRelayNodesAndHazards()
{
	m_relayNodes.clear();
	m_hazardZones.clear();
	m_patrolHazards.clear();
	m_recoveryBeacons.clear();

	auto addRelay = [this](const Vector3& position, float radius, float seed)
	{
		RelayNodeInfo relay = {};
		relay.position = position;
		relay.radius = radius;
		relay.captureProgress = 0.0f;
		relay.captured = false;
		relay.pulseSeed = seed;
		m_relayNodes.push_back(relay);
	};

	auto addHazard = [this](const Vector3& center, const Vector2& halfExtent, float phaseOffsetSec, float warmupSec, float activeSec, float damagePerSec)
	{
		HazardZoneInfo zone = {};
		zone.center = center;
		zone.halfExtent = halfExtent;
		zone.phaseOffsetSec = phaseOffsetSec;
		zone.warmupSec = warmupSec;
		zone.activeSec = activeSec;
		zone.damagePerSec = damagePerSec;
		m_hazardZones.push_back(zone);
	};

	auto addBeacon = [this](const Vector3& position, float radius, float cooldownSec, float seed)
	{
		RecoveryBeaconInfo beacon = {};
		beacon.position = position;
		beacon.radius = radius;
		beacon.cooldownSec = 0.0f;
		beacon.maxCooldownSec = cooldownSec;
		beacon.pulseSeed = seed;
		m_recoveryBeacons.push_back(beacon);
	};

	auto addPatrolHazard = [this](const Vector3& start, const Vector3& end, float radius, float cycleSpeed, float damagePerSec, float phaseSeed)
	{
		PatrolHazardInfo hazard = {};
		hazard.start = start;
		hazard.end = end;
		hazard.radius = radius;
		hazard.cycleSpeed = cycleSpeed;
		hazard.damagePerSec = damagePerSec;
		hazard.phaseSeed = phaseSeed;
		m_patrolHazards.push_back(hazard);
	};

	switch (m_stageThemeIndex)
	{
	case 1:
		addRelay(Vector3(0.0f, 0.8f, 4.0f), 4.0f, 0.3f);
		addBeacon(Vector3(0.0f, 0.8f, -8.0f), 2.1f, 16.0f, 0.5f);
		break;

	case 2:
		addRelay(Vector3(-18.0f, 0.8f, 18.0f), 4.2f, 0.2f);
		addRelay(Vector3(0.0f, 0.8f, 0.0f), 4.8f, 0.8f);
		addRelay(Vector3(18.0f, 0.8f, -18.0f), 4.2f, 1.4f);
		addHazard(Vector3(-8.5f, 0.0f, 0.0f), Vector2(4.0f, 25.0f), 0.0f, 1.8f, 2.2f, 11.0f);
		addHazard(Vector3(8.5f, 0.0f, 0.0f), Vector2(4.0f, 25.0f), 1.7f, 1.8f, 2.2f, 11.0f);
		addBeacon(Vector3(-21.0f, 0.8f, -2.5f), 2.1f, 17.0f, 0.3f);
		addBeacon(Vector3(21.0f, 0.8f, 2.5f), 2.1f, 17.0f, 0.9f);
		addPatrolHazard(Vector3(-22.0f, 0.8f, -12.0f), Vector3(22.0f, 0.8f, -12.0f), 2.4f, 0.42f, 13.0f, 0.1f);
		addPatrolHazard(Vector3(22.0f, 0.8f, 12.0f), Vector3(-22.0f, 0.8f, 12.0f), 2.4f, 0.38f, 13.0f, 0.6f);
		break;
	case 3:
		addRelay(Vector3(-19.0f, 0.8f, -14.0f), 4.1f, 0.0f);
		addRelay(Vector3(0.0f, 0.8f, 0.0f), 5.0f, 0.7f);
		addRelay(Vector3(19.0f, 0.8f, 14.0f), 4.1f, 1.5f);
		addHazard(Vector3(0.0f, 0.0f, -16.0f), Vector2(23.0f, 3.0f), 0.0f, 1.5f, 2.1f, 12.5f);
		addHazard(Vector3(0.0f, 0.0f, 16.0f), Vector2(23.0f, 3.0f), 1.35f, 1.5f, 2.1f, 12.5f);
		addHazard(Vector3(0.0f, 0.0f, 0.0f), Vector2(3.2f, 23.0f), 2.7f, 1.4f, 1.9f, 13.0f);
		addBeacon(Vector3(-22.0f, 0.8f, 22.0f), 2.2f, 18.0f, 0.2f);
		addBeacon(Vector3(22.0f, 0.8f, -22.0f), 2.2f, 18.0f, 1.1f);
		addPatrolHazard(Vector3(-18.0f, 0.8f, 22.0f), Vector3(18.0f, 0.8f, -22.0f), 2.7f, 0.31f, 14.0f, 0.2f);
		addPatrolHazard(Vector3(18.0f, 0.8f, 22.0f), Vector3(-18.0f, 0.8f, -22.0f), 2.7f, 0.34f, 14.0f, 0.75f);
		break;
	default:
		addRelay(Vector3(-16.0f, 0.8f, -8.0f), 3.9f, 0.1f);
		addRelay(Vector3(0.0f, 0.8f, 9.0f), 4.2f, 0.8f);
		addRelay(Vector3(17.0f, 0.8f, 18.0f), 4.0f, 1.3f);
		addHazard(Vector3(0.0f, 0.0f, -12.0f), Vector2(20.0f, 2.6f), 0.0f, 1.9f, 1.8f, 10.0f);
		addHazard(Vector3(14.0f, 0.0f, 5.0f), Vector2(3.0f, 16.0f), 1.8f, 1.9f, 1.8f, 10.0f);
		addBeacon(Vector3(-20.0f, 0.8f, 20.0f), 2.0f, 16.0f, 0.1f);
		addBeacon(Vector3(20.0f, 0.8f, -20.0f), 2.0f, 16.0f, 0.9f);
		addPatrolHazard(Vector3(-16.0f, 0.8f, -20.0f), Vector3(16.0f, 0.8f, 18.0f), 2.1f, 0.33f, 11.5f, 0.05f);
		break;
	}

	m_requiredRelayCount = std::min<int>(static_cast<int>(m_relayNodes.size()), Action::BattleRuleBook::GetInstance().GetRequiredRelayCount());
}

// 指定座標がハザード領域内か判定して返す。
bool GameScene::IsInsideHazardZone(const Vector3& position, size_t zoneIndex) const
{
	if (zoneIndex >= m_hazardZones.size())
	{
		return false;
	}

	const HazardZoneInfo& zone = m_hazardZones[zoneIndex];
	return (position.x >= zone.center.x - zone.halfExtent.x &&
		position.x <= zone.center.x + zone.halfExtent.x &&
		position.z >= zone.center.z - zone.halfExtent.y &&
		position.z <= zone.center.z + zone.halfExtent.y);
}

// 指定範囲内の生存敵数を返す。
int GameScene::CountLivingEnemiesNear(const Vector3& position, float radius) const
{
	const float radiusSq = radius * radius;
	int count = 0;
	for (size_t i = 0; i < m_enemies.size(); ++i)
	{
		const Action::EnemyState& enemy = m_enemies[i];
		if (enemy.state == Action::EnemyStateType::Dead)
		{
			continue;
		}
		Vector3 delta = enemy.position - position;
		delta.y = 0.0f;
		if (delta.LengthSquared() <= radiusSq)
		{
			++count;
		}
	}
	return count;
}

// 制圧済み中継地点数を返す。
int GameScene::GetCapturedRelayCount() const
{
	int captured = 0;
	for (size_t i = 0; i < m_relayNodes.size(); ++i)
	{
		if (m_relayNodes[i].captured)
		{
			++captured;
		}
	}
	return captured;
}

// クリア条件に必要な中継地点数を返す。
int GameScene::GetRequiredRelayCount() const
{
	return m_requiredRelayCount;
}

// 指定ハザードの巡回基準座標を返す。
Vector3 GameScene::EvaluatePatrolHazardPosition(size_t hazardIndex) const
{
	if (hazardIndex >= m_patrolHazards.size())
	{
		return Vector3::Zero;
	}

	const PatrolHazardInfo& hazard = m_patrolHazards[hazardIndex];
	const float t = std::sinf(m_sceneTime * hazard.cycleSpeed + hazard.phaseSeed * DirectX::XM_2PI) * 0.5f + 0.5f;
	return hazard.start + (hazard.end - hazard.start) * t;
}

// アリーナ目標レイヤの進行表示を更新する。
void GameScene::UpdateArenaObjectiveLayer(float dt)
{
	m_objectiveBannerTimer = std::max(0.0f, m_objectiveBannerTimer - dt);
	GameAudio::AudioSystem& audio = GameAudio::AudioSystem::GetInstance();

	for (size_t beaconIndex = 0; beaconIndex < m_recoveryBeacons.size(); ++beaconIndex)
	{
		RecoveryBeaconInfo& beacon = m_recoveryBeacons[beaconIndex];
		beacon.cooldownSec = std::max(0.0f, beacon.cooldownSec - dt);
		if (beacon.cooldownSec > 0.0f)
		{
			continue;
		}

		Vector3 beaconDelta = m_player.position - beacon.position;
		beaconDelta.y = 0.0f;
		if (beaconDelta.LengthSquared() > (beacon.radius * beacon.radius))
		{
			continue;
		}

		const float timeBefore = m_gameState.stageTimer;
		m_gameState.stageTimer = std::min(Action::BattleRuleBook::GetInstance().GetStageStartTimeSec(), m_gameState.stageTimer + Action::BattleRuleBook::GetInstance().GetBeaconTimeBonusSec());
		beacon.cooldownSec = beacon.maxCooldownSec;
		if (m_gameState.stageTimer > timeBefore + 0.01f)
		{
			m_objectiveBannerTimer = kObjectiveBannerDurationSec;
			m_objectiveBannerText = std::wstring(L"Supply recovered  +") + std::to_wstring(static_cast<int>(Action::BattleRuleBook::GetInstance().GetBeaconTimeBonusSec())) + L" sec";
			++m_recoveryBeaconUseCount;
			AddCameraShake(0.08f, 0.12f);
			audio.PlaySe(GameAudio::SeId::BeaconHeal, 0.85f);
			audio.PlaySe(GameAudio::SeId::BonusTime, 0.56f);
		}
	}

	for (size_t relayIndex = 0; relayIndex < m_relayNodes.size(); ++relayIndex)
	{
		RelayNodeInfo& relay = m_relayNodes[relayIndex];
		if (relay.captured)
		{
			continue;
		}

		Vector3 delta = m_player.position - relay.position;
		delta.y = 0.0f;
		const bool inside = (delta.LengthSquared() <= (relay.radius * relay.radius));
		const bool contested = CountLivingEnemiesNear(relay.position, relay.radius + 4.5f) > 0;
		const float progressBefore = relay.captureProgress;
		if (inside)
		{
			relay.captureProgress += dt * (contested ? 0.16f : 0.44f);
		}
		else
		{
			relay.captureProgress -= dt * 0.12f;
		}
		relay.captureProgress = Utility::MathEx::Clamp(relay.captureProgress, 0.0f, 1.0f);
		if (inside && !contested && progressBefore <= 0.001f && relay.captureProgress > 0.001f)
		{
			audio.PlaySe(GameAudio::SeId::RelayStart, 0.68f);
		}

		if (!relay.captured && relay.captureProgress >= 1.0f)
		{
			relay.captured = true;
			m_gameState.stageTimer = std::min(Action::BattleRuleBook::GetInstance().GetStageStartTimeSec(), m_gameState.stageTimer + Action::BattleRuleBook::GetInstance().GetRelayTimeBonusSec());
			m_objectiveBannerTimer = kObjectiveBannerDurationSec;
			m_objectiveBannerText = std::wstring(L"Relay secured  +") + std::to_wstring(static_cast<int>(Action::BattleRuleBook::GetInstance().GetRelayTimeBonusSec())) + L" sec";
			AddCameraShake(0.14f, 0.16f);
			audio.PlaySe(GameAudio::SeId::RelaySecure, 0.88f);
			audio.PlaySe(GameAudio::SeId::BonusTime, 0.62f);
		}
	}

	for (size_t patrolIndex = 0; patrolIndex < m_patrolHazards.size(); ++patrolIndex)
	{
		const PatrolHazardInfo& hazard = m_patrolHazards[patrolIndex];
		const Vector3 center = EvaluatePatrolHazardPosition(patrolIndex);
		Vector3 playerDelta = m_player.position - center;
		playerDelta.y = 0.0f;
		if (playerDelta.LengthSquared() <= (hazard.radius * hazard.radius) && m_player.damageGraceTimer <= 0.0f)
		{
			const float penalty = hazard.damagePerSec * dt;
			m_gameState.stageTimer = std::max(0.0f, m_gameState.stageTimer - penalty);
			const int damageDelta = std::max(0, static_cast<int>(std::round(penalty)));
			m_gameState.damageTaken += damageDelta;
			m_damageBloodTimer = Utility::MathEx::Clamp(m_damageBloodTimer + 0.05f + penalty * 0.015f, 0.0f, 1.0f);
			m_player.damageGraceTimer = m_combat.GetTuning().damageGraceSec;
			audio.PlaySe(GameAudio::SeId::PlayerHit, 0.76f);
		}

		for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
		{
			Action::EnemyState& enemy = m_enemies[enemyIndex];
			if (enemy.state == Action::EnemyStateType::Dead)
			{
				continue;
			}
			Vector3 enemyDelta = enemy.position - center;
			enemyDelta.y = 0.0f;
			if (enemyDelta.LengthSquared() > (hazard.radius * hazard.radius))
			{
				continue;
			}
			enemy.hp -= hazard.damagePerSec * 0.55f * dt;
			if (enemy.hp <= 0.0f)
			{
				enemy.hp = 0.0f;
				enemy.state = Action::EnemyStateType::Dead;
				++m_gameState.killCount;
				m_hitBloodTimer = Utility::MathEx::Clamp(m_hitBloodTimer + 0.05f, 0.0f, 1.0f);
				audio.PlaySe(GameAudio::SeId::EnemyDestroy, 0.72f);
			}
		}
	}

	for (size_t zoneIndex = 0; zoneIndex < m_hazardZones.size(); ++zoneIndex)
	{
		const HazardZoneInfo& zone = m_hazardZones[zoneIndex];
		const float cycle = std::max(0.1f, zone.warmupSec + zone.activeSec);
		const float local = std::fmod(m_sceneTime + zone.phaseOffsetSec, cycle);
		const bool active = (local >= zone.warmupSec);
		if (!active)
		{
			continue;
		}

		if (IsInsideHazardZone(m_player.position, zoneIndex) && m_player.damageGraceTimer <= 0.0f)
		{
			const float penalty = zone.damagePerSec * dt;
			m_gameState.stageTimer = std::max(0.0f, m_gameState.stageTimer - penalty);
			const int damageDelta = std::max(0, static_cast<int>(std::round(penalty)));
			m_gameState.damageTaken += damageDelta;
			m_damageBloodTimer = Utility::MathEx::Clamp(m_damageBloodTimer + 0.08f + penalty * 0.015f, 0.0f, 1.0f);
			m_player.damageGraceTimer = m_combat.GetTuning().damageGraceSec;
			audio.PlaySe(GameAudio::SeId::PlayerHit, 0.76f);
		}

		for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
		{
			Action::EnemyState& enemy = m_enemies[enemyIndex];
			if (enemy.state == Action::EnemyStateType::Dead)
			{
				continue;
			}
			if (!IsInsideHazardZone(enemy.position, zoneIndex))
			{
				continue;
			}
			enemy.hp -= zone.damagePerSec * 0.7f * dt;
			if (enemy.hp <= 0.0f)
			{
				enemy.hp = 0.0f;
				enemy.state = Action::EnemyStateType::Dead;
				++m_gameState.killCount;
				m_hitBloodTimer = Utility::MathEx::Clamp(m_hitBloodTimer + 0.08f, 0.0f, 1.0f);
				audio.PlaySe(GameAudio::SeId::EnemyDestroy, 0.72f);
			}
		}
	}
}


