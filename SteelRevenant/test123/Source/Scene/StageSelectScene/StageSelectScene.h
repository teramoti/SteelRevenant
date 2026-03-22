//------------------------//------------------------
// Contents(処理内容) ステージ選択画面の更新と描画処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// StageSelectScene
//-----------------------------------------------------------------------------
// 目的:
// - タイトルから直接ゲームへ遷移せず、ステージ選択を挟む
// - 将来の複数ステージ拡張に備えた導線を先に作る
// 操作:
// - W/S or ↑/↓ でステージ選択
// - Enter/Space で開始
// - Esc でタイトルへ戻る
//-----------------------------------------------------------------------------

#include "../Base/ActionSceneBase.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

class StageSelectScene : public ActionSceneBase
{
public:
	// ステージ選択シーンを生成する。
	StageSelectScene(SceneManager* sceneManager);
	// 確保済みリソースを解放する。
	~StageSelectScene();

public:
	// 選択状態とUI描画資産を初期化する。
	void Initialize() override;
	// ステージ選択入力とシーン遷移を更新する。
	void Update(const DX::StepTimer& stepTimer) override;
	// ステージ選択UIを描画する。
	void Render() override;
	// 一時リソースを破棄して終了処理を行う。
	void Finalize() override;

private:
	// SpriteBatch上に単色矩形を描画する。
	void DrawSolidRect(
		DirectX::SpriteBatch* batch,
		const DirectX::SimpleMath::Vector2& position,
		const DirectX::SimpleMath::Vector2& size,
		const DirectX::SimpleMath::Color& color) const;

private:
	// 現在選択中のステージ番号(0始まり)。
	int m_selectedStage;
	// 点滅演出用タイマ。
	float m_blinkTimer;
	// カード描画に使う1x1テクスチャ。
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uiSolidTexture;
	// クリック演出の残り時間。
	float m_clickFxTimer;
	// クリック演出の中心座標。
	DirectX::SimpleMath::Vector2 m_clickFxPos;
};


