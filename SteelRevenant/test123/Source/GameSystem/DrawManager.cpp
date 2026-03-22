#ifndef NOMINMAX
#define NOMINMAX
#endif
//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 謠冗判蜈ｱ騾壹せ繝・・繝医・蛻晄悄蛹悶→驕ｩ逕ｨ繧貞ｮ溯｣・☆繧九・//------------------------
#include "DrawManager.h"

namespace System
{
    void DrawManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_context     = context;
        m_states      = std::make_unique<DirectX::CommonStates>(device);
        m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);
    }
    void DrawManager::ApplyPrimitiveState() const
    {
        if (!m_context || !m_states) return;
        m_context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
        m_context->RSSetState(m_states->CullCounterClockwise());
    }
    void DrawManager::ApplyAlphaBlendState() const
    {
        if (!m_context || !m_states) return;
        m_context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);
        m_context->RSSetState(m_states->CullNone());
    }
    void DrawManager::Begin() const
    {
        if (m_spriteBatch) m_spriteBatch->Begin();
    }
    void DrawManager::End() const
    {
        if (m_spriteBatch) m_spriteBatch->End();
    }
}

