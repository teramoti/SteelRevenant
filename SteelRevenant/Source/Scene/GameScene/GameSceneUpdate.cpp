//------------------------//------------------------
// Contents(処理内容) 敵ドロップアイテムの生成・更新・描画を実装する。
// Bug#6修正:
//   修正前: 毎フレーム全 Dead 敵に対してドロップ判定が走り1体から連続ドロップ。
//   修正後: prevStates と比較し「前フレーム生存→今フレーム Dead」の敵のみ処理。
//           <cstdlib> インクルードをヘッダ側に追加済み。
//------------------------//------------------------
#include "GameSceneUpdate.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"
#include <algorithm>
#include <cmath>

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void GameSceneUpdate::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_itemPrimitive = DirectX::GeometricPrimitive::CreateSphere(context, 0.55f, 10);
    Reset();
    (void)device;
}

void GameSceneUpdate::Reset() { m_items.clear(); }

// Bug#6修正: 新規 Dead 判定
void GameSceneUpdate::ProcessEnemyDrops(
    const std::vector<Action::EnemyState>& enemies,
    std::vector<Action::EnemyStateType>&   prevStates)
{
    if (prevStates.size() != enemies.size())
        prevStates.resize(enemies.size(), Action::EnemyStateType::Idle);

    for (size_t i = 0; i < enemies.size(); ++i)
    {
        const bool wasAlive = (prevStates[i] != Action::EnemyStateType::Dead);
        const bool isDead   = (enemies[i].state == Action::EnemyStateType::Dead);

        if (wasAlive && isDead) // このフレームに初めて死亡した敵のみ
        {
            if ((std::rand() % 100) < static_cast<int>(kDropChance * 100.0f))
                SpawnDrop(enemies[i].position, DropItemType::Health);
        }
        prevStates[i] = enemies[i].state;
    }
}

void GameSceneUpdate::SpawnDrop(const Vector3& pos, DropItemType type)
{
    m_items.push_back({ type, pos, true, 0.0f, 15.0f });
}

void GameSceneUpdate::UpdateDropItems(Action::PlayerState& player, Action::GameState& gameState, float dt)
{
    for (DropItem& item : m_items)
    {
        if (!item.active) continue;
        item.bobTimer += dt;
        item.lifetime -= dt;
        if (item.lifetime <= 0.0f) { item.active = false; continue; }

        const float dx = player.position.x - item.position.x;
        const float dz = player.position.z - item.position.z;
        if (dx*dx + dz*dz > kPickupRadius*kPickupRadius) continue;

        item.active = false;
        gameState.stageTimer  += kHealthRestore;
        gameState.damageTaken  = std::max(0, gameState.damageTaken - static_cast<int>(kHealthRestore));
    }
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
        [](const DropItem& d){ return !d.active && d.lifetime <= 0.0f; }), m_items.end());
}

void GameSceneUpdate::Render(ID3D11DeviceContext* context, const Matrix& view, const Matrix& proj)
{
    if (!m_itemPrimitive) return;
    System::DrawManager::GetInstance().ApplyAlphaBlendState();
    for (const DropItem& item : m_items)
    {
        if (!item.active) continue;
        const float bob   = 0.18f * std::sin(item.bobTimer * 3.0f);
        const float alpha = (item.lifetime < 3.0f) ? item.lifetime / 3.0f : 0.85f;
        const Matrix w = Matrix::CreateTranslation(item.position.x, item.position.y+0.6f+bob, item.position.z);
        m_itemPrimitive->Draw(w, view, proj,
            DirectX::SimpleMath::Color(healthItemColor.R(),healthItemColor.G(),healthItemColor.B(),alpha));
    }
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    (void)context;
}
