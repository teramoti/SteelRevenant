#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Ensure any 'min'/'max' macros from Windows headers do not break std::min/std::max usage.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <cmath>
#include <algorithm>
#include <SimpleMath.h>

namespace Utility { namespace MathEx
{
    template<typename T>
    inline T Clamp(T value, T minVal, T maxVal) { return std::max(minVal, (std::min)(maxVal, value)); }

    inline float WrapRadians(float r)
    {
        constexpr float kPi = 3.14159265358979323846f, kTwoPi = kPi * 2.0f;
        while (r >  kPi) r -= kTwoPi;
        while (r < -kPi) r += kTwoPi;
        return r;
    }

    inline DirectX::SimpleMath::Vector3 SafeNormalize(const DirectX::SimpleMath::Vector3& v)
    {
        const float lenSq = v.LengthSquared();
        return (lenSq < 1e-8f) ? DirectX::SimpleMath::Vector3::Zero : v / std::sqrt(lenSq);
    }

    inline float Lerp(float a, float b, float t) { return a + (b - a) * Clamp(t, 0.0f, 1.0f); }
    inline float ExpBlend(float gain, float dt)   { return Clamp(1.0f - std::exp(-gain * dt), 0.0f, 1.0f); }
}}


