//------------------------//------------------------
// Contents(処理内容) 斬撃ヒット演出の生成、更新、描画を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include <vector>

#include <GeometricPrimitive.h>
#include <SimpleMath.h>

namespace SceneFx
{
	class SlashHitEffectSystem
	{
	public:
		// 演出状態を初期化する。
		void Reset();
		// ヒット発生位置に演出を生成する。
		void Spawn(const DirectX::SimpleMath::Vector3& position, float baseYaw, const DirectX::SimpleMath::Color& tint);
		// 演出寿命を更新する。
		void Update(float dt);
		// 有効な演出を描画する。
		void Draw(
			DirectX::GeometricPrimitive& streakMesh,
			DirectX::GeometricPrimitive& particleMesh,
			const DirectX::SimpleMath::Matrix& view,
			const DirectX::SimpleMath::Matrix& proj) const;
	private:
		struct SlashBurst
		{
			DirectX::SimpleMath::Vector3 position;
			float ageSec;
			float lifetimeSec;
			float yaw;
			float seed;
			DirectX::SimpleMath::Color tint;
		};

		std::vector<SlashBurst> m_bursts;
	};
}

