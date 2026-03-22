#pragma once

#include "AudioTypes.h"

namespace GameAudio
{
    const AudioCueDef* FindSeCueDef(SeId id);
    const AudioCueDef* FindBgmCueDef(BgmId id);
}
