//------------------------//------------------------
// Contents(処理内容) シーンの切り替えと生存期間を実装する。
//------------------------//------------------------
#include "SceneManager.h"
#include "../TitleScene/TitleScene.h"
#include "../GameScene/GameScene.h"
#include "../ResultScene/ResultScene.h"

SceneManager::SceneManager()
    : m_currentId(TITLE_SCENE)
    , m_nextScene(TITLE_SCENE)
    , m_hasNext(false)
{
}

SceneManager::~SceneManager()
{
    FinalizeActiveScene();
}

// 指定 ID のシーンを即時生成する（Initialize は呼ばない）。
void SceneManager::SetScene(SceneID id)
{
    m_currentId = id;
    switch (id)
    {
    case TITLE_SCENE:  m_activeScene = std::make_unique<TitleScene>();  break;
    case GAME_SCENE:   m_activeScene = std::make_unique<GameScene>();   break;
    case RESULT_SCENE: m_activeScene = std::make_unique<ResultScene>(); break;
    default:           m_activeScene = std::make_unique<TitleScene>();  break;
    }
}

void SceneManager::InitilizeActiveScene()
{
    if (m_activeScene) m_activeScene->Initialize();
}

void SceneManager::UpdateActiveScene(const DX::StepTimer& timer)
{
    ApplyPendingTransition();
    if (m_activeScene) m_activeScene->Update(timer);
}

void SceneManager::RenderActiveSceneRender()
{
    if (m_activeScene) m_activeScene->Render();
}

void SceneManager::FinalizeActiveScene()
{
    if (m_activeScene)
    {
        m_activeScene->Finalize();
        m_activeScene.reset();
    }
}

// 保留中のシーン遷移を適用する。
void SceneManager::ApplyPendingTransition()
{
    if (!m_hasNext) return;
    m_hasNext = false;

    FinalizeActiveScene();
    SetScene(m_nextScene);
    InitilizeActiveScene();
}
