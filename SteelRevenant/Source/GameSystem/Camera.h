// Contents(処理内容) カメラのビュー・射影行列および基本的なパラメータを管理する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <SimpleMath.h>

namespace SteelRevenant
{
    class Camera
    {
    public:
        Camera(float width, float height);
        virtual ~Camera();

        virtual void Update();

        DirectX::SimpleMath::Matrix& GetView();
        DirectX::SimpleMath::Matrix& GetProj();
        const DirectX::SimpleMath::Matrix& GetView() const; // added
        const DirectX::SimpleMath::Matrix& GetProj() const; // added

        // New conventional setters
        void SetEyePos(const DirectX::SimpleMath::Vector3& eyepos);
        void SetRefPos(const DirectX::SimpleMath::Vector3& refpos);
        void SetUpVec(const DirectX::SimpleMath::Vector3& upvec);
        void SetFovY(float fovY);
        void SetAspect(float aspect);
        void SetNearClip(float nearclip);
        void SetFarClip(float farclip);
        void SetTargetPos(const DirectX::SimpleMath::Vector3& target);

        // smoothing control (0.0 = instant, 1.0 = very slow)
        void SetSmoothingFactors(float eyeLerp, float refLerp) { m_eyeLerp = eyeLerp; m_refLerp = refLerp; }

        // Backwards-compatible wrappers (original names)
        void Seteyepos(const DirectX::SimpleMath::Vector3& eyepos) { SetEyePos(eyepos); }
        void Setrefpos(const DirectX::SimpleMath::Vector3& refpos) { SetRefPos(refpos); }
        void Setupvec(const DirectX::SimpleMath::Vector3& upvec) { SetUpVec(upvec); }
        void SetfovY(float fovY) { SetFovY(fovY); }
        void Settaspect(float aspect) { SetAspect(aspect); }
        void SetanerClip(float nearclip) { SetNearClip(nearclip); }
        void SetfarClip(float farclip) { SetFarClip(farclip); }
        // 既存の値渡し版は曖昧性を生むため削除し、const参照版のみを公開する。

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

        // smoothing factors in [0,1], where 0 = immediate, higher = slower
        float m_eyeLerp = 0.0f;
        float m_refLerp = 0.0f;
    };
}


