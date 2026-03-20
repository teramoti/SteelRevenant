//------------------------//------------------------
// Contents(処理内容) シーンUIで共通利用する移動、決定、戻るの効果音をまとめる。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 20
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#pragma once

#include "../../Utility/Sound/AudioSystem.h"

namespace SceneUiSound
{
	inline void PlayMove()
	{
		GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::UiMove);
	}

	inline void PlayConfirm()
	{
		GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::UiConfirm);
	}

	inline void PlayBack()
	{
		GameAudio::AudioSystem::GetInstance().PlaySe(GameAudio::SeId::UiBack);
	}
}
