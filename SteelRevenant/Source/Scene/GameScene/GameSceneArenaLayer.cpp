//------------------------//------------------------
// Contents(処理内容) 速度UPアイテムのスポーン・更新・描画を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "GameScene.h"

#include "../../Utility/SimpleMathEx.h"
#include "../../Utility/Sound/AudioSystem.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr float kItemLifetimeSec    = 12.0f;  ///< アイテムの存在時間 (秒)
    constexpr float kItemPickupRadius   = 1.6f;   ///< 取得判定半径 (m)
    constexpr float kItemDropChance     = 0.30f;  ///< 敵撃破時のドロップ確率 (30%)
    constexpr float kSpeedUpDurationSec = 8.0f;   ///< 速度UP効果の持続時間 (秒)
    constexpr float kSpeedUpMultiplier  = 1.6f;   ///< 速度UP倍率
    constexpr int   kMaxItemsOnField    = 3;       ///< フィールド上の最大アイテム数
}

// 指定座標に速度UPアイテムをスポーンする。
void GameScene::SpawnSpeedUpItem(const Vector3& position)
{
    // フィールド上限を超えないようにする
    int activeCount = 0;
    for (const auto& item : m_speedUpItems)
    {
        if (item.active) { ++activeCount; }
    }
    if (activeCount >= kMaxItemsOnField) { return; }

    SpeedUpItem item;
    item.position    = position + Vector3(0.0f, 0.4f, 0.0f);
    item.lifetimeSec = kItemLifetimeSec;
    item.pulseSeed   = position.x * 0.31f + position.z * 0.17f;
    item.active      = true;

    // 空きスロットを再利用する
    for (auto& slot : m_speedUpItems)
    {
        if (!slot.active)
        {
            slot = item;
            return;
        }
    }
    m_speedUpItems.push_back(item);
}

// アイテムの寿命・取得判定を 1 フレーム更新する。
void GameScene::UpdateSpeedUpItems(float dt)
{
    const bool wasActive = m_speedUpTimer > 0.0f;

    // 速度UPタイマーを進める
    if (m_speedUpTimer > 0.0f)
    {
        m_speedUpTimer = std::max(0.0f, m_speedUpTimer - dt);
    }

    const bool isActive = m_speedUpTimer > 0.0f;

    // 効果が切れた瞬間だけ速度をリセットする（毎フレーム書き換えない）
    if (wasActive && !isActive)
    {
        Action::CombatTuning tuning = m_combat.GetTuning();
        tuning.walkSpeed = 7.2f;
        m_combat.SetTuning(tuning);
    }

    for (auto& item : m_speedUpItems)
    {
        if (!item.active) { continue; }

        // 寿命更新
        item.lifetimeSec -= dt;
        if (item.lifetimeSec <= 0.0f)
        {
            item.active = false;
            continue;
        }

        // プレイヤーとの取得判定
        const float dx = m_player.position.x - item.position.x;
        const float dz = m_player.position.z - item.position.z;
        const float distSq = dx * dx + dz * dz;
        if (distSq <= kItemPickupRadius * kItemPickupRadius)
        {
            item.active    = false;
            m_speedUpTimer = kSpeedUpDurationSec;

            // 速度UP適用
            Action::CombatTuning t = m_combat.GetTuning();
            t.walkSpeed = 7.2f * kSpeedUpMultiplier;
            m_combat.SetTuning(t);

            GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::BeaconHeal, 0.85f);
        }
    }
}

// 速度UPアイテムをフィールドに描画する。
void GameScene::DrawSpeedUpItems()
{
    if (!m_effectOrbMesh || !m_obstacleMesh) { return; }

    for (const auto& item : m_speedUpItems)
    {
        if (!item.active) { continue; }

        const float pulse = std::sinf(m_sceneTime * 3.2f + item.pulseSeed) * 0.5f + 0.5f;
        const float hover = std::sinf(m_sceneTime * 2.4f + item.pulseSeed) * 0.18f;

        // アイテム本体（明るい黄緑の球）
        const float scale = 0.32f + pulse * 0.06f;
        m_effectOrbMesh->Draw(
            Matrix::CreateScale(scale) *
            Matrix::CreateTranslation(item.position + Vector3(0.0f, hover, 0.0f)),
            m_view, m_proj,
            Color(0.30f, 1.00f, 0.45f, 0.95f));

        // 外周グロー（半透明・やや大きい）
        m_effectOrbMesh->Draw(
            Matrix::CreateScale(scale * 1.8f) *
            Matrix::CreateTranslation(item.position + Vector3(0.0f, hover, 0.0f)),
            m_view, m_proj,
            Color(0.20f, 0.90f, 0.35f, 0.22f + pulse * 0.12f));

        // 地面のサークルマーカー
        m_effectOrbMesh->Draw(
            Matrix::CreateScale(0.9f, 0.04f, 0.9f) *
            Matrix::CreateTranslation(item.position.x, 0.10f, item.position.z),
            m_view, m_proj,
            Color(0.25f, 0.95f, 0.40f, 0.35f + pulse * 0.15f));

        // 残り時間が少ないと点滅（3秒以下）
        if (item.lifetimeSec < 3.0f)
        {
            const float blinkAlpha = std::sinf(m_sceneTime * 8.0f) > 0.0f ? 0.9f : 0.1f;
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.18f, 0.18f, 0.04f) *
                Matrix::CreateTranslation(item.position + Vector3(0.0f, hover + 0.5f, 0.0f)),
                m_view, m_proj,
                Color(1.0f, 1.0f, 0.3f, blinkAlpha));
        }
    }
}
