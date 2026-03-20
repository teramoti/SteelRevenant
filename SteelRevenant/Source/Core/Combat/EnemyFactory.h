#pragma once

//=============================================================================
// EnemyFactory.h
//
// 【役割】
//   危険度に応じた敵インスタンスを生成するファクトリクラス。
//   「どんな敵を作るか」というルールをここに集約することで、
//   SurvivalDirector や GameScene が敵生成の詳細を知らなくて済む。
//
// 【設計パターン】
//   - Factory Method（static メソッド版）:
//       インスタンスを持たない純粋なファクトリとして実装する。
//       将来「難易度プリセット別の敵プール」が必要になった場合は
//       Strategy を追加することで SurvivalDirector 側を変更せずに対応できる。
//
// 【生成ルール概要】
//   dangerLevel 1   : BladeRush のみ（標準ステータス）
//   dangerLevel 2〜 : BladeFlank が一定割合で混入
//   spawnSerial      : 連番を使ってアーキタイプを決定論的に割り当てる
//                      （同じ条件で同じ敵構成を再現できる）
//=============================================================================

#include <SimpleMath.h>
#include "CombatTypes.h"

namespace Core
{
    //=========================================================================
    // EnemyFactory
    //=========================================================================
    class EnemyFactory
    {
    public:
        /// @brief 危険度とスポーン通番に応じた敵 1 体を生成する。
        /// @param spawnBase    スポーン基準座標（周辺にランダム配置される）
        /// @param dangerLevel  現在の危険度 (1〜)
        /// @param spawnSerial  スポーン通番（アーキタイプ割り当てに使用）
        /// @return 初期化済みの EnemyState
        static EnemyState CreateEnemy(
            const DirectX::SimpleMath::Vector3& spawnBase,
            int dangerLevel,
            int spawnSerial);

    private:
        EnemyFactory() = delete; ///< 全メソッドが static のためインスタンス化を禁止する
    };

} // namespace Core
