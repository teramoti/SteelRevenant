#ifndef GAME_DEFINED
#define GAME_DEFINED

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
#include "Source/Utility/dx.h"
#include "Source/Utility/DirectX11.h"
#include "Source/Scene/SceneManager/SceneManager.h"

class Window;

class Game
{
public:
    Game(int width, int height);

    void Initialize();
    MSG Tick();
    void Finalize();

    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);
    void GetDefaultSize(int& width, int& height) const;

private:
    void Update(const DX::StepTimer& timer);
    void Render();
    void Clear();
    void Present();
    // FPSを描画する 
    void DrawFPS();

private:
    HINSTANCE  m_hInstance;
    int        m_nCmdShow;
    HWND       m_hWnd;
    int        m_width;
    int        m_height;

    std::unique_ptr<Window>                 m_window;
    std::unique_ptr<DirectX::SpriteBatch>   m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont>    m_font;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;
    DirectX::Mouse::ButtonStateTracker      m_mouseTracker;

    DX::StepTimer                           m_timer;
    DirectX11&                              m_directX = DirectX11::Get();
    std::unique_ptr<SceneManager>           m_sceneManager;
};

#endif // GAME_DEFINED

