#pragma once
#include <d3d11.h>
#include <memory>
#include <CommonStates.h>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <Effects.h>
#include "../Utility/SingletonBase.h"

namespace System
{
    // GeometricPrimitive 描画に必要な共通ステートを管理するシングルトン。
    class DrawManager : public Utility::SingletonBase<DrawManager>
    {
        friend class Utility::SingletonBase<DrawManager>;
    public:
        void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

        ID3D11DeviceContext* GetContext()       const { return m_context; }
        DirectX::CommonStates* GetStates()      const { return m_states.get(); }

        // プリミティブ描画前に標準ステートを適用する。
        void ApplyPrimitiveState() const;

        // アルファブレンド有効ステートを適用する。
        void ApplyAlphaBlendState() const;

    private:
        DrawManager() = default;

        ID3D11DeviceContext*                      m_context = nullptr;
        std::unique_ptr<DirectX::CommonStates>    m_states;
    };
}
