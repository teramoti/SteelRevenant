//------------------------//------------------------
// Contents(処理内容) 装飾付きUI文字描画処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "UIShaderText.h"

#include "../Utility/DirectX11.h"

#include <cmath>
#include <exception>
#include <cstdio>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

namespace
{
	// 0.0 から 1.0 の範囲へ収める。
	float Clamp01(float v)
	{
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}

	inline void DebugPrint(const char* /*msg*/)
	{
		// No-op in release to avoid spamming debugger output and impacting performance.
	}
}

namespace System
{
	// インスタンス生成時に内部時間をゼロへ初期化する。
	UIShaderText::UIShaderText()
		: m_time(0.0f)
	{
	}

	// 保持フォントを解放する。
	UIShaderText::~UIShaderText()
	{
		m_font.reset();
	}

	// フォントをロードし、欠損グリフ時の代替文字を設定する。
	bool UIShaderText::Initialize(const wchar_t* fontPath)
	{
		if (!fontPath) return false;
		char buf[512];
		// Log attempt
		sprintf_s(buf, "UIShaderText::Initialize - trying font '%ls'\n", fontPath);
		DebugPrint(buf);
		try
		{
			m_font = std::make_unique<DirectX::SpriteFont>(DirectX11::Get().GetDevice().Get(), fontPath);
			// フォントに存在しない文字を描画しようとしても落ちないようにする。
			// （日本語/英語が混在するHUDでも安全に運用できる）
			try
			{
				m_font->SetDefaultCharacter(L'?');
			}
			catch (const std::exception& e)
			{
				sprintf_s(buf, "UIShaderText::Initialize - SetDefaultCharacter failed: %s\n", e.what());
				DebugPrint(buf);
			}
			catch (...)
			{
				DebugPrint("UIShaderText::Initialize - SetDefaultCharacter failed (unknown)\n");
			}
			return true;
		}
		catch (const std::exception& e)
		{
			sprintf_s(buf, "UIShaderText::Initialize - exception loading '%ls': %s\n", fontPath, e.what());
			DebugPrint(buf);
			m_font.reset();
			return false;
		}
		catch (...)
		{
			sprintf_s(buf, "UIShaderText::Initialize - unknown exception loading '%ls'\n", fontPath);
			DebugPrint(buf);
			m_font.reset();
			return false;
		}
	}

	// 点滅・パルス演出用の経過時間を進める。
	void UIShaderText::Update(float dt)
	{
		m_time += dt;
	}

	// 疑似シェーダー風の装飾付きテキスト描画を行う。
	void UIShaderText::Draw(
		DirectX::SpriteBatch* spriteBatch,
		const std::wstring& text,
		const Vector2& position,
		const UIShaderStyle& style,
		float scale) const
	{
		if (spriteBatch == nullptr || m_font == nullptr)
		{
			return;
		}

		if (style.blink && style.blinkPeriod > 0.01f)
		{
			const float blinkPhase = std::fmod(m_time, style.blinkPeriod) / style.blinkPeriod;
			if (blinkPhase > 0.5f)
			{
				return;
			}
		}

		// パルス色補間
		const float pulse = (std::sinf(m_time * style.pulseSpeed) * 0.5f + 0.5f) * style.pulseAmount;
		Color mainColor = style.baseColor;
		mainColor.x = Clamp01(mainColor.x + pulse);
		mainColor.y = Clamp01(mainColor.y + pulse);
		mainColor.z = Clamp01(mainColor.z + pulse);

		const Vector2 scaleVec(scale, scale);

		try
		{
			// 4方向アウトライン（疑似シェーダー風エッジ）
			m_font->DrawString(spriteBatch, text.c_str(), position + Vector2(-1.0f, 0.0f), style.outlineColor, 0.0f, Vector2::Zero, scaleVec);
			m_font->DrawString(spriteBatch, text.c_str(), position + Vector2(1.0f, 0.0f), style.outlineColor, 0.0f, Vector2::Zero, scaleVec);
			m_font->DrawString(spriteBatch, text.c_str(), position + Vector2(0.0f, -1.0f), style.outlineColor, 0.0f, Vector2::Zero, scaleVec);
			m_font->DrawString(spriteBatch, text.c_str(), position + Vector2(0.0f, 1.0f), style.outlineColor, 0.0f, Vector2::Zero, scaleVec);

			// メイン文字
			m_font->DrawString(spriteBatch, text.c_str(), position, mainColor, 0.0f, Vector2::Zero, scaleVec);
		}
		catch (const std::exception&)
		{
			// Begin/End不整合やグリフ不整合があっても、ゲーム進行を優先して描画をスキップする。
		}
		catch (...)
		{
			// 予期しない例外も同様に握りつぶし、クラッシュを防止する。
		}
	}

	// 描画時と同一条件で文字列の表示サイズを返す。
	Vector2 UIShaderText::Measure(const std::wstring& text, float scale) const
	{
		if (m_font == nullptr)
		{
			return Vector2::Zero;
		}

		try
		{
			const Vector2 measured = m_font->MeasureString(text.c_str());
			return measured * scale;
		}
		catch (...)
		{
			return Vector2::Zero;
		}
	}
}




