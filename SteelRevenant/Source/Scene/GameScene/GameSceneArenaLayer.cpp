//------------------------//------------------------
// Contents(処理内容) 速度UPアイテムの管理・描画を実装する。
//------------------------//------------------------
// Bug#5修正:
//   修正前: UpdateSpeedUpItems が毎フレーム SetTuning を呼んでいたため
//           速度管理が壊れていた。
//   修正後: アイテム取得時(効果開始)と効果切れ時(効果終了)の
//           「状態遷移のタイミングのみ」 SetTuning を呼ぶ。
//           毎フレームの SetTuning 呼び出しを完全に除去した。
//------------------------//------------------------
#include "GameSceneArenaLayer.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"
#include "../../Utility/SimpleMathEx.h"

#include <cmath>

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void GameSceneArenaLayer::Initialize(
    ID3D11Device*        device,
    ID3D11DeviceContext* context,
    int                  stageIndex)
{
    m_itemPrimitive = DirectX::GeometricPrimitive::CreateOctahedron(context, 0.7f);
    Reset(stageIndex);
    (void)device;
}

// ステージに応じたアイテム配置を設定する。
void GameSceneArenaLayer::Reset(int stageIndex)
{
    m_items.clear();
    m_speedActive = false;
    m_speedTimer  = 0.0f;

    // ステージごとに配置を変える
    const float baseR = (stageIndex == 3) ? 12.0f : 10.0f;
    const int   count = (stageIndex == 1) ? 3 : (stageIndex == 2) ? 4 : 5;

    for (int i = 0; i < count; ++i)
    {
        const float angle = DirectX::XM_2PI * static_cast<float>(i) / static_cast<float>(count);
        SpeedUpItem item;
        item.position = Vector3(
            std::cos(angle) * baseR,
            0.8f,
            std::sin(angle) * baseR);
        item.active   = true;
        item.bobTimer = static_cast<float>(i) * 0.4f; // 位相をずらす
        m_items.push_back(item);
    }
}

// 速度UPアイテムの更新。
// Bug#5修正: SetTuning は「取得時」と「効果切れ時」のみ呼ぶ。
void GameSceneArenaLayer::UpdateSpeedUpItems(
    Action::PlayerState&   player,
    Action::CombatSystem&  combatSystem,
    float                  dt)
{
    // ボブアニメ更新
    for (SpeedUpItem& item : m_items)
    {
        if (item.active)
            item.bobTimer += dt;
    }

    // --- 効果タイマー処理 ---
    if (m_speedActive)
    {
        m_speedTimer -= dt;
        if (m_speedTimer <= 0.0f)
        {
            // 効果切れ: この瞬間だけ SetTuning でリセット (Bug#5修正の核心)
            m_speedTimer  = 0.0f;
            m_speedActive = false;

            Action::CombatTuning resetTuning = combatSystem.GetTuning();
            resetTuning.walkSpeed = m_baseWalkSpeed;
            combatSystem.SetTuning(resetTuning);
        }
        // 効果中は何もしない（毎フレーム SetTuning を呼ばない）
        return;
    }

    // --- プレイヤーとの接触判定 ---
    for (SpeedUpItem& item : m_items)
    {
        if (!item.active) continue;

        const float dx = player.position.x - item.position.x;
        const float dz = player.position.z - item.position.z;
        const float distSq = dx * dx + dz * dz;

        if (distSq > kPickupRadius * kPickupRadius) continue;

        // 取得: この瞬間だけ SetTuning を呼んで速度を上げる (Bug#5修正)
        item.active = false;

        const Action::CombatTuning& currentTuning = combatSystem.GetTuning();
        m_baseWalkSpeed = currentTuning.walkSpeed;
        m_speedActive   = true;
        m_speedTimer    = kSpeedDuration;

        Action::CombatTuning boostedTuning = currentTuning;
        boostedTuning.walkSpeed = m_baseWalkSpeed * kSpeedBoostFactor;
        combatSystem.SetTuning(boostedTuning);

        break; // 1フレームに1取得まで
    }
}

void GameSceneArenaLayer::Render(
    ID3D11DeviceContext*  context,
    const Matrix&         view,
    const Matrix&         projection)
{
    if (!m_itemPrimitive) return;

    System::DrawManager::GetInstance().ApplyAlphaBlendState();

    for (const SpeedUpItem& item : m_items)
    {
        if (!item.active) continue;

        const float bob   = 0.2f * std::sin(item.bobTimer * 2.5f);
        const float spin  = item.bobTimer * 1.8f;
        const float pulse = 0.85f + 0.15f * std::sin(item.bobTimer * 4.0f);

        const Matrix world =
            Matrix::CreateScale(pulse)
            * Matrix::CreateRotationY(spin)
            * Matrix::CreateTranslation(
                item.position.x,
                item.position.y + bob,
                item.position.z);

        m_itemPrimitive->Draw(world, view, projection,
            DirectX::SimpleMath::Color(
                speedItemColor.R(),
                speedItemColor.G(),
                speedItemColor.B(), 0.90f));
    }

    System::DrawManager::GetInstance().ApplyPrimitiveState();
    (void)context;
}
