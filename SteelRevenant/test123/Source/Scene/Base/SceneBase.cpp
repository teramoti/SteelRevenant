//------------------------//------------------------
// Contents(処理内容) 全シーン共通の基底処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "SceneBase.h"

#include "../SceneManager/SceneManager.h"

// メンバ関数の定義 ===========================================================
// コンストラクタ。
SceneBase::SceneBase(SceneManager* sceneManager,bool SceneFlag)
	: m_sceneManager(sceneManager)
	, m_SceneFlag(SceneFlag)
{

}

// デストラクタ。
SceneBase::~SceneBase()
{
	
}


