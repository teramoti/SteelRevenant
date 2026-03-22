#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <string>
#include <memory>
#include <unordered_map>
#include "../SingletonBase.h"
#include "AudioTypes.h"

namespace GameAudio
{

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

        // BGM control
        void PlayBgm(BgmId id);
        void StopBgm();

    private:
        AudioSystem();
        ~AudioSystem();
        static const wchar_t* GetFileName(SeId id);
        static const wchar_t* GetFileName(BgmId id);
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        AudioVolumeSettings   m_vol;
    };
}


