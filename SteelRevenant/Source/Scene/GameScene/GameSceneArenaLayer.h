#pragma once
#include <d3d11.h>
#include <vector>
#include <memory>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>
#include "../../Action/CombatSystem.h"

// 速度UPアイテムの状態。
struct SpeedUpItem
{
    DirectX::SimpleMath::Vector3 position;
    bool   active     = true;
    float  bobTimer   = 0.0f;  // ふわふわアニメ用
};

// 速度UPアイテムの管理・描画を担当するクラス。
// Bug#5修正: 毎フレーム SetTuning を呼ばず、効果終了時のみリセットする。
class GameSceneArenaLayer
{
public:
    GameSceneArenaLayer() = default;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int stageIndex);

    // Bug#5修正: speedActive フラグと残り時間で SetTuning 呼び出しを制御。
    void UpdateSpeedUpItems(
        Action::PlayerState&    player,
        Action::CombatSystem&   combatSystem,
        float                   dt);

    void Render(
        ID3D11DeviceContext*                context,
        const DirectX::SimpleMath::Matrix&  view,
        const DirectX::SimpleMath::Matrix&  projection);

    void Reset(int stageIndex);

    bool IsSpeedActive() const { return m_speedActive; }
    float GetSpeedRemaining() const { return m_speedTimer; }

private:
    std::vector<SpeedUpItem>                      m_items;
    std::unique_ptr<DirectX::GeometricPrimitive>  m_itemPrimitive;

    // Bug#5修正: 速度アップ状態を一元管理するフラグ・タイマー
    bool  m_speedActive        = false;
    float m_speedTimer         = 0.0f;
    float m_baseWalkSpeed      = 0.0f;

    static constexpr float kPickupRadius     = 1.5f;
    static constexpr float kSpeedBoostFactor = 1.6f;
    static constexpr float kSpeedDuration    = 5.0f;
};
