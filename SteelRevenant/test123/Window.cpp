#ifndef NOMINMAX
#define NOMINMAX
#endif
//------------------------
// Contents(蜃ｦ逅・・螳ｹ) Win32 繧ｦ繧｣繝ｳ繝峨え縺ｮ逕滓・縺ｨ邂｡逅・ｒ螳溯｣・☆繧九・//------------------------
#include "Window.h"
#include "Game.h"
#include <Keyboard.h>
#include <Mouse.h>

Window::Window(HINSTANCE hInstance, int nCmdShow)
    : m_hInstance(hInstance), m_nCmdShow(nCmdShow), m_hWnd(nullptr), m_width(800), m_height(600)
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
    wcex.hIconSm       = wcex.hIcon;
    RegisterClassExW(&wcex);
}

Window::~Window()
{
    if (m_hWnd)
    {
        SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, 0);
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
    UnregisterClassW(kClassName, m_hInstance);
}

void Window::Initialize(int width, int height)
{
    m_width = width; m_height = height;
    RECT rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    m_hWnd = CreateWindowExW(0, kClassName, L"SteelRevenant", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, m_hInstance, nullptr);
    ShowWindow(m_hWnd, m_nCmdShow);
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* game = reinterpret_cast<Game*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    switch (message)
    {
    case WM_ACTIVATEAPP:
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
        if (wParam) { if (game) game->OnActivated(); }
        else        { if (game) game->OnDeactivated(); }
        break;
    case WM_INPUT: case WM_MOUSEMOVE: case WM_LBUTTONDOWN: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_MBUTTONDOWN: case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:  case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_MOUSEHOVER:
        DirectX::Mouse::ProcessMessage(message, wParam, lParam); break;
    case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam); break;
    case WM_SIZE:
        if (game && wParam != SIZE_MINIMIZED)
            game->OnWindowSizeChanged(static_cast<int>(LOWORD(lParam)), static_cast<int>(HIWORD(lParam)));
        break;
    case WM_DESTROY: PostQuitMessage(0); break;
    case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

