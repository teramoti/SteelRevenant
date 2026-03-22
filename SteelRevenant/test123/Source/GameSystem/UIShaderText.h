//------------------------//------------------------
// Contents(処理内容) 装飾付きUI文字描画処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// UIShaderText
//-----------------------------------------------------------------------------
// SDF文字をベースに「シェーダー風」装飾を与える描画ヘルパー。
// 実際には SpriteFont 描画を重ねて、縁取り・点滅・色遷移を実現する。
//-----------------------------------------------------------------------------

#include <memory>
#include <string>

#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

namespace System
{
	struct UIShaderStyle
	{
		DirectX::SimpleMath::Color baseColor;
		DirectX::SimpleMath::Color outlineColor;
		float pulseSpeed;
		float pulseAmount;
		bool blink;
		float blinkPeriod;

		// UI シェーダーテキスト用の既定スタイルを初期化する。
		UIShaderStyle()
			: baseColor(DirectX::Colors::White)
			, outlineColor(DirectX::Colors::Black)
			, pulseSpeed(2.0f)
			, pulseAmount(0.2f)
			, blink(false)
			, blinkPeriod(0.5f)
		{
		}
	};

	class UIShaderText
	{
	public:
		// テキスト描画ヘルパーを初期化する。
		UIShaderText();
		// 保持フォントを破棄する。
		~UIShaderText();

		// SpriteFontをロードして描画準備を行う。
		bool Initialize(const wchar_t* fontPath);
		// 時間依存エフェクト用の内部タイマを更新する。
		void Update(float dt);

		// 縁取り/点滅/パルス付きで文字列を描画する。
		void Draw(
			DirectX::SpriteBatch* spriteBatch,
			const std::wstring& text,
			const DirectX::SimpleMath::Vector2& position,
			const UIShaderStyle& style,
			float scale = 1.0f) const;

		// 描画時と同じフォントで文字列サイズを取得する。
		DirectX::SimpleMath::Vector2 Measure(const std::wstring& text, float scale = 1.0f) const;

	private:
		std::unique_ptr<DirectX::SpriteFont> m_font;
		float m_time;
	};
}


