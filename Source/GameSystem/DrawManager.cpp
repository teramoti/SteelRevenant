//------------------------//------------------------
// Contents(処理内容) 2Dスプライト描画管理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "../Utility/dx.h"

#include <CommonStates.h>
#include <windows.h>
#include <algorithm>
#include <string>
#include <vector>
#include <wrl/client.h>

#include "DrawManager.h"

namespace
{
	// 指定パスの通常ファイルが存在するかを返す。
	bool FileExists(const std::wstring& path)
	{
		const DWORD attr = ::GetFileAttributesW(path.c_str());
		return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}

	// 画像パスの区切り文字をバックスラッシュへ揃える。
	std::wstring NormalizePath(const wchar_t* fileName)
	{
		std::wstring path = fileName != nullptr ? fileName : L"";
		std::replace(path.begin(), path.end(), L'/', L'\\');
		return path;
	}

	// テクスチャ読み込み候補のパス一覧を構築する。
	std::vector<std::wstring> BuildTextureCandidates(const std::wstring& normalizedPath)
	{
		std::vector<std::wstring> candidates;
		if (normalizedPath.empty())
		{
			return candidates;
		}

		candidates.push_back(normalizedPath);
		candidates.push_back(L".\\" + normalizedPath);
		candidates.push_back(L"..\\" + normalizedPath);
		candidates.push_back(L"..\\..\\" + normalizedPath);
		candidates.push_back(L"x64\\Debug\\" + normalizedPath);
		candidates.push_back(L"x64\\Release\\" + normalizedPath);
		candidates.push_back(L"Resources\\Textures\\" + normalizedPath);
		candidates.push_back(L"..\\Resources\\Textures\\" + normalizedPath);
		candidates.push_back(L"..\\..\\Resources\\Textures\\" + normalizedPath);
		candidates.push_back(L"Source\\" + normalizedPath);
		candidates.push_back(L"Source\\x64\\Debug\\" + normalizedPath);
		candidates.push_back(L"Source\\x64\\Release\\" + normalizedPath);
		return candidates;
	}

	// 読み込み失敗時に使う 1x1 テクスチャを生成する。
	HRESULT CreateFallbackTexture(ID3D11Device* device, ID3D11ShaderResourceView** outSrv)
	{
		if (device == nullptr || outSrv == nullptr)
		{
			return E_INVALIDARG;
		}

		const UINT pixel = 0xFFFFFFFFu;
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = 1;
		desc.Height = 1;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &pixel;
		initData.SysMemPitch = sizeof(pixel);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		HRESULT hr = device->CreateTexture2D(&desc, &initData, texture.GetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		return device->CreateShaderResourceView(texture.Get(), &srvDesc, outSrv);
	}

	// 候補パスを順に試し、利用可能なテクスチャを読み込む。
	HRESULT LoadTextureWithFallbackPaths(ID3D11Device* device, const wchar_t* fileName, ID3D11ShaderResourceView** outSrv)
	{
		if (device == nullptr || outSrv == nullptr)
		{
			return E_INVALIDARG;
		}

		*outSrv = nullptr;
		const std::wstring normalizedPath = NormalizePath(fileName);
		for (const std::wstring& candidate : BuildTextureCandidates(normalizedPath))
		{
			if (!FileExists(candidate))
			{
				continue;
			}
			HRESULT hr = DirectX::CreateWICTextureFromFile(device, candidate.c_str(), nullptr, outSrv);
			if (SUCCEEDED(hr) && *outSrv != nullptr)
			{
				return hr;
			}
		}
		return CreateFallbackTexture(device, outSrv);
	}
}

// 描画に必要なデバイス/コンテキストを設定する。
void System::DrawManager::Initialize(ID3D11Device* pDevise, ID3D11DeviceContext* pContext)
{
	SetDevise(pDevise);
	mSpriteBatch = std::make_unique<DirectX::SpriteBatch>(pContext);
}

// DrawDataに従って1スプライト描画する。
void System::DrawManager::Draw(DrawData& data)
{
	if (mSpriteBatch == nullptr || data.GetTexture() == nullptr || *data.GetTexture() == nullptr)
	{
		return;
	}
	mSpriteBatch->Draw(
		(*data.GetTexture()),
		data.GetPos(),
		data.GetRectPtn(),
		DirectX::Colors::White,
		data.GetRot(),
		data.GetOrigin(),
		data.GetScale());
}

// SpriteBatchの描画開始処理。
void System::DrawManager::Begin()
{
	if (mSpriteBatch == nullptr || mpDevice == nullptr)
	{
		return;
	}
	DirectX::CommonStates states(mpDevice);
	mSpriteBatch->Begin(DirectX::SpriteSortMode_Deferred, states.NonPremultiplied());
}

// SpriteBatchの描画終了処理。
void System::DrawManager::End()
{
	if (mSpriteBatch != nullptr)
	{
		mSpriteBatch->End();
	}
}

// 画像ファイルを読み込み、SRVへ設定する。
bool System::DrawManager::LoadTexture(DrawData& data, wchar_t* pFileName)
{
	if (mpDevice == nullptr)
	{
		return false;
	}
	if (*data.GetTexture() != nullptr)
	{
		(*data.GetTexture())->Release();
		*data.GetTexture() = nullptr;
	}
	return SUCCEEDED(LoadTextureWithFallbackPaths(mpDevice, pFileName, data.GetTexture()));
}

// 画像ファイルを読み込み、SRVへ設定する。
bool System::DrawManager::LoadTexture(DxTexture& pTexture, wchar_t* pFileName)
{
	if (mpDevice == nullptr)
	{
		return false;
	}
	if (pTexture != nullptr)
	{
		pTexture->Release();
		pTexture = nullptr;
	}
	return SUCCEEDED(LoadTextureWithFallbackPaths(mpDevice, pFileName, &pTexture));
}

