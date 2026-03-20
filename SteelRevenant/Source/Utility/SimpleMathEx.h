#pragma once

#include <cmath>
#include <algorithm>
#include <SimpleMath.h>

namespace Utility
{
    namespace MathEx
    {
        // 値を [minVal, maxVal] に収める。
        template<typename T>
        inline T Clamp(T value, T minVal, T maxVal)
        {
            return std::max(minVal, std::min(maxVal, value));
        }

        // ラジアンを [-pi, pi] に正規化する。
        inline float WrapRadians(float r)
        {
            constexpr float kPi    = 3.14159265358979323846f;
            constexpr float kTwoPi = kPi * 2.0f;
            while (r >  kPi) r -= kTwoPi;
            while (r < -kPi) r += kTwoPi;
            return r;
        }

        // 長さゼロベクトルを安全に正規化する（ゼロの場合はゼロを返す）。
        inline DirectX::SimpleMath::Vector3 SafeNormalize(const DirectX::SimpleMath::Vector3& v)
        {
            const float lenSq = v.LengthSquared();
            if (lenSq < 1e-8f) return DirectX::SimpleMath::Vector3::Zero;
            return v / std::sqrt(lenSq);
        }

        // 線形補間。
        inline float Lerp(float a, float b, float t)
        {
            return a + (b - a) * Clamp(t, 0.0f, 1.0f);
        }

        // 指数減衰ブレンド係数を返す（gain が大きいほど速い）。
        inline float ExpBlend(float gain, float dt)
        {
            return Clamp(1.0f - std::exp(-gain * dt), 0.0f, 1.0f);
        }
    }
}
