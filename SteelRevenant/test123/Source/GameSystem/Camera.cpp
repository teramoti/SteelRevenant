#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Camera.h"

using namespace Teramoto;

Camera::Camera(float width, float height)
    : m_Eyepos(0.0f, 0.0f, 3.0f)
    , m_Refpos(0.0f, 0.0f, 0.0f)
    , m_Upvec(0.0f, 1.0f, 0.0f)
{
    m_FovY = DirectX::XMConvertToRadians(60.0f);
    m_Aspect = height > 0.0f ? width / height : 1.0f;
    m_NearClip = 0.1f;
    m_FarClip = 2500.0f;
    Update();
}

Camera::~Camera() = default;

void Camera::Update()
{
    m_View = DirectX::SimpleMath::Matrix::CreateLookAt(m_Eyepos, m_Refpos, m_Upvec);
    m_Proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        m_FovY,
        m_Aspect,
        m_NearClip,
        m_FarClip);
}

DirectX::SimpleMath::Matrix& Camera::GetView()
{
    return m_View;
}

DirectX::SimpleMath::Matrix& Camera::GetProj()
{
    return m_Proj;
}

void Camera::Seteyepos(const DirectX::SimpleMath::Vector3& eyepos)
{
    m_Eyepos = eyepos;
}

void Camera::Setrefpos(const DirectX::SimpleMath::Vector3& refpos)
{
    m_Refpos = refpos;
}

void Camera::Setupvec(const DirectX::SimpleMath::Vector3& upvec)
{
    m_Upvec = upvec;
}

void Camera::SetfovY(float fovY)
{
    m_FovY = fovY;
}

void Camera::Settaspect(float aspect)
{
    m_Aspect = aspect;
}

void Camera::SetanerClip(float nearclip)
{
    m_NearClip = nearclip;
}

void Camera::SetfarClip(float farclip)
{
    m_FarClip = farclip;
}

void Camera::SetTargetPos(DirectX::SimpleMath::Vector3 target)
{
    m_target = target;
}

