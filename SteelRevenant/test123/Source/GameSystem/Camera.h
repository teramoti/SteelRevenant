#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <SimpleMath.h>

namespace Teramoto
{
    class Camera
    {
    public:
        Camera(float width, float height);
        virtual ~Camera();

        virtual void Update();

        DirectX::SimpleMath::Matrix& GetView();
        DirectX::SimpleMath::Matrix& GetProj();

        void Seteyepos(const DirectX::SimpleMath::Vector3& eyepos);
        void Setrefpos(const DirectX::SimpleMath::Vector3& refpos);
        void Setupvec(const DirectX::SimpleMath::Vector3& upvec);
        void SetfovY(float fovY);
        void Settaspect(float aspect);
        void SetanerClip(float nearclip);
        void SetfarClip(float farclip);
        void SetTargetPos(DirectX::SimpleMath::Vector3 target);

    protected:
        DirectX::SimpleMath::Matrix m_View;
        DirectX::SimpleMath::Vector3 m_Eyepos;
        DirectX::SimpleMath::Vector3 m_Refpos;
        DirectX::SimpleMath::Vector3 m_Upvec;
        DirectX::SimpleMath::Matrix m_Proj;
        DirectX::SimpleMath::Vector3 m_target;
        float m_FovY = 0.0f;
        float m_Aspect = 1.0f;
        float m_NearClip = 0.1f;
        float m_FarClip = 2500.0f;
    };
}

