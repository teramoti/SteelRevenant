#pragma once

#include <string>

namespace GameAudio
{
    enum class AudioBus
    {
        Bgm,
        Se,
        Ui,
    };

    enum class BgmId
    {
        Title,
        StageSelect,
        DefenseCorridor,
        CoreSector,
        ResultClear,
        ResultFail,
    };

    enum class SeId
    {
        UiMove,
        UiConfirm,
        UiBack,
        PlayerShot,
        EnemyShot,
        PlayerHit,
        GuardBlock,
        MeleeSlash,
        EnemyDestroy,
        RelayStart,
        RelaySecure,
        BeaconHeal,
        WarningAlert,
        BonusTime,
    };

    struct AudioVolumeSettings
    {
        float master = 1.0f;
        float bgm = 1.0f;
        float se = 1.0f;
        float ui = 1.0f;
    };

    struct AudioCueDef
    {
        std::wstring path;
        AudioBus bus = AudioBus::Se;
        bool loop = false;
        float defaultVolume = 1.0f;
    };
}
