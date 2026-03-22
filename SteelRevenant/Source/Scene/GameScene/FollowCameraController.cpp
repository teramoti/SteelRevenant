#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// KH風3D追従カメラの状態管理と更新ロジックを実装する。
//------------------------

#include "FollowCameraController.h"

#include "../../Utility/SimpleMathEx.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace
{
    // 最短角度差を返す（ラジアン）
    float ShortestAngleDiff(float target, float source)
    {
        float diff = target - source;
        constexpr float kPi    = DirectX::XM_PI;
        constexpr float kTwoPi = kPi * 2.0f;
        while (diff >  kPi) diff -= kTwoPi;
        while (diff < -kPi) diff += kTwoPi;
        return diff;
    }

    // 度数法 → ラジアン
    constexpr float DegToRad(float deg)
    {
        return deg * (DirectX::XM_PI / 180.0f);
    }
}

namespace Camera
{

// --- 初期化・リセット ---

void FollowCameraController::Initialize(
    const FollowCameraSettings& settings,
    float playerYaw,
    float screenWidth,
    float screenHeight)
{
    m_settings  = settings;
    m_yaw       = playerYaw;
    m_pitch     = -DegToRad(m_settings.placement.pitchDeg);
    m_fov       = m_settings.defaultFov;
    m_targetFov = m_settings.defaultFov;
    m_position  = Vector3::Zero;
    m_focus     = Vector3::Zero;

    m_camera = std::make_unique<SteelRevenant::Camera>(screenWidth, screenHeight);
    m_camera->SetFovY(m_fov);
    m_camera->SetAspect((screenHeight > 0.0f) ? (screenWidth / screenHeight) : (16.0f / 9.0f));
    m_camera->SetUpVec(Vector3::Up);
}

void FollowCameraController::ApplySettings(const FollowCameraSettings& settings)
{
    m_settings = settings;
    m_pitch    = -DegToRad(m_settings.placement.pitchDeg);
}

void FollowCameraController::Reset(float playerYaw)
{
    m_yaw       = playerYaw;
    m_pitch     = -DegToRad(m_settings.placement.pitchDeg);
    m_fov       = m_settings.defaultFov;
    m_targetFov = m_settings.defaultFov;
    m_position  = Vector3::Zero;
    m_focus     = Vector3::Zero;
}

// --- 毎フレーム更新 ---

void FollowCameraController::Update(
    const Vector3& playerPos,
    float playerYaw,
    const std::vector<Action::EnemyState>& enemies,
    int lockEnemyIndex,
    const std::vector<DirectX::SimpleMath::Vector4>& wallRects,
    const Vector2& shakeOffset,
    float cameraHeightOverride,
    float aspect)
{
    // yaw 追従
    TrackYaw(playerYaw, enemies, lockEnemyIndex, playerPos);

    // 希望カメラ位置を算出
    const float height = (cameraHeightOverride > 0.0f)
                       ? cameraHeightOverride
                       : m_settings.placement.height;
    const Vector3 desiredPos = ComputeDesiredPosition(playerPos, playerYaw, height);

    // 壁遮蔽回避
    const Vector3 playerHead = playerPos + Vector3(0.0f, m_settings.occlusion.playerHeadY, 0.0f);
    const Vector3 adjustedPos = ResolveWallOcclusion(playerHead, desiredPos, wallRects);

    // XZ/Y 分離補間
    SmoothPosition(adjustedPos, shakeOffset);

    // 注視点更新
    UpdateFocusTarget(playerPos, playerYaw, enemies, lockEnemyIndex);

    // 行列再構築
    RebuildMatrices(aspect);
}

// --- カメラ基底ベクトル ---

Vector3 FollowCameraController::GetForward() const
{
    const Matrix rot = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);
    Vector3 fwd = Vector3::Transform(Vector3(0, 0, 1), rot);
    return Utility::MathEx::SafeNormalize(fwd);
}

Vector3 FollowCameraController::GetRight() const
{
    const Matrix rot = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);
    return Vector3::Transform(Vector3(1, 0, 0), rot);
}

// --- yaw 追従 ---

void FollowCameraController::TrackYaw(
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
    m_yaw += diff * m_settings.smoothing.yawLerp;

    // pitch は常に固定値
    m_pitch = -DegToRad(m_settings.placement.pitchDeg);
}

// --- 希望位置算出 ---

Vector3 FollowCameraController::ComputeDesiredPosition(
    const Vector3& playerPos,
    float playerYaw,
    float height) const
{
    // カメラ右方向（肩オフセット用）
    const Matrix camRot = Matrix::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.0f);
    const Vector3 camRight = Vector3::Transform(Vector3(1, 0, 0), camRot);

    // プレイヤー前方ベクトル
    const Matrix playerRot = Matrix::CreateRotationY(playerYaw);
    const Vector3 playerForward = Utility::MathEx::SafeNormalize(
        Vector3::Transform(Vector3(0, 0, 1), playerRot));

    // プレイヤー後方 + 高さ + 肩オフセット
    return playerPos
         - playerForward * m_settings.placement.distance
         + Vector3(0.0f, height, 0.0f)
         + camRight * m_settings.placement.shoulderOffset;
}

// --- 壁遮蔽回避 ---

Vector3 FollowCameraController::ResolveWallOcclusion(
    const Vector3& playerHead,
    const Vector3& desiredPos,
    const std::vector<DirectX::SimpleMath::Vector4>& wallRects) const
{
    if (wallRects.empty())
    {
        return desiredPos;
    }

    const Vector3 segDir = desiredPos - playerHead;
    const float segLen = segDir.Length();
    if (segLen <= 0.001f)
    {
        return desiredPos;
    }

    const auto& occ = m_settings.occlusion;
    bool  occluded   = false;
    float occlusionT = 1.0f;

    // 遠方から手前へサンプリングし、最初の遮蔽を検出する
    for (int s = occ.sampleCount; s >= 2; --s)
    {
        const float t = static_cast<float>(s) / static_cast<float>(occ.sampleCount);
        const Vector3 sample = playerHead + segDir * t;

        for (const auto& r : wallRects)
        {
            const float minX = r.x, minZ = r.y, maxX = r.z, maxZ = r.w;
            if (sample.x >= (minX - occ.margin) && sample.x <= (maxX + occ.margin) &&
                sample.z >= (minZ - occ.margin) && sample.z <= (maxZ + occ.margin))
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
    const float pullT = std::max(occ.minT, occlusionT - occ.pullBack);
    return playerHead + segDir * pullT;
}

// --- XZ/Y 分離補間 ---

void FollowCameraController::SmoothPosition(
    const Vector3& adjustedPos,
    const Vector2& shakeOffset)
{
    const auto& sm = m_settings.smoothing;
    const float xzLerp = std::clamp(sm.positionLerp, 0.02f, 0.45f);
    const float yLerp  = std::clamp(sm.verticalLerp, 0.01f, 0.30f);

    // XZ は通常補間
    m_position.x += (adjustedPos.x + shakeOffset.x * sm.shakeScale - m_position.x) * xzLerp;
    m_position.z += (adjustedPos.z                                  - m_position.z) * xzLerp;

    // Y は弱補間 + フレーム内最大変化量クランプ
    const float desiredY = adjustedPos.y + shakeOffset.y * sm.shakeScale;
    float newY = m_position.y + (desiredY - m_position.y) * yLerp;
    newY = std::clamp(newY, m_position.y - sm.maxVerticalDeltaPerFrame,
                            m_position.y + sm.maxVerticalDeltaPerFrame);
    m_position.y = newY;
}

// --- 注視点更新 ---

void FollowCameraController::UpdateFocusTarget(
    const Vector3& playerPos,
    float playerYaw,
    const std::vector<Action::EnemyState>& enemies,
    int lockEnemyIndex)
{
    const auto& sm = m_settings.smoothing;
    const auto& pl = m_settings.placement;
    const auto& lo = m_settings.lockOn;
    const float xzLerp = std::clamp(sm.positionLerp, 0.02f, 0.45f);

    const bool hasLock = lockEnemyIndex >= 0
        && lockEnemyIndex < static_cast<int>(enemies.size())
        && enemies[static_cast<size_t>(lockEnemyIndex)].state != Action::EnemyStateType::Dead;

    if (hasLock)
    {
        // ロックオン中: プレイヤー胸元と敵位置のブレンド
        const Vector3 enemyChest = enemies[static_cast<size_t>(lockEnemyIndex)].position
                                 + Vector3(0.0f, pl.focusHeight, 0.0f);
        const Vector3 playerChest = playerPos + Vector3(0.0f, pl.focusHeight, 0.0f);
        const Vector3 desired = enemyChest * lo.enemyBias + playerChest * lo.playerBias;

        m_focus.x += (desired.x - m_focus.x) * xzLerp;
        m_focus.z += (desired.z - m_focus.z) * xzLerp;

        float newY = m_focus.y + (desired.y - m_focus.y) * sm.verticalLerp;
        newY = std::clamp(newY, m_focus.y - sm.maxVerticalDeltaPerFrame,
                                m_focus.y + sm.maxVerticalDeltaPerFrame);
        m_focus.y = newY;
    }
    else
    {
        // フリー: プレイヤー前方の胸元高さを注視する
        const Matrix playerRot = Matrix::CreateRotationY(playerYaw);
        const Vector3 playerFwd = Utility::MathEx::SafeNormalize(
            Vector3::Transform(Vector3(0, 0, 1), playerRot));
        const Vector3 desiredXZ = playerPos + playerFwd * pl.forwardLook;

        m_focus.x += (desiredXZ.x - m_focus.x) * xzLerp;
        m_focus.z += (desiredXZ.z - m_focus.z) * xzLerp;

        const float desiredY = playerPos.y + pl.focusHeight;
        float newY = m_focus.y + (desiredY - m_focus.y) * sm.verticalLerp;
        newY = std::clamp(newY, m_focus.y - sm.maxVerticalDeltaPerFrame,
                                m_focus.y + sm.maxVerticalDeltaPerFrame);
        m_focus.y = newY;
    }
}

// --- 行列再構築 ---

void FollowCameraController::RebuildMatrices(float aspect)
{
    // FOV 平滑化
    m_fov += (m_targetFov - m_fov) * m_settings.smoothing.fovBlend;

    if (m_camera)
    {
        m_camera->SetEyePos(m_position);
        m_camera->SetTargetPos(m_focus);
        m_camera->SetFovY(m_fov);
        m_camera->SetAspect(aspect);
        m_camera->Update();
        m_view = m_camera->GetView();
        m_proj = m_camera->GetProj();
    }
    else
    {
        m_view = Matrix::CreateLookAt(m_position, m_focus, Vector3::Up);
        m_proj = Matrix::CreatePerspectiveFieldOfView(m_fov, aspect, 0.1f, 1000.0f);
    }
}

} // namespace Camera
