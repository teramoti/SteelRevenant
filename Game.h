//------------------------//------------------------
// Contents(処理内容) ゲーム本体クラスと共有リソース保持メンバを宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#ifndef GAME_DEFINED
#define GAME_DEFINED

// Windows ヘッダによる min/max マクロ定義を無効化する。
#ifndef NOMINMAX
#define NOMINMAX
#endif

// 依存ヘッダ
#include <windows.h>
#include <iostream>
#include <string>
#include <iomanip>

#include <memory>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <Model.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <GeometricPrimitive.h>
#include <WICTextureLoader.h>
#include <algorithm>
#include <CommonStates.h>

#include "StepTimer.h"
#include "Source\\Utility\\dx.h"
#include "Window.h"
#include "Source\\Utility\\DirectX11.h"
#include "Source/Scene/SceneManager/SceneManager.h"

class Window;

// アプリ全体の初期化、更新、描画を管理する。
class Game
{
public:
	// 実行時の画面サイズを受け取り、初期状態を構築する。
	Game(int width, int height);
	// 実行に必要な共通リソースを初期化する。
	void Initialize();
	// メッセージループとゲームループを実行する。
	MSG Tick();
	// 保持中のリソースを解放する。
	void Finalize();

	// アプリ状態通知
	void OnActivated();
	// アプリが非アクティブ化された際の処理を行う。
	void OnDeactivated();
	// アプリの一時停止要求時に必要な処理を行う。
	void OnSuspending();
	// 一時停止からの復帰時に入力とタイマ状態を立て直す。
	void OnResuming();
	// ウィンドウサイズ変更に合わせて描画側の状態を更新する。
	void OnWindowSizeChanged(int width, int height);

	// ウィンドウ設定
	void GetDefaultSize(int& width, int& height) const;

private:
	// 1 フレーム分のゲーム状態を更新する。
	void Update(const DX::StepTimer& timer);
	// 現在フレームを描画する。
	void Render();
	// 描画前にバックバッファをクリアする。
	void Clear();
	// バックバッファを画面へ表示する。
	void Present();
	// FPS 表示を描画する。
	void DrawFPS();

private:
	// インスタンスハンドル
	HINSTANCE m_hInstance;
	// 実行時のウィンドウの大きさ
	int m_nCmdShow;
	// ウィンドウハンドル
	HWND m_hWnd;
	// 出力幅
	int m_width;
	// 出力高
	int m_height;

	// ウィンドウ管理
	std::unique_ptr<Window> m_window;
	// Direct3D リソース管理
	std::unique_ptr<DirectX11> m_graphics;
	// フレーム時間管理
	DX::StepTimer m_timer;

	// キーボード 
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	// キーボードトラッカー
	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	// マウス
	std::unique_ptr<DirectX::Mouse> m_mouse;
	// マウストラッカー
	DirectX::Mouse::ButtonStateTracker m_mouseTracker;

	// 2D 描画バッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	// フォント
	std::unique_ptr<DirectX::SpriteFont> m_font;

	// コモンステート
	std::unique_ptr<DirectX::CommonStates> m_commonStates;

	// DirectX11 シングルトン参照
	DirectX11& m_directX = DirectX11::Get();
	// シーン管理
	std::unique_ptr<SceneManager> m_sceneManager;

};

#endif	// GAME_DEFINED


