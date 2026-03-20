//------------------------//------------------------
// Contents(処理内容) Direct3D 11 デバイスとスワップチェーンを管理する。
//------------------------//------------------------
#include "DirectX11.h"
#include <stdexcept>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

DirectX11* DirectX11::s_instance = nullptr;

DirectX11& DirectX11::Get()   { if (!s_instance) s_instance = new DirectX11(); return *s_instance; }
void DirectX11::Dispose()     { delete s_instance; s_instance = nullptr; }
DirectX11::DirectX11()        = default;

void DirectX11::CreateDevice()
{
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
    Microsoft::WRL::ComPtr<ID3D11Device> dev;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        levels, static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION,
        dev.GetAddressOf(), &m_featureLevel, ctx.GetAddressOf());
    if (FAILED(hr))
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags,
            levels, static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION,
            dev.GetAddressOf(), &m_featureLevel, ctx.GetAddressOf());
    if (FAILED(hr)) throw std::runtime_error("D3D11CreateDevice failed.");
    dev.As(&m_d3dDevice);
    ctx.As(&m_d3dContext);
    CreateResources();
}

void DirectX11::CreateResources()
{
    m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_renderTargetView.Reset(); m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT W = static_cast<UINT>(std::max(m_width, 1));
    const UINT H = static_cast<UINT>(std::max(m_height, 1));

    if (m_swapChain)
    {
        if (FAILED(m_swapChain->ResizeBuffers(2, W, H, DXGI_FORMAT_B8G8R8A8_UNORM, 0)))
            throw std::runtime_error("ResizeBuffers failed.");
    }
    else
    {
        Microsoft::WRL::ComPtr<IDXGIDevice1>  dxgiDevice;
        Microsoft::WRL::ComPtr<IDXGIAdapter>  dxgiAdapter;
        Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
        m_d3dDevice.As(&dxgiDevice);
        dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
        dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2; sd.BufferDesc.Width = W; sd.BufferDesc.Height = H;
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.BufferDesc.RefreshRate = { 60, 1 };
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = m_hWnd; sd.SampleDesc.Count = 1; sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        if (FAILED(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf())))
            throw std::runtime_error("CreateSwapChain failed.");
        dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());

    CD3D11_TEXTURE2D_DESC depthDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, W, H, 1, 1, D3D11_BIND_DEPTH_STENCIL);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    m_d3dDevice->CreateTexture2D(&depthDesc, nullptr, depthStencil.GetAddressOf());
    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &dsvDesc, m_depthStencilView.GetAddressOf());
}

void DirectX11::OnDeviceLost()
{
    m_renderTargetView.Reset(); m_depthStencilView.Reset();
    m_swapChain.Reset(); m_d3dContext.Reset(); m_d3dDevice.Reset();
    CreateDevice();
}
