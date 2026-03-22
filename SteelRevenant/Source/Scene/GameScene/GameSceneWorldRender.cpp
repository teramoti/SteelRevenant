#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GameScene.h"

#include "../../Utility/DirectX11.h"
#include "Visual\GameSceneVisualPalette.h"

// 描画エントリ: ワールドの全体描画を管理する。
// - 背景 (バックドロップ)
// - ステージ (アリーナ)
// - スピードアップアイテム
// - アクター (プレイヤー / 敵)
void GameScene::DrawWorld()
{
    if (!m_floorMesh || !m_playerMesh || !m_enemyMesh || !m_weaponMesh || !m_skyMesh || !m_effectOrbMesh || !m_effectTrailMesh)
    {
        return;
    }

    DrawWorldBackdrop();
    DrawWorldArena();
    DrawSpeedUpItems();
    DrawWorldActors();
}

// スピードアップアイテムの描画 (アリーナ層へ委譲)
void GameScene::DrawSpeedUpItems()
{
    m_arenaLayer.Render(DirectX11::Get().GetContext().Get(), m_view, m_proj);
}

// バックドロップ描画: シーンの遠景を複数レイヤーで描画する
// 各レイヤーはカメラ位置に追従して描かれ、回転やスケールを持つ
void GameScene::DrawWorldBackdrop()
{
    using DirectX::SimpleMath::Color;
    using DirectX::SimpleMath::Matrix;
    using DirectX::SimpleMath::Vector3;

    const SceneFx::StageRenderPalette palette = SceneFx::BuildStageRenderPalette(m_stageThemeIndex, m_sceneTime);
    const auto StageTint = [&palette](const Color& base, float alphaScale = 1.0f) -> Color
    {
        return SceneFx::ApplyStageTint(palette, base, alphaScale);
    };

    const Vector3 skyAnchor(m_player.position.x, 0.0f, m_player.position.z);

    const Color skyColors[] =
    {
        palette.skyZenithColor,
        palette.skyMidColor,
        palette.skyBaseColor,
        palette.skyHazeColor,
        palette.horizonGlowColor
    };

    struct SkyLayerDef { float scaleXZ; float scaleY; float offsetY; float rotSpeed; };
    constexpr SkyLayerDef kSkyLayers[] =
    {
        { 220.0f, 220.0f,   0.0f,  0.030f },
        { 190.0f, 190.0f,  -6.0f, -0.022f },
        { 168.0f, 140.0f, -18.0f,  0.015f },
        { 148.0f,  90.0f, -32.0f, -0.010f },
        { 132.0f,  55.0f, -48.0f,  0.007f },
    };

    for (int li = 0; li < static_cast<int>(std::size(kSkyLayers)); ++li)
    {
        const SkyLayerDef& layer = kSkyLayers[li];
        m_skyMesh->Draw(
            Matrix::CreateScale(layer.scaleXZ, layer.scaleY, layer.scaleXZ) *
            Matrix::CreateRotationY(m_sceneTime * layer.rotSpeed) *
            Matrix::CreateTranslation(skyAnchor.x, layer.offsetY, skyAnchor.z),
            m_view, m_proj, StageTint(skyColors[li]));
    }

    m_skyMesh->Draw(
        Matrix::CreateScale(190.0f, 22.0f, 190.0f) *
        Matrix::CreateTranslation(skyAnchor.x, -12.0f, skyAnchor.z),
        m_view, m_proj, StageTint(Color(
            palette.horizonGlowColor.R(),
            palette.horizonGlowColor.G(),
            palette.horizonGlowColor.B(),
            0.60f)));

    const float cloudDriftA = m_sceneTime * 0.012f;
    const float cloudDriftB = -m_sceneTime * 0.008f;
    m_skyMesh->Draw(
        Matrix::CreateScale(180.0f, 18.0f, 180.0f) *
        Matrix::CreateRotationY(cloudDriftA) *
        Matrix::CreateTranslation(skyAnchor.x, 40.0f, skyAnchor.z),
        m_view, m_proj, StageTint(palette.cloudColor, 0.65f));
    m_skyMesh->Draw(
        Matrix::CreateScale(160.0f, 14.0f, 160.0f) *
        Matrix::CreateRotationY(cloudDriftB) *
        Matrix::CreateTranslation(skyAnchor.x, 28.0f, skyAnchor.z),
        m_view, m_proj, StageTint(Color(
            palette.cloudColor.R(),
            palette.cloudColor.G(),
            palette.cloudColor.B(),
            palette.cloudColor.A() * 0.85f)));

    if (palette.showNightElements)
    {
        const Vector3 moonPos = skyAnchor + Vector3(34.0f, 62.0f, -26.0f);
        m_effectOrbMesh->Draw(
            Matrix::CreateScale(5.5f) * Matrix::CreateTranslation(moonPos),
            m_view, m_proj, StageTint(palette.moonColor));

        m_skyMesh->Draw(
            Matrix::CreateScale(18.0f, 80.0f, 18.0f) *
            Matrix::CreateRotationZ(0.08f) *
            Matrix::CreateTranslation(moonPos.x - 4.0f, 22.0f, moonPos.z + 2.0f),
            m_view, m_proj, StageTint(palette.beamColor, 0.7f));
    }

    const Color buildingBase = StageTint(Color(0.04f, 0.05f, 0.08f, 0.95f));
    const Color buildingRim = StageTint(Color(
        palette.skyHazeColor.R() * 0.9f,
        palette.skyHazeColor.G() * 0.9f,
        palette.skyHazeColor.B() * 0.9f,
        0.55f));
    const Color windowBase = StageTint(Color(
        palette.accentColor.R(),
        palette.accentColor.G() * 0.82f + 0.10f,
        0.18f,
        0.40f));

    struct BuildingDef { float x; float z; float w; float h; float d; };
    constexpr BuildingDef kBuildings[] =
    {
        { -38.0f, -54.0f,  6.0f, 18.0f, 5.0f },
        { -24.0f, -56.0f,  4.0f, 12.0f, 4.0f },
        { -12.0f, -58.0f,  9.0f, 28.0f, 7.0f },
        {   0.0f, -57.0f,  5.0f, 14.0f, 5.0f },
        {  14.0f, -58.0f,  7.0f, 22.0f, 6.0f },
        {  28.0f, -56.0f,  4.0f, 10.0f, 4.0f },
        {  40.0f, -54.0f, 10.0f, 32.0f, 8.0f },
        { -36.0f,  54.0f,  8.0f, 20.0f, 6.0f },
        {  -8.0f,  57.0f,  5.0f, 16.0f, 5.0f },
        {  10.0f,  58.0f,  7.0f, 24.0f, 6.0f },
        {  30.0f,  56.0f,  4.0f, 11.0f, 4.0f },
        {  46.0f,  54.0f,  6.0f, 18.0f, 5.0f },
        { -56.0f, -32.0f,  5.0f, 15.0f, 5.0f },
        { -58.0f,  -4.0f,  6.0f, 26.0f, 6.0f },
        { -57.0f,  20.0f,  4.0f, 12.0f, 4.0f },
        { -55.0f,  40.0f,  8.0f, 22.0f, 7.0f },
        {  54.0f, -24.0f,  5.0f, 14.0f, 5.0f },
        {  57.0f,   6.0f,  6.0f, 30.0f, 6.0f },
        {  55.0f,  30.0f,  4.0f, 13.0f, 4.0f },
    };

    for (int bi = 0; bi < static_cast<int>(std::size(kBuildings)); ++bi)
    {
        const auto& b = kBuildings[bi];
        const float seed = static_cast<float>(bi);

        m_obstacleMesh->Draw(
            Matrix::CreateScale(b.w, b.h, b.d) *
            Matrix::CreateTranslation(b.x, b.h * 0.5f, b.z),
            m_view, m_proj, buildingBase);

        m_obstacleMesh->Draw(
            Matrix::CreateScale(b.w + 0.4f, 0.20f, b.d + 0.4f) *
            Matrix::CreateTranslation(b.x, b.h + 0.10f, b.z),
            m_view, m_proj, buildingRim);

        const float flickerA = std::sinf(m_sceneTime * 1.1f + seed * 0.83f) * 0.10f + 0.90f;
        const float flickerB = std::sinf(m_sceneTime * 0.7f + seed * 1.27f) * 0.08f + 0.92f;

        m_obstacleMesh->Draw(
            Matrix::CreateScale(b.w * 0.50f, b.h * 0.30f, 0.14f) *
            Matrix::CreateTranslation(b.x, b.h * 0.68f, b.z + b.d * 0.5f + 0.06f),
            m_view, m_proj, Color(
                windowBase.R() * flickerA,
                windowBase.G() * flickerA,
                windowBase.B(),
                windowBase.A()));

        if (bi % 3 != 0)
        {
            m_obstacleMesh->Draw(
                Matrix::CreateScale(b.w * 0.40f, b.h * 0.20f, 0.12f) *
                Matrix::CreateTranslation(b.x, b.h * 0.32f, b.z + b.d * 0.5f + 0.05f),
                m_view, m_proj, Color(
                    windowBase.R() * flickerB,
                    windowBase.G() * flickerB,
                    windowBase.B() * 0.80f,
                    windowBase.A() * 0.70f));
        }

        if (b.h >= 18.0f)
        {
            m_obstacleMesh->Draw(
                Matrix::CreateScale(0.18f, 4.0f, 0.18f) *
                Matrix::CreateTranslation(b.x, b.h + 2.0f, b.z),
                m_view, m_proj, buildingBase);

            const float blink = (std::sinf(m_sceneTime * 2.2f + seed * 1.4f) > 0.2f) ? 1.0f : 0.15f;
            m_effectOrbMesh->Draw(
                Matrix::CreateScale(0.22f) *
                Matrix::CreateTranslation(b.x, b.h + 4.1f, b.z),
                m_view, m_proj, Color(1.0f, 0.12f, 0.06f, blink * 0.95f));
        }
    }

    const Color fogColor = StageTint(Color(
        palette.skyHazeColor.R(),
        palette.skyHazeColor.G(),
        palette.skyHazeColor.B(),
        0.42f));
    m_skyMesh->Draw(
        Matrix::CreateScale(130.0f, 14.0f, 130.0f) *
        Matrix::CreateTranslation(skyAnchor.x, 3.0f, skyAnchor.z),
        m_view, m_proj, fogColor);
    m_skyMesh->Draw(
        Matrix::CreateScale(110.0f, 6.0f, 110.0f) *
        Matrix::CreateTranslation(skyAnchor.x, -1.0f, skyAnchor.z),
        m_view, m_proj, Color(fogColor.R(), fogColor.G(), fogColor.B(), 0.55f));

    if (palette.showLightning)
    {
        const float flashAlpha = Utility::MathEx::Clamp(palette.lightningFlicker * 0.55f, 0.0f, 1.0f);
        m_skyMesh->Draw(
            Matrix::CreateScale(210.0f, 210.0f, 210.0f) *
            Matrix::CreateTranslation(skyAnchor.x, 0.0f, skyAnchor.z),
            m_view, m_proj, Color(0.80f, 0.90f, 1.0f, flashAlpha));
    }
}


