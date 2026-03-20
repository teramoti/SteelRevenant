//------------------------//------------------------
// Contents(処理内容) キーボード・マウス入力の一元管理を実装する。
//------------------------//------------------------
#include "InputManager.h"

namespace System
{
    void InputManager::Update(
        const DirectX::Keyboard::State* kb, const DirectX::Keyboard::KeyboardStateTracker* kbT,
        const DirectX::Mouse::State* ms,    const DirectX::Mouse::ButtonStateTracker* msT)
    {
        m_kb = kb; m_kbT = kbT; m_ms = ms; m_msT = msT;
        m_delta = ms
            ? DirectX::SimpleMath::Vector2(static_cast<float>(ms->x), static_cast<float>(ms->y)) * m_sensitivity
            : DirectX::SimpleMath::Vector2::Zero;
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
}
