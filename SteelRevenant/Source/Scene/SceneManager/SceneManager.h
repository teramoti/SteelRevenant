#pragma once
#include <memory>
#include "../IScene.h"

// シーン識別子。
enum SceneID
{
    TITLE_SCENE  = 0,
    GAME_SCENE   = 1,
    RESULT_SCENE = 2,
};

// アクティブシーンの切り替えと生存期間を管理する。
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

    // シーンからの遷移要求を受け付ける。
    void RequestTransition(SceneID id) { m_nextScene = id; m_hasNext = true; }

private:
    void ApplyPendingTransition();

    std::unique_ptr<IScene> m_activeScene;
    SceneID                 m_currentId;
    SceneID                 m_nextScene;
    bool                    m_hasNext;
};
