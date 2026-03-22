#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
// Object3D - 譌ｧ譚･縺ｮ3D繧ｪ繝悶ず繧ｧ繧ｯ繝亥渕蠎輔け繝ｩ繧ｹ縲・// 迴ｾ蝨ｨ縺ｮ繧ｲ繝ｼ繝縺ｯ GeometricPrimitive 縺ｮ縺ｿ菴ｿ逕ｨ縺ｮ縺溘ａ縲・// Model萓晏ｭ倬Κ蛻・ｒ繧ｹ繧ｿ繝門喧縺励※繧ｳ繝ｳ繝代う繝ｫ繧ｨ繝ｩ繝ｼ繧貞屓驕ｿ縺吶ｋ縲・#include <windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include "Camera.h"
#include "../Utility/DirectX11.h"
namespace Teramoto
{
    class Object3D
    {
    public:
        Object3D();
        virtual ~Object3D();
        virtual void Update() {}
        virtual void Draw()   {}
        virtual void Load(const wchar_t*) {}
    protected:
        DirectX::SimpleMath::Vector3    m_translation;
        DirectX::SimpleMath::Vector3    m_rotation;
        DirectX::SimpleMath::Vector3    m_scale = DirectX::SimpleMath::Vector3(1,1,1);
    };
}

