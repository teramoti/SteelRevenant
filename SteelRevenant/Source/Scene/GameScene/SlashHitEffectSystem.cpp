#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "SlashHitEffectSystem.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace SceneFx
{

    void SlashHitEffectSystem::Reset()
    {
        for (SlashBurst& burst : m_bursts)
        {
            burst = {};
            burst.active = false;
        }
        ClearTrail();
    }

    void SlashHitEffectSystem::Spawn(const Vector3& position, float baseYaw, const Color& tint, bool killBurst)
    {
        size_t slotIndex = kMaxBursts;
        for (size_t i = 0; i < m_bursts.size(); ++i)
        {
            if (!m_bursts[i].active)
            {
                slotIndex = i;
                break;
            }
        }

        if (slotIndex == kMaxBursts)
        {
            float oldestAge = -1.0f;
            for (size_t i = 0; i < m_bursts.size(); ++i)
            {
                if (m_bursts[i].ageSec > oldestAge)
                {
                    oldestAge = m_bursts[i].ageSec;
                    slotIndex = i;
                }
            }
        }

        SlashBurst& burst = m_bursts[slotIndex];
        burst = {};
        // 敵の足元座標から刃が実際に当たる高さ（腹?胸）へオフセットする
        // 通常ヒット: Y+0.88（大剣の刃先が胴体中央に届く高さ）
        // キルヒット: Y+1.05（頭部寄りに演出を上げる）
        burst.position = position + Vector3(0.0f, killBurst ? 1.05f : 0.88f, 0.0f);
        burst.ageSec = 0.0f;
        burst.lifetimeSec = killBurst ? 0.18f : 0.10f;
        burst.yaw = baseYaw;
        burst.seed = baseYaw * 1.618f + position.x * 0.37f + position.z * 0.23f;
        burst.tint = tint;
        burst.killBurst = killBurst;
        burst.active = true;
    }

    void SlashHitEffectSystem::SpawnKill(const Vector3& position, float baseYaw, const Color& tint)
    {
        Spawn(position, baseYaw, tint, true);
    }

    void SlashHitEffectSystem::RecordTrailSample(
        const Vector3& base,
        const Vector3& tip,
        int comboIndex,
        int comboLevel)
    {
        // 最も古い非アクティブスロットを探す。なければ最古のスロットを上書きする
        size_t slot = kMaxTrailSamples;
        float oldestAge = -1.0f;
        size_t oldestSlot = 0;
        for (size_t i = 0; i < m_trailSamples.size(); ++i)
        {
            if (!m_trailSamples[i].active)
            {
                slot = i;
                break;
            }
            if (m_trailSamples[i].ageSec > oldestAge)
            {
                oldestAge = m_trailSamples[i].ageSec;
                oldestSlot = i;
            }
        }
        if (slot == kMaxTrailSamples)
        {
            slot = oldestSlot;
        }

        TrailSample& s = m_trailSamples[slot];
        s.base       = base;
        s.tip        = tip;
        s.ageSec     = 0.0f;
        s.comboIndex = comboIndex;
        s.comboLevel = comboLevel;
        s.active     = true;
    }

    void SlashHitEffectSystem::ClearTrail()
    {
        for (TrailSample& s : m_trailSamples)
        {
            s = {};
            s.active = false;
        }
    }

    void SlashHitEffectSystem::Update(float dt)
    {
        for (SlashBurst& burst : m_bursts)
        {
            if (!burst.active) continue;
            burst.ageSec += dt;
            if (burst.ageSec >= burst.lifetimeSec)
            {
                burst.active = false;
            }
        }
        // 軌跡サンプルの寿命を進め、期限切れを非アクティブにする
        for (TrailSample& s : m_trailSamples)
        {
            if (!s.active) continue;
            s.ageSec += dt;
            if (s.ageSec >= kTrailSampleLifetime)
            {
                s.active = false;
            }
        }
    }

    void SlashHitEffectSystem::Draw(
        DirectX::GeometricPrimitive& streakMesh,
        DirectX::GeometricPrimitive& particleMesh,
        const Matrix& view,
        const Matrix& proj) const
    {
        // 斬撃軌跡：隣接するサンプル点間をセグメントとして描画する
        // ageSec が小さいほど新しいサンプルなので、curr より ageSec が小さい次のサンプルを探す
        // 時間経過で透明度が下がり自然に消える
        for (size_t i = 0; i < m_trailSamples.size(); ++i)
        {
            const TrailSample& curr = m_trailSamples[i];
            if (!curr.active) continue;

            // 現在のサンプルより新しい（ageSec が小さい）有効サンプルを隣接点として探す
            const TrailSample* next = nullptr;
            for (size_t j = 0; j < m_trailSamples.size(); ++j)
            {
                if (j == i) continue;
                if (!m_trailSamples[j].active) continue;
                if (m_trailSamples[j].ageSec >= curr.ageSec) continue;
                if (next == nullptr || m_trailSamples[j].ageSec > next->ageSec)
                {
                    next = &m_trailSamples[j];
                }
            }
            if (!next) continue;

            const float lifeT = std::clamp(curr.ageSec / kTrailSampleLifetime, 0.0f, 1.0f);
            const float fade  = 1.0f - lifeT;

            // コンボ段階に応じた幅・色の強度を決める
            // comboIndex 1: 標準、2: やや広め、3: 強め（コンボレベルも考慮）
            const float comboScale = (curr.comboIndex == 3) ? 1.30f
                                   : (curr.comboIndex == 2) ? 1.14f
                                   : 1.0f;
            const float levelScale = 1.0f + static_cast<float>(curr.comboLevel) * 0.08f;
            const float widthBase  = 0.055f * comboScale * levelScale; // 剣の重さを感じる幅
            const float trailWidth = widthBase * (0.70f + fade * 0.30f);
            const float alphaCore  = (curr.comboIndex == 3) ? (0.28f + fade * 0.38f)
                                   : (curr.comboIndex == 2) ? (0.22f + fade * 0.32f)
                                   : (0.16f + fade * 0.28f);
            const float alphaRim   = alphaCore * 0.55f;

            // コアカラー（コンボ段階ごとに色相を変える）
            // 1段: シルバー、2段: シアン寄り、3段: 紫
            const Color coreColor = (curr.comboIndex == 3)
                ? Color(0.72f, 0.44f, 0.98f, alphaCore)
                : (curr.comboIndex == 2)
                    ? Color(0.38f, 0.88f, 0.96f, alphaCore)
                    : Color(0.80f, 0.84f, 0.92f, alphaCore);
            const Color rimColor(
                coreColor.R() * 0.85f + 0.12f,
                coreColor.G() * 0.85f + 0.12f,
                coreColor.B() * 0.85f + 0.12f,
                alphaRim);

            // 軌跡セグメントの向きと長さを計算する
            const Vector3 currMid = (curr.base + curr.tip) * 0.5f;
            const Vector3 nextMid = (next->base + next->tip) * 0.5f;
            const Vector3 segment = currMid - nextMid;
            const float   segLen  = segment.Length();
            const Vector3 span    = curr.tip - curr.base;
            const float   spanLen = span.Length();

            if (segLen < 0.02f || spanLen < 0.06f) continue;

            const float   segYaw   = std::atan2(segment.x, segment.z);
            const float   segPitch = -std::atan2(segment.y,
                std::sqrt(segment.x * segment.x + segment.z * segment.z));
            const Vector3 segCenter = nextMid + segment * 0.5f;

            // コア軌跡（太め・不透明寄り）
            streakMesh.Draw(
                Matrix::CreateScale(spanLen * trailWidth * 5.0f, trailWidth, segLen) *
                Matrix::CreateRotationZ(0.12f) *
                Matrix::CreateRotationX(segPitch) *
                Matrix::CreateRotationY(segYaw) *
                Matrix::CreateTranslation(segCenter),
                view, proj, coreColor);

            // リム軌跡（細め・半透明）
            streakMesh.Draw(
                Matrix::CreateScale(spanLen * trailWidth * 7.0f, trailWidth * 0.42f, segLen * 0.90f) *
                Matrix::CreateRotationZ(0.12f) *
                Matrix::CreateRotationX(segPitch) *
                Matrix::CreateRotationY(segYaw) *
                Matrix::CreateTranslation(segCenter),
                view, proj, rimColor);
        }

        // ヒットバースト（衝撃点エフェクト）
        for (const SlashBurst& burst : m_bursts)
        {
            if (!burst.active) continue;

            const float safeLifetime = std::max(0.001f, burst.lifetimeSec);
            const float lifeT = std::clamp(burst.ageSec / safeLifetime, 0.0f, 1.0f);
            const float fade  = 1.0f - lifeT;

            const Vector3 forward(std::sin(burst.yaw), 0.0f, std::cos(burst.yaw));
            const Vector3 right(forward.z, 0.0f, -forward.x);
            const Vector3 origin = burst.position + forward * 0.06f;

            // 十字ストリーク水平方向：刃が通った方向に短い閃光を描く
            // fade で急速に縮小し「ヒット瞬間だけ光る」挙動にする
            const float streakLen  = (burst.killBurst ? 0.46f : 0.32f) * fade;
            const float streakW    = (burst.killBurst ? 0.044f : 0.032f) * (0.60f + fade * 0.40f);
            const Color streakColor(
                std::clamp(burst.tint.R() * 1.10f, 0.0f, 1.0f),
                std::clamp(burst.tint.G() * 1.10f, 0.0f, 1.0f),
                std::clamp(burst.tint.B() * 1.10f, 0.0f, 1.0f),
                burst.killBurst ? (0.55f * fade) : (0.42f * fade));

            // 水平ストリーク（プレイヤーの向きに沿った方向）
            streakMesh.Draw(
                Matrix::CreateScale(streakW, streakW * 0.28f, streakLen) *
                Matrix::CreateRotationY(burst.yaw) *
                Matrix::CreateTranslation(origin),
                view, proj, streakColor);

            // 斜めストリーク（刃の角度に近い斜め60度）
            streakMesh.Draw(
                Matrix::CreateScale(streakW * 0.70f, streakW * 0.28f, streakLen * 0.70f) *
                Matrix::CreateRotationZ(burst.killBurst ? 0.62f : 0.52f) *
                Matrix::CreateRotationY(burst.yaw + 0.78f) *
                Matrix::CreateTranslation(origin),
                view, proj, streakColor);

            // 衝撃フラッシュ球：ヒット瞬間に最大、fade で収縮する
            const float flashScale = (burst.killBurst ? 0.22f : 0.15f) * fade;
            const Color flashColor(
                std::clamp(burst.tint.R() * 1.22f + 0.14f, 0.0f, 1.0f),
                std::clamp(burst.tint.G() * 1.22f + 0.14f, 0.0f, 1.0f),
                std::clamp(burst.tint.B() * 1.22f + 0.14f, 0.0f, 1.0f),
                burst.killBurst ? (0.72f * fade) : (0.56f * fade));
            particleMesh.Draw(
                Matrix::CreateScale(flashScale, flashScale * 0.62f, flashScale) *
                Matrix::CreateTranslation(origin),
                view, proj, flashColor);

            // 衝撃リング：登場直後は小さく、lifeT に従い外側へ広がって消える
            // fade の二乗で透明度を素早く落とし残留感を抑える
            const float ringScale = (burst.killBurst ? 0.10f : 0.08f) + lifeT * (burst.killBurst ? 0.52f : 0.36f);
            const Color ringColor(
                std::clamp(burst.tint.R() * 1.04f, 0.0f, 1.0f),
                std::clamp(burst.tint.G() * 1.04f, 0.0f, 1.0f),
                std::clamp(burst.tint.B() * 1.04f, 0.0f, 1.0f),
                (burst.killBurst ? 0.46f : 0.32f) * fade * fade);
            particleMesh.Draw(
                Matrix::CreateScale(ringScale, 0.022f, ringScale) *
                Matrix::CreateTranslation(origin),
                view, proj, ringColor);

            if (burst.killBurst)
            {
                // キル時のみ外周リングを追加して演出を強める
                const float outerRing = 0.18f + lifeT * 0.84f;
                particleMesh.Draw(
                    Matrix::CreateScale(outerRing, 0.018f, outerRing) *
                    Matrix::CreateTranslation(origin + Vector3(0.0f, 0.06f, 0.0f)),
                    view, proj,
                    Color(
                        std::clamp(burst.tint.R() * 1.08f, 0.0f, 1.0f),
                        std::clamp(burst.tint.G() * 1.08f, 0.0f, 1.0f),
                        std::clamp(burst.tint.B() * 1.08f, 0.0f, 1.0f),
                        0.28f * fade * fade));
            }
        }
    }
}
