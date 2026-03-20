//------------------------//------------------------
// Contents(処理内容) 本編ゲームシーンのフレーム更新処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 17
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "GameScene.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <Keyboard.h>
#include <Mouse.h>

#include "../../GameSystem/InputManager.h"
#include "../../Utility/SimpleMathEx.h"
#include "../../Utility/Sound/AudioSystem.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace
{
	constexpr float kCameraPitchMin = -0.75f;
	constexpr float kCameraPitchMax = 0.35f;
	constexpr float kCameraShakeSettleSec = 0.18f;
	constexpr float kHitStopSecPerHit = 0.028f;
	constexpr float kHitStopSecMax = 0.065f;
	constexpr float kEnemyProjectileLifetimeSec = 1.8f;

	// 線分と球の交差判定を返す。
	bool SegmentHitsSphere(const Vector3& start, const Vector3& end, const Vector3& center, float radius)
	{
		const Vector3 segment = end - start;
		const float segmentLengthSq = segment.LengthSquared();
		if (segmentLengthSq <= 0.000001f)
		{
			return (center - start).LengthSquared() <= radius * radius;
		}
		const float t = Utility::MathEx::Clamp((center - start).Dot(segment) / segmentLengthSq, 0.0f, 1.0f);
		const Vector3 closest = start + segment * t;
		return (center - closest).LengthSquared() <= radius * radius;
	}
}

// 現在入力を戦闘更新用のスナップショットへまとめる。
Action::InputSnapshot GameScene::BuildInputSnapshot(float dt) const
{
	const System::InputManager& inputManager = System::InputManager::GetInstance();
	const DirectX::Keyboard::State kb = inputManager.GetKeyboardState();
	const DirectX::Keyboard::KeyboardStateTracker keyTracker = inputManager.GetKeyboardTracker();
	const DirectX::Mouse::ButtonStateTracker mouseTracker = inputManager.GetMouseTracker();
	const DirectX::Mouse::State mouseState = inputManager.GetMouseState();

	Action::InputSnapshot in;
	in.dt = dt;
	in.mouseDelta = inputManager.GetMouseDelta();
	in.moveForward = kb.W || kb.Up;
	in.moveBack = kb.S || kb.Down;
	in.moveRight = kb.D || kb.Right;
	in.moveLeft = kb.A || kb.Left;
	in.attackPressed = (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED);
	in.guardHeld = kb.RightControl || kb.LeftControl || mouseState.rightButton;
	in.lockTogglePressed = keyTracker.pressed.F;
	return in;
}

// ポーズ切り替え入力を処理し、切り替え有無を返す。
bool GameScene::HandlePauseToggle(const DirectX::Keyboard::KeyboardStateTracker& keyTracker)
{
	if (!keyTracker.pressed.Escape || m_gameState.IsFinished())
	{
		return false;
	}

	m_isPaused = !m_isPaused;
	if (m_isPaused)
	{
		m_pauseSelectedIndex = 0;
		DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
		SetSystemCursorVisible(true);
	}
	else
	{
		DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
		SetSystemCursorVisible(false);
	}
	return true;
}

// ポーズ中の入力とメニュー状態を更新する。
bool GameScene::UpdatePausedState(
	float dt,
	const DirectX::Mouse::ButtonStateTracker& mouseTracker,
	const DirectX::Mouse::State& mouseState)
{
	if (!m_isPaused || m_gameState.IsFinished())
	{
		return false;
	}

	UpdatePauseMenu(mouseTracker, mouseState);
	if (m_textRenderer != nullptr)
	{
		m_textRenderer->Update(dt);
	}
	return true;
}

// デバッグ用調整値ショートカットを処理する。
void GameScene::UpdateDebugTuningHotkeys(const DirectX::Keyboard::KeyboardStateTracker& keyTracker)
{
	if (!m_showHudDetail)
	{
		return;
	}

	System::InputManager& inputManager = System::InputManager::GetInstance();
	if (keyTracker.pressed.PageUp)
	{
		inputManager.SetMouseSensitivity(inputManager.GetMouseSensitivity() + 0.01f);
	}
	if (keyTracker.pressed.PageDown)
	{
		inputManager.SetMouseSensitivity(inputManager.GetMouseSensitivity() - 0.01f);
	}
	if (keyTracker.pressed.Insert)
	{
		m_attackAssistRangeView = Utility::MathEx::Clamp(m_attackAssistRangeView + 0.1f, 1.4f, 4.5f);
		m_combat.SetAttackAssistConfig(m_attackAssistRangeView, m_attackAssistDotView);
	}
	if (keyTracker.pressed.Delete)
	{
		m_attackAssistRangeView = Utility::MathEx::Clamp(m_attackAssistRangeView - 0.1f, 1.4f, 4.5f);
		m_combat.SetAttackAssistConfig(m_attackAssistRangeView, m_attackAssistDotView);
	}
	if (keyTracker.pressed.Home)
	{
		m_attackAssistDotView = Utility::MathEx::Clamp(m_attackAssistDotView - 0.02f, -0.25f, 0.95f);
		m_combat.SetAttackAssistConfig(m_attackAssistRangeView, m_attackAssistDotView);
	}
	if (keyTracker.pressed.End)
	{
		m_attackAssistDotView = Utility::MathEx::Clamp(m_attackAssistDotView + 0.02f, -0.25f, 0.95f);
		m_combat.SetAttackAssistConfig(m_attackAssistRangeView, m_attackAssistDotView);
	}
}

// ロックオン対象の選択状態を更新する。
void GameScene::UpdateLockOn(const Action::InputSnapshot& input)
{
	if (input.lockTogglePressed)
	{
		if (m_player.lockEnemyIndex >= 0)
		{
			m_player.lockEnemyIndex = -1;
		}
		else
		{
			m_player.lockEnemyIndex = m_combat.FindNearestEnemy(m_enemies, m_player.position, 24.0f);
		}
	}

	if (m_player.lockEnemyIndex >= 0)
	{
		if (m_player.lockEnemyIndex >= static_cast<int>(m_enemies.size()) ||
			m_enemies[static_cast<size_t>(m_player.lockEnemyIndex)].state == Action::EnemyStateType::Dead)
		{
			m_player.lockEnemyIndex = -1;
		}
	}
}

// プレイヤーと敵の戦闘フレームを更新する。
void GameScene::UpdateCombatFrame(float dt, const Action::InputSnapshot& input)
{
	if (!m_gameState.IsFinished())
	{
		m_gameState.Tick(dt);
		if (m_gameState.IsFinished())
		{
			m_gameState.score = ComputeStageScore();
			return;
		}

		m_survivalDirector.Update(dt, m_gameState.killCount, m_gameState.survivalTimeSec, CountLivingEnemies());
		m_gameState.dangerLevel = m_survivalDirector.GetDangerLevel();
		m_gameState.peakDangerLevel = m_survivalDirector.GetPeakDangerLevel();

		const Action::CombatTuning& tuning = m_combat.GetTuning();
		const float predictedYaw = Utility::MathEx::WrapRadians(m_player.yaw + input.mouseDelta.x * tuning.playerYawScale);
		const Vector3 cameraForward(std::sin(predictedYaw), 0.0f, std::cos(predictedYaw));
		const Vector3 cameraRight(cameraForward.z, 0.0f, -cameraForward.x);

		const int damageBefore = m_gameState.damageTaken;
		std::vector<bool> hitStateBefore;
		hitStateBefore.reserve(m_enemies.size());
		for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
		{
			hitStateBefore.push_back(m_enemies[enemyIndex].hitByCurrentSwing);
		}

		const Vector3 previousPlayerPosition = m_player.position;
		m_combat.UpdatePlayer(m_player, input, cameraForward, cameraRight, m_gameState, ResolveGroundHeight(m_player.position));
		ResolvePlayerObstacleCollision(previousPlayerPosition);
		m_player.position.y = ResolveGroundHeight(m_player.position);
		m_combat.UpdateEnemies(m_enemies, m_player, input, m_grid, m_solver, m_gameState);
		UpdateEnemyProjectiles(dt);
		if (m_gameState.IsFinished())
		{
			m_gameState.score = ComputeStageScore();
			return;
		}
		for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
		{
			Action::EnemyState& enemy = m_enemies[enemyIndex];
			if (!enemy.firedProjectileThisFrame)
			{
				continue;
			}

			Color projectileColor(1.0f, 0.72f, 0.18f, 1.0f);
			switch (enemy.archetype)
			{
			case Action::EnemyArchetype::BladeRush:
				projectileColor = Color(1.00f, 0.28f, 0.12f, 1.0f);
				break;
			case Action::EnemyArchetype::BladeFlank:
				projectileColor = Color(0.82f, 0.12f, 0.62f, 1.0f);
				break;
			case Action::EnemyArchetype::GunHold:
				projectileColor = Color(0.28f, 0.72f, 1.00f, 1.0f);
				break;
			case Action::EnemyArchetype::GunPressure:
			default:
				projectileColor = Color(1.00f, 0.78f, 0.18f, 1.0f);
				break;
			}

			EnemyProjectileInfo projectile;
			projectile.position = enemy.projectileSpawnPosition;
			projectile.velocity = enemy.projectileVelocity;
			projectile.color = projectileColor;
			projectile.radius = (enemy.archetype == Action::EnemyArchetype::GunHold) ? 0.32f : 0.38f;
			projectile.lifetimeSec = kEnemyProjectileLifetimeSec;
			projectile.damage = enemy.projectileDamage;
			m_enemyProjectiles.push_back(projectile);
			enemy.firedProjectileThisFrame = false;
		}
		m_combat.ResolvePlayerAttack(m_player, m_enemies, m_gameState);
		UpdateArenaObjectiveLayer(dt);

		int newlyHitCount = 0;
		for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
		{
			if (enemyIndex >= hitStateBefore.size())
			{
				continue;
			}
			if (!hitStateBefore[enemyIndex] && m_enemies[enemyIndex].hitByCurrentSwing)
			{
				++newlyHitCount;
				const Vector3 hitDir = Utility::MathEx::SafeNormalize(m_enemies[enemyIndex].position - m_player.position);
				const float hitYaw = std::atan2(hitDir.x, hitDir.z);
				Color hitTint(0.92f, 0.08f, 0.06f, 1.0f);
				switch (m_enemies[enemyIndex].archetype)
				{
				case Action::EnemyArchetype::BladeRush:
					hitTint = Color(1.00f, 0.28f, 0.12f, 1.0f);
					break;
				case Action::EnemyArchetype::BladeFlank:
					hitTint = Color(0.82f, 0.12f, 0.62f, 1.0f);
					break;
				case Action::EnemyArchetype::GunHold:
					hitTint = Color(0.28f, 0.72f, 1.00f, 1.0f);
					break;
				case Action::EnemyArchetype::GunPressure:
				default:
					hitTint = Color(1.00f, 0.78f, 0.18f, 1.0f);
					break;
				}
				m_slashHitEffects.Spawn(m_enemies[enemyIndex].position, hitYaw, hitTint);
			}
		}
		if (newlyHitCount > 0)
		{
			const float hitBoost = 0.38f + std::min(0.42f, static_cast<float>(newlyHitCount) * 0.10f);
			m_hitBloodTimer = Utility::MathEx::Clamp(m_hitBloodTimer + hitBoost, 0.0f, 1.0f);
			AddCameraShake(0.11f + std::min(0.16f, static_cast<float>(newlyHitCount) * 0.035f), 0.13f);
			m_hitStopTimer = std::max(m_hitStopTimer, std::min(kHitStopSecMax, kHitStopSecPerHit * static_cast<float>(newlyHitCount)));
		}
		if (m_gameState.damageTaken > damageBefore)
		{
			const int deltaDamage = m_gameState.damageTaken - damageBefore;
			const float damageBoost = 0.3f + std::min(0.45f, static_cast<float>(deltaDamage) * 0.04f);
			m_damageBloodTimer = Utility::MathEx::Clamp(m_damageBloodTimer + damageBoost, 0.0f, 1.0f);
			AddCameraShake(0.12f + std::min(0.16f, static_cast<float>(deltaDamage) * 0.01f), 0.16f);
		}
		UpdateSurvivalFlow();
	}
	else
	{
		m_finishDelay -= dt;
		if (m_finishDelay <= 0.0f)
		{
			PushResultAndExit();
			return;
		}
	}
}

// 敵の飛翔弾を更新して命中判定を行う。
void GameScene::UpdateEnemyProjectiles(float dt)
{
	if (m_enemyProjectiles.empty())
	{
		return;
	}

	const Vector3 playerHitCenter = m_player.position + Vector3(0.0f, 0.9f, 0.0f);
	const float playerHitRadius = 0.55f;

	for (size_t projectileIndex = 0; projectileIndex < m_enemyProjectiles.size();)
	{
		EnemyProjectileInfo& projectile = m_enemyProjectiles[projectileIndex];
		const Vector3 previousPosition = projectile.position;
		projectile.position += projectile.velocity * dt;
		projectile.lifetimeSec -= dt;

		bool removeProjectile = (projectile.lifetimeSec <= 0.0f);
		if (!removeProjectile && IsInsideObstacle(projectile.position, projectile.radius * 0.65f))
		{
			removeProjectile = true;
		}

		if (!removeProjectile &&
			SegmentHitsSphere(previousPosition, projectile.position, playerHitCenter, projectile.radius + playerHitRadius) &&
			m_player.damageGraceTimer <= 0.0f)
		{
			float damage = projectile.damage;
			if (m_player.guarding)
			{
				damage *= m_combat.GetTuning().enemyRangedGuardDamageScale;
			}
			m_gameState.stageTimer = std::max(0.0f, m_gameState.stageTimer - 1.0f);
			m_gameState.damageTaken += static_cast<int>(std::round(damage));
			if (m_gameState.stageTimer <= 0.0f)
			{
				m_gameState.stageTimer = 0.0f;
				m_gameState.timeExpired = true;
			}
			m_player.damageGraceTimer = m_combat.GetTuning().damageGraceSec;
			GameAudio::AudioSystem::GetInstance().PlaySe(
				m_player.guarding ? GameAudio::SeId::GuardBlock : GameAudio::SeId::PlayerHit,
				m_player.guarding ? 0.70f : 0.82f);
			removeProjectile = true;
		}

		if (removeProjectile)
		{
			m_enemyProjectiles.erase(m_enemyProjectiles.begin() + static_cast<std::ptrdiff_t>(projectileIndex));
			continue;
		}

		++projectileIndex;
	}
}

// 入力、戦闘、演出、進行を 1 フレーム更新する。
void GameScene::Update(const DX::StepTimer& stepTimer)
{
	const float dt = static_cast<float>(stepTimer.GetElapsedSeconds());
	if (dt <= 0.0f)
	{
		return;
	}

	m_sceneTime += dt;
	m_hitBloodTimer = std::max(0.0f, m_hitBloodTimer - dt * 2.4f);
	m_damageBloodTimer = std::max(0.0f, m_damageBloodTimer - dt * 1.7f);
	m_cameraShakeTimer = std::max(0.0f, m_cameraShakeTimer - dt);
	m_cameraShakeStrength = std::max(0.0f, m_cameraShakeStrength - dt * 2.8f);
	m_slashHitEffects.Update(dt);

	System::InputManager& inputManager = System::InputManager::GetInstance();
	const DirectX::Keyboard::KeyboardStateTracker keyTracker = inputManager.GetKeyboardTracker();
	const DirectX::Mouse::ButtonStateTracker mouseTracker = inputManager.GetMouseTracker();
	const DirectX::Mouse::State mouseState = inputManager.GetMouseState();
	m_pauseClickFxTimer = std::max(0.0f, m_pauseClickFxTimer - dt);

	HandlePauseToggle(keyTracker);
	if (UpdatePausedState(dt, mouseTracker, mouseState))
	{
		return;
	}

	const Action::InputSnapshot input = BuildInputSnapshot(dt);
	UpdateDebugTuningHotkeys(keyTracker);
	m_mouseSensitivityView = inputManager.GetMouseSensitivity();

	if (keyTracker.pressed.F3)
	{
		m_showPathDebug = !m_showPathDebug;
	}
	if (keyTracker.pressed.F4)
	{
		m_showHudDetail = !m_showHudDetail;
	}

	UpdateLockOn(input);

	if (m_hitStopTimer > 0.0f && !m_gameState.IsFinished())
	{
		m_hitStopTimer = std::max(0.0f, m_hitStopTimer - dt);
		if (m_textRenderer != nullptr)
		{
			m_textRenderer->Update(dt);
		}
		UpdateCamera(0.0f, input);
		return;
	}

	UpdateCombatFrame(dt, input);

	if (m_textRenderer != nullptr)
	{
		m_textRenderer->Update(dt);
	}

	UpdateCamera(dt, input);
}

// サバイバル進行と敵補充状態を更新する。
void GameScene::UpdateSurvivalFlow()
{
	m_survivalDirector.Update(0.0f, m_gameState.killCount, m_gameState.survivalTimeSec, CountLivingEnemies());
	m_gameState.dangerLevel = m_survivalDirector.GetDangerLevel();
	m_gameState.peakDangerLevel = m_survivalDirector.GetPeakDangerLevel();
	if (!m_gameState.IsFinished())
	{
		SpawnEnemyBatch(false);
	}
	m_gameState.score = ComputeStageScore();
}


