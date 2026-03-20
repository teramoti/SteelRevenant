//------------------------//------------------------
// Contents(処理内容) Declares the DirectXTK GraphicsMemory API and related types.
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

#include <cstddef>
#include <memory>

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
    inline namespace DX11
    {
        class GraphicsMemory
        {
        public:
            DIRECTX_TOOLKIT_API
            #if defined(_XBOX_ONE) && defined(_TITLE)
                GraphicsMemory(
                    _In_ ID3D11DeviceX* device,
                    unsigned int backBufferCount = 2);
        #else
                GraphicsMemory(
                    _In_ ID3D11Device* device,
                    unsigned int backBufferCount = 2);
        #endif

            DIRECTX_TOOLKIT_API GraphicsMemory(GraphicsMemory&&) noexcept;
            DIRECTX_TOOLKIT_API GraphicsMemory& operator= (GraphicsMemory&&) noexcept;

            GraphicsMemory(GraphicsMemory const&) = delete;
            GraphicsMemory& operator=(GraphicsMemory const&) = delete;

            DIRECTX_TOOLKIT_API virtual ~GraphicsMemory();

            DIRECTX_TOOLKIT_API void* __cdecl Allocate(
                _In_opt_ ID3D11DeviceContext* context,
                size_t size,
                int alignment);

            DIRECTX_TOOLKIT_API void __cdecl Commit();

            // Singleton
            DIRECTX_TOOLKIT_API static GraphicsMemory& __cdecl Get();

        private:
            // Private implementation.
            class Impl;

            std::unique_ptr<Impl> pImpl;
        };
    }
}

