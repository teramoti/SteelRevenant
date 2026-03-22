#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11_1.h>
#include <wrl/client.h>

class DirectX11
{
public:
    static DirectX11& Get();
    static void Dispose();

    void SetHWnd(HWND h)     { m_hWnd   = h; }
    void SetWidth(int w)     { m_width  = w; }
    void SetHeight(int h)    { m_height = h; }

    void CreateDevice();
    void CreateResources();
    void OnDeviceLost();

    Microsoft::WRL::ComPtr<ID3D11Device>&           GetDevice()           { return m_d3dDevice; }
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>&     GetContext()          { return m_d3dContext; }
    Microsoft::WRL::ComPtr<IDXGISwapChain>&          GetSwapChain()        { return m_swapChain; }
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>&  GetRenderTargetView() { return m_renderTargetView; }
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>&  GetDepthStencilView() { return m_depthStencilView; }
    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    DirectX11();
    ~DirectX11() = default;
    DirectX11(const DirectX11&) = delete;
    DirectX11& operator=(const DirectX11&) = delete;

    HWND m_hWnd = nullptr;
    int  m_width = 800, m_height = 600;
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

    Microsoft::WRL::ComPtr<ID3D11Device>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    static DirectX11* s_instance;
};

