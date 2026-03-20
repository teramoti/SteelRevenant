//------------------------//------------------------
// Contents(処理内容) 斬撃ヒット演出の生成、更新、描画を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 18
//------------------------//------------------------
#include "SlashHitEffectSystem.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace SceneFx
{
	namespace
	{
		constexpr int kMaxBursts = 16;
		constexpr int kBloodDropletCount = 15;
	}

	// 演出状態を初期化する。
	void SlashHitEffectSystem::Reset()
	{
		m_bursts.clear();
	}

	// ヒット発生位置に演出を生成する。
	void SlashHitEffectSystem::Spawn(const Vector3& position, float baseYaw, const Color& tint)
	{
		SlashBurst burst = {};
		burst.position = position + Vector3(0.0f, 1.0f, 0.0f);
		burst.ageSec = 0.0f;
		burst.lifetimeSec = 0.24f;
		burst.yaw = baseYaw;
		burst.seed = baseYaw * 1.618f + position.x * 0.37f + position.z * 0.23f;
		burst.tint = tint;
		m_bursts.push_back(burst);

		if (m_bursts.size() > kMaxBursts)
		{
			m_bursts.erase(m_bursts.begin(), m_bursts.begin() + (m_bursts.size() - kMaxBursts));
		}
	}

	// 演出寿命を更新する。
	void SlashHitEffectSystem::Update(float dt)
	{
		if (m_bursts.empty())
		{
			return;
		}

		for (size_t i = 0; i < m_bursts.size(); ++i)
		{
			m_bursts[i].ageSec += dt;
		}

		m_bursts.erase(
			std::remove_if(
				m_bursts.begin(),
				m_bursts.end(),
				[](const SlashBurst& burst)
				{
					return burst.ageSec >= burst.lifetimeSec;
				}),
			m_bursts.end());
	}

	// 有効な演出を描画する。
	void SlashHitEffectSystem::Draw(
		DirectX::GeometricPrimitive& streakMesh,
		DirectX::GeometricPrimitive& particleMesh,
		const Matrix& view,
		const Matrix& proj) const
	{
		for (size_t i = 0; i < m_bursts.size(); ++i)
		{
			const SlashBurst& burst = m_bursts[i];
			const float safeLifetime = std::max(0.001f, burst.lifetimeSec);
			const float lifeT = std::max(0.0f, std::min(1.0f, burst.ageSec / safeLifetime));
			const float fade = 1.0f - lifeT;
			const float slashFlash = std::sinf((1.0f - lifeT) * DirectX::XM_PIDIV2);

			const Vector3 forward(std::sinf(burst.yaw), 0.0f, std::cosf(burst.yaw));
			const Vector3 right(forward.z, 0.0f, -forward.x);
			Vector3 sprayDir = forward + right * 0.62f + Vector3(0.0f, 0.24f, 0.0f);
			sprayDir.Normalize();
			const Vector3 burstOrigin = burst.position + forward * 0.10f + Vector3(0.0f, 0.04f, 0.0f);

			const Color slashShadowColor(0.10f, 0.01f, 0.01f, 0.14f + fade * 0.22f);
			const Color slashColor(1.0f, 0.97f, 0.94f, 0.30f + fade * 0.52f);
			const Color slashAccentColor(
				std::max(0.86f, burst.tint.x),
				std::max(0.82f, burst.tint.y),
				std::max(0.82f, burst.tint.z),
				0.18f + fade * 0.28f);
			const Color bloodCoreColor(0.86f, 0.05f, 0.05f, 0.18f + fade * 0.54f);
			const Color bloodEdgeColor(0.58f, 0.02f, 0.03f, 0.12f + fade * 0.32f);

			const Matrix shadowWorld =
				Matrix::CreateScale(0.22f, 0.05f, 1.36f - lifeT * 0.20f) *
				Matrix::CreateRotationZ(0.58f) *
				Matrix::CreateRotationX(-0.22f) *
				Matrix::CreateRotationY(burst.yaw - 0.10f) *
				Matrix::CreateTranslation(burstOrigin + right * 0.08f - forward * 0.04f);
			streakMesh.Draw(shadowWorld, view, proj, slashShadowColor);

			const Matrix mainSlashWorld =
				Matrix::CreateScale(0.13f, 0.03f, 1.78f - lifeT * 0.26f) *
				Matrix::CreateRotationZ(0.62f) *
				Matrix::CreateRotationX(-0.18f) *
				Matrix::CreateRotationY(burst.yaw + 0.04f) *
				Matrix::CreateTranslation(burstOrigin + right * 0.03f);
			streakMesh.Draw(mainSlashWorld, view, proj, slashColor);

			const Matrix accentSlashWorld =
				Matrix::CreateScale(0.07f, 0.022f, 1.14f - lifeT * 0.14f) *
				Matrix::CreateRotationZ(-0.36f) *
				Matrix::CreateRotationX(0.14f) *
				Matrix::CreateRotationY(burst.yaw + 0.34f) *
				Matrix::CreateTranslation(burstOrigin + right * 0.22f + forward * 0.10f + Vector3(0.0f, 0.03f, 0.0f));
			streakMesh.Draw(accentSlashWorld, view, proj, slashAccentColor);

			for (int dropletIndex = 0; dropletIndex < kBloodDropletCount; ++dropletIndex)
			{
				const float fd = static_cast<float>(dropletIndex);
				const float spreadT = fd / static_cast<float>(std::max(1, kBloodDropletCount - 1));
				const float phase = burst.seed + fd * 0.79f;
				const float distance = (0.15f + spreadT * 1.04f) * (0.66f + slashFlash * 0.72f);
				const float lateral = (spreadT - 0.5f) * (0.34f + spreadT * 0.60f) + std::sinf(phase * 1.4f) * 0.05f;
				const float lift = 0.06f + (1.0f - spreadT) * 0.08f + std::cosf(phase * 1.7f) * 0.03f - lifeT * 0.12f;
				const float squash = 0.07f + std::fmod(fd, 3.0f) * 0.025f;
				const float length = squash * (1.15f + spreadT * 1.8f);
				const Vector3 dropletPos =
					burstOrigin +
					sprayDir * distance +
					right * lateral +
					Vector3(0.0f, lift, 0.0f);
				const float dropletYaw = burst.yaw + 0.28f + std::sinf(phase) * 0.42f;
				const float dropletPitch = -0.18f + std::cosf(phase * 1.3f) * 0.16f;
				const Color dropletColor = (dropletIndex % 3 == 0) ? bloodEdgeColor : bloodCoreColor;
				const Matrix dropletWorld =
					Matrix::CreateScale(squash, 0.028f + spreadT * 0.02f, length) *
					Matrix::CreateRotationX(dropletPitch) *
					Matrix::CreateRotationY(dropletYaw) *
					Matrix::CreateTranslation(dropletPos);
				streakMesh.Draw(dropletWorld, view, proj, dropletColor);
			}

			for (int blotIndex = 0; blotIndex < 3; ++blotIndex)
			{
				const float fb = static_cast<float>(blotIndex);
				const float phase = burst.seed * (1.2f + fb * 0.17f) + fb * 1.41f;
				const float distance = 0.18f + fb * 0.16f + slashFlash * 0.14f;
				const Vector3 blotPos =
					burstOrigin +
					sprayDir * distance +
					right * (-0.12f + fb * 0.12f + std::sinf(phase) * 0.04f) +
					Vector3(0.0f, 0.02f + std::cosf(phase) * 0.02f, 0.0f);
				const float blotScale = 0.09f + fb * 0.05f + slashFlash * 0.03f;
				const Matrix blotWorld =
					Matrix::CreateScale(blotScale * 0.95f, blotScale * 0.50f, blotScale * 0.72f) *
					Matrix::CreateRotationY(burst.yaw + std::sinf(phase) * 0.55f) *
					Matrix::CreateTranslation(blotPos);
				particleMesh.Draw(blotWorld, view, proj, (blotIndex == 1) ? bloodCoreColor : bloodEdgeColor);
			}
		}
	}
}
