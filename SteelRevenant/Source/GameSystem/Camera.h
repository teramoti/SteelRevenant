//------------------------//------------------------
// Contents(処理内容) 共通カメラの行列更新処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

namespace Teramoto
{
	//----------------------------------------------------------------------------- 
	// Camera
	//----------------------------------------------------------------------------- 
	// ビュー行列・射影行列を管理する基底カメラ。
	// 各シーン固有カメラは本クラスを継承して Update を拡張する。
	//----------------------------------------------------------------------------- 
	class Camera
	{
	protected:
		// 現在フレームのビュー行列。
		DirectX::SimpleMath::Matrix m_View;
		// カメラ位置。
		DirectX::SimpleMath::Vector3 m_Eyepos;
		// 注視点。
		DirectX::SimpleMath::Vector3 m_Refpos;
		// 上方向ベクトル。
		DirectX::SimpleMath::Vector3 m_Upvec;
		// 現在フレームの射影行列。
		DirectX::SimpleMath::Matrix m_Proj;
		// 追従対象のワールド位置。
		DirectX::SimpleMath::Vector3 m_target;
		// 垂直視野角。
		float m_FovY;
		// アスペクト比。
		float m_Aspect;
		// 近クリップ距離。
		float m_NearClip;
		// 遠クリップ距離。
		float m_FarClip;

	public:
		// ビューポート幅・高さから既定カメラを構築する。
		Camera(float w, float h);
		// カメラリソースを破棄する。
		virtual ~Camera();

		// ビュー/射影行列を更新する。
		virtual void Update();

		// 最新ビュー行列を参照で取得する。
		DirectX::SimpleMath::Matrix& GetView();
		// 最新射影行列を参照で取得する。
		DirectX::SimpleMath::Matrix& GetProj();

		// カメラ位置を設定する。
		void Seteyepos(const DirectX::SimpleMath::Vector3& eyepos);
		// 注視点を設定する。
		void Setrefpos(const DirectX::SimpleMath::Vector3& refpos);
		// 上方向ベクトルを設定する。
		void Setupvec(const DirectX::SimpleMath::Vector3& upvec);
		// 垂直視野角を設定する。
		void SetfovY(float fovY);
		// アスペクト比を設定する。
		void Settaspect(float aspect);
		// 近クリップ距離を設定する。
		void SetanerClip(float nearclip);
		// 遠クリップ距離を設定する。
		void SetfarClip(float farclip);

		// 追従対象位置を設定する。
		void SetTargetPos(DirectX::SimpleMath::Vector3 target);
	};
}

