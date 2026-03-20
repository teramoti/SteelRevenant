//------------------------//------------------------
// Contents(処理内容) バトルの進行状態遷移を実装する。
// Bug#4修正: 中継地点・ビーコン・ハザード残存コードなし。
//------------------------//------------------------
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
