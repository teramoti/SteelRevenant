//------------------------//------------------------
// Contents(処理内容) Direct3D デバイスと描画リソース管理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "DirectX11.h"
#include "../../CheckMemory.h"

// ハイブリッドグラフィックシステムに対しデフォルトで個別に優先する方を示す
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// シングルトンのためのポインタ変数を初期化する
std::unique_ptr<DirectX11> DirectX11::m_directX(nullptr);

// デバイスを生成する
void DirectX11::CreateDevice()
{
	UINT creationFlags = 0;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static const D3D_FEATURE_LEVEL featureLevels[] = 
	{
		// サポート対象の機能レベルを高い順に列挙する。
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// DX11 APIデバイスオブジェクトを生成し対応するコンテキストを取得する
	HRESULT hr = D3D11CreateDevice(
		nullptr,                 // 規定のアダプタを使用するためにnullptrを指定する specify nullptr to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		// 生成されたDirect3Dデバイスを返す returns the Direct3D device created
		m_device.ReleaseAndGetAddressOf(),    
		// 生成されたデバイスの機能レベルを返す returns feature level of device created
		&m_featureLevel,                  
		// デバイスイミディエイトコンテキストを返す returns the device immediate context
		m_context.ReleaseAndGetAddressOf()   
	);

	if (hr == E_INVALIDARG) 
	{
		// DirectX 11.0プラットフォームがD3D_FEATURE_LEVEL_11_1を認識しない場合、認識されるまで再試行する必要がある
		hr = D3D11CreateDevice(nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			creationFlags,
			&featureLevels[1],
			_countof(featureLevels) - 1,
			D3D11_SDK_VERSION,
			m_device.ReleaseAndGetAddressOf(),
			&m_featureLevel,
			m_context.ReleaseAndGetAddressOf()
		);
	}

	DX::ThrowIfFailed(hr);

#ifndef NDEBUG
	Microsoft::WRL::ComPtr<ID3D11Debug> d3dDebug;
	if (SUCCEEDED(m_device.As(&d3dDebug))) {
		Microsoft::WRL::ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue))) {
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// 必要に応じて抑制対象メッセージIDを追加する。
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif
	// DirectX 11.1が存在する場合 DirectX 11.1 if present
	if (SUCCEEDED(m_device.As(&m_device1)))
		(void)m_context.As(&m_context1);

	// デバイス依存オブジェクトはCreateResources側で初期化する。
}

// SizeChangedイベントでウィンドウを変更するすべてのメモリリソースを配置する
void DirectX11::CreateResources()
{
	// 直前のウィンドウサイズを指定されたコンテキストでクリアする Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_context->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_context->Flush();

	UINT backBufferWidth = static_cast<UINT>(m_width);
	UINT backBufferHeight = static_cast<UINT>(m_height);
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT backBufferCount = 2;

	// スワップチェインが既に存在する場合サイズを変更するかスワップチェインを生成する
	if (m_swapChain)
	{
		HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// デバイスが何らかの理由で削除された場合新しいデバイスとスワップチェインを生成する必要がある
			OnDeviceLost();
			// すべてがセットアップされた。この関数での実行を継続しない
			// OnDeviceLostがこの関数を再度呼び出し、正しく新デバイスの再設定を可能にする
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else {
		// まず、D3DデバイスからDirectX Graphics Interface(DXGI)デバイスを検索する 
		Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
		DX::ThrowIfFailed(m_device.As(&dxgiDevice));

		// デバイスが実行中の物理アダプタ（GPUまたはカード）を識別する 
		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

		// 生成されたファクトリオブジェクトを取得する
		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
		DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory2;
		if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
		{
			// DirectX 11.1またはそれ以降 DirectX 11.1 or later
			// スワップチェインのためのディスクリプタを生成する
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
			swapChainDesc.Width = backBufferWidth;
			swapChainDesc.Height = backBufferHeight;
			swapChainDesc.Format = backBufferFormat;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = backBufferCount;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
			fsSwapChainDesc.Windowed = TRUE;

			// Win32ウィンドウからスワップチェインを生成する Create a SwapChain from a Win32 window.
			DX::ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(
				m_device.Get(),
				m_hWnd,
				&swapChainDesc,
				&fsSwapChainDesc,
				nullptr,
				m_swapChain1.ReleaseAndGetAddressOf()
			));

			DX::ThrowIfFailed(m_swapChain1.As(&m_swapChain));
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
			swapChainDesc.BufferCount = backBufferCount;
			swapChainDesc.BufferDesc.Width = backBufferWidth;
			swapChainDesc.BufferDesc.Height = backBufferHeight;
			swapChainDesc.BufferDesc.Format = backBufferFormat;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = m_hWnd;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.Windowed = TRUE;

			DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_device.Get(), &swapChainDesc, m_swapChain.ReleaseAndGetAddressOf()));
		}

		// このテンプレートは排他的なフルスクリーンモードをサポートしない
		// DXGIに対してALT+ENTERショートカットの応答を防止する
		DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));
	}

	// 最終的な3Dレンダーターゲットとなるウィンドウへのバックバッファを取得する
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

	// バインド時に使用するレンダーターゲット上のビューインターフェースを生成する
	DX::ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

	// デプス/ステンシルバッファとして2-Dサーフェイスを配置し、バインド時に使用する
	// サーフェイス上のデプスステンシルビューを生成する
	CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(m_device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));
	
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(m_device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

	m_states = std::make_unique<DirectX::CommonStates>(m_directX->GetDevice().Get());

	m_effect = std::make_unique<DirectX::EffectFactory>(m_directX->GetDevice().Get());
	// ウィンドウサイズ依存オブジェクトの初期化はここで完了している。
}

// デバイスロストが発生した場合
void DirectX11::OnDeviceLost() 
{
	m_depthStencilView.Reset();
	m_renderTargetView.Reset();
	m_swapChain1.Reset();
	m_swapChain.Reset();
	m_context1.Reset();
	m_context.Reset();
	m_device1.Reset();
	m_device.Reset();
	// デバイスを生成する
	CreateDevice();
	// リソースを生成する
	CreateResources();
}

