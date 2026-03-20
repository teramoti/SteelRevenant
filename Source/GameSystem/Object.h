//------------------------//------------------------
// Contents(処理内容) 3Dオブジェクト基底クラスの更新と描画処理を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include <windows.h>
#include <wrl/client.h>
#include <Effects.h>
#include <CommonStates.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <Model.h>
#include <Keyboard.h>
// 既存カメラ型への参照依存
#include "Camera.h"
#include "../Utility/DirectX11.h"
#pragma once
//オブジェクト3Dクラス
namespace Teramoto
{
	class Object3D
	{
	public:
	protected:
		//モデル
		std::unique_ptr<DirectX::Model> m_modelData;
		//汎用ステート
		std::unique_ptr<DirectX::CommonStates> m_States;
		std::unique_ptr<DirectX::EffectFactory> m_Factory;
		//カメラ
		Camera* ccamera;
		//スケーリング
		DirectX::SimpleMath::Vector3 m_scale;
		//回転角
		//DirectX::SimpleMath::Vector3 m_rotation;
		//平行
		DirectX::SimpleMath::Vector3 m_translation;
		//行列
		DirectX::SimpleMath::Matrix m_world;
		DirectX::SimpleMath::Vector3 m_rotatrionV;
		DirectX::SimpleMath::Quaternion m_rotation;
		// 描画デバイス管理シングルトンへの参照。
		DirectX11& m_directX11 = DirectX11::Get();
		//角度
		float m_angle;
	public:
		//コンストラクタ
		Object3D();
		// Object3D が保持するリソースを解放する。
		~Object3D();
		//更新
		void Update();
		//描画
		virtual void Draw();
		// 変換状態の設定。
		// スケールを設定する。
		void SetScale(DirectX::SimpleMath::Vector3& scale)
		{
			m_scale = scale;
		}
		// 平行移動量を設定する。
		void SetTranslation(DirectX::SimpleMath::Vector3& translation)
		{
			m_translation = translation;
		}
		// ワールド行列を設定する。
		void SetWorld(DirectX::SimpleMath::Matrix world)
		{
			m_world = world;
		}
		// 回転をクォータニオンで設定する。
		void SetRotation(DirectX::SimpleMath::Quaternion rotation)
		{
			m_rotation = rotation;
		}
		// 回転角を設定する。
		void SetAngle(float angle)
		{
			m_angle = angle;
		}
		// 変換状態の取得。
		// 保持している回転角を返す。
		float GetAngle()
		{
			return  m_angle;
		}
		// スケールを参照で返す。
		DirectX::SimpleMath::Vector3& GetScale()
		{
			return m_scale;
		}
		// 平行移動量を参照で返す。
		DirectX::SimpleMath::Vector3& GetTranslation()
		{
			return m_translation;
		}
		// ワールド行列を参照で返す。
		DirectX::SimpleMath::Matrix& GetWorld()
		{
			return m_world;
		}
		// 回転クォータニオンを値で返す。
		DirectX::SimpleMath::Quaternion GetRotation()
		{
			return m_rotation;
		}
		// モデル描画に使うライト設定を反映する。
		void SetLight();
	};
}
