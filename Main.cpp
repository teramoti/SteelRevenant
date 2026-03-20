//------------------------//------------------------
// Contents(処理内容) WinMain エントリポイントと起動時の初期化を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <exception>

#include "Window.h"
#include "CheckMemory.h"

#include "Game.h"

// メインソース。
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// メモリリークレポートを出力ウィンドウのデバッグウィンドウに出力する
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// 出力先を出力ウィンドウに戻す
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 実行ディレクトリ依存でリソース読み込みが失敗しないよう、
	// カレントディレクトリを実行ファイル配置先へ揃える。
	wchar_t modulePath[MAX_PATH] = {};
	if (::GetModuleFileNameW(nullptr, modulePath, MAX_PATH) > 0)
	{
		std::wstring executablePath(modulePath);
		const size_t separator = executablePath.find_last_of(L"\\/");
		if (separator != std::wstring::npos)
		{
			const std::wstring executableDir = executablePath.substr(0, separator);
			::SetCurrentDirectoryW(executableDir.c_str());
		}
	}

	//COMライブラリを初期化する。
	if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
		return 1;

	int width = 800;
	int height = 600;
	MSG msg = {};

	try
	{
		//Gameオブジェクトの生成
		Game game(width, height);
		//ゲームの実行をする
		msg = game.Tick();
	}
	catch (const std::exception& ex)
	{
		std::wstring message = L"SteelRevenant caught a runtime exception.\n";
		std::string what = ex.what() != nullptr ? ex.what() : "std::exception";
		message.append(what.begin(), what.end());
		::MessageBoxW(nullptr, message.c_str(), L"SteelRevenant", MB_OK | MB_ICONERROR);
		msg.message = WM_QUIT;
		msg.wParam = 1;
	}
	catch (...)
	{
		::MessageBoxW(nullptr, L"SteelRevenant caught an unknown runtime exception.", L"SteelRevenant", MB_OK | MB_ICONERROR);
		msg.message = WM_QUIT;
		msg.wParam = 1;
	}

	//COMイニシャライズの終了処理(解放)
	CoUninitialize();

	return (int)msg.wParam;
}

