#pragma once
#include "../IScene.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "../../Action/GameState.h"

class ResultScene : public IScene
{
public:
    void Initialize() override;
    void Update(const DX::StepTimer& timer) override;
    void Render()  override;
    void Finalize() override;

    // GameScene から結果を受け取る（グローバル経由）
    static Action::GameState s_lastResult;

private:
    float m_timer     = 0.0f;
    float m_exitTimer = 5.0f;
    DirectX::SpriteBatch* m_spriteBatch = nullptr;
    DirectX::SpriteFont*  m_font        = nullptr;
};
