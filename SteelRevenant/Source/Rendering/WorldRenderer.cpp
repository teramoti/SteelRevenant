#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "WorldRenderer.h"

#include "../GameSystem/DrawManager.h"
#include "../Scene/GameScene/Visual/GameSceneVisualPalette.h"

#include <algorithm>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace Rendering
{
    WorldRenderer::WorldRenderer(GeometrySet& geometry)
        : m_geometry(geometry)
    {
    }

    void WorldRenderer::Draw(const WorldRenderContext& ctx) const
    {
        DrawBackdrop(ctx);
        DrawArena(ctx);
        DrawActors(ctx);

        if (ctx.showPathDebug)
        {
            DrawPathDebug(ctx);
        }
    }

    void WorldRenderer::DrawBackdrop(const WorldRenderContext& ctx) const
    {
        if (!m_geometry.sky)
        {
            return;
        }

        System::DrawManager::GetInstance().ApplyPrimitiveState();

        const SceneFx::StageRenderPalette palette = SceneFx::BuildStageRenderPalette(1, ctx.sceneTime);
        const float offsets[] = { 80.0f, 40.0f, 0.0f, -40.0f, -80.0f };
        const float scales[] = { 500.0f, 490.0f, 480.0f, 470.0f, 460.0f };
        const Color colors[] = {
            palette.skyZenithColor,
            palette.skyZenithColor,
            palette.skyMidColor,
            palette.skyBaseColor,
            palette.skyBaseColor
        };

        for (int index = 0; index < 5; ++index)
        {
            const Matrix world =
                Matrix::CreateScale(scales[index]) *
                Matrix::CreateTranslation(0.0f, offsets[index], 0.0f);
            m_geometry.sky->Draw(world, ctx.view, ctx.proj, colors[index]);
        }
    }

    void WorldRenderer::DrawArena(const WorldRenderContext& ctx) const
    {
        System::DrawManager::GetInstance().ApplyPrimitiveState();
        DrawFloor(ctx);
        DrawObstacles(ctx);
        DrawRelayNodes(ctx);
        DrawHazardZones(ctx);
        DrawRecoveryBeacons(ctx);
    }

    void WorldRenderer::DrawFloor(const WorldRenderContext& ctx) const
    {
        if (!m_geometry.floor)
        {
            return;
        }

        SceneFx::FloorPalette palette;
        if (ctx.floorStyle)
        {
            palette = ctx.floorStyle->BuildFloorPalette(ctx.sceneTime);
        }

        const Matrix floorWorld = Matrix::CreateScale(56.0f, 0.2f, 56.0f);
        m_geometry.floor->Draw(floorWorld, ctx.view, ctx.proj, palette.baseColor);

        if (!ctx.floorStyle || !m_geometry.obstacle)
        {
            return;
        }

        std::vector<FloorDrawCommand> commands;
        ctx.floorStyle->BuildDetailCommands(ctx.sceneTime, commands);
        for (const auto& command : commands)
        {
            m_geometry.obstacle->Draw(command.world, ctx.view, ctx.proj, command.color);
        }
    }

    void WorldRenderer::DrawObstacles(const WorldRenderContext& ctx) const
    {
        if (!m_geometry.obstacle || !ctx.obstacleWorlds)
        {
            return;
        }

        const Color obstacleColor(0.20f, 0.22f, 0.26f, 1.0f);
        for (const auto& world : *ctx.obstacleWorlds)
        {
            m_geometry.obstacle->Draw(world, ctx.view, ctx.proj, obstacleColor);
        }
    }

    void WorldRenderer::DrawRelayNodes(const WorldRenderContext& ctx) const
    {
        (void)ctx;
    }

    void WorldRenderer::DrawHazardZones(const WorldRenderContext& ctx) const
    {
        (void)ctx;
    }

    void WorldRenderer::DrawRecoveryBeacons(const WorldRenderContext& ctx) const
    {
        (void)ctx;
    }

    void WorldRenderer::DrawActors(const WorldRenderContext& ctx) const
    {
        System::DrawManager::GetInstance().ApplyPrimitiveState();
        DrawEnemies(ctx);
        DrawPlayer(ctx);
        DrawSlashEffects(ctx);
    }

    void WorldRenderer::DrawPlayer(const WorldRenderContext& ctx) const
    {
        if (!ctx.player || !m_geometry.player)
        {
            return;
        }

        const Color baseColor(0.55f, 0.65f, 0.80f, 1.0f);
        const Color hitColor(1.0f, 0.4f, 0.1f, 1.0f);
        const float hitBlend = std::max(0.0f, (std::min)(1.0f, ctx.hitBloodTimer));
        const Color drawColor = Color::Lerp(baseColor, hitColor, hitBlend);

        const Matrix rotation = Matrix::CreateRotationY(ctx.player->yaw);
        const Matrix bodyWorld = rotation * Matrix::CreateTranslation(ctx.player->position);
        m_geometry.player->Draw(bodyWorld, ctx.view, ctx.proj, drawColor);

        const Matrix headWorld = Matrix::CreateTranslation(0.0f, 1.1f, 0.0f) * bodyWorld;
        m_geometry.player->Draw(headWorld, ctx.view, ctx.proj, drawColor);

        if (!m_geometry.weapon)
        {
            return;
        }

        const float pitch = -0.65f + ctx.attackBlend * 1.8f;
        const Matrix swordLocal =
            Matrix::CreateRotationX(pitch) *
            Matrix::CreateTranslation(0.35f, 0.5f, 0.55f);
        const Color swordColor = ctx.attackBlend > 0.3f
            ? Color(1.0f, 0.95f, 0.6f, 1.0f)
            : Color(0.85f, 0.88f, 0.95f, 1.0f);
        m_geometry.weapon->Draw(swordLocal * bodyWorld, ctx.view, ctx.proj, swordColor);
    }

    void WorldRenderer::DrawEnemies(const WorldRenderContext& ctx) const
    {
        if (!ctx.enemies)
        {
            return;
        }

        for (const auto& enemy : *ctx.enemies)
        {
            DrawSingleEnemy(enemy, ctx);
        }
    }

    void WorldRenderer::DrawSingleEnemy(const Action::EnemyState& enemy, const WorldRenderContext& ctx) const
    {
        if (!m_geometry.enemy)
        {
            return;
        }

        if (enemy.state == Action::EnemyStateType::Dead)
        {
            const Matrix deadWorld =
                Matrix::CreateScale(1.0f, 0.15f, 1.0f) *
                Matrix::CreateTranslation(enemy.position.x, 0.1f, enemy.position.z);
            m_geometry.enemy->Draw(deadWorld, ctx.view, ctx.proj, Color(0.25f, 0.10f, 0.10f, 1.0f));
            return;
        }

        const Color baseColor = enemy.archetype == Action::EnemyArchetype::BladeRush
            ? Color(0.75f, 0.20f, 0.15f, 1.0f)
            : Color(0.55f, 0.15f, 0.55f, 1.0f);
        const float reactBlend = std::max(0.0f, (std::min)(1.0f, enemy.hitReactTimer / 0.26f));
        const Color drawColor = Color::Lerp(baseColor, Color(1.0f, 0.6f, 0.1f, 1.0f), reactBlend);

        const Matrix bodyWorld =
            Matrix::CreateRotationY(enemy.yaw) *
            Matrix::CreateTranslation(enemy.position);
        m_geometry.enemy->Draw(bodyWorld, ctx.view, ctx.proj, drawColor);

        const Matrix headWorld = Matrix::CreateTranslation(0.0f, 1.1f, 0.0f) * bodyWorld;
        m_geometry.enemy->Draw(headWorld, ctx.view, ctx.proj, drawColor);
    }

    void WorldRenderer::DrawSlashEffects(const WorldRenderContext& ctx) const
    {
        if (!ctx.slashEffects || !m_geometry.effectTrail || !m_geometry.effectOrb)
        {
            return;
        }

        System::DrawManager::GetInstance().ApplyAlphaBlendState();
        ctx.slashEffects->Draw(*m_geometry.effectTrail, *m_geometry.effectOrb, ctx.view, ctx.proj);
        System::DrawManager::GetInstance().ApplyPrimitiveState();
    }

    void WorldRenderer::DrawPathDebug(const WorldRenderContext& ctx) const
    {
        if (!ctx.blockedCells)
        {
            return;
        }

        DirectX::GeometricPrimitive* mesh = m_geometry.debugCell
            ? m_geometry.debugCell.get()
            : m_geometry.obstacle.get();
        if (!mesh)
        {
            return;
        }

        System::DrawManager::GetInstance().ApplyAlphaBlendState();
        for (const auto& cell : *ctx.blockedCells)
        {
            const Matrix world =
                Matrix::CreateScale(0.9f, 0.05f, 0.9f) *
                Matrix::CreateTranslation(
                    static_cast<float>(cell.x) - 29.5f,
                    0.1f,
                    static_cast<float>(cell.y) - 29.5f);
            mesh->Draw(world, ctx.view, ctx.proj, Color(1.0f, 0.2f, 0.2f, 0.35f));
        }
        System::DrawManager::GetInstance().ApplyPrimitiveState();
    }
} // namespace Rendering

