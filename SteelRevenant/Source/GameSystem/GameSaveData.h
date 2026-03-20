#pragma once
#include "../Utility/SingletonBase.h"
#include "../Utility/Sound/AudioSystem.h"

class GameSaveData : public Utility::SingletonBase<GameSaveData>
{
    friend class Utility::SingletonBase<GameSaveData>;
public:
    float GetMouseSensitivity() const { return m_mouseSensitivity; }
    void  SetMouseSensitivity(float v) { m_mouseSensitivity = v; }
    GameAudio::AudioVolumeSettings GetAudioVolumeSettings() const { return m_audio; }
    void SetAudioVolumeSettings(const GameAudio::AudioVolumeSettings& s) { m_audio = s; }
private:
    GameSaveData() = default;
    float m_mouseSensitivity = 0.5f;
    GameAudio::AudioVolumeSettings m_audio;
};
