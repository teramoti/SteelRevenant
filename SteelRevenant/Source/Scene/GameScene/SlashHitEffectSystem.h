#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <array>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>

namespace SceneFx
{
    class SlashHitEffectSystem
    {
    public:
        void Reset();

        // ヒットエフェクトをスポーンする
        void Spawn(
            const DirectX::SimpleMath::Vector3& position,
            float baseYaw,
            const DirectX::SimpleMath::Color& tint,
            bool killBurst = false);
        void SpawnKill(
            const DirectX::SimpleMath::Vector3& position,
            float baseYaw,
            const DirectX::SimpleMath::Color& tint);

        // 攻撃中フレームごとに刃の位置を記録する（攻撃中のみ呼ぶ）
        void RecordTrailSample(
            const DirectX::SimpleMath::Vector3& base,
            const DirectX::SimpleMath::Vector3& tip,
            int comboIndex,
            int comboLevel);

        // 攻撃終了時に軌跡バッファをリセットする
        void ClearTrail();

        void Update(float dt);
        void Draw(
            DirectX::GeometricPrimitive& streakMesh,
            DirectX::GeometricPrimitive& particleMesh,
            const DirectX::SimpleMath::Matrix& view,
            const DirectX::SimpleMath::Matrix& proj) const;

    private:
        // ヒットバースト（衝撃点エフェクト）
        struct SlashBurst
        {
            DirectX::SimpleMath::Vector3 position;
            float ageSec;
            float lifetimeSec;
            float yaw;
            float seed;
            DirectX::SimpleMath::Color tint;
            bool killBurst;
            bool active;
        };

        // 斬撃軌跡のサンプル点（刃の根本と刃先を1組として保持）
        struct TrailSample
        {
            DirectX::SimpleMath::Vector3 base; // グリップ側（刃の根本）
            DirectX::SimpleMath::Vector3 tip;  // 刃先
            float ageSec;                      // 記録からの経過時間
            int comboIndex;                    // コンボ段階（1/2/3）
            int comboLevel;                    // コンボレベル（見た目の強度に使用）
            bool active;
        };

        static constexpr size_t kMaxBursts  = 8;
        // 軌跡サンプルは1攻撃あたり最大16点を保持する
        static constexpr size_t kMaxTrailSamples = 16;
        // サンプル1点あたりの残留時間（秒）
        static constexpr float kTrailSampleLifetime = 0.10f;

        std::array<SlashBurst,   kMaxBursts>       m_bursts{};
        std::array<TrailSample,  kMaxTrailSamples>  m_trailSamples{};
    };
}

