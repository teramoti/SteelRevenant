#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// KH風3D追従カメラの状態管理と更新ロジックを担当する。
// 設定値は FollowCameraSettings に分離されている。
//------------------------

#include <memory>
#include <vector>
#include <SimpleMath.h>

#include "FollowCameraSettings.h"
#include "../../GameSystem/Camera.h"
#include "../../Action/CombatSystem.h"

namespace Camera
{
    class FollowCameraController
    {
    public:
        // カメラを初期状態にする
        void Initialize(const FollowCameraSettings& settings,
                        float playerYaw,
                        float screenWidth,
                        float screenHeight);

        // 設定を差し替える（ステージ切替時など）
        void ApplySettings(const FollowCameraSettings& settings);

        // カメラ状態をリセットする（プレイヤー復帰時など）
        void Reset(float playerYaw);

        // 毎フレームの更新（GameScene から呼ばれる唯一のエントリ）
        void Update(
            const DirectX::SimpleMath::Vector3& playerPos,
            float playerYaw,
            const std::vector<Action::EnemyState>& enemies,
            int lockEnemyIndex,
            const std::vector<DirectX::SimpleMath::Vector4>& wallRects,
            const DirectX::SimpleMath::Vector2& shakeOffset,
            float cameraHeightOverride,
            float aspect);

        // 計算結果の取得
        const DirectX::SimpleMath::Vector3& GetCameraPosition() const { return m_position; }
        const DirectX::SimpleMath::Vector3& GetFocusPosition()  const { return m_focus; }
        const DirectX::SimpleMath::Matrix&  GetView()           const { return m_view; }
        const DirectX::SimpleMath::Matrix&  GetProj()           const { return m_proj; }

        // カメラ基底ベクトル（プレイヤー移動方向の算出に使用）
        DirectX::SimpleMath::Vector3 GetForward() const;
        DirectX::SimpleMath::Vector3 GetRight()   const;

        float GetYaw()   const { return m_yaw; }
        float GetPitch() const { return m_pitch; }
        float GetFov()   const { return m_fov; }

        void SetTargetFov(float fov) { m_targetFov = fov; }

    private:
        // yaw をプレイヤーまたはロックオン対象に追従させる
        void TrackYaw(float playerYaw,
                      const std::vector<Action::EnemyState>& enemies,
                      int lockEnemyIndex,
                      const DirectX::SimpleMath::Vector3& playerPos);

        // 希望カメラ位置を算出する
        DirectX::SimpleMath::Vector3 ComputeDesiredPosition(
            const DirectX::SimpleMath::Vector3& playerPos,
            float playerYaw,
            float height) const;

        // 壁遮蔽を検出してカメラ位置を補正する
        DirectX::SimpleMath::Vector3 ResolveWallOcclusion(
            const DirectX::SimpleMath::Vector3& playerHead,
            const DirectX::SimpleMath::Vector3& desiredPos,
            const std::vector<DirectX::SimpleMath::Vector4>& wallRects) const;

        // カメラ位置を XZ/Y 分離補間で平滑化する
        void SmoothPosition(
            const DirectX::SimpleMath::Vector3& adjustedPos,
            const DirectX::SimpleMath::Vector2& shakeOffset);

        // 注視点を更新する（ロックオン有無で分岐）
        void UpdateFocusTarget(
            const DirectX::SimpleMath::Vector3& playerPos,
            float playerYaw,
            const std::vector<Action::EnemyState>& enemies,
            int lockEnemyIndex);

        // ビュー・射影行列を再計算する
        void RebuildMatrices(float aspect);

        FollowCameraSettings m_settings;
        std::unique_ptr<SteelRevenant::Camera> m_camera;

        DirectX::SimpleMath::Vector3 m_position;
        DirectX::SimpleMath::Vector3 m_focus;
        DirectX::SimpleMath::Matrix  m_view;
        DirectX::SimpleMath::Matrix  m_proj;

        float m_yaw       = 0.0f;
        float m_pitch     = 0.0f;
        float m_fov       = 0.0f;
        float m_targetFov = 0.0f;
    };
}
