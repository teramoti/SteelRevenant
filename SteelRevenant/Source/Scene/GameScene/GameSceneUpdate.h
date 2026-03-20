#pragma once
#include <d3d11.h>
#include <vector>
#include <memory>
#include <cstdlib>    // Bug#6修正: <cstdlib> を追加
#include <SimpleMath.h>
#include <GeometricPrimitive.h>
#include "../../Action/CombatSystem.h"
#include "../../Action/GameState.h"

// ドロップアイテムの種類。
enum class DropItemType { Health };

// ドロップアイテム 1 件の状態。
struct DropItem
{
    DropItemType                  type;
    DirectX::SimpleMath::Vector3  position;
    bool                          active    = true;
    float                         bobTimer  = 0.0f;
    float                         lifetime  = 15.0f; // 15 秒で消える
};

// アイテムドロップの発生・更新・描画を担当するクラス。
// Bug#6修正: 「新たに死亡した敵のみ」ドロップ判定を行う。
class GameSceneUpdate
{
public:
    GameSceneUpdate() = default;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    // Bug#6修正: prev_states を比較して「このフレームに新たに死亡した敵」を特定する。
    void ProcessEnemyDrops(
        const std::vector<Action::EnemyState>& enemies,
        std::vector<Action::EnemyStateType>&   prevStates);

    void UpdateDropItems(Action::PlayerState& player, Action::GameState& gameState, float dt);

    void Render(
        ID3D11DeviceContext*                context,
        const DirectX::SimpleMath::Matrix&  view,
        const DirectX::SimpleMath::Matrix&  projection);

    void Reset();

    const std::vector<DropItem>& GetItems() const { return m_items; }

private:
    void SpawnDrop(const DirectX::SimpleMath::Vector3& pos, DropItemType type);

    std::vector<DropItem>                         m_items;
    std::unique_ptr<DirectX::GeometricPrimitive>  m_itemPrimitive;

    static constexpr float kPickupRadius  = 1.5f;
    static constexpr float kHealthRestore = 25.0f;
    static constexpr float kDropChance    = 0.30f;  // 30% でドロップ
};
