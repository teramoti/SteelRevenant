#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class Game;

class Window
{
public:
    Window(HINSTANCE hInstance, int nCmdShow);
    ~Window();
    void Initialize(int width, int height);
    HWND GetHWnd()   const { return m_hWnd; }
    int  GetWidth()  const { return m_width; }
    int  GetHeight() const { return m_height; }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HINSTANCE m_hInstance;
    int       m_nCmdShow;
    HWND      m_hWnd;
    int       m_width;
    int       m_height;
    static constexpr wchar_t kClassName[] = L"SteelRevenantWnd";
};

