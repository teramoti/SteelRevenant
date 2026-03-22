#ifndef NOMINMAX
#define NOMINMAX
#endif
//------------------------
// Contents(蜃ｦ逅・・螳ｹ) 繝舌ヨ繝ｫ縺ｮ騾ｲ陦檎憾諷矩・遘ｻ繧貞ｮ溯｣・☆繧九・// Bug#4菫ｮ豁｣: 荳ｭ邯吝慍轤ｹ繝ｻ繝薙・繧ｳ繝ｳ繝ｻ繝上じ繝ｼ繝画ｮ句ｭ倥さ繝ｼ繝峨↑縺励・//------------------------
#include "GameSceneFlow.h"
#include <algorithm>

void GameSceneFlow::Reset(float startTimeSec)
{
    m_phase          = BattlePhase::Countdown;
    m_countdownTimer = 3.0f;
    m_resultTimer    = 0.0f;
    m_readyToExit    = false;
    (void)startTimeSec;
}

void GameSceneFlow::Update(float dt, bool stageFinished)
{
    switch (m_phase)
    {
    case BattlePhase::Countdown:
        m_countdownTimer -= dt;
        if (m_countdownTimer <= 0.0f)
        {
            m_countdownTimer = 0.0f;
            m_phase = BattlePhase::Battle;
        }
        break;

    case BattlePhase::Battle:
        if (stageFinished)
        {
            m_phase       = BattlePhase::Result;
            m_resultTimer = kResultDelaySec;
        }
        break;

    case BattlePhase::Result:
        m_resultTimer -= dt;
        if (m_resultTimer <= 0.0f)
            m_readyToExit = true;
        break;
    }
}

