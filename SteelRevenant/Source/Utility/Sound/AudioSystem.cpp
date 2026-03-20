//------------------------//------------------------
// Contents(処理内容) 効果音の読み込みと再生を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "AudioSystem.h"

#include <filesystem>
#include <vector>

// DirectXTK Audio が利用可能かどうかを確認する。
// Audio.h が存在する場合のみ有効化する（プロジェクト設定依存）。
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

    AudioSystem::AudioSystem()
        : m_impl(std::make_unique<Impl>())
    {
    }

    // 音声ルートディレクトリを指定して AudioEngine を初期化する。
    void AudioSystem::Initialize(const std::wstring& audioRootPath)
    {
        m_audioRoot    = audioRootPath;
        m_initialized  = false;

#if AUDIO_ENABLED
        try
        {
            DirectX::AUDIO_ENGINE_FLAGS flags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
            flags |= DirectX::AudioEngine_Debug;
#endif
            m_impl->engine = std::make_unique<DirectX::AudioEngine>(flags);
            m_impl->active = true;

            // 効果音をあらかじめロードしておく。
            namespace fs = std::filesystem;
            const fs::path root(audioRootPath);

            for (int id = 0; id < static_cast<int>(SeId::Count); ++id)
            {
                const wchar_t* fname = GetFileName(static_cast<SeId>(id));
                if (!fname) continue;

                const fs::path filePath = root / fname;
                if (!fs::exists(filePath)) continue;

                try
                {
                    m_impl->effects[id] = std::make_unique<DirectX::SoundEffect>(
                        m_impl->engine.get(), filePath.wstring().c_str());
                }
                catch (...) { /* ファイルが壊れている場合は無視して続行 */ }
            }
            m_initialized = true;
        }
        catch (...) { /* Audio 初期化失敗時は無音で続行 */ }
#endif
    }

    // AudioEngine を毎フレーム更新する。
    void AudioSystem::Update()
    {
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine)
        {
            if (!m_impl->engine->Update())
            {
                // デバイスロスト: 再初期化を試みる。
                if (m_impl->engine->IsCriticalError())
                {
                    m_impl->engine->Reset();
                }
            }
        }
#endif
    }

    // 指定効果音を音量付きで再生する。
    void AudioSystem::PlaySe(SeId id, float volume)
    {
#if AUDIO_ENABLED
        if (!m_impl->active || !m_impl->engine) return;

        const int key = static_cast<int>(id);
        auto it = m_impl->effects.find(key);
        if (it == m_impl->effects.end() || !it->second) return;

        const float finalVolume = volume * m_volumeSettings.se * m_volumeSettings.master;
        it->second->Play(finalVolume, 0.0f, 0.0f);
#else
        (void)id; (void)volume;
#endif
    }

    // 音量設定を適用する。
    void AudioSystem::ApplyVolumeSettings(const AudioVolumeSettings& settings)
    {
        m_volumeSettings = settings;
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine)
        {
            m_impl->engine->SetMasterVolume(settings.master);
        }
#endif
    }

    // 全音声を一時停止する。
    void AudioSystem::SuspendAll()
    {
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine)
            m_impl->engine->Suspend();
#endif
    }

    // 全音声を再開する。
    void AudioSystem::ResumeAll()
    {
#if AUDIO_ENABLED
        if (m_impl->active && m_impl->engine)
            m_impl->engine->Resume();
#endif
    }

    // リソースを解放する。
    void AudioSystem::Shutdown()
    {
#if AUDIO_ENABLED
        m_impl->effects.clear();
        m_impl->engine.reset();
        m_impl->active = false;
#endif
        m_initialized = false;
    }

    // SeId に対応するファイル名を返す。
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
