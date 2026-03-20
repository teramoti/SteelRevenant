#include "AudioSystem.h"

#include <algorithm>
#include <exception>

using namespace DirectX;

namespace GameAudio
{
    namespace
    {
        constexpr float kMinPitch = -1.0f;
        constexpr float kMaxPitch = 1.0f;
        constexpr float kMinPan = -1.0f;
        constexpr float kMaxPan = 1.0f;
    }

    float AudioSystem::Clamp01(float value) const noexcept
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    float AudioSystem::GetBusVolume(AudioBus bus) const noexcept
    {
        switch (bus)
        {
        case AudioBus::Bgm:
            return m_volume.bgm;
        case AudioBus::Se:
            return m_volume.se;
        case AudioBus::Ui:
            return m_volume.ui;
        default:
            return 1.0f;
        }
    }

    float AudioSystem::GetEffectiveOneShotVolume(const AudioCueDef& cue, float volumeScale) const noexcept
    {
        const float busVolume = GetBusVolume(cue.bus);
        const float scale = std::max(0.0f, volumeScale);
        const float finalVolume = cue.defaultVolume * busVolume * scale;
        return Clamp01(finalVolume);
    }

    float AudioSystem::GetEffectiveInstanceVolume(const AudioCueDef& cue) const noexcept
    {
        const float busVolume = GetBusVolume(cue.bus);
        const float finalVolume = cue.defaultVolume * busVolume;
        return Clamp01(finalVolume);
    }

    bool AudioSystem::Initialize(const std::wstring& audioRoot)
    {
        try
        {
            AUDIO_ENGINE_FLAGS flags = AudioEngine_Default;

        #ifdef _DEBUG
            flags |= AudioEngine_Debug;
        #endif

            m_engine = std::make_unique<AudioEngine>(flags);

            BuildCatalog(audioRoot);

            if (!LoadAll())
            {
                Shutdown();
                return false;
            }

            m_engine->SetMasterVolume(Clamp01(m_volume.master));

            m_initialized = true;
            return true;
        }
        catch (const std::exception&)
        {
            Shutdown();
            return false;
        }
    }

    void AudioSystem::Shutdown()
    {
        if (!m_engine && !m_initialized)
        {
            return;
        }

        StopBgm(true);

        if (m_engine)
        {
            m_engine->Suspend();
        }

        m_currentBgm.reset();
        m_bgmEffects.clear();
        m_seEffects.clear();
        m_bgmDefs.clear();
        m_seDefs.clear();

        if (m_engine)
        {
            m_engine->TrimVoicePool();
        }

        m_engine.reset();
        m_initialized = false;
    }

    void AudioSystem::BuildCatalog(const std::wstring& audioRoot)
    {
        m_bgmDefs.clear();
        m_seDefs.clear();

        const std::wstring root = audioRoot;

        // training_zone は登録しません。
        m_bgmDefs.emplace(BgmId::Title, AudioCueDef{
            root + L"/title_theme.wav",
            AudioBus::Bgm,
            true,
            1.0f
        });

        m_bgmDefs.emplace(BgmId::StageSelect, AudioCueDef{
            root + L"/stage_select.wav",
            AudioBus::Bgm,
            true,
            1.0f
        });

        m_bgmDefs.emplace(BgmId::DefenseCorridor, AudioCueDef{
            root + L"/defense_corridor.wav",
            AudioBus::Bgm,
            true,
            1.0f
        });

        m_bgmDefs.emplace(BgmId::CoreSector, AudioCueDef{
            root + L"/core_sector.wav",
            AudioBus::Bgm,
            true,
            1.0f
        });

        m_bgmDefs.emplace(BgmId::ResultClear, AudioCueDef{
            root + L"/result_clear.wav",
            AudioBus::Bgm,
            false,
            1.0f
        });

        m_bgmDefs.emplace(BgmId::ResultFail, AudioCueDef{
            root + L"/result_fail.wav",
            AudioBus::Bgm,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::UiMove, AudioCueDef{
            root + L"/ui_move.wav",
            AudioBus::Ui,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::UiConfirm, AudioCueDef{
            root + L"/ui_confirm.wav",
            AudioBus::Ui,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::UiBack, AudioCueDef{
            root + L"/ui_back.wav",
            AudioBus::Ui,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::PlayerShot, AudioCueDef{
            root + L"/player_shot.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::EnemyShot, AudioCueDef{
            root + L"/enemy_shot.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::PlayerHit, AudioCueDef{
            root + L"/player_hit.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::GuardBlock, AudioCueDef{
            root + L"/guard_block.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::MeleeSlash, AudioCueDef{
            root + L"/melee_slash.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::EnemyDestroy, AudioCueDef{
            root + L"/enemy_destroy.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::RelayStart, AudioCueDef{
            root + L"/relay_start.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::RelaySecure, AudioCueDef{
            root + L"/relay_secure.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::BeaconHeal, AudioCueDef{
            root + L"/beacon_heal.wav",
            AudioBus::Se,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::WarningAlert, AudioCueDef{
            root + L"/warning_alert.wav",
            AudioBus::Ui,
            false,
            1.0f
        });

        m_seDefs.emplace(SeId::BonusTime, AudioCueDef{
            root + L"/bonus_time.wav",
            AudioBus::Ui,
            false,
            1.0f
        });
    }

    bool AudioSystem::LoadAll()
    {
        try
        {
            m_bgmEffects.clear();
            m_seEffects.clear();

            for (const auto& [id, def] : m_bgmDefs)
            {
                m_bgmEffects.emplace(id, std::make_unique<SoundEffect>(m_engine.get(), def.path.c_str()));
            }

            for (const auto& [id, def] : m_seDefs)
            {
                m_seEffects.emplace(id, std::make_unique<SoundEffect>(m_engine.get(), def.path.c_str()));
            }

            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    void AudioSystem::Update()
    {
        if (!m_initialized || !m_engine)
        {
            return;
        }

        // DirectXTK の AudioEngine は毎フレーム Update が必要です。
        // Update は bool を返しますが、この実装では戻り値の判定を呼び出し側に広げていません。
        m_engine->Update();
    }

    void AudioSystem::SuspendAll()
    {
        if (!m_initialized || !m_engine)
        {
            return;
        }

        m_engine->Suspend();
    }

    void AudioSystem::ResumeAll()
    {
        if (!m_initialized || !m_engine)
        {
            return;
        }

        m_engine->Resume();

        // AudioEngine の復帰後に、現在のループ BGM を再生状態へ戻したい場合は再開します。
        if (m_currentBgm)
        {
            m_currentBgm->Play(true);
            ApplyCurrentBgmVolume();
        }
    }

    void AudioSystem::ApplyCurrentBgmVolume()
    {
        if (!m_currentBgm || !m_currentBgmId.has_value())
        {
            return;
        }

        const auto defIt = m_bgmDefs.find(*m_currentBgmId);
        if (defIt == m_bgmDefs.end())
        {
            return;
        }

        const float volume = GetEffectiveInstanceVolume(defIt->second);
        m_currentBgm->SetVolume(volume);
    }

    void AudioSystem::PlayBgm(BgmId id, bool restart)
    {
        if (!m_initialized || !m_engine)
        {
            return;
        }

        if (!restart && m_currentBgmId.has_value() && *m_currentBgmId == id && m_currentBgm)
        {
            return;
        }

        StopBgm(true);

        const auto defIt = m_bgmDefs.find(id);
        const auto fxIt = m_bgmEffects.find(id);

        if (defIt == m_bgmDefs.end() || fxIt == m_bgmEffects.end())
        {
            return;
        }

        // SoundEffectInstance はループと再生中の音量変更に使います。
        m_currentBgm = fxIt->second->CreateInstance();
        m_currentBgmId = id;

        ApplyCurrentBgmVolume();
        m_currentBgm->Play(defIt->second.loop);
    }

    void AudioSystem::StopBgm(bool immediate)
    {
        if (m_currentBgm)
        {
            m_currentBgm->Stop(immediate);
            m_currentBgm.reset();
        }

        m_currentBgmId.reset();
    }

    void AudioSystem::PlaySe(SeId id, float volumeScale, float pitch, float pan)
    {
        if (!m_initialized || !m_engine)
        {
            return;
        }

        const auto defIt = m_seDefs.find(id);
        const auto fxIt = m_seEffects.find(id);

        if (defIt == m_seDefs.end() || fxIt == m_seEffects.end())
        {
            return;
        }

        const float finalVolume = GetEffectiveOneShotVolume(defIt->second, volumeScale);
        const float finalPitch = std::clamp(pitch, kMinPitch, kMaxPitch);
        const float finalPan = std::clamp(pan, kMinPan, kMaxPan);

        // one-shot は SoundEffect::Play を使います。
        // ループや再生中変更が不要な SE に向いています。
        fxIt->second->Play(finalVolume, finalPitch, finalPan);
    }

    void AudioSystem::SetMasterVolume(float value)
    {
        m_volume.master = Clamp01(value);

        if (m_engine)
        {
            m_engine->SetMasterVolume(m_volume.master);
        }
    }

    void AudioSystem::SetBusVolume(AudioBus bus, float value)
    {
        const float clamped = Clamp01(value);

        switch (bus)
        {
        case AudioBus::Bgm:
            m_volume.bgm = clamped;
            ApplyCurrentBgmVolume();
            break;

        case AudioBus::Se:
            m_volume.se = clamped;
            break;

        case AudioBus::Ui:
            m_volume.ui = clamped;
            break;

        default:
            break;
        }
    }

    void AudioSystem::ApplyVolumeSettings(const AudioVolumeSettings& settings)
    {
        SetMasterVolume(settings.master);
        SetBusVolume(AudioBus::Bgm, settings.bgm);
        SetBusVolume(AudioBus::Se, settings.se);
        SetBusVolume(AudioBus::Ui, settings.ui);
    }
}
