#ifndef NOMINMAX
#define NOMINMAX
#endif
//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 繧ｭ繝ｼ繝懊・繝峨・繝槭え繧ｹ蜈･蜉帙・荳蜈・ｮ｡逅・ｒ螳溯｣・☆繧九・//------------------------
#include "InputManager.h"

#include <algorithm>

namespace System
{
    void InputManager::Update(
        const DirectX::Keyboard::State* kb, const DirectX::Keyboard::KeyboardStateTracker* kbT,
        const DirectX::Mouse::State* ms,    const DirectX::Mouse::ButtonStateTracker* msT)
    {
        m_kb = kb; m_kbT = kbT; m_ms = ms; m_msT = msT;
        if (ms == nullptr || m_ignoreMouseDeltaFrames > 0)
        {
            m_delta = DirectX::SimpleMath::Vector2::Zero;
            m_ignoreMouseDeltaFrames = std::max(0, m_ignoreMouseDeltaFrames - 1);
            return;
        }

        const float clampedX = (std::max)(-48.0f, (std::min)(48.0f, static_cast<float>(ms->x)));
        const float clampedY = (std::max)(-48.0f, (std::min)(48.0f, static_cast<float>(ms->y)));
        m_delta = DirectX::SimpleMath::Vector2(clampedX, clampedY) * m_sensitivity;
    }

    bool InputManager::IsKeyDown(DirectX::Keyboard::Keys k)     const { return m_kb  && m_kb->IsKeyDown(k); }
    bool InputManager::IsKeyPressed(DirectX::Keyboard::Keys k)  const { return m_kbT && m_kbT->IsKeyPressed(k); }
    bool InputManager::IsKeyReleased(DirectX::Keyboard::Keys k) const { return m_kbT && m_kbT->IsKeyReleased(k); }

    bool InputManager::IsMouseButtonDown(int btn) const
    {
        if (!m_ms) return false;
        switch (btn) { case 0: return m_ms->leftButton; case 1: return m_ms->rightButton; case 2: return m_ms->middleButton; }
        return false;
    }
    bool InputManager::IsMouseButtonPressed(int btn) const
    {
        if (!m_msT) return false;
        using S = DirectX::Mouse::ButtonStateTracker::ButtonState;
        switch (btn) { case 0: return m_msT->leftButton==S::PRESSED; case 1: return m_msT->rightButton==S::PRESSED; case 2: return m_msT->middleButton==S::PRESSED; }
        return false;
    }

    void InputManager::ResetMouseDelta(int ignoreFrames)
    {
        m_delta = DirectX::SimpleMath::Vector2::Zero;
        m_ignoreMouseDeltaFrames = std::max(0, ignoreFrames);
    }
}

