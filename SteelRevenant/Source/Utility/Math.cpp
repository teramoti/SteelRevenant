#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Math.h"

#include <cmath>
#include <cstdlib>
#include <ctime>

float SteelRevenant::Math::TargetAngle(float myPos1, float myPos2, float targetPos1, float targetPos2)
{
    const float deltaPos1 = myPos1 - targetPos1;
    const float deltaPos2 = myPos2 - targetPos2;
    return static_cast<float>(std::atan2(deltaPos2, deltaPos1));
}

int SteelRevenant::Math::GetRand(int min, int max)
{
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }

    return std::rand() % (max - min + 1) + min;
}

float SteelRevenant::Math::ToRad(float deg)
{
    return deg * (PI / 180.0f);
}

float SteelRevenant::Math::ToDeg(float rad)
{
    return rad * (180.0f / PI);
}


