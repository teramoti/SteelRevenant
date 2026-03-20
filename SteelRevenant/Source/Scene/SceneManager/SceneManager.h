#pragma once
#include <memory>
#include "../IScene.h"

enum SceneID
{
    TITLE_SCENE        = 0,
    GAME_SCENE         = 1,
    RESULT_SCENE       = 2,
    STAGE_SELECT_SCENE = 3,
    SETTINGS_SCENE     = 4,
};

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();
    void SetScene(SceneID id);
    void InitilizeActiveScene();
    void UpdateActiveScene(const DX::StepTimer& timer);
    void RenderActiveSceneRender();
    void FinalizeActiveScene();
    void RequestTransition(SceneID id) { m_nextScene = id; m_hasNext = true; }

private:
    void ApplyPendingTransition();
    std::unique_ptr<IScene> m_activeScene;
    SceneID m_currentId = TITLE_SCENE;
    SceneID m_nextScene = TITLE_SCENE;
    bool    m_hasNext   = false;
};
