#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GameScene.h"

#include <algorithm>
#include <cmath>

#include "../../Action/BattleRuleBook.h"
#include "../../Utility/SimpleMathEx.h"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr float kGroundHeight = 0.8f;
    constexpr float kSpawnSpacing = 2.0f;
    constexpr float kSpawnSpacingSq = kSpawnSpacing * kSpawnSpacing;
    constexpr float kOpeningMinDistance = 10.0f;
    constexpr float kOpeningMaxDistance = 24.0f;
    constexpr float kSustainMinDistance = 6.0f;
    constexpr float kRearNearDistance = 10.0f;
    constexpr float kPlayerSafetyRadius = 6.0f;
    constexpr float kFrontAngleRad = DirectX::XMConvertToRadians(65.0f);
    constexpr float kSideMinAngleRad = DirectX::XMConvertToRadians(70.0f);
    constexpr float kSideMaxAngleRad = DirectX::XMConvertToRadians(115.0f);
    constexpr float kRearBlockDot = -0.5f;

    float FlatDistanceSq(const Vector3& a, const Vector3& b)
    {
        const Vector3 d = a - b;
        return d.x * d.x + d.z * d.z;
    }

    bool HasSpacingFromList(const std::vector<Vector3>& points, const Vector3& candidate, float minDistanceSq)
    {
        for (size_t i = 0; i < points.size(); ++i)
        {
            if (FlatDistanceSq(points[i], candidate) < minDistanceSq)
            {
                return false;
            }
        }
        return true;
    }

    float BuildCandidateScore(const Vector3& position, int stageThemeIndex)
    {
        return std::sinf(position.x * 0.73f + position.z * 1.17f + static_cast<float>(stageThemeIndex) * 0.91f);
    }

    void StableSortCandidates(std::vector<Vector3>& candidates, int stageThemeIndex)
    {
        std::stable_sort(candidates.begin(), candidates.end(), [stageThemeIndex](const Vector3& lhs, const Vector3& rhs)
        {
            return BuildCandidateScore(lhs, stageThemeIndex) > BuildCandidateScore(rhs, stageThemeIndex);
        });
    }

    void AppendFilteredCandidates(std::vector<Vector3>& ordered, const std::vector<Vector3>& source, size_t wantedCount)
    {
        size_t accepted = 0;
        for (size_t i = 0; i < source.size() && accepted < wantedCount; ++i)
        {
            if (!HasSpacingFromList(ordered, source[i], kSpawnSpacingSq))
            {
                continue;
            }
            ordered.push_back(source[i]);
            ++accepted;
        }
    }

    void AppendRemainingCandidates(std::vector<Vector3>& ordered, const std::vector<Vector3>& source)
    {
        for (size_t i = 0; i < source.size(); ++i)
        {
            if (!HasSpacingFromList(ordered, source[i], kSpawnSpacingSq))
            {
                continue;
            }
            ordered.push_back(source[i]);
        }
    }
}

void GameScene::SetupStage()
{
    m_grid.Initialize(60, 60, 1.0f, Vector2(-30.0f, -30.0f));

    for (int x = 0; x < m_grid.GetWidth(); ++x)
    {
        m_grid.SetBlocked(Action::PathGrid::GridCoord(x, 0), true);
        m_grid.SetBlocked(Action::PathGrid::GridCoord(x, m_grid.GetHeight() - 1), true);
    }
    for (int y = 0; y < m_grid.GetHeight(); ++y)
    {
        m_grid.SetBlocked(Action::PathGrid::GridCoord(0, y), true);
        m_grid.SetBlocked(Action::PathGrid::GridCoord(m_grid.GetWidth() - 1, y), true);
    }

    m_obstacleWorlds.clear();
    BuildObstacleBounds();
    for (size_t i = 0; i < m_obstacleBounds.size(); ++i)
    {
        const ObstacleBounds& bounds = m_obstacleBounds[i];
        const float scaleX = std::max(0.5f, (bounds.maxX - bounds.minX) * 0.5f);
        const float scaleZ = std::max(0.5f, (bounds.maxZ - bounds.minZ) * 0.5f);
        const float centerX = (bounds.minX + bounds.maxX) * 0.5f;
        const float centerZ = (bounds.minZ + bounds.maxZ) * 0.5f;

        float obstacleHeight = 2.0f;
        if (m_stageThemeIndex == 2)
        {
            obstacleHeight = 2.6f + static_cast<float>(i % 3) * 0.45f;
        }
        else if (m_stageThemeIndex == 3)
        {
            obstacleHeight = 2.2f + static_cast<float>((i + 1) % 4) * 0.52f;
        }

        m_obstacleWorlds.push_back(
            Matrix::CreateScale(scaleX, obstacleHeight, scaleZ) *
            Matrix::CreateTranslation(centerX, obstacleHeight * 0.5f, centerZ));
        m_grid.SetBlockedRect(bounds.minX, bounds.minZ, bounds.maxX, bounds.maxZ, true);
    }

    m_spawnPoints.clear();
    switch (m_stageThemeIndex)
    {
    case 2:
        m_spawnPoints.push_back(Vector3(-24.0f, kGroundHeight, -22.0f));
        m_spawnPoints.push_back(Vector3(24.0f, kGroundHeight, -22.0f));
        m_spawnPoints.push_back(Vector3(-24.0f, kGroundHeight, 22.0f));
        m_spawnPoints.push_back(Vector3(24.0f, kGroundHeight, 22.0f));
        m_spawnPoints.push_back(Vector3(-25.0f, kGroundHeight, 0.0f));
        m_spawnPoints.push_back(Vector3(25.0f, kGroundHeight, 0.0f));
        break;

    case 3:
        m_spawnPoints.push_back(Vector3(-24.0f, kGroundHeight, -18.0f));
        m_spawnPoints.push_back(Vector3(24.0f, kGroundHeight, 18.0f));
        m_spawnPoints.push_back(Vector3(-18.0f, kGroundHeight, 24.0f));
        m_spawnPoints.push_back(Vector3(18.0f, kGroundHeight, -24.0f));
        m_spawnPoints.push_back(Vector3(-24.0f, kGroundHeight, 18.0f));
        m_spawnPoints.push_back(Vector3(24.0f, kGroundHeight, -18.0f));
        break;

    default:
        m_spawnPoints.push_back(Vector3(-24.0f, kGroundHeight, -24.0f));
        m_spawnPoints.push_back(Vector3(24.0f, kGroundHeight, -24.0f));
        m_spawnPoints.push_back(Vector3(-24.0f, kGroundHeight, 24.0f));
        m_spawnPoints.push_back(Vector3(24.0f, kGroundHeight, 24.0f));
        m_spawnPoints.push_back(Vector3(0.0f, kGroundHeight, 26.0f));
        m_spawnPoints.push_back(Vector3(0.0f, kGroundHeight, -26.0f));
        break;
    }

    m_minimapBlockedCells = m_grid.GetBlockedCells();
}

void GameScene::BuildObstacleBounds()
{
    m_obstacleBounds.clear();
    switch (m_stageThemeIndex)
    {
    case 1:
        break;

    case 2:
        m_obstacleBounds.push_back(ObstacleBounds{ -6.0f, -18.0f, 6.0f, -12.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ -6.0f, 12.0f, 6.0f, 18.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ -22.0f, -4.0f, -14.0f, 4.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ 14.0f, -4.0f, 22.0f, 4.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ -10.0f, -4.0f, -4.0f, 4.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ 4.0f, -4.0f, 10.0f, 4.0f });
        break;

    case 3:
        m_obstacleBounds.push_back(ObstacleBounds{ -5.0f, -5.0f, 5.0f, 5.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ -18.0f, -18.0f, -12.0f, -12.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ 12.0f, 12.0f, 18.0f, 18.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ -18.0f, 12.0f, -12.0f, 18.0f });
        m_obstacleBounds.push_back(ObstacleBounds{ 12.0f, -18.0f, 18.0f, -12.0f });
        break;

    default:
        break;
    }
}

bool GameScene::IsInsideObstacle(const Vector3& position, float radius) const
{
    for (size_t i = 0; i < m_obstacleBounds.size(); ++i)
    {
        const ObstacleBounds& bounds = m_obstacleBounds[i];
        if (position.x > (bounds.minX - radius) && position.x < (bounds.maxX + radius) &&
            position.z > (bounds.minZ - radius) && position.z < (bounds.maxZ + radius))
        {
            return true;
        }
    }
    return false;
}

float GameScene::ResolveGroundHeight(const Vector3& position) const
{
    (void)position;
    return kGroundHeight;
}

void GameScene::ResolvePlayerObstacleCollision(const Vector3& previousPosition)
{
    const float collisionRadius = 0.55f;
    const float keepHeight = m_player.position.y;
    Vector3 desired = m_player.position;
    desired.y = kGroundHeight;

    Vector3 previous = previousPosition;
    previous.y = kGroundHeight;

    const float travel = (desired - previous).Length();
    const int sweepSteps = std::max(1, static_cast<int>(std::ceil(travel / 0.2f)));
    Vector3 lastSafe = previous;
    bool blockedOnSweep = false;
    for (int step = 1; step <= sweepSteps; ++step)
    {
        const float t = static_cast<float>(step) / static_cast<float>(sweepSteps);
        Vector3 probe = previous + (desired - previous) * t;
        probe.y = kGroundHeight;
        if (IsInsideObstacle(probe, collisionRadius))
        {
            blockedOnSweep = true;
            break;
        }
        lastSafe = probe;
    }

    if (!blockedOnSweep && !IsInsideObstacle(desired, collisionRadius))
    {
        m_player.position = desired;
        m_player.position.y = keepHeight;
        return;
    }

    Vector3 resolved = lastSafe;
    Vector3 tryX(desired.x, kGroundHeight, lastSafe.z);
    Vector3 tryZ(lastSafe.x, kGroundHeight, desired.z);
    const bool xOk = !IsInsideObstacle(tryX, collisionRadius);
    const bool zOk = !IsInsideObstacle(tryZ, collisionRadius);

    if (xOk && zOk)
    {
        Vector3 merged(tryX.x, kGroundHeight, tryZ.z);
        resolved = IsInsideObstacle(merged, collisionRadius) ? tryX : merged;
    }
    else if (xOk)
    {
        resolved = tryX;
    }
    else if (zOk)
    {
        resolved = tryZ;
    }

    m_player.position = resolved;
    m_player.position.y = keepHeight;
}

std::vector<Vector3> GameScene::BuildSpawnCandidates(bool initialSpawn) const
{
    std::vector<Vector3> frontCandidates;
    std::vector<Vector3> leftCandidates;
    std::vector<Vector3> rightCandidates;
    std::vector<Vector3> sustainCandidates;
    std::vector<Vector3> ordered;

    Vector3 forward(std::sin(m_player.yaw), 0.0f, std::cos(m_player.yaw));
    forward = Utility::MathEx::SafeNormalize(forward);
    if (forward.LengthSquared() <= 0.0001f)
    {
        forward = Vector3::UnitZ;
    }

    const float frontDotMin = std::cos(kFrontAngleRad);
    const float sideDotMin = std::cos(kSideMaxAngleRad);
    const float sideDotMax = std::cos(kSideMinAngleRad);
    const float openingMinDistSq = kOpeningMinDistance * kOpeningMinDistance;
    const float openingMaxDistSq = kOpeningMaxDistance * kOpeningMaxDistance;
    const float sustainMinDistSq = kSustainMinDistance * kSustainMinDistance;
    const float rearNearDistSq = kRearNearDistance * kRearNearDistance;

    for (int y = 0; y < m_grid.GetHeight(); ++y)
    {
        for (int x = 0; x < m_grid.GetWidth(); ++x)
        {
            const Action::PathGrid::GridCoord coord(x, y);
            if (!m_grid.IsWalkable(coord))
            {
                continue;
            }

            Vector3 candidate = m_grid.GridToWorld(coord, kGroundHeight);
            if (IsInsideObstacle(candidate, 0.55f))
            {
                continue;
            }

            bool tooCloseToEnemy = false;
            for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
            {
                if (m_enemies[enemyIndex].state == Action::EnemyStateType::Dead)
                {
                    continue;
                }
                if (FlatDistanceSq(candidate, m_enemies[enemyIndex].position) < kSpawnSpacingSq)
                {
                    tooCloseToEnemy = true;
                    break;
                }
            }
            if (tooCloseToEnemy)
            {
                continue;
            }

            Vector3 delta = candidate - m_player.position;
            delta.y = 0.0f;
            const float distSq = delta.LengthSquared();
            if (distSq <= 0.0001f)
            {
                continue;
            }

            Vector3 dir = delta;
            dir.Normalize();
            const float forwardDot = forward.Dot(dir);
            const float cross = forward.x * dir.z - forward.z * dir.x;

            if (initialSpawn)
            {
                if (distSq < openingMinDistSq || distSq > openingMaxDistSq || distSq < (kPlayerSafetyRadius * kPlayerSafetyRadius))
                {
                    continue;
                }

                if (forwardDot >= frontDotMin)
                {
                    frontCandidates.push_back(candidate);
                }
                else if (forwardDot >= sideDotMin && forwardDot <= sideDotMax)
                {
                    if (cross >= 0.0f)
                    {
                        leftCandidates.push_back(candidate);
                    }
                    else
                    {
                        rightCandidates.push_back(candidate);
                    }
                }
            }
            else
            {
                if (distSq < sustainMinDistSq)
                {
                    continue;
                }
                if (distSq < rearNearDistSq && forwardDot <= kRearBlockDot)
                {
                    continue;
                }
                sustainCandidates.push_back(candidate);
            }
        }
    }

    if (initialSpawn)
    {
        StableSortCandidates(frontCandidates, m_stageThemeIndex);
        StableSortCandidates(leftCandidates, m_stageThemeIndex);
        StableSortCandidates(rightCandidates, m_stageThemeIndex);

        const int openingAliveCount = Action::BattleRuleBook::GetInstance().GetOpeningAliveCount();
        const size_t frontWanted = static_cast<size_t>(std::round(static_cast<float>(openingAliveCount) * 0.6f));
        const size_t leftWanted = static_cast<size_t>(std::round(static_cast<float>(openingAliveCount) * 0.2f));
        const size_t rightWanted = static_cast<size_t>(std::max(0, openingAliveCount - static_cast<int>(frontWanted) - static_cast<int>(leftWanted)));
        ordered.reserve(frontCandidates.size() + leftCandidates.size() + rightCandidates.size());
        AppendFilteredCandidates(ordered, frontCandidates, frontWanted);
        AppendFilteredCandidates(ordered, leftCandidates, leftWanted);
        AppendFilteredCandidates(ordered, rightCandidates, rightWanted);
        AppendRemainingCandidates(ordered, frontCandidates);
        AppendRemainingCandidates(ordered, leftCandidates);
        AppendRemainingCandidates(ordered, rightCandidates);
    }
    else
    {
        StableSortCandidates(sustainCandidates, m_stageThemeIndex);
        ordered.reserve(sustainCandidates.size());
        AppendRemainingCandidates(ordered, sustainCandidates);
    }

    if (ordered.empty())
    {
        return m_spawnPoints;
    }
    return ordered;
}

void GameScene::SpawnEnemyBatch(bool initialSpawn)
{
    std::vector<Action::EnemyState> spawnedEnemies;
    if (initialSpawn)
    {
        const std::vector<Vector3> spawnCandidates = BuildSpawnCandidates(true);
        spawnedEnemies = m_survivalDirector.BuildInitialSpawn(spawnCandidates);
    }
    else
    {
        const int livingEnemyCount = CountLivingEnemies();
        if (!m_survivalDirector.CanSpawn(livingEnemyCount))
        {
            return;
        }
        const std::vector<Vector3> spawnCandidates = BuildSpawnCandidates(false);
        spawnedEnemies = m_survivalDirector.BuildSpawnBatch(spawnCandidates, livingEnemyCount);
    }

    for (size_t enemyIndex = 0; enemyIndex < spawnedEnemies.size(); ++enemyIndex)
    {
        m_enemies.push_back(spawnedEnemies[enemyIndex]);
    }
}

int GameScene::CountLivingEnemies() const
{
    int livingCount = 0;
    for (size_t enemyIndex = 0; enemyIndex < m_enemies.size(); ++enemyIndex)
    {
        if (m_enemies[enemyIndex].state != Action::EnemyStateType::Dead)
        {
            ++livingCount;
        }
    }
    return livingCount;
}

