#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// TPS カメラの位置・注視点・衝突回避・FOV 平滑化を管理する。
//------------------------

#include <vector>
#include <SimpleMath.h>

#include "../../GameSystem/Camera.h"
#include "../../Action/CombatSystem.h"

class GameSceneCameraController
{
public:
    // 初期化: プレイヤーのヨー角に合わせてカメラ状態をリセットする
    void Reset(float playerYaw, float screenWidth, float screenHeight);

    // カメラヨーをプレイヤー/ロックオン対象に追従させる
    void TrackCameraYaw(float playerYaw,
                        const std::vector<Action::EnemyState>& enemies,
                        int lockEnemyIndex,
                        const DirectX::SimpleMath::Vector3& playerPos);

    // カメラ位置・注視点・行列を更新する
    void UpdateTransform(
        const DirectX::SimpleMath::Vector3& playerPos,
        float playerYaw,
        const std::vector<Action::EnemyState>& enemies,
        int lockEnemyIndex,
        const std::vector<DirectX::SimpleMath::Vector4>& wallRects,
        const DirectX::SimpleMath::Vector2& shakeOffset,
        float cameraHeightOverride,
        float aspect);

    // カメラ基底ベクトルを取得する（移動方向の算出に使用）
    DirectX::SimpleMath::Vector3 GetCameraForward() const;
    DirectX::SimpleMath::Vector3 GetCameraRight() const;

    // 行列の取得
    const DirectX::SimpleMath::Matrix& GetView() const { return m_view; }
    const DirectX::SimpleMath::Matrix& GetProj() const { return m_proj; }

    float GetYaw() const   { return m_yaw; }
    float GetPitch() const { return m_pitch; }
    float GetFov() const   { return m_fov; }

    void SetTargetFov(float fov) { m_targetFov = fov; }

private:
    // カメラ希望位置を壁との衝突で補正する
    DirectX::SimpleMath::Vector3 ResolveWallOcclusion(
        const DirectX::SimpleMath::Vector3& playerHead,
        const DirectX::SimpleMath::Vector3& desiredPos,
        const std::vector<DirectX::SimpleMath::Vector4>& wallRects) const;

    // カメラ位置を平滑化する
    void SmoothCameraPosition(
        const DirectX::SimpleMath::Vector3& adjustedPos,
        const DirectX::SimpleMath::Vector2& shakeOffset);

    // 注視点を更新する（ロックオン有無で分岐）
    void UpdateLookTarget(
        const DirectX::SimpleMath::Vector3& playerPos,
        float playerYaw,
        const std::vector<Action::EnemyState>& enemies,
        int lockEnemyIndex);

    // ビュー・射影行列を再計算する
    void RebuildMatrices(float aspect);

    std::unique_ptr<SteelRevenant::Camera> m_camera;

    DirectX::SimpleMath::Vector3 m_position;
    DirectX::SimpleMath::Vector3 m_target;
    DirectX::SimpleMath::Matrix  m_view;
    DirectX::SimpleMath::Matrix  m_proj;

    float m_yaw       = 0.0f;
    float m_pitch     = 0.0f;
    float m_fov       = 0.0f;
    float m_targetFov = 0.0f;

    // カメラ配置定数
    static constexpr float kHeight         = 5.0f;
    static constexpr float kBack           = 8.0f;
    static constexpr float kShoulderOffset = 0.35f;
    static constexpr float kSmoothFactor   = 0.12f;
    static constexpr float kFixedPitch     = -0.35f;
    static constexpr float kPlayerHeadY    = 1.2f;
    static constexpr float kChestOffsetY   = 0.9f;
    static constexpr float kForwardLook    = 6.0f;
    static constexpr float kYawLerp        = 0.12f;
    static constexpr float kXZLerpMin      = 0.05f;
    static constexpr float kXZLerpMax      = 0.6f;
    static constexpr float kYLerp          = 0.06f;
    static constexpr float kMaxDeltaY      = 0.18f;
    static constexpr float kFovBlend       = 0.08f;
    static constexpr float kShakeScale     = 0.05f;
    static constexpr float kDefaultFov     = 3.14159265f / 4.0f;

    // 壁衝突回避定数
    static constexpr int   kOcclusionSamples  = 28;
    static constexpr float kOcclusionMargin   = 0.35f;
    static constexpr float kOcclusionPullBack = 0.08f;
    static constexpr float kOcclusionMinT     = 0.25f;

    // ロックオン注視点ブレンド比率
    static constexpr float kLockEnemyBias  = 0.45f;
    static constexpr float kLockPlayerBias = 0.55f;
};
