//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 繧ｷ繝ｼ繝ｳUI縺ｧ蜈ｱ騾壼茜逕ｨ縺吶ｋ遘ｻ蜍輔∵ｱｺ螳壹∵綾繧九・蜉ｹ譫憺浹繧偵∪縺ｨ繧√ｋ縲・//------------------------
// user(菴懈・閠・ Keishi Teramoto
// Created date(菴懈・譌･) 2026 / 03 / 20
// last updated (譛邨よ峩譁ｰ譌･) 2026 / 03 / 20
//------------------------
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

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

