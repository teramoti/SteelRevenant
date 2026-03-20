//------------------------//------------------------
// Contents(処理内容) 背景・遠景・空の描画を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "../GameScene.h"

#include <cmath>

#include "GameSceneVisualPalette.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    //-------------------------------------------------------------------------
    // 遠景ビル群の固定配置テーブル
    // x, z : ワールド座標  w : 幅  h : 高さ  d : 奥行き
    //-------------------------------------------------------------------------
    struct BuildingDef { float x; float z; float w; float h; float d; };
    constexpr BuildingDef kBuildings[] =
    {
        // 北側
        { -38.0f, -54.0f,  6.0f, 18.0f, 5.0f },
        { -24.0f, -56.0f,  4.0f, 12.0f, 4.0f },
        { -12.0f, -58.0f,  9.0f, 28.0f, 7.0f },
        {   0.0f, -57.0f,  5.0f, 14.0f, 5.0f },
        {  14.0f, -58.0f,  7.0f, 22.0f, 6.0f },
        {  28.0f, -56.0f,  4.0f, 10.0f, 4.0f },
        {  40.0f, -54.0f, 10.0f, 32.0f, 8.0f },
        // 南側
        { -36.0f,  54.0f,  8.0f, 20.0f, 6.0f },
        {  -8.0f,  57.0f,  5.0f, 16.0f, 5.0f },
        {  10.0f,  58.0f,  7.0f, 24.0f, 6.0f },
        {  30.0f,  56.0f,  4.0f, 11.0f, 4.0f },
        {  46.0f,  54.0f,  6.0f, 18.0f, 5.0f },
        // 西側
        { -56.0f, -32.0f,  5.0f, 15.0f, 5.0f },
        { -58.0f,  -4.0f,  6.0f, 26.0f, 6.0f },
        { -57.0f,  20.0f,  4.0f, 12.0f, 4.0f },
        { -55.0f,  40.0f,  8.0f, 22.0f, 7.0f },
        // 東側
        {  54.0f, -24.0f,  5.0f, 14.0f, 5.0f },
        {  57.0f,   6.0f,  6.0f, 30.0f, 6.0f },
        {  55.0f,  30.0f,  4.0f, 13.0f, 4.0f },
    };
    constexpr int kBuildingCount = static_cast<int>(sizeof(kBuildings) / sizeof(kBuildings[0]));

    //-------------------------------------------------------------------------
    // スカイ層の定義
    // 天頂→地平線 に向かって5層を重ね、グラデーションを作る
    //
    // 【仕組み】
    // DirectX の球メッシュは内面描画（裏向き）になるため、
    // 大きい球を外側（天頂）、小さい球を内側（地平線付近）に置くことで
    // 上から下へのグラデーションを擬似的に表現できる。
    //-------------------------------------------------------------------------
    struct SkyLayerDef
    {
        float scaleXZ;   // 水平スケール
        float scaleY;    // 垂直スケール
        float offsetY;   // 中心Y オフセット（負 = 下寄り = 地平線側）
        float rotSpeed;  // 回転速度
    };
    constexpr SkyLayerDef kSkyLayers[] =
    {
        { 220.0f, 220.0f,   0.0f,  0.030f }, // 天頂層（最外・最暗）
        { 190.0f, 190.0f,  -6.0f, -0.022f }, // 上空層
        { 168.0f, 140.0f, -18.0f,  0.015f }, // 中空層（ヘイズ）
        { 148.0f,  90.0f, -32.0f, -0.010f }, // 地平線層
        { 132.0f,  55.0f, -48.0f,  0.007f }, // 地平線グロー（最内・最明）
    };
    constexpr int kSkyLayerCount = static_cast<int>(sizeof(kSkyLayers) / sizeof(kSkyLayers[0]));
}

void GameScene::DrawWorldBackdrop()
{
    const SceneFx::StageRenderPalette palette =
        SceneFx::BuildStageRenderPalette(m_stageThemeIndex, m_sceneTime);
    const auto ST = [&palette](const Color& base, float a = 1.0f) -> Color
    {
        return SceneFx::ApplyStageTint(palette, base, a);
    };

    const Vector3 skyAnchor(m_player.position.x, 0.0f, m_player.position.z);

    //=========================================================================
    // スカイドーム（5層グラデーション）
    //
    // 市販ゲームのスカイドームは必ず複数層。
    // 天頂は濃く・暗く、地平線に向かって明るく・色が変わる。
    // 各層に異なる回転速度を与えることで雲・大気の動きを演出する。
    //=========================================================================

    // ステージごとの空のカラーを5段階で定義
    Color skyColors[kSkyLayerCount];
    switch (std::max(1, std::min(3, m_stageThemeIndex)))
    {
    case 1: // ネオン都市夜景
        // 天頂: 深い夜空 → 地平線: 都市光のオレンジ残光
        skyColors[0] = Color(0.04f, 0.04f, 0.10f, 0.95f); // 天頂: 深夜
        skyColors[1] = Color(0.08f, 0.08f, 0.18f, 0.80f); // 上空
        skyColors[2] = Color(0.12f, 0.10f, 0.22f, 0.65f); // 中空: 都市光混じり
        skyColors[3] = Color(0.28f, 0.14f, 0.10f, 0.50f); // 地平線: 夕暮れ残光
        skyColors[4] = Color(0.60f, 0.28f, 0.08f, 0.40f); // グロー: オレンジ
        break;
    case 2: // 廃工場・溶鉱炉
        // 天頂: 煤煙の暗褐色 → 地平線: 溶鉱炉の赤オレンジ
        skyColors[0] = Color(0.08f, 0.04f, 0.02f, 0.95f); // 天頂: 煤煙
        skyColors[1] = Color(0.16f, 0.07f, 0.03f, 0.82f); // 上空
        skyColors[2] = Color(0.28f, 0.12f, 0.04f, 0.68f); // 中空: 橙霞
        skyColors[3] = Color(0.55f, 0.22f, 0.04f, 0.55f); // 地平線: 炎色
        skyColors[4] = Color(1.00f, 0.45f, 0.06f, 0.45f); // グロー: 溶鉱炉
        break;
    case 3: // 廃都市・吹雪
        // 天頂: 鉛色の嵐 → 地平線: 稲光の白
        skyColors[0] = Color(0.14f, 0.16f, 0.20f, 0.95f); // 天頂: 嵐雲
        skyColors[1] = Color(0.22f, 0.26f, 0.34f, 0.80f); // 上空
        skyColors[2] = Color(0.32f, 0.38f, 0.48f, 0.65f); // 中空: 吹雪
        skyColors[3] = Color(0.48f, 0.55f, 0.66f, 0.50f); // 地平線: 雪光
        skyColors[4] = Color(0.72f, 0.82f, 0.95f, 0.40f); // グロー: 稲光
        break;
    default:
        for (int li = 0; li < kSkyLayerCount; ++li)
            skyColors[li] = palette.skyBaseColor;
        break;
    }

    // 各層を描画
    for (int li = 0; li < kSkyLayerCount; ++li)
    {
        const SkyLayerDef& layer = kSkyLayers[li];
        m_skyMesh->Draw(
            Matrix::CreateScale(layer.scaleXZ, layer.scaleY, layer.scaleXZ) *
            Matrix::CreateRotationY(m_sceneTime * layer.rotSpeed) *
            Matrix::CreateTranslation(skyAnchor.x, layer.offsetY, skyAnchor.z),
            m_view, m_proj, ST(skyColors[li]));
    }

    //=========================================================================
    // 雲層（スカイドームの上にさらに重ねる）
    //
    // 扁平な球を複数枚、高度を変えてゆっくり流す。
    // α値を低く抑えることで「うっすら見える雲」を表現。
    //=========================================================================
    const float cloudAlpha = (m_stageThemeIndex == 3) ? 0.28f : 0.14f; // 吹雪は雲多め
    const Color cloudColor = ST(Color(
        skyColors[2].R() * 1.3f,
        skyColors[2].G() * 1.3f,
        skyColors[2].B() * 1.4f, cloudAlpha));

    // 雲層A（高い）
    m_skyMesh->Draw(
        Matrix::CreateScale(180.0f, 18.0f, 180.0f) *
        Matrix::CreateRotationY(m_sceneTime * 0.012f) *
        Matrix::CreateTranslation(skyAnchor.x, 40.0f, skyAnchor.z),
        m_view, m_proj, cloudColor);

    // 雲層B（中高度、逆回転で立体感）
    m_skyMesh->Draw(
        Matrix::CreateScale(160.0f, 14.0f, 160.0f) *
        Matrix::CreateRotationY(-m_sceneTime * 0.008f) *
        Matrix::CreateTranslation(skyAnchor.x, 28.0f, skyAnchor.z),
        m_view, m_proj, Color(cloudColor.R(), cloudColor.G(), cloudColor.B(), cloudAlpha * 0.7f));

    //=========================================================================
    // 遠景ビル群（シルエット）
    //
    // 暗いシルエット本体 + 屋上リムライト + 窓の光 + アンテナ点滅
    // この組み合わせで「夜の都市」が成立する。
    //=========================================================================
    const Color buildingBase = ST(Color(0.04f, 0.05f, 0.08f, 0.95f));
    const Color buildingRim  = ST(Color(
        skyColors[3].R() * 0.7f,
        skyColors[3].G() * 0.6f,
        skyColors[3].B() * 0.5f, 0.60f));
    const Color windowBase = ST(Color(
        skyColors[4].R() * 0.9f,
        skyColors[4].G() * 0.8f,
        0.18f, 0.40f));

    for (int bi = 0; bi < kBuildingCount; ++bi)
    {
        const BuildingDef& b = kBuildings[bi];
        const float seed = static_cast<float>(bi);

        // 本体シルエット
        m_obstacleMesh->Draw(
            Matrix::CreateScale(b.w, b.h, b.d) *
            Matrix::CreateTranslation(b.x, b.h * 0.5f, b.z),
            m_view, m_proj, buildingBase);

        // 屋上リムライト（地平線色の反射）
        m_obstacleMesh->Draw(
            Matrix::CreateScale(b.w + 0.4f, 0.20f, b.d + 0.4f) *
            Matrix::CreateTranslation(b.x, b.h + 0.10f, b.z),
            m_view, m_proj, buildingRim);

        // 窓の光（各ビルでゆらぎ位相を変える）
        const float flickerA = std::sinf(m_sceneTime * 1.1f + seed * 0.83f) * 0.10f + 0.90f;
        const float flickerB = std::sinf(m_sceneTime * 0.7f + seed * 1.27f) * 0.08f + 0.92f;
        // 上半分の窓
        m_obstacleMesh->Draw(
            Matrix::CreateScale(b.w * 0.50f, b.h * 0.30f, 0.14f) *
            Matrix::CreateTranslation(b.x, b.h * 0.68f, b.z + b.d * 0.5f + 0.06f),
            m_view, m_proj, Color(windowBase.R() * flickerA, windowBase.G() * flickerA,
                                   windowBase.B(), windowBase.A()));
        // 下半分の窓（別系統の点滅）
        if (bi % 3 != 0)
        {
            m_obstacleMesh->Draw(
                Matrix::CreateScale(b.w * 0.40f, b.h * 0.20f, 0.12f) *
                Matrix::CreateTranslation(b.x, b.h * 0.32f, b.z + b.d * 0.5f + 0.05f),
                m_view, m_proj, Color(windowBase.R() * flickerB, windowBase.G() * flickerB,
                                       windowBase.B() * 0.8f, windowBase.A() * 0.7f));
        }

        // アンテナ（高いビルのみ）
        if (b.h >= 18.0f)
        {
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.18f, 4.0f, 0.18f) *
                Matrix::CreateTranslation(b.x, b.h + 2.0f, b.z),
                m_view, m_proj, buildingBase);

            // 先端の赤ランプ（ビルごとに点滅タイミングをずらす）
            const float blink = std::sinf(m_sceneTime * 2.2f + seed * 1.4f) > 0.2f ? 1.0f : 0.15f;
            m_effectOrbMesh->Draw(
                Matrix::CreateScale(0.22f) *
                Matrix::CreateTranslation(b.x, b.h + 4.1f, b.z),
                m_view, m_proj, Color(1.0f, 0.12f, 0.06f, blink * 0.95f));
        }
    }

    //=========================================================================
    // 地平線の霧（ビル群と床の境界をぼかす）
    //
    // 下部に白みがかった半透明の球を置くことで
    // 「遠くがかすんで見える」自然な霧感を出す。
    //=========================================================================
    const Color fogColor = ST(Color(
        skyColors[3].R() * 0.8f,
        skyColors[3].G() * 0.8f,
        skyColors[3].B() * 0.9f, 0.42f));

    m_skyMesh->Draw(
        Matrix::CreateScale(130.0f, 14.0f, 130.0f) *
        Matrix::CreateTranslation(skyAnchor.x, 3.0f, skyAnchor.z),
        m_view, m_proj, fogColor);

    // 床際の最下層霧（一番濃い）
    m_skyMesh->Draw(
        Matrix::CreateScale(110.0f, 6.0f, 110.0f) *
        Matrix::CreateTranslation(skyAnchor.x, -1.0f, skyAnchor.z),
        m_view, m_proj, Color(fogColor.R(), fogColor.G(), fogColor.B(), 0.55f));
}
