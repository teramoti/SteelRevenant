//------------------------//------------------------
// Contents(処理内容) 敵ドロップアイテムの生成・更新・描画を実装する。
//------------------------//------------------------
// Bug#6修正:
//   修正前: 毎フレーム全 Dead 敵に対してドロップ判定が走り、
//           1体から毎フレームアイテムが出続けた。
//   修正後: prevStates と enemies の状態を比較し、
//           「前フレームは生存・今フレームは Dead」になった敵のみ対象にする。
//   また <cstdlib> のインクルードを追加済み（ヘッダ側に記載）。
//------------------------//------------------------
#include "GameSceneUpdate.h"
#include "GameSceneVisualPalette.h"
#include "../../GameSystem/DrawManager.h"

#include <cmath>
#include <cstdlib>  // Bug#6: rand() 使用のため

using namespace GameSceneVisualPalette;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void GameSceneUpdate::Initialize(
    ID3D11Device*        device,
    ID3D11DeviceContext* context)
{
    m_itemPrimitive = DirectX::GeometricPrimitive::CreateSphere(context, 0.55f, 10);
    Reset();
    (void)device;
}

void GameSceneUpdate::Reset()
{
    m_items.clear();
}

// 敵ドロップ処理。
// Bug#6修正: 前フレームの状態と比較して「新規 Dead」のみを処理する。
void GameSceneUpdate::ProcessEnemyDrops(
    const std::vector<Action::EnemyState>& enemies,
    std::vector<Action::EnemyStateType>&   prevStates)
{
    // prevStates のサイズを enemies に合わせる（初回・スポーン増加時）
    if (prevStates.size() != enemies.size())
    {
        prevStates.resize(enemies.size(), Action::EnemyStateType::Idle);
    }

    for (size_t i = 0; i < enemies.size(); ++i)
    {
        const bool wasAlive = (prevStates[i] != Action::EnemyStateType::Dead);
        const bool isDead   = (enemies[i].state == Action::EnemyStateType::Dead);

        // 「このフレームに新たに死亡した」敵のみドロップ判定
        if (wasAlive && isDead)
        {
            // 30% の確率でヘルスアイテムをドロップ
            const int roll = std::rand() % 100;
            if (roll < static_cast<int>(kDropChance * 100.0f))
            {
                SpawnDrop(enemies[i].position, DropItemType::Health);
            }
        }

        // 前フレーム状態を更新
        prevStates[i] = enemies[i].state;
    }
}

// ドロップアイテムを生成する。
void GameSceneUpdate::SpawnDrop(const Vector3& pos, DropItemType type)
{
    DropItem item;
    item.type     = type;
    item.position = pos;
    item.active   = true;
    item.bobTimer = 0.0f;
    item.lifetime = 15.0f;
    m_items.push_back(item);
}

// ドロップアイテムの更新（プレイヤー接触・ライフタイム）。
void GameSceneUpdate::UpdateDropItems(
    Action::PlayerState& player,
    Action::GameState&   gameState,
    float                dt)
{
    for (DropItem& item : m_items)
    {
        if (!item.active) continue;

        item.bobTimer  += dt;
        item.lifetime  -= dt;

        if (item.lifetime <= 0.0f)
        {
            item.active = false;
            continue;
        }

        const float dx = player.position.x - item.position.x;
        const float dz = player.position.z - item.position.z;
        const float distSq = dx * dx + dz * dz;

        if (distSq > kPickupRadius * kPickupRadius) continue;

        // 取得
        item.active = false;
        switch (item.type)
        {
        case DropItemType::Health:
        {
            // タイマー回復（被ダメージ分を少し取り返す）
            const float restore = kHealthRestore;
            gameState.stageTimer = std::min(
                gameState.stageTimer + restore,
                gameState.stageTimer + restore); // clamp は BattleRuleBook の初期値上限が必要なため単純加算
            gameState.damageTaken = std::max(0, gameState.damageTaken - static_cast<int>(kHealthRestore));
            break;
        }
        }
    }

    // 不活性アイテムを定期的に除去（線形掃引でOK）
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(),
            [](const DropItem& d) { return !d.active && d.lifetime <= 0.0f; }),
        m_items.end());
}

void GameSceneUpdate::Render(
    ID3D11DeviceContext*  context,
    const Matrix&         view,
    const Matrix&         projection)
{
    if (!m_itemPrimitive) return;

    System::DrawManager::GetInstance().ApplyAlphaBlendState();

    for (const DropItem& item : m_items)
    {
        if (!item.active) continue;

        const float bob   = 0.18f * std::sin(item.bobTimer * 3.0f);
        const float alpha = (item.lifetime < 3.0f)
            ? item.lifetime / 3.0f   // 消える直前に点滅
            : 0.85f;

        const Matrix world =
            Matrix::CreateTranslation(
                item.position.x,
                item.position.y + 0.6f + bob,
                item.position.z);

        m_itemPrimitive->Draw(world, view, projection,
            DirectX::SimpleMath::Color(
                healthItemColor.R(),
                healthItemColor.G(),
                healthItemColor.B(), alpha));
    }

    System::DrawManager::GetInstance().ApplyPrimitiveState();
    (void)context;
}
