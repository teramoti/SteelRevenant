#pragma once

//=============================================================================
// CombatTypes.h
//
// 【役割】
//   戦闘システム全体で共有するデータ型を一箇所に集約する。
//   CombatSystem / EnemyFactory / SurvivalDirector / GameScene など
//   複数のモジュールが同じ型を参照するため、循環インクルードを防ぐ目的で
//   実装クラスから切り離している。
//
// 【設計メモ】
//   すべての型は POD に近い構造体か列挙型。
//   Navigation::PathGrid のみに依存し、他のゲームシステムには非依存。
//=============================================================================

#include <vector>
#include <SimpleMath.h>
#include "../Navigation/PathGrid.h"

namespace Core
{
    //-------------------------------------------------------------------------
    // CombatTuning  ― 戦闘パラメータ調整値
    //
    // ゲームバランス調整時にここを変更するだけで全システムへ反映される。
    // デフォルト値は BattleRuleBook::MakeDefaultTuning() で生成する。
    //-------------------------------------------------------------------------
    struct CombatTuning
    {
        // プレイヤー移動
        float playerYawScale;           ///< マウス水平入力に対する旋回スケール
        float walkSpeed;                ///< 通常移動速度 (m/s)
        float moveAccelGain;            ///< 加速ゲイン（大きいほど即座に最高速へ）
        float moveStopGain;             ///< 減速ゲイン（大きいほど即座に停止）
        float damageGraceSec;           ///< 被弾後の無敵時間 (秒)
        float playerBoundsMin;          ///< プレイヤー X/Z 移動の下限 (m)
        float playerBoundsMax;          ///< プレイヤー X/Z 移動の上限 (m)

        // プレイヤー攻撃
        float playerAttackDamage1;      ///< コンボ 1 段目ダメージ
        float playerAttackDamage2;      ///< コンボ 2 段目ダメージ
        float playerAttackDamage3;      ///< コンボ 3 段目ダメージ
        float comboDamageBonusPerLevel; ///< コンボ Lv ごとのダメージ倍率加算
        float comboLevelGauge1;         ///< Lv1 到達に必要なゲージ量
        float comboLevelGauge2;         ///< Lv2 到達に必要なゲージ量
        float comboLevelGauge3;         ///< Lv3 到達に必要なゲージ量
        float comboMissPenalty;         ///< 空振り時のゲージ減少量
        float lockAssistRangeMultiplier;///< ロックオン時の攻撃判定距離倍率
        float lockAssistDotRelax;       ///< ロックオン時の方向アシスト緩和量
        float attackAssistRangeDefault; ///< デフォルト攻撃アシスト距離 (m)
        float attackAssistDotDefault;   ///< デフォルト攻撃アシスト方向閾値

        // コンボタイミング
        float comboAttackWindowSec;     ///< 攻撃受付ウィンドウ (秒)
        float comboCooldownSec;         ///< 攻撃後クールダウン (秒)
        float comboChainWindowSec;      ///< 連続入力チェーン有効時間 (秒)
        float comboGaugeDecayPerSec;    ///< ゲージ自然減衰量 (/秒)
        float comboGaugeHitGain;        ///< 命中時ゲージ加算量
        float comboGaugeKillBonus;      ///< 撃破時ゲージボーナス

        // 敵 AI 行動
        float enemyRepathSec;                   ///< 経路再計算間隔 (秒)
        float enemyMeleeAttackTriggerDistance;  ///< 近接攻撃開始距離 (m)
        float enemyMeleeAttackReach;            ///< 近接攻撃の到達距離 (m)
        float enemyFlankOffset;                 ///< フランク役の側面オフセット (m)
        float enemyAttackWindupSec;             ///< 攻撃予備動作時間 (秒)
        float enemyMoveSpeedBase;               ///< 敵基本移動速度 (m/s)
        float enemyMoveSpeedPerDanger;          ///< 危険度 1 段階ごとの速度加算
        float enemyIdleWakeupDistance;          ///< Idle → Chase 遷移距離 (m)
        float enemyReturnWakeupDistance;        ///< Return → Chase 再起動距離 (m)
        float enemyLeashDistance;               ///< 最大追跡距離（超えると帰還）
        float enemyReturnArriveDistance;        ///< 帰還完了とみなす距離 (m)
        float enemyHitStunSec;                  ///< 被弾硬直時間 (秒)

        // 敵ダメージ
        float enemyMeleeDamageBase;         ///< 敵近接基本ダメージ
        float enemyMeleeDamagePerDanger;    ///< 危険度 1 段階ごとのダメージ加算
        float enemyMeleeGuardDamageScale;   ///< ガード時ダメージ軽減率
    };

    //-------------------------------------------------------------------------
    // InputSnapshot  ― 1 フレーム分の入力スナップショット
    //
    // デバイス依存コードを CombatSystem から切り離し、テスト容易性を高める。
    // GameScene::BuildInputSnapshot() が生成して CombatSystem へ渡す。
    //-------------------------------------------------------------------------
    struct InputSnapshot
    {
        float dt;                               ///< フレーム経過時間 (秒)
        DirectX::SimpleMath::Vector2 mouseDelta;///< マウス相対移動量 (ピクセル)
        bool moveForward;                       ///< 前進入力
        bool moveBack;                          ///< 後退入力
        bool moveLeft;                          ///< 左移動入力
        bool moveRight;                         ///< 右移動入力
        bool attackPressed;                     ///< 攻撃ボタン押下（このフレームのみ true）
        bool guardHeld;                         ///< ガードボタン保持中
        bool lockTogglePressed;                 ///< ロックオン切り替え（このフレームのみ true）
    };

    //-------------------------------------------------------------------------
    // PlayerState  ― プレイヤー実行時状態
    //-------------------------------------------------------------------------
    struct PlayerState
    {
        DirectX::SimpleMath::Vector3 position;     ///< ワールド座標
        float yaw;                                 ///< 水平回転角 (ラジアン)
        float hp;                                  ///< 現在 HP
        bool  guarding;                            ///< ガード中フラグ
        float damageGraceTimer;                    ///< 被弾無敵残り時間 (秒)
        DirectX::SimpleMath::Vector3 moveVelocity; ///< 移動速度ベクトル
        float attackTimer;                         ///< 攻撃アニメーション残り時間 (秒)
        float attackCooldown;                      ///< 次攻撃まで待機時間 (秒)
        float comboTimer;                          ///< コンボチェーン有効残り時間 (秒)
        int   comboIndex;                          ///< 現在コンボ段数 (0-2)
        float comboGauge;                          ///< コンボゲージ量 (0-100)
        int   comboLevel;                          ///< コンボレベル (0-3)
        int   lockEnemyIndex;                      ///< ロックオン対象インデックス (-1 = なし)
        bool  attackTriggeredThisFrame;            ///< このフレームに攻撃が発動したか

        /// @brief 実行時状態を初期値で構築する。
        PlayerState()
            : position(0.0f, 0.8f, 0.0f), yaw(0.0f), hp(100.0f)
            , guarding(false), damageGraceTimer(0.0f)
            , moveVelocity(DirectX::SimpleMath::Vector3::Zero)
            , attackTimer(0.0f), attackCooldown(0.0f)
            , comboTimer(0.0f), comboIndex(0), comboGauge(0.0f), comboLevel(0)
            , lockEnemyIndex(-1), attackTriggeredThisFrame(false)
        {}
    };

    //-------------------------------------------------------------------------
    // EnemyStateType  ― 敵 AI 状態マシン列挙
    //
    // 遷移図:
    //   Idle ─┬─(接近)─▶ Chase ─(攻撃距離)─▶ Attack
    //         └─(徘徊)─▶ Wander               │
    //   Return ◀─(離脱)─ Chase ◀──────────────┘
    //   Dead   ← HP=0 どこからでも遷移
    //-------------------------------------------------------------------------
    enum class EnemyStateType
    {
        Idle,    ///< 待機（プレイヤーが遠い）
        Wander,  ///< 徘徊（スポーン周辺をランダム移動）
        Chase,   ///< 追跡（A* 経路でプレイヤーへ接近）
        Attack,  ///< 攻撃（予備動作 → 近接ヒット判定）
        Return,  ///< 帰還（リーシュ距離超過でスポーン地点へ戻る）
        Dead     ///< 死亡（描画のみ残し AI 更新停止）
    };

    //-------------------------------------------------------------------------
    // EnemyMoveRole  ― 敵の役割分担
    //-------------------------------------------------------------------------
    enum class EnemyMoveRole
    {
        DirectPressure, ///< 正面からプレイヤーを圧迫する
        Flank           ///< 側面へ回り込む
    };

    //-------------------------------------------------------------------------
    // EnemyArchetype  ― 敵の行動タイプ
    //-------------------------------------------------------------------------
    enum class EnemyArchetype
    {
        BladeRush,  ///< 突進型: 速度高・攻撃力高・ライフ低
        BladeFlank  ///< 迂回型: 速度中・攻撃力中・フランク優先
    };

    //-------------------------------------------------------------------------
    // EnemyState  ― 敵 1 体の実行時状態
    //
    // pathCursor を持つことで A* 計算済み経路を 1 フレームずつ消費する。
    //-------------------------------------------------------------------------
    struct EnemyState
    {
        DirectX::SimpleMath::Vector3 position;       ///< ワールド座標
        DirectX::SimpleMath::Vector3 spawnPosition;  ///< スポーン地点（帰還先）
        float yaw;                                   ///< 水平回転角 (ラジアン)
        float hp;                                    ///< 現在 HP
        float maxHp;                                 ///< 最大 HP
        EnemyStateType  state;                       ///< 現在の AI 状態
        EnemyArchetype  archetype;                   ///< 行動タイプ
        EnemyMoveRole   moveRole;                    ///< 役割分担
        float moveSpeedScale;                        ///< 移動速度倍率
        float attackRangeScale;                      ///< 攻撃距離倍率
        float attackDamageScale;                     ///< 攻撃力倍率
        float attackWindupScale;                     ///< 予備動作時間倍率
        float repathIntervalScale;                   ///< 経路再計算間隔倍率
        float leashScale;                            ///< リーシュ距離倍率
        float stateTimer;                            ///< 現在状態の経過時間 (秒)
        float repathTimer;                           ///< 次回経路再計算まで残り時間 (秒)
        float wanderPauseSec;                        ///< 徘徊中の停止残り時間 (秒)
        bool  hitByCurrentSwing;                     ///< 現コンボで既にヒット済みか
        bool  hasWanderTarget;                       ///< 徘徊目標座標が有効か
        DirectX::SimpleMath::Vector3 wanderTarget;   ///< 徘徊目標座標
        std::vector<Navigation::PathGrid::GridCoord> path; ///< A* 計算済み経路
        size_t pathCursor;                           ///< 次の目標ノードインデックス

        /// @brief 実行時状態を初期値で構築する。
        EnemyState()
            : position(0.0f, 0.8f, 0.0f), spawnPosition(0.0f, 0.8f, 0.0f)
            , yaw(0.0f), hp(40.0f), maxHp(40.0f)
            , state(EnemyStateType::Idle), archetype(EnemyArchetype::BladeRush)
            , moveRole(EnemyMoveRole::DirectPressure)
            , moveSpeedScale(1.0f), attackRangeScale(1.0f), attackDamageScale(1.0f)
            , attackWindupScale(1.0f), repathIntervalScale(1.0f), leashScale(1.0f)
            , stateTimer(0.0f), repathTimer(0.0f), wanderPauseSec(0.0f)
            , hitByCurrentSwing(false), hasWanderTarget(false)
            , wanderTarget(DirectX::SimpleMath::Vector3::Zero)
            , pathCursor(0)
        {}
    };

} // namespace Core
