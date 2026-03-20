//------------------------//------------------------
// Contents(処理内容) UI描画補助関数群を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "UiUtil.h"

#include <iomanip>
#include <sstream>

namespace UiUtil
{
	// 1x1単色テクスチャを生成し、SpriteBatchの矩形描画に利用できるSRVを返す。
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateSolidTexture(ID3D11Device* device, std::uint32_t rgba)
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
		if (device == nullptr)
		{
			return shaderResourceView;
		}

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = 1;
		textureDesc.Height = 1;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA textureData = {};
		textureData.pSysMem = &rgba;
		textureData.SysMemPitch = sizeof(std::uint32_t);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		if (FAILED(device->CreateTexture2D(&textureDesc, &textureData, texture.GetAddressOf())))
		{
			return shaderResourceView;
		}

		if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, shaderResourceView.GetAddressOf())))
		{
			shaderResourceView.Reset();
		}

		return shaderResourceView;
	}

	// 点pが [pos, pos + size] の矩形内にあるかを判定する。
	bool IsInsideRect(const DirectX::SimpleMath::Vector2& p, const DirectX::SimpleMath::Vector2& pos, const DirectX::SimpleMath::Vector2& size)
	{
		return (p.x >= pos.x && p.x <= (pos.x + size.x) && p.y >= pos.y && p.y <= (pos.y + size.y));
	}

	// 小数値を固定小数点のwstringへ整形する。
	std::wstring ToWStringFixed(float value, int precision)
	{
		std::wstringstream ss;
		ss << std::fixed << std::setprecision(precision) << value;
		return ss.str();
	}
}

