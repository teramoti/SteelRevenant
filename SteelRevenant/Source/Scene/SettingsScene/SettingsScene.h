//------------------------//------------------------
// Contents(処理内容) 設定画面の更新と前画面復帰処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// SettingsScene
//-----------------------------------------------------------------------------
// 役割:
// - タイトルから呼び出す設定画面
// - マウス感度と音量の調整
// - キーボードとマウスクリックの両操作に対応
//-----------------------------------------------------------------------------

#include "../Base/ActionSceneBase.h"
#include "../SceneManager/SceneManager.h"
#include "../../Utility/Sound/AudioTypes.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

class SettingsScene : public ActionSceneBase
{
public:
	// 設定シーンを生成する。
	SettingsScene(SceneManager* sceneManager);
	// 保持リソースを解放する。
	~SettingsScene();

public:
	// 設定画面の状態を初期化する。
	void Initialize() override;
	// 入力処理と設定値更新を行う。
	void Update(const DX::StepTimer& stepTimer) override;
	// 設定UIを描画する。
	void Render() override;
	// 一時リソースを解放する。
	void Finalize() override;

private:
	// 単色矩形を描画する。
	void DrawSolidRect(
		DirectX::SpriteBatch* batch,
		const DirectX::SimpleMath::Vector2& position,
		const DirectX::SimpleMath::Vector2& size,
		const DirectX::SimpleMath::Color& color) const;

	// マウス感度を適用範囲内で更新する。
	void ApplyMouseSensitivity(float value);
	// 指定インデックスの設定値を返す。
	float GetSettingValue(int index) const;
	// 指定インデックスの設定値を反映する。
	void SetSettingValue(int index, float value);
	// 指定バスの音量を適用範囲内で更新する。
	void ApplyAudioBusVolume(GameAudio::AudioBus bus, float value);
	// 前画面または一時停止中のゲームへ戻る。
	void ReturnToBackScene(bool returnToSuspendedGame, SceneId backScene);

private:
	int m_selectedItem;
	float m_sceneTime;
	float m_mouseSensitivity;
	GameAudio::AudioVolumeSettings m_audioVolume;
	// UI描画で使う単色テクスチャ。
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uiSolidTexture;
	// クリック演出の残り時間。
	float m_clickFxTimer;
	// クリック演出の中心座標。
	DirectX::SimpleMath::Vector2 m_clickFxPos;
};

