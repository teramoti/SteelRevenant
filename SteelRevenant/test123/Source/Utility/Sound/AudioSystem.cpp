#ifndef NOMINMAX
#define NOMINMAX
#endif
// AudioSystem - wrapper around DirectXTK Audio for SE and BGM playback
// This file implements effect (SE) loading and basic background music (BGM) support.

#include "AudioSystem.h"
#include <filesystem>
#include <fstream>
#include <ctime>
#include <mutex>

namespace {
    static void LogAudioEvent(const char* /*kind*/, const wchar_t* /*file*/, bool /*loaded*/, int /*id*/)
    {
    }
}

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
        // BGM storage: keep SoundEffect so instances remain valid, and instances for playback
        std::unordered_map<int, std::unique_ptr<DirectX::SoundEffect>> bgmEffects;
        std::unordered_map<int, std::unique_ptr<DirectX::SoundEffectInstance>> bgmInstances;
#endif
        bool active = false;
        int currentBgm = -1;
    };

    AudioSystem::AudioSystem() : m_impl(std::make_unique<Impl>()) {}

    AudioSystem::~AudioSystem() = default;

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
                if (!fs::exists(p)) {
                    LogAudioEvent("LoadSE", f, false, i);
                    continue;
                }
                try { m_impl->effects[i] = std::make_unique<DirectX::SoundEffect>(m_impl->engine.get(), p.wstring().c_str()); }
                catch (...) { m_impl->effects[i].reset(); }
                LogAudioEvent("LoadSE", f, m_impl->effects[i] != nullptr, i);
            }

            // load bgm files if available
            for (int i = 0; i <= static_cast<int>(BgmId::ResultFail); ++i)
            {
                const wchar_t* bf = GetFileName(static_cast<BgmId>(i));
                if (!bf) continue;
                fs::path bp = fs::path(root) / bf;
                if (!fs::exists(bp)) { LogAudioEvent("LoadBGM", bf, false, i); continue; }
                try {
                    m_impl->bgmEffects[i] = std::make_unique<DirectX::SoundEffect>(m_impl->engine.get(), bp.wstring().c_str());
                    auto inst = m_impl->bgmEffects[i]->CreateInstance();
                    if (inst) {
                        m_impl->bgmInstances[i] = std::move(inst);
                    }
                }
                catch (...) { m_impl->bgmEffects[i].reset(); m_impl->bgmInstances[i].reset(); }
                LogAudioEvent("LoadBGM", bf, (m_impl->bgmEffects[i] != nullptr), i);
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
        LogAudioEvent("PlaySE", GetFileName(id), true, static_cast<int>(id));
#else
        (void)id; (void)volume;
#endif
    }

    void AudioSystem::PlayBgm(BgmId id)
    {
#if AUDIO_ENABLED
        if (!m_impl->active || !m_impl->engine) return;
        const int key = static_cast<int>(id);
        if (m_impl->currentBgm == key) return; // already playing

        // stop current
        if (m_impl->currentBgm >= 0)
        {
            auto itOld = m_impl->bgmInstances.find(m_impl->currentBgm);
            if (itOld != m_impl->bgmInstances.end() && itOld->second) {
                try { itOld->second->Stop(); } catch(...) {}
            }
            m_impl->currentBgm = -1;
        }

        auto it = m_impl->bgmInstances.find(key);
        if (it != m_impl->bgmInstances.end() && it->second)
        {
            try { it->second->Play(true); m_impl->currentBgm = key; } catch(...) { try { it->second->Play(); m_impl->currentBgm = key; } catch(...) {} }
        }
        else
        {
            // fallback: if no instance but effect exists, play one-shot (non-looped)
            auto efIt = m_impl->bgmEffects.find(key);
            if (efIt != m_impl->bgmEffects.end() && efIt->second)
            {
                try { efIt->second->Play(m_vol.bgm * m_vol.master, 0.0f, 0.0f); m_impl->currentBgm = key; } catch(...) {}
            }
        }
        LogAudioEvent("PlayBgm", GetFileName(id), true, static_cast<int>(id));
#else
        (void)id;
#endif
    }

    void AudioSystem::StopBgm()
    {
#if AUDIO_ENABLED
        if (!m_impl->active || !m_impl->engine) return;
        if (m_impl->currentBgm >= 0)
        {
            auto it = m_impl->bgmInstances.find(m_impl->currentBgm);
            if (it != m_impl->bgmInstances.end() && it->second) {
                try { it->second->Stop(); } catch(...) {}
            }
            m_impl->currentBgm = -1;
        }
#endif
    }

    void AudioSystem::ApplyVolumeSettings(const AudioVolumeSettings& s)
    {
        m_vol = s;
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine) m_impl->engine->SetMasterVolume(s.master);
        // set bgm instance volumes
        for (auto& kv : m_impl->bgmInstances) {
            try { if (kv.second) kv.second->SetVolume(s.bgm * s.master); } catch(...) {}
        }
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
        // stop bgm
        for (auto& kv : m_impl->bgmInstances) { try { if (kv.second) kv.second->Stop(); } catch(...) {} }
        m_impl->bgmInstances.clear(); m_impl->bgmEffects.clear();
        m_impl->effects.clear(); m_impl->engine.reset(); m_impl->active = false;
#endif
    }

    const wchar_t* AudioSystem::GetFileName(SeId id)
    {
        switch (id)
        {
        case SeId::MeleeSlash:    return L"Se/Combat/melee_slash.wav";
        case SeId::GuardBlock:    return L"Se/Combat/guard_block.wav";
        case SeId::PlayerHit:     return L"Se/Player/player_hit.wav";
        case SeId::HitPlayer:     return L"Se/Player/player_hit.wav";
        case SeId::HitEnemy:      return L"Se/Combat/hit_enemy.wav";
        case SeId::EnemyDestroy:  return L"Se/Combat/enemy_destroy.wav";
        case SeId::EnemyDie:      return L"Se/Enemy/enemy_die.wav";
        case SeId::EnemySpawn:    return L"Se/Enemy/enemy_spawn.wav";
        case SeId::EnemyShot:     return L"Se/Enemy/enemy_shot.wav";
        case SeId::PlayerShot:    return L"Se/Player/player_shot.wav";
        case SeId::LaserCharge:   return L"Se/Enemy/laser_charge.wav";
        case SeId::LaserFire:     return L"Se/Enemy/laser_fire.wav";
        case SeId::ChargeWarning: return L"Se/System/charge_warning.wav";
        case SeId::DashSlide:     return L"Se/Player/dash_slide.wav";
        case SeId::WarningAlert:  return L"Se/System/warning_alert.wav";
        case SeId::TimeLow:       return L"Se/System/time_low.wav";
        case SeId::BonusTime:     return L"Se/System/bonus_time.wav";
        case SeId::BeaconHeal:    return L"Se/Item/beacon_heal.wav";
        case SeId::RelayStart:    return L"Se/System/relay_start.wav";
        case SeId::RelaySecure:   return L"Se/System/relay_secure.wav";
        case SeId::Combo1:        return L"Se/Combat/combo1.wav";
        case SeId::Combo2:        return L"Se/Combat/combo2.wav";
        case SeId::Combo3:        return L"Se/Combat/combo3.wav";
        case SeId::UiMove:        return L"Se/Ui/ui_move.wav";
        case SeId::UiSelect:      return L"Se/Ui/ui_select.wav";
        case SeId::UiConfirm:     return L"Se/Ui/ui_confirm.wav";
        case SeId::UiCancel:      return L"Se/Ui/ui_cancel.wav";
        case SeId::UiBack:        return L"Se/Ui/ui_back.wav";
        case SeId::StageSelect:   return L"Bgm/title/stage_select.wav";
        case SeId::ResultClear:   return L"Bgm/result/result_clear.wav";
        case SeId::ResultFail:    return L"Bgm/result/result_fail.wav";
        case SeId::ItemPickup:    return L"Se/Item/item_pickup.wav";
        case SeId::WaveClear:     return L"Se/System/wave_clear.wav";
        case SeId::GameOver:      return L"Bgm/result/game_over.wav";
        default:                  return nullptr;
        }
    }

    const wchar_t* AudioSystem::GetFileName(BgmId id)
    {
        switch (id)
        {
        case BgmId::Title:           return L"Bgm/title/title_theme.wav";
        case BgmId::StageSelect:     return L"Bgm/title/stage_select.wav";
        case BgmId::DefenseCorridor: return L"Bgm/stage/defense_corridor.wav";
        case BgmId::CoreSector:      return L"Bgm/stage/core_sector.wav";
        case BgmId::ResultClear:     return L"Bgm/result/result_clear.wav";
        case BgmId::ResultFail:      return L"Bgm/result/result_fail.wav";
        default:                     return nullptr;
        }
    }
}

