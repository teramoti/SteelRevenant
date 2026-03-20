#pragma once

//--------------------------------------------------------------------------------------
// File: GameScene.h
//
// 本編シーンの中核クラス。
// - 戦闘進行、敵補充、ステージギミック、カメラ、HUD描画を統括する
// - 個別ロジックは CombatSystem / SurvivalDirector へ委譲する
// - シーンとしては初期化、更新順序、描画順序、遷移判定を管理する
//--------------------------------------------------------------------------------------

#include "../Base/ActionSceneBase.h"

#include <vector>
#include <memory>
#include <string>
#include <wrl/client.h>
#include <GeometricPrimitive.h>
#include <CommonStates.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <Keyboard.h>
#include <Mouse.h>

#include "../../Utility/DirectX11.h"
#include "../../Action/GameState.h"
#include "../../Action/CombatSystem.h"
#include "../../Action/PathGrid.h"
#include "../../Action/AStarSolver.h"
#include "../../Action/SurvivalDirector.h"
#include "../../GameSystem/UIShaderText.h"
#include "StageFloorStyle.h"
#include "SlashHitEffectSystem.h"

class GameScene : public ActionSceneBase
{
public:
	// ゲームシーンの共有参照と初期状態を構築する。
	GameScene(SceneManager* scenemaneger);
	// ゲームシーンが保持する一時リソースを解放する。
	~GameScene();

public:
	// 本編シーンの描画、戦闘、進行状態を初期化する。
	void Initialize() override;
	// 入力、戦闘、演出、進行を 1 フレーム更新する。
	void Update(const DX::StepTimer& stepTimer) override;
	// 本編シーンのワールド描画と UI 描画を実行する。
	void Render() override;
	// 本編シーンの一時リソースと状態を破棄する。
	void Finalize() override;

private:
	// 現在ステージの配置と描画リソースをまとめて初期化する。
	void SetupStage();
	// 中継地点とハザード領域を配置する。
	void SetupRelayNodesAndHazards();
	// アリーナ目標レイヤの進行表示を更新する。
	void UpdateArenaObjectiveLayer(float dt);
	// 指定座標がハザード領域内か判定して返す。
	bool IsInsideHazardZone(const DirectX::SimpleMath::Vector3& position, size_t zoneIndex) const;
	// 指定ハザードの巡回基準座標を返す。
	DirectX::SimpleMath::Vector3 EvaluatePatrolHazardPosition(size_t hazardIndex) const;
	// 指定範囲内の生存敵数を返す。
	int CountLivingEnemiesNear(const DirectX::SimpleMath::Vector3& position, float radius) const;
	// 制圧済み中継地点数を返す。
	int GetCapturedRelayCount() const;
	// クリア条件に必要な中継地点数を返す。
	int GetRequiredRelayCount() const;
	// 現在不足している敵を補充する。
	void SpawnEnemyBatch(bool initialSpawn);
	// 現在の条件から敵出現候補位置を収集する。
	std::vector<DirectX::SimpleMath::Vector3> BuildSpawnCandidates(bool initialSpawn) const;
	// 現在生存している敵数を返す。
	int CountLivingEnemies() const;
	// 障害物の当たり判定情報を再構築する。
	void BuildObstacleBounds();
	// プレイヤーと障害物のめり込みを補正する。
	void ResolvePlayerObstacleCollision(const DirectX::SimpleMath::Vector3& previousPosition);
	// 指定座標が障害物内部か判定して返す。
	bool IsInsideObstacle(const DirectX::SimpleMath::Vector3& position, float radius) const;
	// 指定座標に対応する地面高さを返す。
	float ResolveGroundHeight(const DirectX::SimpleMath::Vector3& position) const;
	// 入力とプレイヤー状態に応じてカメラを更新する。
	void UpdateCamera(float dt, const Action::InputSnapshot& input);
	// 現在入力を戦闘更新用のスナップショットへまとめる。
	Action::InputSnapshot BuildInputSnapshot(float dt) const;
	// ポーズ切り替え入力を処理し、切り替え有無を返す。
	bool HandlePauseToggle(const DirectX::Keyboard::KeyboardStateTracker& keyTracker);
	// ポーズ中の入力とメニュー状態を更新する。
	bool UpdatePausedState(
		float dt,
		const DirectX::Mouse::ButtonStateTracker& mouseTracker,
		const DirectX::Mouse::State& mouseState);
	// デバッグ用調整値ショートカットを処理する。
	void UpdateDebugTuningHotkeys(const DirectX::Keyboard::KeyboardStateTracker& keyTracker);
	// ロックオン対象の選択状態を更新する。
	void UpdateLockOn(const Action::InputSnapshot& input);
	// プレイヤーと敵の戦闘フレームを更新する。
	void UpdateCombatFrame(float dt, const Action::InputSnapshot& input);
	// サバイバル進行と結果遷移条件を更新する。
	void UpdateSurvivalFlow();
	// 現在状態からライブスコアを算出する。
	int ComputeStageScore() const;
	// 集計した戦闘結果を保存して結果シーンへ進める。
	void PushResultAndExit();
	// 3D ワールド描画を実行する。
	void DrawWorld();
	// スカイ、遠景、雲など背景演出を描画する。
	void DrawWorldBackdrop();
	// 床、障害物、ギミックなどアリーナ要素を描画する。
	void DrawWorldArena();
	// プレイヤー、敵、斬撃エフェクトを描画する。
	void DrawWorldActors();
	// HUD とポーズ UI を描画する。
	void DrawUI();
	// 攻撃演出用の補間率を返す。
	float GetAttackBlend() const;
	// 残り時間を MM:SS 形式の文字列へ整形する。
	std::wstring BuildTimerMMSS(float seconds) const;
	// 単色矩形を UI 描画用に描く。
	void DrawSolidRect(
		DirectX::SpriteBatch* batch,
		const DirectX::SimpleMath::Vector2& position,
		const DirectX::SimpleMath::Vector2& size,
		const DirectX::SimpleMath::Color& color) const;
	// ワールド座標をミニマップ座標へ変換する。
	DirectX::SimpleMath::Vector2 ToMiniMapPoint(
		const DirectX::SimpleMath::Vector3& worldPos,
		const DirectX::SimpleMath::Vector2& mapTopLeft,
		float mapSize) const;
	// ミニマップと各種マーカーを描画する。
	void DrawMiniMap(DirectX::SpriteBatch* batch, System::UIShaderText* uiText, const System::UIShaderStyle& style);
	// ポーズメニューの選択状態を更新する。
	void UpdatePauseMenu(
		const DirectX::Mouse::ButtonStateTracker& mouseTracker,
		const DirectX::Mouse::State& mouseState);
	// ポーズ画面のオーバーレイ UI を描画する。
	void DrawPauseOverlay(
		DirectX::SpriteBatch* batch,
		System::UIShaderText* uiText,
		const System::UIShaderStyle& titleStyle,
		const System::UIShaderStyle& selectedStyle,
		const System::UIShaderStyle& normalStyle,
		const System::UIShaderStyle& helpStyle,
		float width,
		float height);
	// 一時的な視覚演出状態を初期値へ戻す。
	void ResetTransientVisualState();
	// カメラシェイク量を加算する。
	void AddCameraShake(float intensity, float durationSec);

private:
	Action::GameState m_gameState;
	Action::CombatSystem m_combat;
	Action::PathGrid m_grid;
	Action::AStarSolver m_solver;
	Action::SurvivalDirector m_survivalDirector;
	Action::PlayerState m_player;
	std::vector<Action::EnemyState> m_enemies;

	std::vector<DirectX::SimpleMath::Matrix> m_obstacleWorlds;
	std::vector<DirectX::SimpleMath::Vector3> m_spawnPoints;
	std::vector<Action::PathGrid::GridCoord> m_minimapBlockedCells;
	struct ObstacleBounds
	{
		float minX;
		float minZ;
		float maxX;
		float maxZ;
	};
	std::vector<ObstacleBounds> m_obstacleBounds;
	struct RelayNodeInfo
	{
		DirectX::SimpleMath::Vector3 position;
		float radius;
		float captureProgress;
		bool captured;
		float pulseSeed;
	};
	struct HazardZoneInfo
	{
		DirectX::SimpleMath::Vector3 center;
		DirectX::SimpleMath::Vector2 halfExtent;
		float phaseOffsetSec;
		float warmupSec;
		float activeSec;
		float damagePerSec;
	};
	struct PatrolHazardInfo
	{
		DirectX::SimpleMath::Vector3 start;
		DirectX::SimpleMath::Vector3 end;
		float radius;
		float cycleSpeed;
		float damagePerSec;
		float phaseSeed;
	};
	struct RecoveryBeaconInfo
	{
		DirectX::SimpleMath::Vector3 position;
		float radius;
		float cooldownSec;
		float maxCooldownSec;
		float pulseSeed;
	};
	std::vector<RelayNodeInfo> m_relayNodes;
	std::vector<HazardZoneInfo> m_hazardZones;
	std::vector<PatrolHazardInfo> m_patrolHazards;
	std::vector<RecoveryBeaconInfo> m_recoveryBeacons;
	int m_requiredRelayCount;
	float m_objectiveBannerTimer;
	std::wstring m_objectiveBannerText;
	SceneFx::SlashHitEffectSystem m_slashHitEffects;
	std::unique_ptr<SceneFx::IStageFloorStyle> m_floorStyle;

	std::unique_ptr<DirectX::GeometricPrimitive> m_floorMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_skyMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_playerMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_enemyMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_weaponMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_obstacleMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_effectOrbMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_effectTrailMesh;
	std::unique_ptr<DirectX::GeometricPrimitive> m_debugCellMesh;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uiSolidTexture;

	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;
	float m_cameraPitch;
	float m_sceneTime;
	float m_hitBloodTimer;
	float m_damageBloodTimer;
	float m_mouseSensitivityView;
	float m_attackAssistRangeView;
	float m_attackAssistDotView;
	float m_cameraShakeTimer;
	float m_cameraShakeStrength;
	float m_hitStopTimer;
	bool m_isPaused;
	int m_pauseSelectedIndex;
	float m_pauseClickFxTimer;
	DirectX::SimpleMath::Vector2 m_pauseClickFxPos;
	int m_recoveryBeaconUseCount;

	float m_finishDelay;
	float m_stageIntroTimer;
	int m_stageThemeIndex;
	bool m_resultPushed;
	bool m_showPathDebug;
	bool m_showHudDetail;
};
