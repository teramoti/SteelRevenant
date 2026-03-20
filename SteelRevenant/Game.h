#ifndef GAME_DEFINED
#define GAME_DEFINED

//=============================================================================
// Game.h
//
// 【役割】
//   アプリケーション本体クラス。
//   ウィンドウ生成・DirectX 初期化・メインループ・シーン管理を統括する。
//=============================================================================

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <memory>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <GeometricPrimitive.h>
#include <CommonStates.h>

#include "StepTimer.h"
#include "Window.h"

// 旧パスのユーティリティ（実装ファイルが旧パスにあるため維持）
#include "Source/Utility/dx.h"
#include "Source/Utility/DirectX11.h"

// シーン管理（旧パス）
#include "Source/Scene/SceneManager/SceneManager.h"

class Window;

//=============================================================================
// Game  ― アプリケーション本体
//=============================================================================
class Game
{
public:
    /// @brief 画面サイズを受け取って初期状態を構築する。
    Game(int width, int height);

    /// @brief 実行に必要な共通リソースを初期化する。
    void Initialize();

    /// @brief メッセージループとゲームループを実行する。
    MSG Tick();

    /// @brief 保持中のリソースを解放する。
    void Finalize();

    // アプリ状態通知
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);
    void GetDefaultSize(int& width, int& height) const;

private:
    /// @brief 1 フレーム分のゲーム状態を更新する。
    void Update(const DX::StepTimer& timer);

    /// @brief 現在フレームを描画する。
    void Render();

    /// @brief 描画前にバックバッファをクリアする。
    void Clear();

    /// @brief バックバッファを画面へ表示する。
    void Present();

private:
    HINSTANCE  m_hInstance;
    int        m_nCmdShow;
    HWND       m_hWnd;
    int        m_width;
    int        m_height;

    std::unique_ptr<Window>                        m_window;
    std::unique_ptr<DirectX::SpriteBatch>          m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont>           m_font;
    std::unique_ptr<DirectX::CommonStates>         m_commonStates;
    std::unique_ptr<DirectX::Keyboard>             m_keyboard;
    std::unique_ptr<DirectX::Mouse>                m_mouse;

    DirectX::Keyboard::KeyboardStateTracker        m_keyboardTracker;
    DirectX::Mouse::ButtonStateTracker             m_mouseTracker;

    DX::StepTimer                                  m_timer;
    DirectX11&                                     m_directX = DirectX11::Get();
    std::unique_ptr<SceneManager>                  m_sceneManager;

    // m_graphics は DirectX11 シングルトンに統合済みのため削除
};

#endif // GAME_DEFINED
