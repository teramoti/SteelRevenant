#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "../SingletonBase.h"

namespace GameAudio
{
    enum class SeId { MeleeSlash, GuardBlock, PlayerHit, EnemyDestroy, ItemPickup, WaveClear, GameOver, Count };
    struct AudioVolumeSettings { float master = 1.0f, se = 1.0f, bgm = 0.7f; };

    class AudioSystem : public Utility::SingletonBase<AudioSystem>
    {
        friend class Utility::SingletonBase<AudioSystem>;
    public:
        void Initialize(const std::wstring& audioRootPath);
        void Update();
        void PlaySe(SeId id, float volume = 1.0f);
        void ApplyVolumeSettings(const AudioVolumeSettings& s);
        void SuspendAll();
        void ResumeAll();
        void Shutdown();

    private:
        AudioSystem();
        ~AudioSystem() = default;
        static const wchar_t* GetFileName(SeId id);
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        AudioVolumeSettings   m_vol;
    };
}
