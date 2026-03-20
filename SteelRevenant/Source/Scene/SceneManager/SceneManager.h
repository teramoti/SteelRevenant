//------------------------//------------------------
// Contents(処理内容) シーン生成、切替、復帰処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include "../../Utility/DirectX11.h"
#include "../../../StepTimer.h"

class SceneBase;

enum SceneId
{
	LOGO_SCENE,
	TITLE_SCENE,
	SELECT_SCENE,
	SETTINGS_SCENE,
	GAME_SCENE,
	RESULT_SCENE,

	NUM_SCENES
};

class SceneManager
{
private:
	SceneBase* m_scenes[NUM_SCENES];
	SceneBase* m_activeScene;
	SceneBase* m_requestedScene;
	SceneBase* m_suspendedScene;
	SceneId m_activeSceneId;
	SceneId m_previousSceneId;
	SceneId m_requestedSceneId;
	SceneId m_suspendedSceneId;

	// 指定されたシーン ID に対応するシーンインスタンスを生成する。
	SceneBase* CreateScene(SceneId sceneId);
	// 保留中のシーン切り替え要求を現在状態へ反映する。
	void ApplyRequestedScene();
	// 退避中のシーンを解放する。
	void ReleaseSuspendedScene();

public:
	// シーン管理に必要な参照と状態を初期化する。
	SceneManager();
	// 管理中シーンの終了処理と解放を行う。
	~SceneManager();

	// 現在のアクティブシーンを初期化する。
	void InitilizeActiveScene();
	// 現在のアクティブシーンを更新する。
	void UpdateActiveScene(const DX::StepTimer& stepTimer);
	// 現在のアクティブシーンを描画する。
	void RenderActiveSceneRender();
	// 指定シーンへの遷移要求を設定する。
	void SetScene(SceneId startSceneId);
	// 現在のシーンを退避しつつオーバーレイシーンへの遷移要求を設定する。
	void PushScene(SceneId overlaySceneId);
	// 退避していたシーンへ復帰し、復帰できたかを返す。
	bool PopScene();
	// 現在のアクティブシーンを終了処理する。
	void FinalizeActiveScene();
	// 管理対象のアクティブシーン参照を差し替える。
	void ChangeScene(SceneBase* nextScene);
	// 退避中のシーンが存在するかを返す。
	bool HasSuspendedScene() const;
	// 退避中シーンの ID を返す。
	SceneId GetSuspendedSceneId() const;
	// 現在のアクティブシーン ID を返す。
	SceneId GetActiveSceneId() const;
	// 直前に表示していたシーン ID を返す。
	SceneId GetPreviousSceneId() const;
};

