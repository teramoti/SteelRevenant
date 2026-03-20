//------------------------//------------------------
// Contents(処理内容) Direct3D デバイスと描画リソース管理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#ifndef DIRECTX11_DEFINED
#define DIRECTX11_DEFINED
#include <windows.h>
#include <memory>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <d3d11.h>
#include <SimpleMath.h>
#include <wrl.h>
#include <wrl/client.h>
#include <CommonStates.h>
#include <Effects.h>
#include "dx.h"
// コピー禁止の基底クラス。
class Uncopyable 
{
protected:
	// コピー禁止ベースクラスを初期化する。
	Uncopyable() = default;
	// コピー禁止ベースクラスを破棄する。
	~Uncopyable() = default;
private:
	// コピー構築を禁止する。
	Uncopyable(const Uncopyable&) = delete;
	// コピー代入を禁止する。
	Uncopyable& operator =(const Uncopyable&) = delete;
};
// Direct3D デバイスと描画先リソースを管理する。
class DirectX11 : private Uncopyable 
{
private:
	// シングルトン生成専用コンストラクタ。
	DirectX11() 
	{
	}
public:
	// シングルトンインスタンスを返す。
	static DirectX11& Get() 
	{
		if (m_directX.get() == nullptr) 
		{
			m_directX.reset(new DirectX11());
		}
		return *m_directX.get();
	}
	// シングルトンインスタンスを破棄する。
	static void Dispose() 
	{
		m_directX.reset();
	}
	// ウィンドウハンドルを取得する 
	HWND GetHWnd() const
	{
		return m_hWnd;
	}
	// ウィンドウハンドルを設定する 
	void SetHWnd(HWND hWnd) 
	{
		m_hWnd = hWnd;
	}
	// ウィンドウ幅を取得する 
	int GetWidth() const
	{
		return m_width;
	}
	// ウィンドウ幅を設定する 
	void SetWidth(int width) 
	{
		m_width = width;
	}
	// ウィンドウ高を取得する 
	int GetHeight() const
	{
		return m_height;
	}
	// ウィンドウ高を設定する 
	void SetHeight(int height) 
	{
		m_height = height;
	}
	// Direct3D デバイスを返す。
	Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const
	{
		return m_device; 
	}
	// Direct3D デバイスを設定する。
	void SetDevice(Microsoft::WRL::ComPtr<ID3D11Device> device) 
	{
		m_device = device;
	}
	// デバイスコンテキストを取得する 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetContext() const
	{
		return m_context;
	}
	// デバイスコンテキストを設定する 
	void SetContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) 
	{
		m_context = context;
	}
	// スワップチェインを取得する 
	Microsoft::WRL::ComPtr<IDXGISwapChain> GetSwapChain() const
	{
		return m_swapChain;
	}
	// スワップチェインを設定する
	void SetSwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain) 
	{
		m_swapChain = swapChain;
	}
	// レンダーターゲットビューを取得する
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const
	{
		return m_renderTargetView;
	}
	// レンダーターゲットビューを設定する
	void SetRenderTargetView(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView) {
		m_renderTargetView = renderTargetView;
	}
	// デプスステンシルビューを取得する 
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDepthStencilView() const
	{
		return m_depthStencilView;
	}
	// デプスステンシルビューを設定する 
	void SetDepthStencilView(Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView) {
		m_depthStencilView = depthStencilView;
	}
	// 共通描画ステートを返す。
	DirectX::CommonStates * GetStates()
	{
		return m_states.get();
	}
	// エフェクトファクトリを返す。
	DirectX::EffectFactory * GetEffect()
	{
		return 	m_effect.get();
	}
	// Direct3D デバイスとスワップチェインを生成する。
	void CreateDevice();
	// 画面サイズ依存の描画リソースを生成する。
	void CreateResources();
	// デバイスロスト発生時に再生成処理を行う。
	void OnDeviceLost();
private:
	static std::unique_ptr<DirectX11> m_directX;
	HWND m_hWnd;
	int  m_width;
	int  m_height;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device1;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	m_context;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context1;
	D3D_FEATURE_LEVEL  m_featureLevel;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain1;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	// コモンステート
	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::EffectFactory> m_effect;
};
#endif	// DirectX11 GRAPHICS
