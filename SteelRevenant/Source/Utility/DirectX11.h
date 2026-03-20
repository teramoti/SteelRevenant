#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11_1.h>
#include <wrl/client.h>

// Direct3D 11 デバイス/スワップチェーンを管理するシングルトン。
class DirectX11
{
public:
    static DirectX11& Get();
    static void Dispose();

    // 初期化パラメータ設定。
    void SetHWnd(HWND hWnd)    { m_hWnd   = hWnd; }
    void SetWidth(int width)   { m_width  = width; }
    void SetHeight(int height) { m_height = height; }

    // デバイスとスワップチェーンを生成する。
    void CreateDevice();

    // スワップチェーンに紐付くリソース（RTV/DSV/ビューポート）を生成する。
    void CreateResources();

    // デバイスロスト時に全リソースを再生成する。
    void OnDeviceLost();

    // アクセサ。
    Microsoft::WRL::ComPtr<ID3D11Device>&           GetDevice()          { return m_d3dDevice; }
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>&     GetContext()         { return m_d3dContext; }
    Microsoft::WRL::ComPtr<IDXGISwapChain>&          GetSwapChain()       { return m_swapChain; }
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>&  GetRenderTargetView(){ return m_renderTargetView; }
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>&  GetDepthStencilView(){ return m_depthStencilView; }

    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    DirectX11();
    ~DirectX11() = default;
    DirectX11(const DirectX11&) = delete;
    DirectX11& operator=(const DirectX11&) = delete;

    HWND m_hWnd;
    int  m_width;
    int  m_height;

    Microsoft::WRL::ComPtr<ID3D11Device>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    D3D_FEATURE_LEVEL m_featureLevel;

    static DirectX11* s_instance;
};
