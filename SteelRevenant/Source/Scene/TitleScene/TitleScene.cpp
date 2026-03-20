//------------------------//------------------------
// Contents(処理内容) タイトルシーンの更新・描画を実装する。
//------------------------//------------------------
#include "TitleScene.h"
#include "../../Utility/DirectX11.h"
#include "../../GameSystem/DrawManager.h"
#include "../SceneManager/SceneManager.h"
#include "../../GameSystem/InputManager.h"
#include <Keyboard.h>
#include <cmath>

extern SceneManager* g_sceneManager;

void TitleScene::Initialize()
{
    m_timer = 0.0f;
    m_blink = 0.0f;
}

void TitleScene::Update(const DX::StepTimer& timer)
{
    const float dt = static_cast<float>(timer.GetElapsedSeconds());
    m_timer += dt;
    m_blink += dt;

    const System::InputManager& input = System::InputManager::GetInstance();
    if (input.IsKeyPressed(DirectX::Keyboard::Enter)
        || input.IsKeyPressed(DirectX::Keyboard::Space)
        || input.IsMouseButtonPressed(0))
    {
        if (g_sceneManager) g_sceneManager->RequestTransition(GAME_SCENE);
    }
}

void TitleScene::Render()
{
    // SpriteBatch は Game.cpp の Render() 内で Begin 済み
    // DrawManager 経由でコンテキストを取得
    ID3D11DeviceContext* context = DirectX11::Get().GetContext().Get();
    (void)context;

    // フォントが注入されていない場合は描画スキップ
    if (!m_font || !m_spriteBatch) return;

    const float W = static_cast<float>(DirectX11::Get().GetWidth());
    const float H = static_cast<float>(DirectX11::Get().GetHeight());

    // タイトル
    m_font->DrawString(m_spriteBatch, L"SteelRevenant",
        DirectX::XMFLOAT2(W * 0.5f - 110.0f, H * 0.35f),
        DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0,0), 1.6f);

    // 点滅テキスト
    const float alpha = 0.5f + 0.5f * std::sin(m_blink * 3.0f);
    m_font->DrawString(m_spriteBatch, L"Press ENTER to Start",
        DirectX::XMFLOAT2(W * 0.5f - 120.0f, H * 0.58f),
        DirectX::XMVECTOR{ alpha, alpha, alpha, alpha },
        0.0f, DirectX::XMFLOAT2(0,0), 1.0f);

    // 操作説明
    m_font->DrawString(m_spriteBatch, L"WASD: Move   Mouse: Aim   LClick: Attack   RClick: Guard   Tab: Lock",
        DirectX::XMFLOAT2(20.0f, H - 36.0f),
        DirectX::XMVECTOR{ 0.6f, 0.6f, 0.6f, 1.0f },
        0.0f, DirectX::XMFLOAT2(0,0), 0.7f);
}

void TitleScene::Finalize() {}
