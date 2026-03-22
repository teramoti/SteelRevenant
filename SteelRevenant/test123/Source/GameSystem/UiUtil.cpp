#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "UiUtil.h"

#include <iomanip>
#include <sstream>

namespace UiUtil
{
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

    bool IsInsideRect(
        const DirectX::SimpleMath::Vector2& point,
        const DirectX::SimpleMath::Vector2& position,
        const DirectX::SimpleMath::Vector2& size)
    {
        return point.x >= position.x && point.x <= (position.x + size.x) &&
               point.y >= position.y && point.y <= (position.y + size.y);
    }

    std::wstring ToWStringFixed(float value, int precision)
    {
        std::wstringstream ss;
        ss << std::fixed << std::setprecision(precision) << value;
        return ss.str();
    }
}

