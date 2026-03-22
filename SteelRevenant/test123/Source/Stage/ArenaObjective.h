#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

// ArenaObjective.h
// Defines stage objective helper types used by GameScene.
// This file contains plain definitions for relay nodes, hazard zones,
// patrol hazards and recovery beacons. The implementation focuses on
// data-only structures with a minimal, clear interface.

#include <string>
#include <vector>
#include <SimpleMath.h>

#include "../Core/Combat/CombatTypes.h"
#include "../Core/Combat/GameState.h"

namespace Stage
{
    // Relay capture point
    struct RelayNode
    {
        DirectX::SimpleMath::Vector3 position;
        float radius = 1.5f;          // capture radius in meters
        float captureProgress = 0.0f; // 0..1
        bool captured = false;
        float pulseSeed = 0.0f;
    };

    // Hazard zone (area that periodically activates and deals time damage)
    struct HazardZone
    {
        DirectX::SimpleMath::Vector3 center;
        DirectX::SimpleMath::Vector2 halfExtent;
        float phaseOffsetSec = 0.0f;
        float warmupSec = 0.0f;
        float activeSec = 0.0f;
        float damagePerSec = 0.0f;
    };

    // Patrol hazard (moving hazard along a segment)
    struct PatrolHazard
    {
        DirectX::SimpleMath::Vector3 start;
        DirectX::SimpleMath::Vector3 end;
        float radius = 1.0f;
        float cycleSpeed = 1.0f;
        float damagePerSec = 0.0f;
        float phaseSeed = 0.0f;
    };

    // Recovery beacon (gives time bonus when used)
    struct RecoveryBeacon
    {
        DirectX::SimpleMath::Vector3 position;
        float radius = 1.0f;
        float cooldownSec = 0.0f;
        float maxCooldownSec = 10.0f;
        float pulseSeed = 0.0f;
    };

    class ArenaObjective
    {
    public:
        ArenaObjective();

        // Setup the objective for the given stage theme index (0..2)
        void Setup(int stageThemeIndex);
        void Reset();

        // Update objective state
        void Update(
            float dt,
            const DirectX::SimpleMath::Vector3& playerPos,
            float& playerHp,
            float sceneTime);

        // HUD banner update
        void UpdateBanner(float dt);
        const std::wstring& GetBannerText() const { return m_bannerText; }
        bool IsBannerVisible() const { return m_bannerTimer > 0.0f; }

        // Accessors
        const std::vector<RelayNode>& GetRelayNodes() const { return m_relayNodes; }
        const std::vector<HazardZone>& GetHazardZones() const { return m_hazardZones; }
        const std::vector<PatrolHazard>& GetPatrolHazards() const { return m_patrolHazards; }
        const std::vector<RecoveryBeacon>& GetRecoveryBeacons() const { return m_recoveryBeacons; }

        int GetCapturedCount() const;
        int GetRequiredCount() const { return m_requiredCount; }
        bool IsClearConditionMet() const;
        int GetBeaconUseCount() const { return m_beaconUseCount; }

        DirectX::SimpleMath::Vector3 EvaluatePatrolPosition(size_t index, float sceneTime) const;
        void UpdateRelayNodes(float dt, const DirectX::SimpleMath::Vector3& playerPos);
        void UpdateHazardZones(float dt, const DirectX::SimpleMath::Vector3& playerPos, float& playerHp, float sceneTime);
        void UpdatePatrolHazards(float dt, const DirectX::SimpleMath::Vector3& playerPos, float& playerHp, float sceneTime);
        void UpdateRecoveryBeacons(float dt, const DirectX::SimpleMath::Vector3& playerPos, float& playerHp);

        bool IsInsideHazardZone(const DirectX::SimpleMath::Vector3& pos, size_t zoneIndex) const;
        void ShowBanner(const std::wstring& text);

    private:
        std::vector<RelayNode>      m_relayNodes;
        std::vector<HazardZone>     m_hazardZones;
        std::vector<PatrolHazard>   m_patrolHazards;
        std::vector<RecoveryBeacon> m_recoveryBeacons;

        int m_requiredCount = 0;
        int m_beaconUseCount = 0;

        std::wstring m_bannerText;
        float m_bannerTimer = 0.0f;
    };

} // namespace Stage


