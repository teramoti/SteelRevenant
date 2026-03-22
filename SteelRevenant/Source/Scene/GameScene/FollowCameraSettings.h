#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// KH風追従カメラの設定値を集約する構造体。
// カメラ配置・壁衝突回避・ロックオンブレンドの全パラメータを保持する。
//------------------------

namespace Camera
{
    // カメラ追従の基本配置設定
    struct FollowPlacement
    {
        float distance    = 6.0f;   // プレイヤー後方への距離
        float height      = 3.0f;   // プレイヤーからの高さ
        float focusHeight = 1.4f;   // 注視点のプレイヤー地面からの高さ（胸元）
        float pitchDeg    = 20.0f;  // 固定ピッチ角（度数法）
        float shoulderOffset = 0.35f; // 肩越しの横方向オフセット
        float forwardLook = 6.0f;   // フリー時の注視点前方距離
    };

    // 補間設定
    struct FollowSmoothing
    {
        float positionLerp = 0.12f; // XZ 位置の補間率
        float verticalLerp = 0.05f; // Y 位置の補間率（弱補間で段差揺れを抑制）
        float maxVerticalDeltaPerFrame = 0.18f; // Y の毎フレーム最大変化量
        float yawLerp      = 0.12f; // yaw 追従の補間率
        float fovBlend     = 0.08f; // FOV 平滑化の補間率
        float shakeScale   = 0.05f; // スクリーンシェイクの適用倍率
    };

    // 壁衝突回避設定
    struct WallOcclusion
    {
        int   sampleCount  = 28;    // 遮蔽チェックのサンプル数
        float margin       = 0.35f; // 壁からのクリアランス余白
        float pullBack     = 0.08f; // 遮蔽検出時の手前引き戻し量
        float minT         = 0.25f; // 引き戻し下限パラメータ
        float playerHeadY  = 1.2f;  // 遮蔽チェック起点のプレイヤー頭部高さ
    };

    // ロックオン時の注視点ブレンド設定
    struct LockOnBlend
    {
        float enemyBias  = 0.45f;   // 注視点における敵位置の比率
        float playerBias = 0.55f;   // 注視点におけるプレイヤー位置の比率
    };

    // 全設定を束ねるルート構造体
    struct FollowCameraSettings
    {
        FollowPlacement placement;
        FollowSmoothing smoothing;
        WallOcclusion   occlusion;
        LockOnBlend     lockOn;
        float defaultFov = 3.14159265f / 4.0f; // デフォルト画角（ラジアン）
    };
}
