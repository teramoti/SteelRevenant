#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

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
        UiSelect,
        UiConfirm,
        UiCancel,
        UiBack,
        PlayerShot,
        EnemyShot,
        PlayerHit,
        HitPlayer,
        HitEnemy,
        MeleeSlash,
        EnemyDestroy,
        EnemyDie,
        EnemySpawn,
        ItemPickup,
        WaveClear,
        GameOver,
        RelayStart,
        RelaySecure,
        BeaconHeal,
        WarningAlert,
        BonusTime,
        LaserCharge,
        LaserFire,
        ChargeWarning,
        DashSlide,
        TimeLow,
        Combo1,
        Combo2,
        Combo3,
        StageSelect,
        ResultClear,
        ResultFail,
        GuardBlock,
        Count,
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

