//------------------------//------------------------
// Contents(処理内容) Declares the DirectXTK XboxDDSTextureLoader API and related types.
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#if !defined(_XBOX_ONE) || !defined(_TITLE)
#error This module only supports Xbox One exclusive apps
#endif

#include <d3d11_x.h>

#include <stdint.h>

namespace Xbox
{
    enum DDS_ALPHA_MODE
    {
        DDS_ALPHA_MODE_UNKNOWN       = 0,
        DDS_ALPHA_MODE_STRAIGHT      = 1,
        DDS_ALPHA_MODE_PREMULTIPLIED = 2,
        DDS_ALPHA_MODE_OPAQUE        = 3,
        DDS_ALPHA_MODE_CUSTOM        = 4,
    };

    HRESULT __cdecl CreateDDSTextureFromMemory(
        _In_ ID3D11DeviceX* d3dDevice,
        _In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
        _In_ size_t ddsDataSize,
        _Outptr_opt_ ID3D11Resource** texture,
        _Outptr_opt_ ID3D11ShaderResourceView** textureView,
        _Outptr_ void** grfxMemory,
        _Out_opt_ DDS_ALPHA_MODE* alphaMode = nullptr, 
        _In_ bool forceSRGB = false);

    HRESULT __cdecl CreateDDSTextureFromFile( _In_ ID3D11DeviceX* d3dDevice,
        _In_z_ const wchar_t* szFileName,
        _Outptr_opt_ ID3D11Resource** texture,
        _Outptr_opt_ ID3D11ShaderResourceView** textureView,
        _Outptr_ void** grfxMemory,
        _Out_opt_ DDS_ALPHA_MODE* alphaMode = nullptr,
        _In_ bool forceSRGB = false);

    void FreeDDSTextureMemory( _In_opt_ void* grfxMemory );
}

