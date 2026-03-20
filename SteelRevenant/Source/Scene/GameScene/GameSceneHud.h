#pragma once
#include <d3d11.h>
#include <memory>
#include <string>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <SimpleMath.h>
#include "../../Action/CombatSystem.h"
#include "../../Action/GameState.h"
#include "../../Action/SurvivalDirector.h"

// HUD (時間・Kill数・Wave・コンボゲージ・スクリーンエフェクト) を描画するクラス。
class GameSceneHud
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
        DirectX::SpriteBatch* spriteBatch, DirectX::SpriteFont* font);

    void Update(float dt);

    void Render(
        const Action::GameState&         gameState,
        const Action::PlayerState&       player,
        const Action::SurvivalDirector&  director,
        bool                             speedActive,
        float                            speedRemaining,
        int                              screenW,
        int                              screenH);

    // 追加機能: ウェーブ開始バナーを表示する。
    void TriggerWaveBanner(int waveNumber, int totalWaves);

    // 追加機能: スクリーンシェイクを発生させる。
    void TriggerScreenShake(float intensity);

    // シェイクオフセットを返す (カメラ計算用)。
    DirectX::SimpleMath::Vector2 GetShakeOffset() const { return m_shakeOffset; }

    // 追加機能: コンボポップアップを発生させる。
    void TriggerComboPopup(int comboIndex, int comboLevel);

private:
    void DrawBar(float x, float y, float w, float h,
        float ratio, DirectX::SimpleMath::Color fill, DirectX::SimpleMath::Color back);
    void DrawText(const std::wstring& text,
        float x, float y, DirectX::SimpleMath::Color color, float scale = 1.0f);

    DirectX::SpriteBatch* m_spriteBatch = nullptr;
    DirectX::SpriteFont*  m_font        = nullptr;

    // スクリーンシェイク
    float m_shakeTimer     = 0.0f;
    float m_shakeIntensity = 0.0f;
    DirectX::SimpleMath::Vector2 m_shakeOffset;

    // ウェーブバナー
    float m_bannerTimer  = 0.0f;
    int   m_bannerWave   = 0;
    int   m_bannerTotal  = 0;

    // コンボポップアップ
    float m_comboPopTimer = 0.0f;
    int   m_comboPopIndex = 0;
    int   m_comboPopLevel = 0;

    // スプライト用テクスチャ (白1x1)
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_whiteTex;
};
