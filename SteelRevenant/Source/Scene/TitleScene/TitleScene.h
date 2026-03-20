//------------------------//------------------------
// Contents(処理内容) タイトル画面の更新と描画処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// TitleScene
//-----------------------------------------------------------------------------
// 剣アクション刷新版のタイトルシーン。
// - 遷移フロー: Title -> (StageSelect / Settings)
// - ActionSceneBaseの共通機能を利用
// - 上下メニューで直感的に選択可能
//-----------------------------------------------------------------------------

#include "../Base/ActionSceneBase.h"
#include "../SceneManager/SceneManager.h"

#include <memory>
#include <vector>
#include <wrl/client.h>
#include <d3d11.h>

#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

class TitleScene : public ActionSceneBase
{
public:
	// シーンマネージャ参照を受け取り、タイトル状態を初期化する。
	TitleScene(SceneManager* sceneManager);
	// タイトルシーンで確保したリソースを解放する。
	~TitleScene();

public:
	// タイトル表示に必要なメッシュとUI状態を初期化する。
	void Initialize() override;
	// 入力処理とシーン遷移を更新する。
	void Update(const DX::StepTimer& stepTimer) override;
	// 3D背景と2DメニューUIを描画する。
	void Render() override;
	// 一時リソースを開放して安全に終了状態へ戻す。
	void Finalize() override;

private:
	// 軌道カメラの行列を更新する。
	void UpdateCamera();
	// タイトル用3Dオブジェクトを描画する。
	void DrawWorld();
	// タイトルUIとヘルプ文を描画する。
	void DrawUI();
	// SpriteBatch上に単色矩形を描画する。
	void DrawSolidRect(
		DirectX::SpriteBatch* batch,
		const DirectX::SimpleMath::Vector2& position,
		const DirectX::SimpleMath::Vector2& size,
		const DirectX::SimpleMath::Color& color) const;

private:
	// 3Dタイトル演出で使用するメッシュ群。
	std::unique_ptr<DirectX::GeometricPrimitive> m_floorMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_skyMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_bladeMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_pillarMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_coreMesh;
	std::vector<DirectX::SimpleMath::Vector3> m_pillarPositions;

	// カメラ状態。
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;

	// UIのカード/ライン描画に使う1x1テクスチャ。
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uiSolidTexture;

	// アニメーションと遷移制御の実行時状態。
	float m_sceneTime;
	float m_transitionTimer;
	float m_hintTimer;
	float m_idleTimer;
	int m_selectedMenu;
	SceneId m_nextScene;
	bool m_inTransition;
};


