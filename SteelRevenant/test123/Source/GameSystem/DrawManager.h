#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d11.h>
#include <memory>
#include <CommonStates.h>
#include <SpriteBatch.h>
#include "../Utility/SingletonBase.h"

namespace System
{
    class DrawManager : public Utility::SingletonBase<DrawManager>
    {
        friend class Utility::SingletonBase<DrawManager>;
    public:
        void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
        ID3D11DeviceContext*    GetContext() const { return m_context; }
        DirectX::CommonStates*  GetStates()  const { return m_states.get(); }
        DirectX::SpriteBatch*   GetBatch()   const { return m_spriteBatch.get(); }
        void ApplyPrimitiveState()  const;
        void ApplyAlphaBlendState() const;
        // ActionSceneBase 縺九ｉ蜻ｼ縺ｰ繧後ｋ SpriteBatch Begin/End
        DirectX::SpriteBatch*   GetSprite()  const { return m_spriteBatch.get(); }
        void Begin() const;
        void End()   const;
    private:
        DrawManager() = default;
        ID3D11DeviceContext*                   m_context = nullptr;
        std::unique_ptr<DirectX::CommonStates> m_states;
        std::unique_ptr<DirectX::SpriteBatch>  m_spriteBatch;
    };
}

