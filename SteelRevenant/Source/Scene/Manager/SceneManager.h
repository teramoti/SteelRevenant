#pragma once

//=============================================================================
// SceneManager.h
//
// 【役割】
//   ゲーム内の全シーンのライフサイクルと遷移を管理するクラス。
//   アクティブシーンの更新・描画と、シーン切り替え要求の処理を担う。
//
// 【設計パターン】
//   - State（GoF）:
//       アクティブなシーンそのものがゲームの「状態」を表す。
//       SceneManager は状態（シーン）の切り替えを管理し、
//       ゲームループは常に同じ Update/Render インタフェースを呼ぶだけでよい。
//   - Push/Pop スタック（簡易版）:
//       PushScene でオーバーレイシーンへ移行し、
//       PopScene で元のシーンへ復帰できる（一段分のみ）。
//       例: ゲーム中にセッティング画面を重ねる場合に使用。
//
// 【シーン遷移の流れ】
//   SetScene(id)
//     → 次フレームの ApplyRequestedScene() で Finalize → Create → Initialize
//   PushScene(id)
//     → 現在シーンを退避して新シーンへ遷移
//   PopScene()
//     → 退避シーンへ復帰（退避がなければ false を返す）
//=============================================================================

#include "../../System/Util/DirectX11.h"
#include "../../../../StepTimer.h"

class SceneBase;

//=============================================================================
// SceneId  ― シーン識別子
//=============================================================================
enum SceneId
{
    LOGO_SCENE,     ///< ロゴ / スプラッシュ画面
    TITLE_SCENE,    ///< タイトル画面
    SELECT_SCENE,   ///< ステージ選択画面
    SETTINGS_SCENE, ///< 設定画面
    GAME_SCENE,     ///< メインゲームプレイ
    RESULT_SCENE,   ///< リザルト画面
    NUM_SCENES      ///< シーン総数（配列サイズ用）
};

//=============================================================================
// SceneManager
//=============================================================================
class SceneManager
{
public:
    /// @brief シーン配列と内部状態を初期化する。
    SceneManager();

    /// @brief 管理中の全シーンの Finalize と解放を行う。
    ~SceneManager();

    //-------------------------------------------------------------------------
    // ゲームループから呼ぶメソッド
    //-------------------------------------------------------------------------

    /// @brief アクティブシーンの Initialize を呼ぶ。
    void InitilizeActiveScene();

    /// @brief アクティブシーンの Update を呼び、保留中の遷移を処理する。
    /// @param stepTimer  フレーム経過時間タイマー
    void UpdateActiveScene(const DX::StepTimer& stepTimer);

    /// @brief アクティブシーンの Render を呼ぶ。
    void RenderActiveSceneRender();

    /// @brief アクティブシーンの Finalize を呼ぶ。
    void FinalizeActiveScene();

    //-------------------------------------------------------------------------
    // シーン遷移
    //-------------------------------------------------------------------------

    /// @brief 指定シーンへの遷移を予約する（次フレームに適用）。
    /// @param id  遷移先シーン ID
    void SetScene(SceneId id);

    /// @brief 現在シーンを退避しつつ、オーバーレイシーンへ遷移する。
    /// @param id  オーバーレイシーン ID
    void PushScene(SceneId id);

    /// @brief 退避していたシーンへ復帰する。
    /// @return 復帰できた場合 true、退避シーンがなければ false
    bool PopScene();

    //-------------------------------------------------------------------------
    // Accessors
    //-------------------------------------------------------------------------

    /// @brief 管理対象のアクティブシーン参照を直接差し替える（内部用）。
    void ChangeScene(SceneBase* nextScene);

    /// @brief 退避中のシーンが存在するか返す。
    bool HasSuspendedScene() const;

    /// @brief 退避中シーンの ID を返す。
    SceneId GetSuspendedSceneId() const;

    /// @brief 現在のアクティブシーン ID を返す。
    SceneId GetActiveSceneId() const;

    /// @brief 直前にアクティブだったシーン ID を返す。
    SceneId GetPreviousSceneId() const;

private:
    //-------------------------------------------------------------------------
    // 内部処理
    //-------------------------------------------------------------------------

    /// @brief 指定 ID に対応するシーンインスタンスを生成する。
    /// @param id  生成するシーン ID
    /// @return 生成した SceneBase ポインタ
    SceneBase* CreateScene(SceneId id);

    /// @brief 保留中のシーン切り替え要求を適用する。
    void ApplyRequestedScene();

    /// @brief 退避中シーンを解放する。
    void ReleaseSuspendedScene();

private:
    SceneBase* m_scenes[NUM_SCENES]; ///< 全シーンインスタンス配列
    SceneBase* m_activeScene;        ///< 現在アクティブなシーン
    SceneBase* m_requestedScene;     ///< 遷移予約シーン
    SceneBase* m_suspendedScene;     ///< Push で退避したシーン

    SceneId m_activeSceneId;    ///< 現在のアクティブシーン ID
    SceneId m_previousSceneId;  ///< 直前のアクティブシーン ID
    SceneId m_requestedSceneId; ///< 遷移予約シーン ID
    SceneId m_suspendedSceneId; ///< 退避シーン ID
};
