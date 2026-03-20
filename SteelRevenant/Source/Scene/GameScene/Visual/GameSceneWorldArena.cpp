//------------------------//------------------------
// Contents(処理内容) 本編ゲームシーンの床・障害物・ギミック描画を分離実装する。
//------------------------//------------------------
#include "../GameScene.h"

#include <cmath>
#include <vector>

#include "GameSceneVisualPalette.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kFloorPulseSpeed = 1.6f;
}

void GameScene::DrawWorldArena()
{
	const SceneFx::StageRenderPalette palette = SceneFx::BuildStageRenderPalette(m_stageThemeIndex, m_sceneTime);
	const int stageTheme = palette.stageTheme;
	const float stagePulse = palette.stagePulse;
	const auto StageTint = [&palette](const Color& base, float alphaScale = 1.0f) -> Color
	{
		return SceneFx::ApplyStageTint(palette, base, alphaScale);
	};
	const float floorPulse = std::sinf(m_sceneTime * kFloorPulseSpeed) * 0.5f + 0.5f;
	const Matrix floorWorld =
		Matrix::CreateScale(58.0f, 0.6f, 58.0f) *
		Matrix::CreateTranslation(0.0f, -0.35f, 0.0f);

	SceneFx::FloorPalette floorPalette;
	floorPalette.baseColor = Color(0.05f + floorPulse * 0.03f, 0.10f + floorPulse * 0.05f, 0.12f + floorPulse * 0.06f, 1.0f);
	floorPalette.topColor = Color(0.09f, 0.16f + floorPulse * 0.08f, 0.2f + floorPulse * 0.06f, 0.96f);
	if (m_floorStyle)
	{
		floorPalette = m_floorStyle->BuildFloorPalette(floorPulse);
	}

	m_floorMesh->Draw(floorWorld, m_view, m_proj, StageTint(floorPalette.baseColor));

	// 床面レイヤー1: ベースパネル
	m_floorMesh->Draw(
		Matrix::CreateScale(54.0f, 0.05f, 54.0f) * Matrix::CreateTranslation(0.0f, 0.02f, 0.0f),
		m_view, m_proj, StageTint(floorPalette.topColor));

	// 床面レイヤー2: 光沢パネル
	m_obstacleMesh->Draw(
		Matrix::CreateScale(50.0f, 0.025f, 50.0f) * Matrix::CreateTranslation(0.0f, 0.055f, 0.0f),
		m_view, m_proj, StageTint(floorPalette.panelColor));

	// グリッドライン (5m間隔 縦横各11本) - 奥行き感を生む
	const Color gridColor = StageTint(Color(
		floorPalette.seamColor.R(), floorPalette.seamColor.G(),
		floorPalette.seamColor.B(), 0.72f));
	const Color gridGlowColor = StageTint(Color(
		floorPalette.accentColor.R(), floorPalette.accentColor.G(),
		floorPalette.accentColor.B(), 0.45f + floorPulse * 0.20f));
	for (int gi = -5; gi <= 5; ++gi)
	{
		const float gf = static_cast<float>(gi) * 5.0f;
		m_obstacleMesh->Draw(Matrix::CreateScale(50.0f, 0.012f, 0.07f) * Matrix::CreateTranslation(0.0f, 0.068f, gf), m_view, m_proj, gridColor);
		m_obstacleMesh->Draw(Matrix::CreateScale(0.07f, 0.012f, 50.0f) * Matrix::CreateTranslation(gf, 0.068f, 0.0f), m_view, m_proj, gridColor);
	}
	// 中央十字アクセントライン
	m_obstacleMesh->Draw(Matrix::CreateScale(50.0f, 0.016f, 0.18f) * Matrix::CreateTranslation(0.0f, 0.072f, 0.0f), m_view, m_proj, gridGlowColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.18f, 0.016f, 50.0f) * Matrix::CreateTranslation(0.0f, 0.072f, 0.0f), m_view, m_proj, gridGlowColor);
	// 外周リング（アリーナ境界）
	const Color rimColor = StageTint(Color(floorPalette.accentColor.R(), floorPalette.accentColor.G(), floorPalette.accentColor.B(), 0.55f + floorPulse * 0.25f));
	m_obstacleMesh->Draw(Matrix::CreateScale(52.0f, 0.022f, 0.22f) * Matrix::CreateTranslation(0.0f, 0.075f, -26.0f), m_view, m_proj, rimColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(52.0f, 0.022f, 0.22f) * Matrix::CreateTranslation(0.0f, 0.075f,  26.0f), m_view, m_proj, rimColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.22f, 0.022f, 52.0f) * Matrix::CreateTranslation(-26.0f, 0.075f, 0.0f), m_view, m_proj, rimColor);
	m_obstacleMesh->Draw(Matrix::CreateScale(0.22f, 0.022f, 52.0f) * Matrix::CreateTranslation( 26.0f, 0.075f, 0.0f), m_view, m_proj, rimColor);

	std::vector<SceneFx::FloorDrawCommand> floorDetailCommands;
	floorDetailCommands.reserve(128);
	if (m_floorStyle)
	{
		m_floorStyle->BuildDetailCommands(m_sceneTime, floorDetailCommands);
	}
	for (size_t commandIndex = 0; commandIndex < floorDetailCommands.size(); ++commandIndex)
	{
		const SceneFx::FloorDrawCommand& command = floorDetailCommands[commandIndex];
		m_obstacleMesh->Draw(command.world, m_view, m_proj, StageTint(command.color));
	}

	for (size_t hazardIndex = 0; hazardIndex < m_hazardZones.size(); ++hazardIndex)
	{
		const HazardZoneInfo& zone = m_hazardZones[hazardIndex];
		const float cycle = std::max(0.1f, zone.warmupSec + zone.activeSec);
		const float local = std::fmod(m_sceneTime + zone.phaseOffsetSec, cycle);
		const bool active = (local >= zone.warmupSec);
		const float phaseT = active
			? Utility::MathEx::Clamp((local - zone.warmupSec) / std::max(0.001f, zone.activeSec), 0.0f, 1.0f)
			: Utility::MathEx::Clamp(local / std::max(0.001f, zone.warmupSec), 0.0f, 1.0f);
		const Color fillColor = active
			? StageTint(Color(1.0f, 0.18f + phaseT * 0.2f, 0.08f, 0.58f))
			: StageTint(Color(1.0f, 0.72f, 0.18f, 0.24f + phaseT * 0.18f));
		const Matrix hazardWorld =
			Matrix::CreateScale(zone.halfExtent.x * 2.0f, 0.05f, zone.halfExtent.y * 2.0f) *
			Matrix::CreateTranslation(zone.center.x, 0.075f, zone.center.z);
		m_floorMesh->Draw(hazardWorld, m_view, m_proj, fillColor);

		const Matrix borderWorld =
			Matrix::CreateScale(zone.halfExtent.x * 2.08f, 0.018f, zone.halfExtent.y * 2.08f) *
			Matrix::CreateTranslation(zone.center.x, 0.11f, zone.center.z);
		m_obstacleMesh->Draw(borderWorld, m_view, m_proj, active ? Color(1.0f, 0.86f, 0.18f, 0.42f) : Color(1.0f, 0.74f, 0.24f, 0.22f));
	}

	for (size_t relayIndex = 0; relayIndex < m_relayNodes.size(); ++relayIndex)
	{
		const RelayNodeInfo& relay = m_relayNodes[relayIndex];
		const float pulse = std::sinf(m_sceneTime * 2.6f + relay.pulseSeed * 4.1f) * 0.5f + 0.5f;
		const bool contested = (!relay.captured) && (CountLivingEnemiesNear(relay.position, relay.radius + 4.5f) > 0);
		const Color relayColor = relay.captured
			? Color(0.34f, 1.0f, 0.58f, 0.92f)
			: (contested ? Color(1.0f, 0.42f, 0.18f, 0.88f) : Color(0.28f, 0.9f, 1.0f, 0.84f));
		const Matrix relayBaseWorld =
			Matrix::CreateScale(relay.radius * (0.92f + pulse * 0.06f), 0.08f, relay.radius * (0.92f + pulse * 0.06f)) *
			Matrix::CreateTranslation(relay.position.x, 0.14f, relay.position.z);
		const Matrix relayRingWorld =
			Matrix::CreateScale(relay.radius * (1.16f + pulse * 0.1f), 0.024f, relay.radius * (1.16f + pulse * 0.1f)) *
			Matrix::CreateTranslation(relay.position.x, 0.22f, relay.position.z);
		const Matrix relayCoreWorld =
			Matrix::CreateScale(0.58f + relay.captureProgress * 0.18f, 0.04f, 0.58f + relay.captureProgress * 0.18f) *
			Matrix::CreateTranslation(relay.position.x, 0.18f, relay.position.z);
		m_floorMesh->Draw(relayBaseWorld, m_view, m_proj, StageTint(relayColor));
		m_obstacleMesh->Draw(relayRingWorld, m_view, m_proj, StageTint(Color(relayColor.x, relayColor.y, relayColor.z, 0.48f)));
		m_obstacleMesh->Draw(relayCoreWorld, m_view, m_proj, StageTint(Color(relayColor.x, relayColor.y, relayColor.z, relay.captured ? 0.52f : 0.36f)));
		if (!relay.captured && relay.captureProgress > 0.0f)
		{
			const Matrix relayProgressWorld =
				Matrix::CreateScale(relay.radius * relay.captureProgress, 0.03f, relay.radius * relay.captureProgress) *
				Matrix::CreateTranslation(relay.position.x, 0.18f, relay.position.z);
			m_obstacleMesh->Draw(relayProgressWorld, m_view, m_proj, StageTint(Color(0.94f, 0.96f, 1.0f, 0.68f)));
		}
	}

	for (size_t patrolIndex = 0; patrolIndex < m_patrolHazards.size(); ++patrolIndex)
	{
		const PatrolHazardInfo& hazard = m_patrolHazards[patrolIndex];
		const Vector3 center = EvaluatePatrolHazardPosition(patrolIndex);
		const float pulse = std::sinf(m_sceneTime * 4.0f + hazard.phaseSeed * 6.0f) * 0.5f + 0.5f;
		const Matrix sweepRingWorld =
			Matrix::CreateScale(hazard.radius * (1.05f + pulse * 0.12f), 0.03f, hazard.radius * (1.05f + pulse * 0.12f)) *
			Matrix::CreateTranslation(center.x, 0.15f, center.z);
		const Matrix sweepCoreWorld =
			Matrix::CreateScale(0.62f + pulse * 0.18f, 0.03f, 0.62f + pulse * 0.18f) *
			Matrix::CreateTranslation(center.x, 0.18f, center.z);
		const Matrix sweepSpokeA =
			Matrix::CreateScale(hazard.radius * 1.9f, 0.018f, 0.14f) *
			Matrix::CreateRotationY(m_sceneTime * (0.8f + hazard.cycleSpeed) + hazard.phaseSeed * 3.1f) *
			Matrix::CreateTranslation(center.x, 0.2f, center.z);
		const Matrix sweepSpokeB =
			Matrix::CreateScale(0.14f, 0.018f, hazard.radius * 1.9f) *
			Matrix::CreateRotationY(-m_sceneTime * (0.72f + hazard.cycleSpeed * 0.7f) - hazard.phaseSeed * 2.7f) *
			Matrix::CreateTranslation(center.x, 0.205f, center.z);
		m_obstacleMesh->Draw(sweepRingWorld, m_view, m_proj, StageTint(Color(1.0f, 0.36f, 0.24f, 0.62f)));
		m_obstacleMesh->Draw(sweepCoreWorld, m_view, m_proj, StageTint(Color(1.0f, 0.62f, 0.3f, 0.28f)));
		m_obstacleMesh->Draw(sweepSpokeA, m_view, m_proj, StageTint(Color(1.0f, 0.78f, 0.28f, 0.52f)));
		m_obstacleMesh->Draw(sweepSpokeB, m_view, m_proj, StageTint(Color(1.0f, 0.55f, 0.18f, 0.46f)));
		const Matrix pathMarker =
			Matrix::CreateScale(0.2f, 0.02f, (hazard.end - hazard.start).Length() * 0.5f) *
			Matrix::CreateRotationY(std::atan2(hazard.end.x - hazard.start.x, hazard.end.z - hazard.start.z)) *
			Matrix::CreateTranslation((hazard.start.x + hazard.end.x) * 0.5f, 0.07f, (hazard.start.z + hazard.end.z) * 0.5f);
		m_floorMesh->Draw(pathMarker, m_view, m_proj, StageTint(Color(1.0f, 0.42f, 0.18f, 0.16f)));
	}

	for (size_t beaconIndex = 0; beaconIndex < m_recoveryBeacons.size(); ++beaconIndex)
	{
		const RecoveryBeaconInfo& beacon = m_recoveryBeacons[beaconIndex];
		const float cooldownRatio = (beacon.maxCooldownSec > 0.0f) ? Utility::MathEx::Clamp(1.0f - beacon.cooldownSec / beacon.maxCooldownSec, 0.0f, 1.0f) : 1.0f;
		const float pulse = std::sinf(m_sceneTime * 2.2f + beacon.pulseSeed * 5.1f) * 0.5f + 0.5f;
		const Color beaconColor = (beacon.cooldownSec <= 0.0f)
			? Color(0.28f, 1.0f, 0.9f, 0.9f)
			: Color(0.3f, 0.58f + cooldownRatio * 0.22f, 0.8f, 0.48f);
		const Matrix beaconBaseWorld =
			Matrix::CreateScale(beacon.radius * (0.88f + pulse * 0.08f), 0.05f, beacon.radius * (0.88f + pulse * 0.08f)) *
			Matrix::CreateTranslation(beacon.position.x, 0.11f, beacon.position.z);
		const Matrix beaconCoreWorld =
			Matrix::CreateScale(0.56f + pulse * 0.06f, 0.032f, 0.56f + pulse * 0.06f) *
			Matrix::CreateTranslation(beacon.position.x, 0.17f, beacon.position.z);
		const Matrix beaconRingWorld =
			Matrix::CreateScale(beacon.radius * (1.14f + pulse * 0.1f), 0.018f, beacon.radius * (1.14f + pulse * 0.1f)) *
			Matrix::CreateTranslation(beacon.position.x, 0.18f, beacon.position.z);
		m_floorMesh->Draw(beaconBaseWorld, m_view, m_proj, StageTint(beaconColor));
		m_obstacleMesh->Draw(beaconCoreWorld, m_view, m_proj, StageTint(Color(beaconColor.x, beaconColor.y, beaconColor.z, 0.38f + cooldownRatio * 0.18f)));
		m_obstacleMesh->Draw(beaconRingWorld, m_view, m_proj, StageTint(Color(0.92f, 1.0f, 1.0f, 0.52f)));
	}

	auto DrawTheme2Strips = [&]()
	{
		for (int lane = 0; lane < 3; ++lane)
		{
			const float fl = static_cast<float>(lane);
			const float z = -14.0f + fl * 14.0f;
			const Matrix stripA =
				Matrix::CreateScale(18.0f, 0.022f, 1.2f) *
				Matrix::CreateTranslation(0.0f, 0.07f, z);
			const Matrix stripB =
				Matrix::CreateScale(18.0f, 0.016f, 0.16f) *
				Matrix::CreateTranslation(0.0f, 0.085f, z);
			m_obstacleMesh->Draw(stripA, m_view, m_proj, StageTint(Color(0.24f, 0.20f, 0.18f, 0.16f)));
			m_obstacleMesh->Draw(stripB, m_view, m_proj, StageTint(Color(0.78f, 0.60f, 0.20f, 0.10f)));
		}
	};

	auto DrawTheme3Arcs = [&]()
	{
		(void)stagePulse;
	};

	auto DrawTheme2Walls = [&]()
	{
	};

	auto DrawTheme3CenterDais = [&]()
	{
		const Matrix centerDais =
			Matrix::CreateScale(8.8f, 0.08f, 8.8f) *
			Matrix::CreateTranslation(0.0f, 0.05f, 0.0f);
		const Matrix centerTrim =
			Matrix::CreateScale(6.6f, 0.03f, 6.6f) *
			Matrix::CreateTranslation(0.0f, 0.09f, 0.0f);
		m_floorMesh->Draw(centerDais, m_view, m_proj, StageTint(Color(0.14f, 0.18f, 0.28f, 0.54f)));
		m_obstacleMesh->Draw(centerTrim, m_view, m_proj, StageTint(Color(0.30f, 0.72f, 0.82f, 0.10f)));
	};

	switch (stageTheme)
	{
	case 2:
		DrawTheme2Strips();
		DrawTheme2Walls();
		break;
	case 3:
		DrawTheme3Arcs();
		DrawTheme3CenterDais();
		break;
	default:
		break;
	}

	for (size_t i = 0; i < m_obstacleWorlds.size(); ++i)
	{
		const float fi = static_cast<float>(i);
		const float pulse = std::sinf(m_sceneTime * 0.7f + fi * 0.6f) * 0.5f + 0.5f;
		const Color obstacleColor(
			Utility::MathEx::Clamp(floorPalette.panelColor.x + 0.02f + pulse * 0.02f, 0.0f, 1.0f),
			Utility::MathEx::Clamp(floorPalette.panelColor.y + 0.02f + pulse * 0.02f, 0.0f, 1.0f),
			Utility::MathEx::Clamp(floorPalette.panelColor.z + 0.02f + pulse * 0.03f, 0.0f, 1.0f),
			1.0f);
		m_obstacleMesh->Draw(m_obstacleWorlds[i], m_view, m_proj, StageTint(obstacleColor));
		const Matrix capWorld =
			Matrix::CreateScale(m_obstacleWorlds[i]._11 * 0.92f, 0.03f, m_obstacleWorlds[i]._33 * 0.92f) *
			Matrix::CreateTranslation(m_obstacleWorlds[i]._41, m_obstacleWorlds[i]._42 + m_obstacleWorlds[i]._22 * 0.5f, m_obstacleWorlds[i]._43);
		m_obstacleMesh->Draw(capWorld, m_view, m_proj, StageTint(Color(floorPalette.accentColor.x, floorPalette.accentColor.y, floorPalette.accentColor.z, 0.10f)));
	}
}
