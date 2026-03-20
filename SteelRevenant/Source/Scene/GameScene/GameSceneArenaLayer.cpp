//------------------------//------------------------
// Contents(処理内容) 速度UPアイテムの管理・描画を実装する。
// Bug#5修正:
//   修正前: 毎フレーム SetTuning を呼んでいたため速度管理が壊れていた。
//   修正後: アイテム取得時(効果開始)と効果切れ時のみ SetTuning を呼ぶ。
//------------------------//------------------------
#include "GameSceneArenaLayer.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"
#include <cmath>

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void GameSceneArenaLayer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int stageIndex)
{
    m_itemPrimitive = DirectX::GeometricPrimitive::CreateOctahedron(context, 0.7f);
    Reset(stageIndex);
    (void)device;
}

void GameSceneArenaLayer::Reset(int stageIndex)
{
    m_items.clear(); m_speedActive = false; m_speedTimer = 0.0f;
    const float R = (stageIndex == 3) ? 12.0f : 10.0f;
    const int   N = (stageIndex == 1) ? 3 : (stageIndex == 2) ? 4 : 5;
    for (int i = 0; i < N; ++i)
    {
        const float a = DirectX::XM_2PI * static_cast<float>(i) / static_cast<float>(N);
        SpeedUpItem item;
        item.position = Vector3(std::cos(a)*R, 0.8f, std::sin(a)*R);
        item.active   = true;
        item.bobTimer = static_cast<float>(i) * 0.4f;
        m_items.push_back(item);
    }
}

// Bug#5修正: SetTuning は「取得時」と「効果切れ時」の遷移瞬間のみ呼ぶ。
void GameSceneArenaLayer::UpdateSpeedUpItems(
    Action::PlayerState& player, Action::CombatSystem& combatSystem, float dt)
{
    for (SpeedUpItem& item : m_items) if (item.active) item.bobTimer += dt;

    if (m_speedActive)
    {
        m_speedTimer -= dt;
        if (m_speedTimer <= 0.0f)
        {
            // 効果切れ: この瞬間だけリセット
            m_speedTimer  = 0.0f;
            m_speedActive = false;
            Action::CombatTuning t = combatSystem.GetTuning();
            t.walkSpeed = m_baseWalkSpeed;
            combatSystem.SetTuning(t);
        }
        return; // 効果中は何もしない (毎フレーム SetTuning を呼ばない)
    }

    for (SpeedUpItem& item : m_items)
    {
        if (!item.active) continue;
        const float dx = player.position.x - item.position.x;
        const float dz = player.position.z - item.position.z;
        if (dx*dx + dz*dz > kPickupRadius*kPickupRadius) continue;

        item.active     = false;
        m_baseWalkSpeed = combatSystem.GetTuning().walkSpeed;
        m_speedActive   = true;
        m_speedTimer    = kSpeedDuration;
        Action::CombatTuning t = combatSystem.GetTuning();
        t.walkSpeed = m_baseWalkSpeed * kSpeedBoostFactor;
        combatSystem.SetTuning(t); // 取得時のみ呼ぶ
        break;
    }
}

void GameSceneArenaLayer::Render(ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj)
{
    if (!m_itemPrimitive) return;
    System::DrawManager::GetInstance().ApplyAlphaBlendState();
    for (const SpeedUpItem& item : m_items)
    {
        if (!item.active) continue;
        const float bob  = 0.2f * std::sin(item.bobTimer * 2.5f);
        const float spin = item.bobTimer * 1.8f;
        const float pulse= 0.85f + 0.15f * std::sin(item.bobTimer * 4.0f);
        const Matrix w = Matrix::CreateScale(pulse)*Matrix::CreateRotationY(spin)
            * Matrix::CreateTranslation(item.position.x, item.position.y+bob, item.position.z);
        m_itemPrimitive->Draw(w, view, proj,
            DirectX::SimpleMath::Color(speedItemColor.R(),speedItemColor.G(),speedItemColor.B(),0.90f));
    }
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    (void)context;
}
