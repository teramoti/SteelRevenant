#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace Teramoto
{
    class Math
    {
    public:
        static constexpr float PI = 3.14159274f;

        template<typename T>
        static T Abs(T num)
        {
            return (num < static_cast<T>(0)) ? -num : num;
        }

        template<typename T>
        static T Difference(T p1, T p2)
        {
            return Abs<T>(p1 - p2);
        }

        static float TargetAngle(float myPos1, float myPos2, float targetPos1, float targetPos2);

        template<typename T>
        static bool Clamp(T value, T num1, T num2)
        {
            if (num1 < num2)
            {
                return value >= num1 && value <= num2;
            }

            if (num1 > num2)
            {
                return value <= num1 && value >= num2;
            }

            return true;
        }

        static int GetRand(int min, int max);
        static float ToRad(float deg);
        static float ToDeg(float rad);
    };
}

