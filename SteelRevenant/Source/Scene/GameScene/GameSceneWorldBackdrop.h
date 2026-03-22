// Contents(処理内容) ゲームシーンの背景（ワールドバックドロップ）を管理・描画する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d11.h>
#include <memory>
#include <SimpleMath.h>
#include <GeometricPrimitive.h>

#include "GameSceneVisualPalette.h" // 背景色パレット定義を使用

// Background sky layers used by front scenes and gameplay.
// 日本語コメント: このクラスはゲームシーンおよびフロントシーンで使用する背景（スカイレイヤー）を管理し描画する。
class GameSceneWorldBackdrop
{
public:
    // 初期化関数: 描画に必要なプリミティブを作成する
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    // 描画関数: 指定されたビュー/射影行列とカメラ位置を用いて背景を描画する
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPos);

private:
    struct SkyLayer
    {
        std::unique_ptr<DirectX::GeometricPrimitive> sphere; // レイヤーごとの球メッシュ
        DirectX::SimpleMath::Color color;                    // レイヤーの色
        float scale, yOffset;                                // スケールと高さオフセット
    };
    static constexpr int kLayerCount = 5;
    SkyLayer m_layers[kLayerCount];
};

// --- 以下に簡易実装をインラインで配置 ---

// Contents(処理内容) 各レイヤーのプリミティブを生成して初期化する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
inline void GameSceneWorldBackdrop::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    // 各レイヤーの色・スケール・オフセットを設定し、球プリミティブを作成する
    using namespace GameSceneVisualPalette;
    const DirectX::SimpleMath::Color colors[kLayerCount] = { skyZenithColor, skyUpperColor, skyMidColor, skyLowerColor, skyHorizonColor };
    const float scales[kLayerCount]   = { 500.0f, 490.0f, 480.0f, 470.0f, 460.0f };
    const float offsets[kLayerCount]  = {  80.0f,  40.0f,   0.0f, -40.0f, -80.0f };

    for (int i = 0; i < kLayerCount; ++i)
    {
        m_layers[i].color   = colors[i];
        m_layers[i].scale   = scales[i];
        m_layers[i].yOffset = offsets[i];
        // 元の実装に合わせ、context を渡して GeometricPrimitive を作成する
        m_layers[i].sphere  = DirectX::GeometricPrimitive::CreateSphere(context, 2.0f, 8, false);
    }
    (void)device;
}

// Contents(処理内容) 各レイヤーをカメラ位置に合わせて描画する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
inline void GameSceneWorldBackdrop::Render(ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection,
    const DirectX::SimpleMath::Vector3& cameraPos)
{
    // 描画ステートを設定したうえで、各レイヤーをカメラ位置に合わせて描画する
    System::DrawManager::GetInstance().ApplyPrimitiveState();
    for (int i = 0; i < kLayerCount; ++i)
    {
        if (!m_layers[i].sphere) continue;
        const DirectX::SimpleMath::Matrix world = DirectX::SimpleMath::Matrix::CreateScale(m_layers[i].scale)
            * DirectX::SimpleMath::Matrix::CreateTranslation(cameraPos.x, cameraPos.y + m_layers[i].yOffset, cameraPos.z);
        m_layers[i].sphere->Draw(world, view, projection, m_layers[i].color);
    }
    (void)context;
}

