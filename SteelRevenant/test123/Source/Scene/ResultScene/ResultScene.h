#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../IScene.h"
#include "../../Action/GameState.h"

#include <d3d11.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <string>
#include <vector>
#include <wrl/client.h>

class ResultScene : public IScene
{
public:
    void Initialize() override;
    void Update(const DX::StepTimer& timer) override;
    void Render() override;
    void Finalize() override;

    static Action::GameState s_lastResult;
    static int s_stageIndex;
    static int s_reachedWave;

private:
    float m_timer = 0.0f;
    float m_exitTimer = 5.0f;
    DirectX::SpriteBatch* m_spriteBatch = nullptr;
    DirectX::SpriteFont* m_font = nullptr;
    int m_selectedIndex = 0;
    std::vector<std::wstring> m_menuItems = { L"リトライ", L"タイトルへ" };
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_whiteTex;
};
