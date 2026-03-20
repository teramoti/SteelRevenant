//------------------------//------------------------
// Contents(処理内容) 効果音の読み込みと再生を実装する。
// DirectXTK Audio が存在しない場合は無音で継続する。
//------------------------//------------------------
#include "AudioSystem.h"
#include <filesystem>

#if __has_include(<Audio.h>)
#define AUDIO_ENABLED 1
#include <Audio.h>
#endif

namespace GameAudio
{
    struct AudioSystem::Impl
    {
#if AUDIO_ENABLED
        std::unique_ptr<DirectX::AudioEngine> engine;
        std::unordered_map<int, std::unique_ptr<DirectX::SoundEffect>> effects;
#endif
        bool active = false;
    };

    AudioSystem::AudioSystem() : m_impl(std::make_unique<Impl>()) {}

    void AudioSystem::Initialize(const std::wstring& root)
    {
#if AUDIO_ENABLED
        try
        {
            m_impl->engine = std::make_unique<DirectX::AudioEngine>(DirectX::AudioEngine_Default);
            m_impl->active = true;
            namespace fs = std::filesystem;
            for (int i = 0; i < static_cast<int>(SeId::Count); ++i)
            {
                const wchar_t* f = GetFileName(static_cast<SeId>(i));
                if (!f) continue;
                fs::path p = fs::path(root) / f;
                if (!fs::exists(p)) continue;
                try { m_impl->effects[i] = std::make_unique<DirectX::SoundEffect>(m_impl->engine.get(), p.wstring().c_str()); }
                catch (...) {}
            }
        }
        catch (...) {}
#else
        (void)root;
#endif
    }

    void AudioSystem::Update()
    {
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine) m_impl->engine->Update();
#endif
    }

    void AudioSystem::PlaySe(SeId id, float volume)
    {
#if AUDIO_ENABLED
        if (!m_impl->active || !m_impl->engine) return;
        auto it = m_impl->effects.find(static_cast<int>(id));
        if (it == m_impl->effects.end() || !it->second) return;
        it->second->Play(volume * m_vol.se * m_vol.master, 0.0f, 0.0f);
#else
        (void)id; (void)volume;
#endif
    }

    void AudioSystem::ApplyVolumeSettings(const AudioVolumeSettings& s)
    {
        m_vol = s;
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine) m_impl->engine->SetMasterVolume(s.master);
#endif
    }

    void AudioSystem::SuspendAll() {
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine) m_impl->engine->Suspend();
#endif
    }
    void AudioSystem::ResumeAll()  {
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine) m_impl->engine->Resume();
#endif
    }
    void AudioSystem::Shutdown()   {
#if AUDIO_ENABLED
        m_impl->effects.clear(); m_impl->engine.reset(); m_impl->active = false;
#endif
    }

    const wchar_t* AudioSystem::GetFileName(SeId id)
    {
        switch (id)
        {
        case SeId::MeleeSlash:   return L"slash.wav";
        case SeId::GuardBlock:   return L"block.wav";
        case SeId::PlayerHit:    return L"player_hit.wav";
        case SeId::EnemyDestroy: return L"enemy_destroy.wav";
        case SeId::ItemPickup:   return L"item_pickup.wav";
        case SeId::WaveClear:    return L"wave_clear.wav";
        case SeId::GameOver:     return L"game_over.wav";
        default:                 return nullptr;
        }
    }
}
