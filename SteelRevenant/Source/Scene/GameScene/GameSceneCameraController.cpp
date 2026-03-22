#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// TPS カメラの位置・注視点・衝突回避・FOV 平滑化を実装する。
//------------------------

#include "GameSceneCameraController.h"

#include "../../Utility/DirectX11.h"
#include "../../Utility/SimpleMathEx.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace
{
    // 最短角度差を返す
    float ShortestAngleDiff(float target, float source)
    {
        float diff = target - source;
        constexpr float kPi    = DirectX::XM_PI;
        constexpr float kTwoPi = kPi * 2.0f;
        while (diff >  kPi) diff -= kTwoPi;
        while (diff < -kPi) diff += kTwoPi;
        return diff;
    }
}

void GameSceneCameraController::Reset(float playerYaw, float screenWidth, float screenHeight)
{
    m_yaw       = playerYaw;
    m_pitch     = 0.0f;
    m_fov       = kDefaultFov;
    m_targetFov = kDefaultFov;
    m_position  = Vector3::Zero;
    m_target    = Vector3::Zero;

    m_camera = std::make_unique<SteelRevenant::Camera>(screenWidth, screenHeight);
    m_camera->SetFovY(m_fov);
    m_camera->SetAspect((screenHeight > 0.0f) ? (screenWidth / screenHeight) : (16.0f / 9.0f));
    m_camera->SetUpVec(Vector3::Up);
}

void GameSceneCameraController::TrackCameraYaw(
    float playerYaw,
    const std::vector<Action::EnemyState>& enemies,
    int lockEnemyIndex,
    const Vector3& playerPos)
{
    float desiredYaw = playerYaw;

    const bool hasLock = lockEnemyIndex >= 0
        && lockEnemyIndex < static_cast<int>(enemies.size())
        && enemies[static_cast<size_t>(lockEnemyIndex)].state != Action::EnemyStateType::Dead;

    if (hasLock)
    {
        const Vector3 toEnemy = enemies[static_cast<size_t>(lockEnemyIndex)].position - playerPos;
        desiredYaw = std::atan2(toEnemy.x, toEnemy.z);
    }

    const float diff = ShortestAngleDiff(desiredYaw, m_yaw);
    m_yaw += diff * kYawLerp;
    m_pitch = kFixedPitch;
}

void GameSceneCameraController::UpdateTransform(
    const Vector3& playerPos,
    float playerYaw,
    const std::vector<Action::EnemyState>& enemies,
    int lockEnemyIndex,
    const std::vector<DirectX::SimpleMath::Vector4>& wallRects,
    const Vector2& shakeOffset,
    float cameraHeightOverride,
    float aspect)
{
    const float height = (cameraHeightOverride > 0.0f) ? cameraHeightOverride : kHeight;

    // カメラ基底ベクトル
    const Matrix camRot  = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);
    const Vector3 camRight = Vector3::Transform(Vector3(1, 0, 0), camRot);

    // プレイヤー前方ベクトル
    const Matrix  playerRot = Matrix::CreateRotationY(playerYaw);
    Vector3 playerForward   = Vector3::Transform(Vector3(0, 0, 1), playerRot);
    Utility::MathEx::SafeNormalize(playerForward);

    // 希望カメラ位置: プレイヤー後方 + 高さ + 肩オフセット
    const Vector3 desiredPos = playerPos - playerForward * kBack
                             + Vector3(0.0f, height, 0.0f)
                             + camRight * kShoulderOffset;

    // 壁衝突回避
    const Vector3 playerHead = playerPos + Vector3(0.0f, kPlayerHeadY, 0.0f);
    const Vector3 adjustedPos = ResolveWallOcclusion(playerHead, desiredPos, wallRects);

    // 平滑化
    SmoothCameraPosition(adjustedPos, shakeOffset);

    // 注視点更新
    UpdateLookTarget(playerPos, playerYaw, enemies, lockEnemyIndex);

    // 行列再構築
    RebuildMatrices(aspect);
}

Vector3 GameSceneCameraController::GetCameraForward() const
{
    const Matrix rot = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);
    Vector3 fwd = Vector3::Transform(Vector3(0, 0, 1), rot);
    return Utility::MathEx::SafeNormalize(fwd);
}

Vector3 GameSceneCameraController::GetCameraRight() const
{
    const Matrix rot = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);
    return Vector3::Transform(Vector3(1, 0, 0), rot);
}

Vector3 GameSceneCameraController::ResolveWallOcclusion(
    const Vector3& playerHead,
    const Vector3& desiredPos,
    const std::vector<DirectX::SimpleMath::Vector4>& wallRects) const
{
    if (wallRects.empty())
    {
        return desiredPos;
    }

    const Vector3 segDir = desiredPos - playerHead;
    const float segLen = std::sqrt(segDir.x * segDir.x + segDir.y * segDir.y + segDir.z * segDir.z);
    if (segLen <= 0.001f)
    {
        return desiredPos;
    }

    bool  occluded   = false;
    float occlusionT = 1.0f;

    // 遠方から手前へサンプリングし、最初の遮蔽を検出する
    for (int s = kOcclusionSamples; s >= 2; --s)
    {
        const float t = static_cast<float>(s) / static_cast<float>(kOcclusionSamples);
        const Vector3 sample = playerHead + segDir * t;

        for (const auto& r : wallRects)
        {
            const float minX = r.x, minZ = r.y, maxX = r.z, maxZ = r.w;
            if (sample.x >= (minX - kOcclusionMargin) && sample.x <= (maxX + kOcclusionMargin) &&
                sample.z >= (minZ - kOcclusionMargin) && sample.z <= (maxZ + kOcclusionMargin))
            {
                occluded   = true;
                occlusionT = t;
                break;
            }
        }
        if (occluded) break;
    }

    if (!occluded)
    {
        return desiredPos;
    }

    // 遮蔽物の手前へ引き戻す
    const float pullT = std::max(kOcclusionMinT, occlusionT - kOcclusionPullBack);
    return playerHead + segDir * pullT;
}

void GameSceneCameraController::SmoothCameraPosition(
    const Vector3& adjustedPos,
    const Vector2& shakeOffset)
{
    const float sf = std::clamp(kSmoothFactor, 0.02f, 0.45f);
    m_position.x += (adjustedPos.x + shakeOffset.x * kShakeScale - m_position.x) * sf;
    m_position.y += (adjustedPos.y + shakeOffset.y * kShakeScale - m_position.y) * sf;
    m_position.z += (adjustedPos.z                               - m_position.z) * sf;
}

void GameSceneCameraController::UpdateLookTarget(
    const Vector3& playerPos,
    float playerYaw,
    const std::vector<Action::EnemyState>& enemies,
    int lockEnemyIndex)
{
    const float sf = std::clamp(kSmoothFactor, 0.02f, 0.45f);
    const float xzLerp = std::clamp(sf * 1.0f, kXZLerpMin, kXZLerpMax);

    const bool hasLock = lockEnemyIndex >= 0
        && lockEnemyIndex < static_cast<int>(enemies.size())
        && enemies[static_cast<size_t>(lockEnemyIndex)].state != Action::EnemyStateType::Dead;

    if (hasLock)
    {
        // ロックオン中: プレイヤー胸元と敵位置のブレンド
        const Vector3 enemyPos = enemies[static_cast<size_t>(lockEnemyIndex)].position
                               + Vector3(0.0f, kChestOffsetY, 0.0f);
        const Vector3 playerChest = playerPos + Vector3(0.0f, kChestOffsetY, 0.0f);
        const Vector3 desired = enemyPos * kLockEnemyBias + playerChest * kLockPlayerBias;

        m_target.x += (desired.x - m_target.x) * xzLerp;
        m_target.z += (desired.z - m_target.z) * xzLerp;

        float newY = m_target.y + (desired.y - m_target.y) * kYLerp;
        newY = std::clamp(newY, m_target.y - kMaxDeltaY, m_target.y + kMaxDeltaY);
        m_target.y = newY;
    }
    else
    {
        // フリー: プレイヤー前方を注視
        const Matrix playerRot = Matrix::CreateRotationY(playerYaw);
        Vector3 playerFwd = Vector3::Transform(Vector3(0, 0, 1), playerRot);
        Utility::MathEx::SafeNormalize(playerFwd);
        const Vector3 desiredXZ = playerPos + playerFwd * kForwardLook;

        m_target.x += (desiredXZ.x - m_target.x) * xzLerp;
        m_target.z += (desiredXZ.z - m_target.z) * xzLerp;

        const float desiredY = playerPos.y + kChestOffsetY;
        float newY = m_target.y + (desiredY - m_target.y) * kYLerp;
        newY = std::clamp(newY, m_target.y - kMaxDeltaY, m_target.y + kMaxDeltaY);
        m_target.y = newY;
    }
}

void GameSceneCameraController::RebuildMatrices(float aspect)
{
    // FOV 平滑化
    m_fov += (m_targetFov - m_fov) * kFovBlend;

    if (m_camera)
    {
        m_camera->SetEyePos(m_position);
        m_camera->SetTargetPos(m_target);
        m_camera->SetFovY(m_fov);
        m_camera->SetAspect(aspect);
        m_camera->Update();
        m_view = m_camera->GetView();
        m_proj = m_camera->GetProj();
    }
    else
    {
        m_view = Matrix::CreateLookAt(m_position, m_target, Vector3::Up);
        m_proj = Matrix::CreatePerspectiveFieldOfView(m_fov, aspect, 0.1f, 1000.0f);
    }
}
