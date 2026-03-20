//------------------------//------------------------
// Contents(処理内容) キーボードとマウス入力の集約処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
//-----------------------------------------------------------------------------
// InputManager
//----------------------------------------------------------------------------- 
// 役割:
// - Keyboard / Mouse の状態をシーン側へ統一提供する
// - 感度適用済みのマウス移動量を保持する
// 注意:
// - 本クラスは各フレームで Game::Update からポインタ更新される設計
//   （即時利用前提。フレームを跨ぐ保持には使わない）
//-----------------------------------------------------------------------------
#include <d3d11.h>
#include <SimpleMath.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <algorithm>
#include "../Utility/SingletonBase.h"
namespace System
{
	class InputManager : public Utility::SingletonBase<InputManager>
	{
	public:
		friend Utility::SingletonBase<InputManager>;
	private:
		// 入力参照と感度初期値を初期化する。
		InputManager();
		// 入力マネージャが保持する参照を破棄する。
		~InputManager();
	public:
		// キーボード状態取得
		DirectX::Keyboard::State GetKeyboardState() const;
		// キーボードトラッカー取得
		DirectX::Keyboard::KeyboardStateTracker GetKeyboardTracker() const;
		// マウス状態取得
		DirectX::Mouse::State GetMouseState() const;
		// マウストラッカー取得
		DirectX::Mouse::ButtonStateTracker GetMouseTracker() const;
		// 感度適用済みマウス移動量取得
		DirectX::SimpleMath::Vector2 GetMouseDelta() const;
		// マウス感度の設定
		void SetMouseSensitivity(float sensitivity);
		// マウス感度の取得
		float GetMouseSensitivity() const;
		// 更新
		void Update(
			DirectX::Keyboard::State* keyboard,
			DirectX::Keyboard::KeyboardStateTracker* keyboardTracker,
			DirectX::Mouse::State* mouse,
			DirectX::Mouse::ButtonStateTracker* mouseTracker);
	private:
		// 現在フレームの入力参照
		DirectX::Keyboard::State* m_keyboard;
		DirectX::Keyboard::KeyboardStateTracker* m_keyboardTracker;
		DirectX::Mouse::State* m_mouse;
		DirectX::Mouse::ButtonStateTracker* m_mouseTracker;
		// 相対マウス移動（感度適用後）
		DirectX::SimpleMath::Vector2 m_mouseDelta;
		// 視点操作に使う感度
		float m_mouseSensitivity;
	};
}
