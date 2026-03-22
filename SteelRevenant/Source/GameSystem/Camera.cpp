//------------------------//------------------------
// 処理内容: カメラのビュー・射影行列の更新と基本操作を実装する。
//------------------------//------------------------
// 作成者: Keishi Teramoto
// 作成日: 2026/03/16
// 最終更新: 2026/03/17
//------------------------//------------------------

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Camera.h"

using namespace SteelRevenant;

Camera::Camera(float width, float height)
    : m_Eyepos(0.0f, 0.0f, 3.0f)
    , m_Refpos(0.0f, 0.0f, 0.0f)
    , m_Upvec(0.0f, 1.0f, 0.0f)
{
    m_FovY = DirectX::XMConvertToRadians(60.0f);
    m_Aspect = height > 0.0f ? width / height : 1.0f;
    m_NearClip = 0.1f;
    m_FarClip = 2500.0f;
    m_target = DirectX::SimpleMath::Vector3::Zero;
    m_eyeLerp = 0.0f;
    m_refLerp = 0.0f;
    Update();
}

Camera::~Camera() = default;

void Camera::Update()
{
    // ターゲットが設定されている場合は参照位置として優先する（ゼロ判定で判定）。
    const float eps = 1e-5f;
    bool hasTarget = m_target.LengthSquared() > eps;
    DirectX::SimpleMath::Vector3 desiredRef = hasTarget ? m_target : m_Refpos;

    // 参照位置を滑らかに補間する（補間係数が 0 の場合は即時反映）。
    if (m_refLerp <= 0.0f)
    {
        m_Refpos = desiredRef;
    }
    else
    {
        m_Refpos += (desiredRef - m_Refpos) * m_refLerp;
    }

    // 必要なら視点位置も滑らかに補間する（互換性のため、明示的でなければ現在値を維持）。
    if (m_eyeLerp > 0.0f)
    {
        DirectX::SimpleMath::Vector3 desiredEye = m_Eyepos; // デフォルトは維持
        // 視点が原点に近ければ、適切なオフセットへ誘導する
        if (m_Eyepos.LengthSquared() <= eps)
        {
            desiredEye = m_Refpos + DirectX::SimpleMath::Vector3(0.0f, 3.0f, -6.0f);
        }
        m_Eyepos += (desiredEye - m_Eyepos) * m_eyeLerp;
    }

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

const DirectX::SimpleMath::Matrix& Camera::GetView() const
{
    return m_View;
}

const DirectX::SimpleMath::Matrix& Camera::GetProj() const
{
    return m_Proj;
}

// 新しい慣習的セッター
void Camera::SetEyePos(const DirectX::SimpleMath::Vector3& eyepos)
{
    m_Eyepos = eyepos;
}

void Camera::SetRefPos(const DirectX::SimpleMath::Vector3& refpos)
{
    m_Refpos = refpos;
}

void Camera::SetUpVec(const DirectX::SimpleMath::Vector3& upvec)
{
    m_Upvec = upvec;
}

void Camera::SetFovY(float fovY)
{
    m_FovY = fovY;
}

void Camera::SetAspect(float aspect)
{
    m_Aspect = (aspect > 0.0f) ? aspect : m_Aspect;
}

void Camera::SetNearClip(float nearclip)
{
    m_NearClip = nearclip;
}

void Camera::SetFarClip(float farclip)
{
    m_FarClip = farclip;
}

void Camera::SetTargetPos(const DirectX::SimpleMath::Vector3& target)
{
    m_target = target;
}


