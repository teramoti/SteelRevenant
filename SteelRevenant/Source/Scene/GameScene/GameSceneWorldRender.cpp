//------------------------//------------------------
// Contents(処理内容) 本編ゲームシーンのワールド描画 dispatcher を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "GameScene.h"

// 実描画は Visual フォルダ配下の Backdrop / Arena / Actors に分離する。
void GameScene::DrawWorld()
{
	if (!m_floorMesh || !m_playerMesh || !m_enemyMesh || !m_weaponMesh || !m_skyMesh || !m_effectOrbMesh || !m_effectTrailMesh)
	{
		return;
	}

	DrawWorldBackdrop();
	DrawWorldArena();
	DrawWorldActors();
}
