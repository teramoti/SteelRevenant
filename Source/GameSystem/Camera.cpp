//------------------------//------------------------
// Contents(処理内容) 共通カメラの行列更新処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "Camera.h"

using namespace Teramoto;

// 画面サイズから基底カメラの既定パラメータを設定する。
Camera::Camera(float w, float h)
	: m_Eyepos(0, 0, 3.0f)
	, m_Refpos(0, 0, 0)
	, m_Upvec(0, 1.0f, 0)
{
	m_FovY = DirectX::XMConvertToRadians(60.0f);
	m_Aspect = static_cast<float>(w) / h;
	m_NearClip = 0.1f;
	m_FarClip = 2500.0f;
}

// 基底カメラの後始末を行う。
Camera::~Camera()
{
}

// ビュー行列と射影行列を最新値へ更新する。
void Camera::Update()
{
	m_View = DirectX::SimpleMath::Matrix::CreateLookAt(m_Eyepos, m_Refpos, m_Upvec);
	m_Proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(m_FovY, m_Aspect, m_NearClip, m_FarClip);
}

// 現在フレームのビュー行列を返す。
DirectX::SimpleMath::Matrix& Camera::GetView()
{
	return m_View;
}

// 現在フレームの射影行列を返す。
DirectX::SimpleMath::Matrix& Camera::GetProj()
{
	return m_Proj;
}

// カメラ位置を設定する。
void Camera::Seteyepos(const DirectX::SimpleMath::Vector3& eyepos)
{
	m_Eyepos = eyepos;
}

// 注視点を設定する。
void Camera::Setrefpos(const DirectX::SimpleMath::Vector3& refpos)
{
	m_Refpos = refpos;
}

// 上方向ベクトルを設定する。
void Camera::Setupvec(const DirectX::SimpleMath::Vector3& upvec)
{
	m_Upvec = upvec;
}

// 垂直視野角を設定する。
void Camera::SetfovY(float fovY)
{
	m_FovY = fovY;
}

// 画面アスペクト比を設定する。
void Camera::Settaspect(float aspect)
{
	m_Aspect = aspect;
}

// 近クリップ距離を設定する。
void Camera::SetanerClip(float nearclip)
{
	m_NearClip = nearclip;
}

// 遠クリップ距離を設定する。
void Camera::SetfarClip(float farclip)
{
	m_FarClip = farclip;
}

// 追従対象位置を設定する。
void Camera::SetTargetPos(DirectX::SimpleMath::Vector3 target)
{
	m_target = target;
}

