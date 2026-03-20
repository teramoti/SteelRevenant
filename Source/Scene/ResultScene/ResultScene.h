//------------------------//------------------------
// Contents(処理内容) リザルト画面の更新と描画処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// ResultScene
//-----------------------------------------------------------------------------
// 新アクションステージの結果表示シーン。
// - 戦績サマリー表示
// - キーボード/マウスの両操作で遷移
//-----------------------------------------------------------------------------

#include "../Base/ActionSceneBase.h"

#include <memory>
#include <wrl/client.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

class ResultScene : public ActionSceneBase
{
public:
	// ResultScene を生成する。
	ResultScene(SceneManager* scenemaneger);
	// 終了時に保持リソースを解放する。
	~ResultScene();

public:
	// 結果表示に必要な状態と資産を初期化する。
	void Initialize() override;
	// 入力を処理し、遷移先を更新する。
	void Update(const DX::StepTimer& stepTimer) override;
	// 結果UIを描画する。
	void Render() override;
	// 確保済みリソースを破棄する。
	void Finalize() override;

private:
	// SpriteBatch上に単色矩形を描画する。
	void DrawSolidRect(
		DirectX::SpriteBatch* batch,
		const DirectX::SimpleMath::Vector2& position,
		const DirectX::SimpleMath::Vector2& size,
		const DirectX::SimpleMath::Color& color) const;

private:
	float m_elapsed;
	// 結果画面メニューの選択インデックス。
	int m_selectedItem;
	// クリック演出の残り時間。
	float m_clickFxTimer;
	// クリック演出の中心座標。
	DirectX::SimpleMath::Vector2 m_clickFxPos;
	// UI矩形描画用1x1白テクスチャ。
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uiSolidTexture;
};

