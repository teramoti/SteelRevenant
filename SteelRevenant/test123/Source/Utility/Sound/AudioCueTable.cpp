#include "AudioCueTable.h"

namespace GameAudio
{
    namespace
    {
        constexpr AudioCueDef kSeCueTable[] =
        {
            { L"Se/Ui/ui_move.wav",            AudioBus::Ui, false, 1.0f },
            { L"Se/Ui/ui_select.wav",          AudioBus::Ui, false, 1.0f },
            { L"Se/Ui/ui_confirm.wav",         AudioBus::Ui, false, 1.0f },
            { L"Se/Ui/ui_cancel.wav",          AudioBus::Ui, false, 1.0f },
            { L"Se/Ui/ui_back.wav",            AudioBus::Ui, false, 1.0f },
            { L"Se/Player/player_shot.wav",    AudioBus::Se, false, 1.0f },
            { L"Se/Enemy/enemy_shot.wav",      AudioBus::Se, false, 1.0f },
            { L"Se/Player/player_hit.wav",     AudioBus::Se, false, 1.0f },
            { L"Se/Player/player_hit.wav",     AudioBus::Se, false, 1.0f },
            { L"Se/Combat/hit_enemy.wav",      AudioBus::Se, false, 1.0f },
            { L"Se/Combat/melee_slash.wav",    AudioBus::Se, false, 1.0f },
            { L"Se/Combat/enemy_destroy.wav",  AudioBus::Se, false, 1.0f },
            { L"Se/Enemy/enemy_die.wav",       AudioBus::Se, false, 1.0f },
            { L"Se/Enemy/enemy_spawn.wav",     AudioBus::Se, false, 1.0f },
            { L"Se/Item/item_pickup.wav",      AudioBus::Se, false, 1.0f },
            { L"Se/System/wave_clear.wav",     AudioBus::Se, false, 1.0f },
            { L"Bgm/result/game_over.wav",     AudioBus::Se, false, 1.0f },
            { L"Se/System/relay_start.wav",    AudioBus::Se, false, 1.0f },
            { L"Se/System/relay_secure.wav",   AudioBus::Se, false, 1.0f },
            { L"Se/Item/beacon_heal.wav",      AudioBus::Se, false, 1.0f },
            { L"Se/System/warning_alert.wav",  AudioBus::Se, false, 1.0f },
            { L"Se/System/bonus_time.wav",     AudioBus::Se, false, 1.0f },
            { L"Se/Enemy/laser_charge.wav",    AudioBus::Se, false, 1.0f },
            { L"Se/Enemy/laser_fire.wav",      AudioBus::Se, false, 1.0f },
            { L"Se/System/charge_warning.wav", AudioBus::Se, false, 1.0f },
            { L"Se/Player/dash_slide.wav",     AudioBus::Se, false, 1.0f },
            { L"Se/System/time_low.wav",       AudioBus::Se, false, 1.0f },
            { L"Se/Combat/combo1.wav",         AudioBus::Se, false, 1.0f },
            { L"Se/Combat/combo2.wav",         AudioBus::Se, false, 1.0f },
            { L"Se/Combat/combo3.wav",         AudioBus::Se, false, 1.0f },
            { L"Se/Ui/ui_select.wav",          AudioBus::Ui, false, 1.0f },
            { L"Se/System/wave_clear.wav",     AudioBus::Se, false, 1.0f },
            { L"Se/System/warning_alert.wav",  AudioBus::Se, false, 1.0f },
            { L"Se/Combat/guard_block.wav",    AudioBus::Se, false, 1.0f },
        };

        constexpr AudioCueDef kBgmCueTable[] =
        {
            { L"Bgm/title/title_theme.wav",      AudioBus::Bgm, true, 1.0f },
            { L"Bgm/title/stage_select.wav",     AudioBus::Bgm, true, 1.0f },
            { L"Bgm/stage/defense_corridor.wav", AudioBus::Bgm, true, 1.0f },
            { L"Bgm/stage/core_sector.wav",      AudioBus::Bgm, true, 1.0f },
            { L"Bgm/result/result_clear.wav",    AudioBus::Bgm, false, 1.0f },
            { L"Bgm/result/result_fail.wav",     AudioBus::Bgm, false, 1.0f },
        };

        static_assert(sizeof(kSeCueTable) / sizeof(kSeCueTable[0]) == static_cast<size_t>(SeId::Count), "kSeCueTable must match SeId::Count");
        static_assert(sizeof(kBgmCueTable) / sizeof(kBgmCueTable[0]) == 6, "kBgmCueTable must match BgmId count");
    }

    const AudioCueDef* FindSeCueDef(SeId id)
    {
        const size_t index = static_cast<size_t>(id);
        if (index >= (sizeof(kSeCueTable) / sizeof(kSeCueTable[0])))
        {
            return nullptr;
        }
        return &kSeCueTable[index];
    }

    const AudioCueDef* FindBgmCueDef(BgmId id)
    {
        const size_t index = static_cast<size_t>(id);
        if (index >= (sizeof(kBgmCueTable) / sizeof(kBgmCueTable[0])))
        {
            return nullptr;
        }
        return &kBgmCueTable[index];
    }
}
