#pragma once
#include "../Utility/SingletonBase.h"
#include "../Utility/Sound/AudioSystem.h"

// セーブデータ（マウス感度・音量）を管理するシングルトン。
// 今回はデフォルト値固定（保存機能は将来拡張）。
class GameSaveData : public Utility::SingletonBase<GameSaveData>
{
    friend class Utility::SingletonBase<GameSaveData>;
public:
    float GetMouseSensitivity() const { return m_mouseSensitivity; }
    void  SetMouseSensitivity(float v) { m_mouseSensitivity = v; }

    GameAudio::AudioVolumeSettings GetAudioVolumeSettings() const { return m_audioVolume; }
    void SetAudioVolumeSettings(const GameAudio::AudioVolumeSettings& s) { m_audioVolume = s; }

private:
    GameSaveData() = default;

    float m_mouseSensitivity = 0.5f;
    GameAudio::AudioVolumeSettings m_audioVolume;
};
