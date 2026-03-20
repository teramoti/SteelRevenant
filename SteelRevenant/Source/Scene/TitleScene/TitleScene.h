#pragma once
#include "../IScene.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>

class TitleScene : public IScene
{
public:
    void Initialize() override;
    void Update(const DX::StepTimer& timer) override;
    void Render()  override;
    void Finalize() override;

private:
    float m_timer   = 0.0f;
    float m_blink   = 0.0f;
    DirectX::SpriteBatch* m_spriteBatch = nullptr;
    DirectX::SpriteFont*  m_font        = nullptr;
};
