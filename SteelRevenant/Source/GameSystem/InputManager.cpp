//------------------------//------------------------
// Contents(処理内容) キーボードとマウス入力の集約処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "InputManager.h"
#include "../Utility/SimpleMathEx.h"

// 入力参照と感度を既定値で初期化する。
System::InputManager::InputManager()
	: m_keyboard(nullptr)
	, m_keyboardTracker(nullptr)
	, m_mouse(nullptr)
	, m_mouseTracker(nullptr)
	, m_mouseDelta(DirectX::SimpleMath::Vector2::Zero)
	, m_mouseSensitivity(0.08f)
{
}

// 動的確保リソースを持たないため何もしない。
System::InputManager::~InputManager()
{
}

// 現在フレームのキーボード状態を返す。
DirectX::Keyboard::State System::InputManager::GetKeyboardState() const
{
	if (m_keyboard == nullptr)
	{
		return DirectX::Keyboard::State();
	}
	return *m_keyboard;
}

// キーボードトラッカー状態を返す。
DirectX::Keyboard::KeyboardStateTracker System::InputManager::GetKeyboardTracker() const
{
	if (m_keyboardTracker == nullptr)
	{
		return DirectX::Keyboard::KeyboardStateTracker();
	}
	return *m_keyboardTracker;
}

// 現在フレームのマウス状態を返す。
DirectX::Mouse::State System::InputManager::GetMouseState() const
{
	if (m_mouse == nullptr)
	{
		DirectX::Mouse::State state = {};
		state.positionMode = DirectX::Mouse::MODE_RELATIVE;
		return state;
	}
	return *m_mouse;
}

// マウストラッカー状態を返す。
DirectX::Mouse::ButtonStateTracker System::InputManager::GetMouseTracker() const
{
	if (m_mouseTracker == nullptr)
	{
		return DirectX::Mouse::ButtonStateTracker();
	}
	return *m_mouseTracker;
}

// 感度適用済みのマウス移動量を返す。
DirectX::SimpleMath::Vector2 System::InputManager::GetMouseDelta() const
{
	return m_mouseDelta;
}

// マウス感度を安全な範囲で設定する。
void System::InputManager::SetMouseSensitivity(float sensitivity)
{
	// 0以下は無効。極端に大きい値も防ぐ。
	m_mouseSensitivity = Utility::MathEx::Clamp(sensitivity, 0.001f, 1.0f);
}

// マウス感度を返す。
float System::InputManager::GetMouseSensitivity() const
{
	return m_mouseSensitivity;
}

// フレーム入力参照を差し替え、差分量を更新する。
void System::InputManager::Update(
	DirectX::Keyboard::State* keyboard,
	DirectX::Keyboard::KeyboardStateTracker* keyboardTracker,
	DirectX::Mouse::State* mouse,
	DirectX::Mouse::ButtonStateTracker* mouseTracker)
{
	m_keyboard = keyboard;
	m_keyboardTracker = keyboardTracker;
	m_mouse = mouse;
	m_mouseTracker = mouseTracker;

	if (m_mouse != nullptr)
	{
		// 相対モードでは x/y がフレーム差分。
		m_mouseDelta.x = static_cast<float>(m_mouse->x) * m_mouseSensitivity;
		m_mouseDelta.y = static_cast<float>(m_mouse->y) * m_mouseSensitivity;
	}
	else
	{
		m_mouseDelta = DirectX::SimpleMath::Vector2::Zero;
	}
}

