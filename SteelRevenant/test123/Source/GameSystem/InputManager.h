#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Keyboard.h>
#include <Mouse.h>
#include <SimpleMath.h>
#include "../Utility/SingletonBase.h"

namespace System
{
    class InputManager : public Utility::SingletonBase<InputManager>
    {
        friend class Utility::SingletonBase<InputManager>;
    public:
        void Update(
            const DirectX::Keyboard::State*                kb,
            const DirectX::Keyboard::KeyboardStateTracker* kbT,
            const DirectX::Mouse::State*                   ms,
            const DirectX::Mouse::ButtonStateTracker*      msT);

        void  SetMouseSensitivity(float s) { m_sensitivity = s; }
        float GetMouseSensitivity() const  { return m_sensitivity; }

        bool IsKeyDown(DirectX::Keyboard::Keys key)     const;
        bool IsKeyPressed(DirectX::Keyboard::Keys key)  const;
        bool IsKeyReleased(DirectX::Keyboard::Keys key) const;
        bool IsMouseButtonDown(int btn)    const;
        bool IsMouseButtonPressed(int btn) const;
        DirectX::SimpleMath::Vector2 GetMouseDelta() const { return m_delta; }
        void ResetMouseDelta(int ignoreFrames = 2);

        // StageSelectScene / SettingsScene 莠呈鋤
        const DirectX::Keyboard::KeyboardStateTracker* GetKeyboardTracker() const { return m_kbT; }
        const DirectX::Mouse::ButtonStateTracker*       GetMouseTracker()    const { return m_msT; }
        const DirectX::Mouse::State*                    GetMouseState()      const { return m_ms; }

    private:
        InputManager() = default;
        const DirectX::Keyboard::State*                m_kb  = nullptr;
        const DirectX::Keyboard::KeyboardStateTracker* m_kbT = nullptr;
        const DirectX::Mouse::State*                   m_ms  = nullptr;
        const DirectX::Mouse::ButtonStateTracker*      m_msT = nullptr;
        DirectX::SimpleMath::Vector2 m_delta;
        float m_sensitivity = 0.12f;
        int m_ignoreMouseDeltaFrames = 0;
    };
}

