#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdint>
#include <string>

#include <d3d11.h>
#include <wrl/client.h>
#include <SimpleMath.h>

namespace UiUtil
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateSolidTexture(
        ID3D11Device* device,
        std::uint32_t rgba = 0xffffffffu);

    bool IsInsideRect(
        const DirectX::SimpleMath::Vector2& point,
        const DirectX::SimpleMath::Vector2& position,
        const DirectX::SimpleMath::Vector2& size);

    std::wstring ToWStringFixed(float value, int precision);
}

