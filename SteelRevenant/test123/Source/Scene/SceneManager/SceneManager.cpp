#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 繧ｷ繝ｼ繝ｳ縺ｮ蛻・ｊ譖ｿ縺医→逕溷ｭ俶悄髢薙ｒ螳溯｣・☆繧九・//------------------------
#include "SceneManager.h"
#include "../TitleScene/TitleScene.h"
#include "../GameScene/GameScene.h"
#include "../ResultScene/ResultScene.h"
#include "../StageSelectScene/StageSelectScene.h"
#include "../SettingsScene/SettingsScene.h"
#include "../../GameSystem/InputManager.h"

#include <Mouse.h>

extern SceneManager* g_sceneManager;

SceneManager::SceneManager()
{
    g_sceneManager = this;
}
SceneManager::~SceneManager()
{
    FinalizeActiveScene();
    if (m_suspendedScene)
    {
        m_suspendedScene->Finalize();
        m_suspendedScene.reset();
    }
    g_sceneManager = nullptr;
}

void SceneManager::SetScene(SceneID id)
{
    m_currentId = id;
    switch (id)
    {
    case GAME_SCENE:         m_activeScene = std::make_unique<GameScene>();        break;
    case RESULT_SCENE:       m_activeScene = std::make_unique<ResultScene>();      break;
    case STAGE_SELECT_SCENE: m_activeScene = std::make_unique<StageSelectScene>(this); break;
    case SETTINGS_SCENE:     m_activeScene = std::make_unique<SettingsScene>(this);    break;
    default:                 m_activeScene = std::make_unique<TitleScene>();       break;
    }
}

void SceneManager::InitilizeActiveScene()           { if (m_activeScene) m_activeScene->Initialize(); }
void SceneManager::RenderActiveSceneRender()        { if (m_activeScene) m_activeScene->Render(); }
void SceneManager::FinalizeActiveScene()            { if (m_activeScene) { m_activeScene->Finalize(); m_activeScene.reset(); } }

void SceneManager::UpdateActiveScene(const DX::StepTimer& timer)
{
    ApplyPendingTransition();
    if (m_activeScene) m_activeScene->Update(timer);
}

void SceneManager::ApplyPendingTransition()
{
    if (!m_hasNext) return;

    const SceneID requestedScene = m_nextScene;
    m_hasNext = false;

    if (m_currentId == GAME_SCENE && requestedScene == SETTINGS_SCENE)
    {
        m_previousId = m_currentId;
        m_suspendedId = m_currentId;
        m_suspendedScene = std::move(m_activeScene);
        SetScene(requestedScene);
        InitilizeActiveScene();
        return;
    }

    if (m_currentId == SETTINGS_SCENE && m_suspendedScene && requestedScene == m_suspendedId)
    {
        m_previousId = m_currentId;
        FinalizeActiveScene();
        m_activeScene = std::move(m_suspendedScene);
        m_currentId = m_suspendedId;
        m_suspendedId = TITLE_SCENE;
        DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
        DirectX::Mouse::Get().SetVisible(true);
        System::InputManager::GetInstance().ResetMouseDelta();
        return;
    }

    if (m_currentId == SETTINGS_SCENE && m_suspendedScene && requestedScene != m_suspendedId)
    {
        m_suspendedScene->Finalize();
        m_suspendedScene.reset();
        m_suspendedId = TITLE_SCENE;
    }

    m_previousId = m_currentId;
    FinalizeActiveScene();
    SetScene(requestedScene);
    InitilizeActiveScene();
}

