#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
namespace Utility
{
    template<typename T>
    class SingletonBase
    {
    public:
        static T& GetInstance() { static T instance; return instance; }
        SingletonBase(const SingletonBase&) = delete;
        SingletonBase& operator=(const SingletonBase&) = delete;
    protected:
        SingletonBase()  = default;
        ~SingletonBase() = default;
    };
}

