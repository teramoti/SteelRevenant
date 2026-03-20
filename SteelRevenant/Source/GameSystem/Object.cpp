//------------------------//------------------------
// Contents(処理内容) 3Dオブジェクト基底クラスの更新と描画処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "Object.h"

// コンストラクタ。
Teramoto::Object3D::Object3D()
{
	m_scale = DirectX::SimpleMath::Vector3(1, 1, 1);
}

// Object3D が保持するリソースを解放する。
Teramoto::Object3D::~Object3D()
{

}

// ObjectクラスのUpdate関数。
void Teramoto::Object3D::Update()
{
	//回転用行列
	DirectX::SimpleMath::Matrix rotationMatrix;
	//回転用行列に回転用変数を入れる
	rotationMatrix = DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation);
	//移動用行列
	DirectX::SimpleMath::Matrix translationMatrix;
	//移動用行列に移動用変数を入れる
	translationMatrix=	DirectX::SimpleMath::Matrix::CreateTranslation(m_translation);

	//ワールドの設定
	m_world =  rotationMatrix * translationMatrix;

}

//描画
void Teramoto::Object3D::Draw()
{

}

// モデル描画に使うライト設定を反映する。
void Teramoto::Object3D::SetLight()
{
	auto SetLight = [&](DirectX::IEffect* effect)
	{
		DirectX::IEffectLights* lights = dynamic_cast<DirectX::IEffectLights*>(effect);
		if (lights)
		{
			lights->SetAmbientLightColor(DirectX::SimpleMath::Vector3(0.1f, 0.1f, 0.1f));
			// アンビエント色を設定する 
			lights->SetLightEnabled(0, true);
			// １番のライトON 
			lights->SetLightEnabled(1, true);
			// ２番のライトOFF 
			lights->SetLightEnabled(2, true);
			// ３番のライトOFF 
			lights->SetLightDiffuseColor(0,DirectX::Colors::White);
			lights->SetLightDiffuseColor(1,DirectX::Colors::White);
			lights->SetLightDiffuseColor(2,DirectX::Colors::White);
			// ライトの色 
			DirectX::SimpleMath::Vector3 light_dir(-1, 1, 0);
			// ライトの方向ベクトル 
			light_dir.Normalize();
			// ライトの方向ベクトルを正規化する
			lights->SetLightDirection(0, light_dir);
			lights->SetLightDirection(1, DirectX::SimpleMath::Vector3(0, 1, -1));
			lights->SetLightDirection(2, DirectX::SimpleMath::Vector3(1, 1, 1));
			// １番のライトの方向を設定する 
			lights->SetPerPixelLighting(true);
			//ピクセルシェーダーでライトの計算をする
		}
	};

	m_modelData->UpdateEffects(SetLight);
}


