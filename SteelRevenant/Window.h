//------------------------//------------------------
// Contents(処理内容) Win32 ウィンドウ生成クラスとメッセージ処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#ifndef  WINDOW_DEFINED
#define  WINDOW_DEFINED
// 依存ヘッダ

#include <windows.h>
#include <Mouse.h>
#include <Keyboard.h>

#include "Game.h"
// Win32 ウィンドウ生成とメッセージ処理をまとめる。
class Window 
{
public:
	// インスタンスハンドルと初期表示状態を保持する。
	Window(HINSTANCE hInstance, int nCmdShow) : m_hInstance(hInstance), m_nCmdShow(nCmdShow) {
	}
	// ウィンドウクラスを登録し、指定サイズのウィンドウを生成する。
	int Initialize(int width, int height) 
	{
		static constexpr wchar_t kWindowClassName[] = L"SteelRevenantWindowClass";
		static constexpr wchar_t kWindowTitle[] = L"STEEL REVENANT";

		// ウィンドウクラスの基本属性と既定アイコンを設定する。
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInstance;
		wcex.hIcon = LoadIcon(m_hInstance, L"IDI_ICON");
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = kWindowClassName;
		wcex.hIconSm = LoadIcon(wcex.hInstance, L"IDI_ICON");
		if (!RegisterClassEx(&wcex))
			return 1;

		// 希望するクライアント領域サイズからウィンドウ外枠込みのサイズを算出する。
		RECT rc;
		rc.top = 0;
		rc.left = 0;
		rc.right = static_cast<LONG>(width);
		rc.bottom = static_cast<LONG>(height);

		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		// 登録済みクラスからメインウィンドウを生成する。
		HWND hWnd = CreateWindowEx(0, kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, m_hInstance, nullptr);

		if (!hWnd)
			return 1;

		ShowWindow(hWnd, m_nCmdShow);
		m_hWnd = hWnd;
		return 0;
	}

	// 作成済みウィンドウハンドルを返す。
	HWND GetHWnd() 
	{ 
		return m_hWnd; 
	}
	// ウィンドウメッセージを処理する。
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	int m_nCmdShow;
};

#endif	// WINDOW_DEFINED


