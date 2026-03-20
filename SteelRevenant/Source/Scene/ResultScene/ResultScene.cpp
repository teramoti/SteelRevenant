//------------------------//------------------------
// Contents(処理内容) リザルトシーンの更新・描画を実装する。
//------------------------//------------------------
#include "ResultScene.h"
#include "../../Utility/DirectX11.h"
#include "../../GameSystem/InputManager.h"
#include "../SceneManager/SceneManager.h"
#include <Keyboard.h>

extern SceneManager* g_sceneManager;

Action::GameState ResultScene::s_lastResult;

void ResultScene::Initialize()
{
    m_timer    = 0.0f;
    m_exitTimer = 6.0f;
}

void ResultScene::Update(const DX::StepTimer& timer)
{
    const float dt = static_cast<float>(timer.GetElapsedSeconds());
    m_timer    += dt;
    m_exitTimer -= dt;

    const System::InputManager& input = System::InputManager::GetInstance();
    if (m_exitTimer <= 0.0f
        || input.IsKeyPressed(DirectX::Keyboard::Enter)
        || input.IsKeyPressed(DirectX::Keyboard::Space)
        || input.IsMouseButtonPressed(0))
    {
        if (g_sceneManager) g_sceneManager->RequestTransition(TITLE_SCENE);
    }
}

void ResultScene::Render()
{
    if (!m_font || !m_spriteBatch) return;

    const float W = static_cast<float>(DirectX11::Get().GetWidth());
    const float H = static_cast<float>(DirectX11::Get().GetHeight());
    const Action::GameState& gs = s_lastResult;

    // タイトル
    const wchar_t* title = gs.stageCleared ? L"STAGE CLEAR!" : L"TIME OVER";
    const DirectX::XMVECTOR titleCol = gs.stageCleared
        ? DirectX::XMVECTOR{0.3f,1.0f,0.5f,1.0f}
        : DirectX::XMVECTOR{1.0f,0.3f,0.3f,1.0f};
    m_font->DrawString(m_spriteBatch, title,
        DirectX::XMFLOAT2(W*0.5f-90.0f, H*0.25f), titleCol, 0.0f,
        DirectX::XMFLOAT2(0,0), 1.8f);

    // スコア
    wchar_t buf[64];
    swprintf_s(buf, L"SCORE     %06d", gs.score);
    m_font->DrawString(m_spriteBatch, buf,
        DirectX::XMFLOAT2(W*0.5f-140.0f, H*0.42f), DirectX::Colors::White,
        0.0f, DirectX::XMFLOAT2(0,0), 1.1f);

    swprintf_s(buf, L"KILLS     %d", gs.killCount);
    m_font->DrawString(m_spriteBatch, buf,
        DirectX::XMFLOAT2(W*0.5f-140.0f, H*0.50f), DirectX::Colors::LightGray,
        0.0f, DirectX::XMFLOAT2(0,0), 1.0f);

    swprintf_s(buf, L"DANGER LV %d", gs.peakDangerLevel);
    m_font->DrawString(m_spriteBatch, buf,
        DirectX::XMFLOAT2(W*0.5f-140.0f, H*0.57f), DirectX::Colors::LightGray,
        0.0f, DirectX::XMFLOAT2(0,0), 1.0f);

    swprintf_s(buf, L"DAMAGE    %d", gs.damageTaken);
    m_font->DrawString(m_spriteBatch, buf,
        DirectX::XMFLOAT2(W*0.5f-140.0f, H*0.64f), DirectX::Colors::LightGray,
        0.0f, DirectX::XMFLOAT2(0,0), 1.0f);

    // タイトルへ戻るメッセージ
    swprintf_s(buf, L"Press ENTER to return  (%.0fs)", std::max(0.0f, m_exitTimer));
    m_font->DrawString(m_spriteBatch, buf,
        DirectX::XMFLOAT2(W*0.5f-150.0f, H*0.78f),
        DirectX::XMVECTOR{0.7f,0.7f,0.7f,1.0f},
        0.0f, DirectX::XMFLOAT2(0,0), 0.85f);
}

void ResultScene::Finalize() {}
