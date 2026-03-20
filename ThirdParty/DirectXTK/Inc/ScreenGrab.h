//------------------------//------------------------
// Contents(処理内容) Declares the DirectXTK ScreenGrab API and related types.
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#if defined(_XBOX_ONE) && defined(_TITLE)
#include <d3d11_x.h>
#else
#include <d3d11_1.h>
#endif

#include <functional>

#if defined(NTDDI_WIN10_FE) || defined(__MINGW32__)
#include <ocidl.h>
#else
#include <OCIdl.h>
#endif

#ifdef _MSC_VER
#pragma comment(lib,"uuid.lib")
#endif

#ifndef DIRECTX_TOOLKIT_API
#ifdef DIRECTX_TOOLKIT_EXPORT
#ifdef __GNUC__
#define DIRECTX_TOOLKIT_API __attribute__ ((dllexport))
#else
#define DIRECTX_TOOLKIT_API __declspec(dllexport)
#endif
#elif defined(DIRECTX_TOOLKIT_IMPORT)
#ifdef __GNUC__
#define DIRECTX_TOOLKIT_API __attribute__ ((dllimport))
#else
#define DIRECTX_TOOLKIT_API __declspec(dllimport)
#endif
#else
#define DIRECTX_TOOLKIT_API
#endif
#endif

namespace DirectX
{
    DIRECTX_TOOLKIT_API
        HRESULT __cdecl SaveDDSTextureToFile(
            _In_ ID3D11DeviceContext* pContext,
            _In_ ID3D11Resource* pSource,
            _In_z_ const wchar_t* fileName) noexcept;

    DIRECTX_TOOLKIT_API
        HRESULT __cdecl SaveWICTextureToFile(
            _In_ ID3D11DeviceContext* pContext,
            _In_ ID3D11Resource* pSource,
            _In_ REFGUID guidContainerFormat,
            _In_z_ const wchar_t* fileName,
            _In_opt_ const GUID* targetFormat = nullptr,
            _In_ std::function<void __cdecl(IPropertyBag2*)> setCustomProps = nullptr,
            _In_ bool forceSRGB = false);
}

