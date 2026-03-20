//------------------------//------------------------
// Contents(処理内容) シーンの切り替えと生存期間を実装する。
//------------------------//------------------------
#include "SceneManager.h"
#include "../TitleScene/TitleScene.h"
#include "../GameScene/GameScene.h"
#include "../ResultScene/ResultScene.h"
#include "../StageSelectScene/StageSelectScene.h"
#include "../SettingsScene/SettingsScene.h"

SceneManager::SceneManager()  = default;
SceneManager::~SceneManager() { FinalizeActiveScene(); }

void SceneManager::SetScene(SceneID id)
{
    m_currentId = id;
    switch (id)
    {
    case GAME_SCENE:         m_activeScene = std::make_unique<GameScene>();        break;
    case RESULT_SCENE:       m_activeScene = std::make_unique<ResultScene>();      break;
    case STAGE_SELECT_SCENE: m_activeScene = std::make_unique<StageSelectScene>(); break;
    case SETTINGS_SCENE:     m_activeScene = std::make_unique<SettingsScene>();    break;
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
    m_hasNext = false;
    FinalizeActiveScene();
    SetScene(m_nextScene);
    InitilizeActiveScene();
}
