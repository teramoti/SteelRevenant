//------------------------//------------------------
// Contents(処理内容) 3Dアクション系シーン共通基底処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "ActionSceneBase.h"

#include "../../GameSystem/DrawManager.h"

#include <algorithm>
#include <windows.h>

// Action系シーンの共通基底を初期化する。
ActionSceneBase::ActionSceneBase(SceneManager* sceneManager, bool sceneFlag)
	: SceneBase(sceneManager, sceneFlag)
	, m_directX(DirectX11::Get())
	, m_textRenderer(nullptr)
	, m_textRendererInitTried(false)
{
}

// 共通で保持しているテキスト描画資産を破棄する。
ActionSceneBase::~ActionSceneBase()
{
	m_textRenderer.reset();
}

// 画面アスペクト比を反映した透視行列を返す。
DirectX::SimpleMath::Matrix ActionSceneBase::BuildPerspective(float fovDeg, float nearZ, float farZ) const
{
	const int width = std::max(1, m_directX.GetWidth());
	const int height = std::max(1, m_directX.GetHeight());
	const float aspect = static_cast<float>(width) / static_cast<float>(height);
	return DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(fovDeg), aspect, nearZ, farZ);
}

// SpriteBatch描画を開始する。
void ActionSceneBase::BeginSpriteLayer() const
{
	System::DrawManager::GetInstance().Begin();
}

// SpriteBatch描画を終了する。
void ActionSceneBase::EndSpriteLayer() const
{
	System::DrawManager::GetInstance().End();
}

// UIテキスト描画ヘルパーを必要時に生成して返す。
System::UIShaderText* ActionSceneBase::EnsureTextRenderer()
{
	if (m_textRenderer != nullptr)
	{
		return m_textRenderer.get();
	}

	// If we've already tried and failed, do not retry every frame.
	if (m_textRendererInitTried)
	{
		return nullptr;
	}

	m_textRenderer = std::make_unique<System::UIShaderText>();
	// 日本語対応フォントを優先し、失敗時は段階フォールバックでUI崩壊を防ぐ。
	static const wchar_t* kFontCandidates[] =
	{
		// 実行位置差異に強くするため、近い場所から順に探索する。
		L"JapaneseUI_18.spritefont",
		L"../JapaneseUI_18.spritefont",
		L"../../JapaneseUI_18.spritefont",
		L"../../../JapaneseUI_18.spritefont",
		L"Source/JapaneseUI_18.spritefont",
		L"../Source/JapaneseUI_18.spritefont",
		L"../../Source/JapaneseUI_18.spritefont",
		L"JapaneseTest.spritefont",
		L"../JapaneseTest.spritefont",
		L"../../JapaneseTest.spritefont",
		L"../../../JapaneseTest.spritefont",
		L"Source/JapaneseTest.spritefont",
		L"../Source/JapaneseTest.spritefont",
		L"../../Source/JapaneseTest.spritefont",
		L"SegoeUI_18.spritefont",
		L"../SegoeUI_18.spritefont",
		L"../../SegoeUI_18.spritefont",
		L"../../../SegoeUI_18.spritefont",
		L"Source/SegoeUI_18.spritefont",
		L"../Source/SegoeUI_18.spritefont",
		L"../../Source/SegoeUI_18.spritefont",
	};

	bool initialized = false;
	for (const wchar_t* candidate : kFontCandidates)
	{
		if (candidate != nullptr && m_textRenderer->Initialize(candidate))
		{
			initialized = true;
			break;
		}
	}

	// すべて失敗した場合は空ポインタ化し、呼び出し側で安全に描画スキップする。
	if (!initialized)
	{
		m_textRenderer.reset();
		m_textRendererInitTried = true; // mark attempted to avoid retrying each frame
		return nullptr;
	}

	return m_textRenderer.get();
}

// OSカーソル表示状態を安定化させる。
void ActionSceneBase::SetSystemCursorVisible(bool visible) const
{
	if (visible)
	{
		while (::ShowCursor(TRUE) < 0)
		{
		}
		::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
	}
	else
	{
		while (::ShowCursor(FALSE) >= 0)
		{
		}
	}
}



