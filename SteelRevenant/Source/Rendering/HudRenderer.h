#pragma once

//=============================================================================
// HudRenderer.h
//
// 【役割】
//   ゲーム中の HUD（Heads-Up Display）描画を一手に担うクラス。
//   以前は GameScene に散在していた以下の描画責務を集約した:
//     - HP バー、コンボゲージ、タイマー表示
//     - ミニマップ（プレイヤー位置・敵マーカー・中継地点マーカー）
//     - 目標バナー（制圧進行・ステージイントロ）
//     - ポーズオーバーレイメニュー
//     - デバッグ情報 (F4 キーで切り替え)
//
// 【設計パターン】
//   - Single Responsibility Principle:
//       GameScene は「何を描くか」を渡すだけ。
//       HudRenderer は「どう描くか」にのみ責任を持つ。
//
// 【使い方】
//   HudRenderer hud;
//   hud.Draw(batch, uiText, context);  // 毎フレーム Render() から呼ぶ
//=============================================================================

#include <string>
#include <vector>
#include <wrl/client.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

#include "../Core/Combat/CombatTypes.h"
#include "../Core/Combat/GameState.h"
#include "../Stage/ArenaObjective.h"

namespace System { class UIShaderText; struct UIShaderStyle; }

namespace Rendering
{
    //=========================================================================
    // HudRenderer
    //=========================================================================
    class HudRenderer
    {
    public:
        //---------------------------------------------------------------------
        // HudContext  ― 1 フレーム分の HUD 描画に必要な全データ
        //
        // GameScene が毎フレーム構築して Draw() へ渡す。
        // HudRenderer 内では読み取りのみ行い、ゲーム状態を変更しない。
        //---------------------------------------------------------------------
        struct HudContext
        {
            // プレイヤー情報
            const Core::PlayerState* player = nullptr;  ///< プレイヤー状態
            float attackBlend = 0.0f;                   ///< 攻撃演出補間率 (0-1)

            // ゲーム進行
            const Core::GameState* gameState = nullptr; ///< ゲーム進行状態
            float hitBloodTimer = 0.0f;                 ///< ヒット血しぶきタイマー (0-1)
            float damageBloodTimer = 0.0f;              ///< 被ダメ血しぶきタイマー (0-1)
            float stageIntroTimer = 0.0f;               ///< ステージイントロタイマー (秒)

            // アリーナ目標
            const Stage::ArenaObjective* objective = nullptr;  ///< アリーナ目標情報

            // 敵情報（ミニマップ用）
            const std::vector<Core::EnemyState>* enemies = nullptr;

            // ポーズ
            bool  isPaused = false;                 ///< ポーズ中フラグ
            int   pauseSelectedIndex = 0;           ///< 現在選択中のメニュー項目
            float pauseClickFxTimer = 0.0f;         ///< クリックエフェクトタイマー (秒)
            DirectX::SimpleMath::Vector2 pauseClickFxPos; ///< クリックエフェクト座標

            // デバッグ
            bool  showDebugDetail = false;          ///< デバッグ詳細を表示するか
            float mouseSensitivity = 0.08f;         ///< 現在のマウス感度
            float attackAssistRange = 2.6f;         ///< 攻撃アシスト距離
            float attackAssistDot = 0.30f;          ///< 攻撃アシスト方向閾値
        };

        //---------------------------------------------------------------------
        // PauseMenuResult  ― ポーズメニュー操作結果
        //---------------------------------------------------------------------
        enum class PauseMenuResult
        {
            None,       ///< 何も起きていない
            Resume,     ///< ゲームへ戻る
            Restart,    ///< ステージをリスタートする
            Title,      ///< タイトルへ戻る
            Quit        ///< アプリ終了
        };

    public:
        /// @brief 描画に必要な共有テクスチャを受け取って構築する。
        /// @param solidTexture  単色矩形描画用テクスチャ (1x1 白ピクセル)
        explicit HudRenderer(
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> solidTexture);

        //---------------------------------------------------------------------
        // Draw  ― メイン描画エントリポイント
        //---------------------------------------------------------------------

        /// @brief HUD 全体を描画する。
        /// @param batch    SpriteBatch（BeginSpriteLayer 済みであること）
        /// @param uiText   UIShaderText ヘルパー
        /// @param context  このフレームの描画コンテキスト
        /// @param width    ビューポート幅 (px)
        /// @param height   ビューポート高さ (px)
        void Draw(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext&     context,
            float width,
            float height) const;

        //---------------------------------------------------------------------
        // Pause Menu  ― ポーズメニュー操作
        //---------------------------------------------------------------------

        /// @brief マウス入力に基づいてポーズメニューの選択状態を更新する。
        /// @param mousePos       現在マウス座標 (スクリーン空間)
        /// @param mouseClicked   左クリックが押されたか
        /// @param[out] outIndex  ホバー中の項目インデックス
        /// @return 確定した操作結果（None なら何も起きていない）
        PauseMenuResult UpdatePauseMenu(
            const DirectX::SimpleMath::Vector2& mousePos,
            bool  mouseClicked,
            int&  outIndex,
            float width,
            float height) const;

    private:
        //---------------------------------------------------------------------
        // 内部描画ヘルパー
        //---------------------------------------------------------------------

        /// @brief HP バーとコンボゲージを描画する。
        void DrawStatusBars(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        /// @brief 残り時間タイマーを描画する。
        void DrawTimer(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        /// @brief 中継地点・危険度・撃破数などの目標情報を描画する。
        void DrawObjectiveInfo(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        /// @brief 目標バナーテキストを画面中央付近に描画する。
        void DrawObjectiveBanner(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        /// @brief ステージイントロ演出を描画する。
        void DrawStageIntro(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        /// @brief ミニマップと各種マーカーを描画する。
        void DrawMiniMap(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        /// @brief 被弾・命中時の画面エフェクト（血しぶき）を描画する。
        void DrawBloodOverlay(
            DirectX::SpriteBatch* batch,
            const HudContext& ctx,
            float width, float height) const;

        /// @brief ポーズオーバーレイ全体を描画する。
        void DrawPauseOverlay(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float width, float height) const;

        /// @brief デバッグ情報パネルを描画する。
        void DrawDebugPanel(
            DirectX::SpriteBatch* batch,
            System::UIShaderText* uiText,
            const HudContext& ctx,
            float uiScale, float width, float height) const;

        /// @brief 単色矩形を描画する。
        void DrawSolidRect(
            DirectX::SpriteBatch* batch,
            const DirectX::SimpleMath::Vector2& pos,
            const DirectX::SimpleMath::Vector2& size,
            const DirectX::SimpleMath::Color&   color) const;

        /// @brief ワールド座標をミニマップ上の 2D 座標へ変換する。
        DirectX::SimpleMath::Vector2 WorldToMiniMap(
            const DirectX::SimpleMath::Vector3& worldPos,
            const DirectX::SimpleMath::Vector2& mapTopLeft,
            float mapSize) const;

        /// @brief 残り時間を MM:SS 形式の文字列に整形する。
        std::wstring FormatTimerMMSS(float seconds) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_solidTexture;
    };

} // namespace Rendering
