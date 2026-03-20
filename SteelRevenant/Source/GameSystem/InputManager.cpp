//------------------------//------------------------
// Contents(処理内容) キーボード・マウス入力の一元管理を実装する。
//------------------------//------------------------
#include "InputManager.h"

namespace System
{
    void InputManager::Update(
        const DirectX::Keyboard::State*                kb,
        const DirectX::Keyboard::KeyboardStateTracker* kbTracker,
        const DirectX::Mouse::State*                   ms,
        const DirectX::Mouse::ButtonStateTracker*      msTracker)
    {
        m_kb        = kb;
        m_kbTracker = kbTracker;
        m_ms        = ms;
        m_msTracker = msTracker;

        if (ms)
        {
            m_mouseDelta.x = static_cast<float>(ms->x) * m_sensitivity;
            m_mouseDelta.y = static_cast<float>(ms->y) * m_sensitivity;
        }
        else
        {
            m_mouseDelta = DirectX::SimpleMath::Vector2::Zero;
        }
    }

    bool InputManager::IsKeyDown(DirectX::Keyboard::Keys key) const
    {
        return m_kb && m_kb->IsKeyDown(key);
    }

    bool InputManager::IsKeyPressed(DirectX::Keyboard::Keys key) const
    {
        return m_kbTracker && m_kbTracker->IsKeyPressed(key);
    }

    bool InputManager::IsKeyReleased(DirectX::Keyboard::Keys key) const
    {
        return m_kbTracker && m_kbTracker->IsKeyReleased(key);
    }

    bool InputManager::IsMouseButtonDown(int button) const
    {
        if (!m_ms) return false;
        switch (button)
        {
        case 0: return m_ms->leftButton;
        case 1: return m_ms->rightButton;
        case 2: return m_ms->middleButton;
        default: return false;
        }
    }

    bool InputManager::IsMouseButtonPressed(int button) const
    {
        if (!m_msTracker) return false;
        using S = DirectX::Mouse::ButtonStateTracker::ButtonState;
        switch (button)
        {
        case 0: return m_msTracker->leftButton   == S::PRESSED;
        case 1: return m_msTracker->rightButton  == S::PRESSED;
        case 2: return m_msTracker->middleButton == S::PRESSED;
        default: return false;
        }
    }
}
