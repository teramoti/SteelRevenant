#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "../SingletonBase.h"

namespace GameAudio
{
    // 効果音の識別子。
    enum class SeId
    {
        MeleeSlash,
        GuardBlock,
        PlayerHit,
        EnemyDestroy,
        ItemPickup,
        WaveClear,
        GameOver,
        Count
    };

    // 音量設定をまとめる構造体。
    struct AudioVolumeSettings
    {
        float master = 1.0f;
        float se     = 1.0f;
        float bgm    = 0.7f;
    };

    // 効果音の再生管理を担当するシステム。
    // DirectX TK Audio を利用する。音声ファイルが存在しない場合は無音で続行する。
    class AudioSystem : public Utility::SingletonBase<AudioSystem>
    {
        friend class Utility::SingletonBase<AudioSystem>;

    public:
        // 音声ルートディレクトリを指定して初期化する。
        void Initialize(const std::wstring& audioRootPath);

        // 毎フレーム呼び出し（AudioEngine の更新）。
        void Update();

        // 指定 ID の効果音を再生する。
        void PlaySe(SeId id, float volume = 1.0f);

        // 音量設定を一括反映する。
        void ApplyVolumeSettings(const AudioVolumeSettings& settings);

        // 全音声を一時停止する。
        void SuspendAll();

        // 全音声を再開する。
        void ResumeAll();

        // リソースを解放する。
        void Shutdown();

    private:
        AudioSystem();
        ~AudioSystem() = default;

        // 効果音ファイル名テーブル（SeId → ファイル名）。
        static const wchar_t* GetFileName(SeId id);

        std::wstring          m_audioRoot;
        AudioVolumeSettings   m_volumeSettings;
        bool                  m_initialized = false;

        // AudioEngine は DirectXTK Audio が利用可能な場合のみ有効。
        // ここでは前方宣言のみとし、.cpp 内で実体を扱う。
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
