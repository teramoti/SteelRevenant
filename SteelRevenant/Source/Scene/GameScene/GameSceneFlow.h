#pragma once

// バトルの進行状態。
enum class BattlePhase
{
    Countdown,   // 開始カウントダウン
    Battle,      // 戦闘中
    Result,      // 結果表示中
};

// バトル進行フローを管理するクラス。
// Bug#4修正: 中継地点・ビーコン残存コードを含まない実装。
class GameSceneFlow
{
public:
    void Reset(float startTimeSec);
    void Update(float dt, bool stageFinished);

    BattlePhase GetPhase()          const { return m_phase; }
    float       GetCountdownTimer() const { return m_countdownTimer; }
    float       GetResultTimer()    const { return m_resultTimer; }
    bool        IsReadyToExit()     const { return m_readyToExit; }

private:
    BattlePhase m_phase          = BattlePhase::Countdown;
    float       m_countdownTimer = 3.0f;
    float       m_resultTimer    = 0.0f;
    bool        m_readyToExit    = false;

    static constexpr float kResultDelaySec = 3.0f;
};
