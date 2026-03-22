#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <memory>
#include <vector>

#include <GeometricPrimitive.h>
#include <SimpleMath.h>

#include "../Action/CombatSystem.h"
#include "../Action/PathGrid.h"
#include "../Scene/GameScene/SlashHitEffectSystem.h"
#include "../Scene/GameScene/StageFloorStyle.h"

namespace Rendering
{
    struct WorldRenderContext
    {
        DirectX::SimpleMath::Matrix view;
        DirectX::SimpleMath::Matrix proj;

        const Action::PlayerState* player = nullptr;
        const std::vector<Action::EnemyState>* enemies = nullptr;

        const IFloorStyle* floorStyle = nullptr;
        const std::vector<DirectX::SimpleMath::Matrix>* obstacleWorlds = nullptr;
        const SceneFx::SlashHitEffectSystem* slashEffects = nullptr;

        float sceneTime = 0.0f;
        float attackBlend = 0.0f;
        float hitBloodTimer = 0.0f;

        bool showPathDebug = false;
        const std::vector<Action::PathGrid::GridCoord>* blockedCells = nullptr;
    };

    class WorldRenderer
    {
    public:
        struct GeometrySet
        {
            std::unique_ptr<DirectX::GeometricPrimitive> floor;
            std::unique_ptr<DirectX::GeometricPrimitive> sky;
            std::unique_ptr<DirectX::GeometricPrimitive> player;
            std::unique_ptr<DirectX::GeometricPrimitive> enemy;
            std::unique_ptr<DirectX::GeometricPrimitive> weapon;
            std::unique_ptr<DirectX::GeometricPrimitive> obstacle;
            std::unique_ptr<DirectX::GeometricPrimitive> effectOrb;
            std::unique_ptr<DirectX::GeometricPrimitive> effectTrail;
            std::unique_ptr<DirectX::GeometricPrimitive> debugCell;
        };

        explicit WorldRenderer(GeometrySet& geometry);

        void Draw(const WorldRenderContext& context) const;

    private:
        void DrawBackdrop(const WorldRenderContext& ctx) const;
        void DrawArena(const WorldRenderContext& ctx) const;
        void DrawFloor(const WorldRenderContext& ctx) const;
        void DrawObstacles(const WorldRenderContext& ctx) const;
        void DrawRelayNodes(const WorldRenderContext& ctx) const;
        void DrawHazardZones(const WorldRenderContext& ctx) const;
        void DrawRecoveryBeacons(const WorldRenderContext& ctx) const;
        void DrawActors(const WorldRenderContext& ctx) const;
        void DrawPlayer(const WorldRenderContext& ctx) const;
        void DrawEnemies(const WorldRenderContext& ctx) const;
        void DrawSingleEnemy(const Action::EnemyState& enemy, const WorldRenderContext& ctx) const;
        void DrawSlashEffects(const WorldRenderContext& ctx) const;
        void DrawPathDebug(const WorldRenderContext& ctx) const;

    private:
        GeometrySet& m_geometry;
    };
} // namespace Rendering

