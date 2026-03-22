#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GameScene.h"

#include "../../Utility/DirectX11.h"

void GameScene::DrawWorld()
{
    if (!m_floorMesh || !m_playerMesh || !m_enemyMesh || !m_weaponMesh || !m_skyMesh || !m_effectOrbMesh || !m_effectTrailMesh)
    {
        return;
    }

    DrawWorldBackdrop();
    DrawWorldArena();
    DrawSpeedUpItems();
    DrawWorldActors();
}

void GameScene::DrawSpeedUpItems()
{
    m_arenaLayer.Render(DirectX11::Get().GetContext().Get(), m_view, m_proj);
}

