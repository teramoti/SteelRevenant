//------------------------//------------------------
// Contents(処理内容) ゲーム本体の初期化、メインループ、最上位描画処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "Game.h"
#include "CheckMemory.h"

#include "Source/GameSystem/InputManager.h"
#include "Source/GameSystem/DrawManager.h"
#include "Source/GameSystem/GameSaveData.h"
#include "Source/Utility/Sound/AudioSystem.h"

#include <array>
#include <filesystem>
#include <vector>
void ExitGame();

namespace
{
	// 実行場所に依存せず音声フォルダを解決する。
	std::wstring ResolveAudioRoot()
	{
		namespace fs = std::filesystem;

		std::vector<fs::path> bases;
		bases.push_back(fs::current_path());

		wchar_t modulePath[MAX_PATH] = {};
		if (::GetModuleFileNameW(nullptr, modulePath, MAX_PATH) > 0)
		{
			bases.push_back(fs::path(modulePath).parent_path());
		}

		const std::array<fs::path, 4> relativeCandidates =
		{
			fs::path(L"Resources") / L"Sound",
			fs::path(L"..") / L"Resources" / L"Sound",
			fs::path(L"..") / L".." / L"Resources" / L"Sound",
			fs::path(L"..") / L".." / L".." / L"Resources" / L"Sound"
		};

		for (const fs::path& base : bases)
		{
			for (const fs::path& relative : relativeCandidates)
			{
				const fs::path candidate = fs::weakly_canonical(base / relative);
				if (fs::exists(candidate) && fs::is_directory(candidate))
				{
					return candidate.wstring();
				}
			}
		}

		return (fs::current_path() / L"Resources" / L"Sound").wstring();
	}
}

// 実行時の画面サイズを受け取り、初期状態を構築する。
Game::Game(int width, int height)
	: m_hWnd(0), m_width(width), m_height(height) {

	// スタートアップ情報を受け取る。
	STARTUPINFO si{};
	// 自プロセスのインスタンスハンドルを取得する。
	m_hInstance = ::GetModuleHandle(NULL);

	// 表示状態の初期値を取得する。
	::GetStartupInfo(&si);
	m_nCmdShow = si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT;
	// ウィンドウ管理オブジェクトを生成する。
	m_window = std::make_unique<Window>(m_hInstance, m_nCmdShow);
}

// 実行に必要な共通リソースを初期化する。
void Game::Initialize()
{
	// ウィンドウを生成する。
	m_window->Initialize(m_width, m_height);
	// 生成済みウィンドウのハンドルを保持する。
	m_hWnd = m_window->GetHWnd();
	::SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Direct3D 初期化に必要なウィンドウ情報を設定する。
	m_directX.SetHWnd(m_hWnd);
	m_directX.SetWidth(m_width);
	m_directX.SetHeight(m_height);

	// デバイスを生成する。
	m_directX.CreateDevice();
	// 画面サイズに依存する描画リソースを生成する。
	m_directX.CreateResources();

	// ゲーム全体を 60 FPS 固定タイムステップで更新する。
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60.0);

	// 入力デバイスを生成する。
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(m_hWnd);
	m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	System::InputManager::GetInstance().SetMouseSensitivity(GameSaveData::GetInstance().GetMouseSensitivity());

	// 2D 描画に使う共通リソースを生成する。
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_directX.GetContext().Get());
	m_font = std::make_unique<DirectX::SpriteFont>(m_directX.GetDevice().Get(), L"SegoeUI_18.spritefont");
	System::DrawManager::GetInstance().Initialize(m_directX.GetDevice().Get(), m_directX.GetContext().Get());

	// 音声サービスを初期化し、現在の設定値を反映する。
	GameAudio::AudioSystem& audio = GameAudio::AudioSystem::GetInstance();
	audio.Initialize(ResolveAudioRoot());
	audio.ApplyVolumeSettings(GameSaveData::GetInstance().GetAudioVolumeSettings());

	// タイトルシーンを初期化して開始状態にする。
	m_sceneManager = std::make_unique<SceneManager>();
	m_sceneManager->SetScene(TITLE_SCENE);
	m_sceneManager->InitilizeActiveScene();
}

// メッセージループとゲームループを実行する。
MSG Game::Tick()
{
	// 受信メッセージを保持する。
	MSG msg = {};

	// 実行前に共通リソースを初期化する。
	Initialize();

	// 終了要求が来るまで更新と描画を繰り返す。
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// 入力とゲーム状態を更新する。
			m_timer.Tick([&]() { Update(m_timer); });
			// 現在フレームを描画する。
			Render();
		}
	}

	// 終了前に保持リソースを解放する。
	Finalize();
	
	return msg;
}

// 1 フレーム分のゲーム状態を更新する。
void Game::Update(const DX::StepTimer& timer)
{
	// 現在のキーボード状態を取得する。
	DirectX::Keyboard::State kb = m_keyboard->GetState();

	// 押下変化を追跡する。
	m_keyboardTracker.Update(kb);

	// 現在のマウス状態を取得し、押下変化を追跡する。
	DirectX::Mouse::State ms = m_mouse->GetState();
	m_mouseTracker.Update(ms);

	// 入力管理へ現在フレームの状態を反映する。
	System::InputManager::GetInstance().Update(&kb, &m_keyboardTracker, &ms, &m_mouseTracker);
	GameAudio::AudioSystem::GetInstance().Update();

	// アクティブシーンへ更新を委譲する。
	m_sceneManager->UpdateActiveScene(timer);
}

// 現在フレームを描画する。
void Game::Render()
{
	// 初回更新前は描画しない。
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}
	// 描画前にバックバッファをクリアする。
	Clear();

	// 2D 描画バッチを開始する。
	m_spriteBatch->Begin();

	// アクティブシーンを描画する。
	m_sceneManager->RenderActiveSceneRender();
	
	m_spriteBatch->End();
	// 描画結果を画面へ表示する。
	Present();
}

// 描画前にバックバッファをクリアする。
void Game::Clear()
{
	// レンダーターゲットを背景色で初期化する。
	const float clearColor[4] = { 0.53f, 0.75f, 0.96f, 1.0f };
	m_directX.GetContext()->ClearRenderTargetView(m_directX.GetRenderTargetView().Get(), clearColor);
	// 深度とステンシルを初期化する。
	m_directX.GetContext()->ClearDepthStencilView(m_directX.GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// 描画先のレンダーターゲットを設定する。
	m_directX.GetContext()->OMSetRenderTargets(1, m_directX.GetRenderTargetView().GetAddressOf(), m_directX.GetDepthStencilView().Get());
	// ビューポートを現在の画面サイズへ合わせる。
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_directX.GetContext()->RSSetViewports(1, &viewport);
}

// バックバッファを画面へ表示する。
void Game::Present()
{
	// VSync 同期待ちありでスワップチェーンを表示する。
	HRESULT hr = m_directX.GetSwapChain()->Present(1, 0);

	// デバイスロスト時は描画リソースを再生成する。
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		m_directX.OnDeviceLost();
	}
	else if (FAILED(hr))
	{
		::OutputDebugStringW(L"Present failed. The game will close safely.\n");
		ExitGame();
	}
}

// 保持中のリソースを解放する。
void Game::Finalize()
{
	if (m_hWnd != nullptr)
	{
		::SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, 0);
	}

	if (m_sceneManager != nullptr)
	{
		m_sceneManager->FinalizeActiveScene();
		m_sceneManager.reset();
	}

	GameAudio::AudioSystem::GetInstance().Shutdown();

	m_keyboard.reset();
	m_mouse.reset();
	m_spriteBatch.reset();
	m_font.reset();
	m_commonStates.reset();
	m_graphics.reset();

	// シーンと音声の解放後にグラフィックス関連リソースを落とす。
	DirectX11::Dispose();
	m_window.reset();
	m_hWnd = nullptr;
}

// アプリ再アクティブ時の入力状態を整える。
void Game::OnActivated()
{
	// マウスを相対モードへ戻す。
	if (m_mouse != nullptr)
	{
		m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	}
	GameAudio::AudioSystem::GetInstance().ResumeAll();
}
// アプリ非アクティブ時の入力状態を整える。
void Game::OnDeactivated()
{
	// マウスを絶対モードへ戻す。
	if (m_mouse != nullptr)
	{
		m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	}
	GameAudio::AudioSystem::GetInstance().SuspendAll();
}
// 一時停止要求時の処理を受け取る。
void Game::OnSuspending()
{
	GameAudio::AudioSystem::GetInstance().SuspendAll();
}

// 一時停止から復帰した直後の時間差をリセットする。
void Game::OnResuming()
{
	m_timer.ResetElapsedTime();
	GameAudio::AudioSystem::GetInstance().ResumeAll();
}
// ウィンドウサイズ変更に合わせて描画リソースを再生成する。
void Game::OnWindowSizeChanged(int width, int height)
{
	m_width = std::max(width, 1);
	m_height = std::max(height, 1);

	DirectX11::Get().CreateResources();
}
// 既定のウィンドウサイズを返す。
void Game::GetDefaultSize(int& width, int& height) const
{
	width = 800;
	height = 600;
}
// 終了メッセージをポストする。
void ExitGame()
{
	PostQuitMessage(0);
}


