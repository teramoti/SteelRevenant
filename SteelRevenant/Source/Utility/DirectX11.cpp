//------------------------//------------------------
// Contents(処理内容) Direct3D 11 デバイス・スワップチェーンを管理する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "DirectX11.h"
#include <stdexcept>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

DirectX11* DirectX11::s_instance = nullptr;

DirectX11& DirectX11::Get()
{
    if (!s_instance)
        s_instance = new DirectX11();
    return *s_instance;
}

void DirectX11::Dispose()
{
    delete s_instance;
    s_instance = nullptr;
}

DirectX11::DirectX11()
    : m_hWnd(nullptr)
    , m_width(800)
    , m_height(600)
    , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
{
}

// D3D11 デバイスとスワップチェーンを生成する。
void DirectX11::CreateDevice()
{
    UINT creationFlags = 0;
#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    Microsoft::WRL::ComPtr<ID3D11Device>        device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>  context;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels, static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        device.GetAddressOf(),
        &m_featureLevel,
        context.GetAddressOf());

    if (FAILED(hr))
    {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            creationFlags,
            featureLevels, static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            device.GetAddressOf(),
            &m_featureLevel,
            context.GetAddressOf());

        if (FAILED(hr))
            throw std::runtime_error("D3D11CreateDevice failed.");
    }

    device.As(&m_d3dDevice);
    context.As(&m_d3dContext);

    CreateResources();
}

// スワップチェーンと RTV/DSV を生成する。
void DirectX11::CreateResources()
{
    // 既存のビューを解放する。
    m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT backBufferWidth  = static_cast<UINT>(std::max(m_width,  1));
    const UINT backBufferHeight = static_cast<UINT>(std::max(m_height, 1));
    constexpr DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    constexpr DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    if (m_swapChain)
    {
        // 既存のスワップチェーンをリサイズする。
        HRESULT hr = m_swapChain->ResizeBuffers(2, backBufferWidth, backBufferHeight, backBufferFormat, 0);
        if (FAILED(hr))
            throw std::runtime_error("ResizeBuffers failed.");
    }
    else
    {
        // スワップチェーンを新規生成する。
        Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice;
        m_d3dDevice.As(&dxgiDevice);

        Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
        dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

        Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
        dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount                        = 2;
        sd.BufferDesc.Width                   = backBufferWidth;
        sd.BufferDesc.Height                  = backBufferHeight;
        sd.BufferDesc.Format                  = backBufferFormat;
        sd.BufferDesc.RefreshRate.Numerator   = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                       = m_hWnd;
        sd.SampleDesc.Count                   = 1;
        sd.Windowed                           = TRUE;
        sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

        HRESULT hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        if (FAILED(hr))
            throw std::runtime_error("CreateSwapChain failed.");

        dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
    }

    // バックバッファから RTV を生成する。
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());

    // 深度ステンシルバッファを生成する。
    CD3D11_TEXTURE2D_DESC depthDesc(depthBufferFormat, backBufferWidth, backBufferHeight,
        1, 1, D3D11_BIND_DEPTH_STENCIL);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    m_d3dDevice->CreateTexture2D(&depthDesc, nullptr, depthStencil.GetAddressOf());

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &dsvDesc, m_depthStencilView.GetAddressOf());
}

// デバイスロスト時に全リソースを再生成する。
void DirectX11::OnDeviceLost()
{
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();
}
