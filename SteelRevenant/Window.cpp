//------------------------//------------------------
// Contents(処理内容) Win32 ウィンドウの生成と管理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "Window.h"
#include "Game.h"

// hInstance と nCmdShow を保持してウィンドウクラスを登録する。
Window::Window(HINSTANCE hInstance, int nCmdShow)
    : m_hInstance(hInstance)
    , m_nCmdShow(nCmdShow)
    , m_hWnd(nullptr)
    , m_width(800)
    , m_height(600)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize        = sizeof(WNDCLASSEXW);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIconW(hInstance, L"IDI_ICON");
    wcex.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = kClassName;
    wcex.hIconSm       = LoadIconW(wcex.hInstance, L"IDI_ICON");
    RegisterClassExW(&wcex);
}

Window::~Window()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
    UnregisterClassW(kClassName, m_hInstance);
}

// 指定サイズのクライアント領域を持つウィンドウを生成して表示する。
void Window::Initialize(int width, int height)
{
    m_width  = width;
    m_height = height;

    RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowExW(
        0,
        kClassName,
        L"SteelRevenant",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInstance,
        nullptr);

    ShowWindow(m_hWnd, m_nCmdShow);
    GetClientRect(m_hWnd, &rc);
}

// ウィンドウメッセージプロシージャ。
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool s_inSizeMove = false;
    static bool s_inSuspend  = false;
    static bool s_minimized  = false;

    auto* game = reinterpret_cast<Game*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_ACTIVATEAPP:
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
        if (wParam)
        {
            if (game) game->OnActivated();
        }
        else
        {
            if (game) game->OnDeactivated();
        }
        break;

    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_inSuspend && game) game->OnSuspending();
                s_inSuspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_inSuspend && game) game->OnResuming();
            s_inSuspend = false;
        }
        else if (!s_inSizeMove && game)
        {
            game->OnWindowSizeChanged(static_cast<int>(LOWORD(lParam)), static_cast<int>(HIWORD(lParam)));
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_inSizeMove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_inSizeMove = false;
        if (game)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            // Alt+Enter: フルスクリーン切替（今回は未対応）
        }
        break;

    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}
