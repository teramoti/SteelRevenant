#pragma once

namespace SystemUI
{
    enum class Variant
    {
        Lightweight = 0,
        Premium = 1
    };

    inline int& _internal_variant()
    {
        static int v = 0;
        return v;
    }

    inline Variant GetVariant() { return static_cast<Variant>(_internal_variant()); }
    inline void SetVariant(Variant v) { _internal_variant() = static_cast<int>(v); }
    inline void ToggleVariant() { _internal_variant() = (_internal_variant() == 0) ? 1 : 0; }
}
