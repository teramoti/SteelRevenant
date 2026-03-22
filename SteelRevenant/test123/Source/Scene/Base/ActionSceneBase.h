//------------------------//------------------------
// Contents(処理内容) 3Dアクション系シーン共通基底処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------

#pragma once

//-----------------------------------------------------------------------------
// ActionSceneBase (Inheritance + Template Method helper)
//-----------------------------------------------------------------------------
// Action系シーンで共通利用する機能を基底へ集約。
// - DirectX11参照
// - SpriteBatch開始/終了
// - UIShaderTextの遅延生成
// - 透視行列生成
//-----------------------------------------------------------------------------

#include "SceneBase.h"

#include <memory>
#include <d3d11.h>
#include <SimpleMath.h>

#include "../../GameSystem/UIShaderText.h"

class ActionSceneBase : public SceneBase
{
public:
	// Action系シーン共通初期化を行う。
	ActionSceneBase(SceneManager* sceneManager, bool sceneFlag);
	// 共通UIリソースを解放する。
	virtual ~ActionSceneBase();

protected:
	// 現在解像度に合わせた透視行列を生成する。
	DirectX::SimpleMath::Matrix BuildPerspective(float fovDeg, float nearZ, float farZ) const;
	// SpriteBatch描画を開始する。
	void BeginSpriteLayer() const;
	// SpriteBatch描画を終了する。
	void EndSpriteLayer() const;
	// UIテキスト描画ヘルパーを遅延生成して返す。
	System::UIShaderText* EnsureTextRenderer();
	// OSカーソル表示状態を明示制御する。
	void SetSystemCursorVisible(bool visible) const;

protected:
	DirectX11& m_directX;
	std::unique_ptr<System::UIShaderText> m_textRenderer;
	// Prevent repeated font initialization attempts when resources are missing.
	bool m_textRendererInitTried = false;
};



