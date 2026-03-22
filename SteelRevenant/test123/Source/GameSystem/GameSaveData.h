#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../Utility/SingletonBase.h"
#include "../Utility/Sound/AudioSystem.h"

#include <array>
#include <string>

struct BattleResultData
{
    int   stageIndex  = 0;
    int   score       = 0;
    int   killCount   = 0;
    float survivalSec = 0.0f;
    bool  cleared     = false;
};

struct AppSettingsData
{
    float                          mouseSensitivity = 0.12f;
    GameAudio::AudioVolumeSettings audioVolume;
};

class GameSaveData : public Utility::SingletonBase<GameSaveData>
{
    friend class Utility::SingletonBase<GameSaveData>;

public:
    void SetStageNum(int num);
    int GetStageNum() const { return m_stageNum; }

    void SetLoadName(std::wstring name);
    void SetLoadColName(std::wstring name);
    void SetLoadEnemyName(std::wstring name);

    void SetBattleResult(const BattleResultData& result);
    const BattleResultData& GetBattleResult() const;

    bool UpdateBestScore(int stageIndex, int score);
    int GetBestScore(int stageIndex) const;

    void SetAppSettings(const AppSettingsData& settings);
    const AppSettingsData& GetAppSettings() const;

    void SetMouseSensitivity(float value);
    float GetMouseSensitivity() const;

    void SetAudioVolumeSettings(const GameAudio::AudioVolumeSettings& settings);
    const GameAudio::AudioVolumeSettings& GetAudioVolumeSettings() const;

    void SaveToDisk() const;
    void LoadFromDisk();

private:
    GameSaveData();

    int                m_stageNum = 1;
    std::wstring       m_name;
    std::wstring       m_colName;
    std::wstring       m_enemyName;
    BattleResultData   m_battleResult;
    std::array<int, 3> m_bestScore = { 0, 0, 0 };
    AppSettingsData    m_appSettings;
};

