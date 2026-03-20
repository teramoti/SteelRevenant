//------------------------//------------------------
// Contents(処理内容) 全シーン共通の基底処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#include "../../Utility/DirectX11.h"
#include "../../../StepTimer.h"
// クラスの宣言 ===============================================================
class SceneManager;
// クラスの定義 ===============================================================
// シーンの基底クラス。
class SceneBase
{
	// データメンバの宣言 -------------------------------------------------
	protected:
		SceneManager* m_sceneManager;
		bool m_SceneFlag;
	// メンバ関数の宣言 ---------------------------------------------------
	// <コンストラクタ>
	public:
		// SceneBase の初期状態を構築する。
		SceneBase(SceneManager* sceneManager,bool SceneFlag);
	// <デストラクタ>
	public:
		// SceneBase が保持するリソースを解放する。
		virtual ~SceneBase();
	// <操作>
	public:
		// <初期化関数>
		virtual void Initialize() = 0;
		// <更新関数>
		virtual void Update(const DX::StepTimer& stepTimer) = 0;
		// <描画関数>
		virtual void Render() = 0;
		// <終了関数>
		virtual void Finalize() = 0;
};
