#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// ポーズ画面の状態管理と入力処理を実装する。
//------------------------

#include "GameScenePauseController.h"

#include <Keyboard.h>
#include <Mouse.h>
#include "../../Utility/DirectX11.h"
#include "../../GameSystem/InputManager.h"
#include "../SceneManager/SceneManager.h"

void GameScenePauseController::Reset()
{
    m_isPaused      = false;
    m_selectedIndex = 0;
}

bool GameScenePauseController::UpdateToggle()
{
    const System::InputManager& input = System::InputManager::GetInstance();
    if (!input.IsKeyPressed(DirectX::Keyboard::Escape))
    {
        return m_isPaused;
    }

    m_isPaused = !m_isPaused;
    m_selectedIndex = 0;

    if (m_isPaused)
    {
        DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
        DirectX::Mouse::Get().SetVisible(true);
        System::InputManager::GetInstance().ResetMouseDelta();
    }
    else
    {
        DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
        DirectX::Mouse::Get().SetVisible(false);
        System::InputManager::GetInstance().ResetMouseDelta();
    }

    return m_isPaused;
}

int GameScenePauseController::UpdateMenuInput(int screenWidth, int screenHeight)
{
    DetectMouseHover(screenWidth, screenHeight);
    NavigateMenuByKeyboard();

    const System::InputManager& input = System::InputManager::GetInstance();
    const bool confirmed = input.IsKeyPressed(DirectX::Keyboard::Enter)
                        || input.IsKeyPressed(DirectX::Keyboard::Space)
                        || input.IsMouseButtonPressed(0);
    if (!confirmed)
    {
        return -1;
    }

    if (m_selectedIndex == kMenuResume)
    {
        m_isPaused = false;
        DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_RELATIVE);
        DirectX::Mouse::Get().SetVisible(false);
        System::InputManager::GetInstance().ResetMouseDelta();
        return -1;
    }

    if (m_selectedIndex == kMenuTitle)
    {
        return TITLE_SCENE;
    }

    if (m_selectedIndex == kMenuSettings)
    {
        return SETTINGS_SCENE;
    }

    if (m_selectedIndex == kMenuQuit)
    {
        return -2;
    }

    return -1;
}

void GameScenePauseController::DetectMouseHover(int screenWidth, int screenHeight)
{
    const System::InputManager& input = System::InputManager::GetInstance();
    const DirectX::Mouse::State* mouseState = input.GetMouseState();
    if (mouseState == nullptr)
    {
        return;
    }

    const float width  = static_cast<float>(screenWidth);
    const float height = static_cast<float>(screenHeight);
    const float bx     = width  * 0.5f - kBoxWidth  * 0.5f;
    const float by     = height * 0.5f - kBoxHeight * 0.5f;
    const float mx     = static_cast<float>(mouseState->x);
    const float my     = static_cast<float>(mouseState->y);

    for (int index = 0; index < kMenuItemCount; ++index)
    {
        const float itemY = by + kItemTopOffset + static_cast<float>(index) * kItemSpacing;
        if (mx >= bx + kItemPaddingX && mx <= bx + kBoxWidth - kItemPaddingX &&
            my >= itemY - kItemPaddingY && my <= itemY + kItemHeight)
        {
            m_selectedIndex = index;
            break;
        }
    }
}

void GameScenePauseController::NavigateMenuByKeyboard()
{
    const System::InputManager& input = System::InputManager::GetInstance();

    if (input.IsKeyPressed(DirectX::Keyboard::Up) || input.IsKeyPressed(DirectX::Keyboard::W))
    {
        m_selectedIndex = (m_selectedIndex + kMenuItemCount - 1) % kMenuItemCount;
    }
    if (input.IsKeyPressed(DirectX::Keyboard::Down) || input.IsKeyPressed(DirectX::Keyboard::S))
    {
        m_selectedIndex = (m_selectedIndex + 1) % kMenuItemCount;
    }
}
