//------------------------//------------------------
// Contents(処理内容) タイトル画面の更新と描画処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "TitleScene.h"

#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/UiUtil.h"
#include "../Base/SceneUiSound.h"
#include "../SceneManager/SceneManager.h"

#include <Keyboard.h>
#include <Mouse.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <windows.h>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

extern void ExitGame();

namespace
{
	// タイトル演出の基本パラメータ。
	constexpr float kTransitionSec = 0.42f;
	constexpr float kAttractHintIdleSec = 8.0f;
	constexpr int kMenuCount = 3;
	// タイトルの縦メニュー配置を画面サイズと項目数から計算する。
	struct TitleMenuLayout
	{
		Vector2 center = Vector2::Zero;
		float uiScale = 1.0f;
		float panelWidth = 0.0f;
		float panelHeight = 0.0f;
		float panelTop = 0.0f;
		float cardWidth = 0.0f;
		float cardHeight = 0.0f;
		float cardGap = 0.0f;
		float listStartY = 0.0f;
	};

	// タイトルメニューの表示領域とカード位置を返す。
	TitleMenuLayout BuildTitleMenuLayout(float width, float height)
	{
		TitleMenuLayout layout;
		layout.uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));
		layout.center = Vector2(width * 0.5f, height * 0.53f);
		layout.panelWidth = 492.0f * layout.uiScale;
		layout.cardWidth = 444.0f * layout.uiScale;
		layout.cardHeight = 56.0f * layout.uiScale;
		layout.cardGap = 10.0f * layout.uiScale;

		const float panelPaddingTop = 14.0f * layout.uiScale;
		const float panelPaddingBottom = 18.0f * layout.uiScale;
		const float listHeight =
			layout.cardHeight * static_cast<float>(kMenuCount) +
			layout.cardGap * static_cast<float>(std::max(0, kMenuCount - 1));
		layout.panelHeight = listHeight + panelPaddingTop + panelPaddingBottom;
		layout.panelTop = layout.center.y - layout.panelHeight * 0.5f;
		layout.listStartY = layout.panelTop + panelPaddingTop;
		return layout;
	}

	// OSカーソル座標をクライアント領域へ変換して返す。
	Vector2 ResolveMouseClientPoint(const DirectX::Mouse::State& mouseState, HWND hwnd, float width, float height)
	{
		Vector2 result(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));
		if (hwnd != nullptr)
		{
			POINT point = {};
			if (::GetCursorPos(&point) != FALSE && ::ScreenToClient(hwnd, &point) != FALSE)
			{
				result.x = static_cast<float>(point.x);
				result.y = static_cast<float>(point.y);
			}
		}

		const float maxX = std::max(0.0f, width - 1.0f);
		const float maxY = std::max(0.0f, height - 1.0f);
		result.x = std::max(0.0f, std::min(result.x, maxX));
		result.y = std::max(0.0f, std::min(result.y, maxY));
		return result;
	}
}

// タイトルシーンを初期化する。
TitleScene::TitleScene(SceneManager* sceneManager)
	: ActionSceneBase(sceneManager, false)
	, m_sceneTime(0.0f)
	, m_transitionTimer(0.0f)
	, m_hintTimer(0.0f)
	, m_idleTimer(0.0f)
	, m_selectedMenu(0)
	, m_nextScene(SELECT_SCENE)
	, m_inTransition(false)
{
	m_SceneFlag = false;
}

// 終了時に安全にリソースを解放する。
TitleScene::~TitleScene()
{
	Finalize();
}

// メッシュ、UIテクスチャ、状態を初期化する。
void TitleScene::Initialize()
{
	m_sceneTime = 0.0f;
	m_transitionTimer = 0.0f;
	m_hintTimer = 0.0f;
	m_idleTimer = 0.0f;
	m_selectedMenu = 0;
	m_nextScene = SELECT_SCENE;
	m_inTransition = false;

	m_floorMesh = DirectX::GeometricPrimitive::CreateCube(m_directX.GetContext().Get());
	m_skyMesh = DirectX::GeometricPrimitive::CreateSphere(m_directX.GetContext().Get(), 1.0f, 20);
	m_bladeMesh = DirectX::GeometricPrimitive::CreateBox(m_directX.GetContext().Get(), DirectX::XMFLOAT3(0.24f, 3.6f, 0.12f));
	m_pillarMesh = DirectX::GeometricPrimitive::CreateCylinder(m_directX.GetContext().Get(), 1.0f, 1.0f, 16);
	m_coreMesh = DirectX::GeometricPrimitive::CreateSphere(m_directX.GetContext().Get(), 1.0f, 16);
	m_uiSolidTexture = UiUtil::CreateSolidTexture(m_directX.GetDevice().Get());

	m_pillarPositions.clear();
	for (int i = 0; i < 16; ++i)
	{
		const float t = DirectX::XM_2PI * (static_cast<float>(i) / 16.0f);
		const float radius = 13.5f + std::sinf(static_cast<float>(i) * 0.31f) * 1.4f;
		m_pillarPositions.push_back(Vector3(std::sinf(t) * radius, 0.0f, std::cosf(t) * radius));
	}

	EnsureTextRenderer();
	// タイトルUIはマウスクリックを使うため絶対座標モードで扱う。
	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	SetSystemCursorVisible(true);
	UpdateCamera();
}

// 入力処理とシーン遷移処理を行う。
void TitleScene::Update(const DX::StepTimer& stepTimer)
{
	const float dt = static_cast<float>(stepTimer.GetElapsedSeconds());
	if (dt <= 0.0f)
	{
		return;
	}

	m_sceneTime += dt;
	m_hintTimer = std::max(0.0f, m_hintTimer - dt);

	if (m_textRenderer != nullptr)
	{
		m_textRenderer->Update(dt);
	}

	// タイトル中は毎フレーム絶対座標モードを保証し、クリック操作を安定化する。
	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	SetSystemCursorVisible(true);

	const System::InputManager& input = System::InputManager::GetInstance();
	const DirectX::Keyboard::KeyboardStateTracker tracker = input.GetKeyboardTracker();
	const DirectX::Mouse::ButtonStateTracker mouseTracker = input.GetMouseTracker();
	const DirectX::Mouse::State mouseState = input.GetMouseState();
	const Vector2 mouseDelta = input.GetMouseDelta();
	const int previousSelectedMenu = m_selectedMenu;

	const bool hadInput =
		tracker.pressed.Up || tracker.pressed.Down || tracker.pressed.W || tracker.pressed.S ||
		tracker.pressed.Enter || tracker.pressed.Space ||
		(mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED) ||
		(mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED) ||
		(mouseDelta.LengthSquared() > 0.03f);

	if (hadInput)
	{
		m_idleTimer = 0.0f;
	}
	else
	{
		m_idleTimer += dt;
	}

	if (!m_inTransition && m_idleTimer >= kAttractHintIdleSec)
	{
		m_hintTimer = std::max(m_hintTimer, 2.2f);
		m_idleTimer = 0.0f;
	}
	if (!m_inTransition)
	{
		auto ExecuteSelectedMenu = [this]() -> bool
		{
			if (m_selectedMenu == 0)
			{
				// Playは必ず任務選択へ遷移し、導線を一本化する。
				SceneUiSound::PlayConfirm();
				m_inTransition = true;
				m_transitionTimer = kTransitionSec;
				m_nextScene = SELECT_SCENE;
				return true;
			}
			if (m_selectedMenu == 1)
			{
				SceneUiSound::PlayConfirm();
				m_inTransition = true;
				m_transitionTimer = kTransitionSec;
				m_nextScene = SETTINGS_SCENE;
				return true;
			}
			if (m_selectedMenu == 2)
			{
				SceneUiSound::PlayConfirm();
				ExitGame();
				return false;
			}
			return false;
		};

		// マウス位置でメニュー選択を追従し、左クリックで即決定する。
		const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
		const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
		const TitleMenuLayout layout = BuildTitleMenuLayout(width, height);
		const Vector2 mousePoint = ResolveMouseClientPoint(mouseState, m_directX.GetHWnd(), width, height);
		const bool trackerClick = (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED);
		const bool holdFallbackClick = mouseState.leftButton && (m_hintTimer <= 0.0f);
		const bool mouseClick = trackerClick || holdFallbackClick;
		for (int i = 0; i < kMenuCount; ++i)
		{
			const float y = layout.listStartY + static_cast<float>(i) * (layout.cardHeight + layout.cardGap);
			const Vector2 cardPos(layout.center.x - layout.cardWidth * 0.5f, y);
			const bool hover =
				(mousePoint.x >= cardPos.x && mousePoint.x <= (cardPos.x + layout.cardWidth) &&
				 mousePoint.y >= cardPos.y && mousePoint.y <= (cardPos.y + layout.cardHeight));
			if (!hover)
			{
				continue;
			}

			m_selectedMenu = i;
			if (mouseClick)
			{
				m_hintTimer = 0.18f;
				if (!ExecuteSelectedMenu())
				{
					return;
				}
			}
			break;
		}

		if (tracker.pressed.Enter || tracker.pressed.Space)
		{
			if (!ExecuteSelectedMenu())
			{
				return;
			}
		}
	}
	else
	{
		m_transitionTimer -= dt;
		if (m_transitionTimer <= 0.0f)
		{
			m_sceneManager->SetScene(m_nextScene);
			return;
		}
	}

	UpdateCamera();

	if (m_selectedMenu != previousSelectedMenu)
	{
		SceneUiSound::PlayMove();
	}
}

// 描画処理を実行する。
void TitleScene::Render()
{
	DrawWorld();
	DrawUI();
}

// 生成済みリソースを破棄する。
void TitleScene::Finalize()
{
	m_pillarPositions.clear();

	m_floorMesh.reset();
	m_skyMesh.reset();
	m_bladeMesh.reset();
	m_pillarMesh.reset();
	m_coreMesh.reset();
	m_uiSolidTexture.Reset();
	m_textRenderer.reset();

	m_sceneTime = 0.0f;
	m_transitionTimer = 0.0f;
	m_hintTimer = 0.0f;
	m_idleTimer = 0.0f;
	m_selectedMenu = 0;
	m_nextScene = SELECT_SCENE;
	m_inTransition = false;
}

// 軌道カメラを更新する。
void TitleScene::UpdateCamera()
{
	const Vector3 cameraPos(-5.8f, 2.7f, 7.4f);
	const Vector3 target(-0.1f, 1.28f, 0.0f);
	m_proj = BuildPerspective(51.0f, 0.1f, 500.0f);
	m_view = Matrix::CreateLookAt(cameraPos, target, Vector3::Up);
}

// 3D空間の背景オブジェクトを描画する。
void TitleScene::DrawWorld()
{
	if (!m_floorMesh || !m_skyMesh || !m_bladeMesh || !m_pillarMesh || !m_coreMesh)
	{
		return;
	}

	const Matrix skyWorld =
		Matrix::CreateScale(180.0f) *
		Matrix::CreateRotationY(0.12f);
	m_skyMesh->Draw(skyWorld, m_view, m_proj, Color(0.04f, 0.09f, 0.15f, 1.0f));

	const Matrix skyHazeWorld =
		Matrix::CreateScale(156.0f, 92.0f, 156.0f) *
		Matrix::CreateRotationY(-0.08f) *
		Matrix::CreateTranslation(0.0f, -24.0f, 0.0f);
	m_skyMesh->Draw(skyHazeWorld, m_view, m_proj, Color(0.08f, 0.18f, 0.24f, 0.78f));

	const Matrix moonWorld =
		Matrix::CreateScale(1.55f) *
		Matrix::CreateTranslation(8.0f, 10.5f, -15.0f);
	m_coreMesh->Draw(moonWorld, m_view, m_proj, Color(0.9f, 0.96f, 1.0f, 0.92f));

	const Matrix floorWorld =
		Matrix::CreateScale(52.0f, 0.4f, 52.0f) *
		Matrix::CreateTranslation(0.0f, -0.35f, 0.0f);
	m_floorMesh->Draw(floorWorld, m_view, m_proj, Color(0.07f, 0.1f, 0.14f, 1.0f));

	const Matrix ringA =
		Matrix::CreateScale(18.0f, 0.05f, 18.0f) *
		Matrix::CreateRotationY(0.18f) *
		Matrix::CreateTranslation(0.0f, 0.05f, 0.0f);
	m_floorMesh->Draw(ringA, m_view, m_proj, Color(0.12f, 0.2f, 0.27f, 0.95f));

	const Matrix ringB =
		Matrix::CreateScale(12.0f, 0.03f, 12.0f) *
		Matrix::CreateRotationY(-0.24f) *
		Matrix::CreateTranslation(0.0f, 0.08f, 0.0f);
	m_floorMesh->Draw(ringB, m_view, m_proj, Color(0.14f, 0.34f, 0.46f, 0.54f));

	for (int cloud = 0; cloud < 6; ++cloud)
	{
		const float fc = static_cast<float>(cloud);
		const float drift = -24.0f + fc * 7.4f;
		const float baseZ = -16.0f + std::fmod(fc * 7.0f, 18.0f);
		const float baseY = 7.0f + std::sinf(fc) * 0.4f;
		for (int puff = 0; puff < 3; ++puff)
		{
			const float fp = static_cast<float>(puff);
			const float offset = fc * 0.64f + fp * 0.78f;
			const Matrix cloudWorld =
				Matrix::CreateScale(2.8f - fp * 0.5f, 0.32f + fp * 0.03f, 1.8f - fp * 0.22f) *
				Matrix::CreateRotationY(std::sinf(offset) * 0.08f) *
				Matrix::CreateTranslation(
					-20.0f + drift + std::cosf(offset) * (1.2f + fp * 0.45f),
					baseY + fp * 0.18f,
					baseZ + std::sinf(offset) * (1.0f + fp * 0.35f));
			m_coreMesh->Draw(cloudWorld, m_view, m_proj, Color(0.18f, 0.26f, 0.34f, 0.54f));
		}
	}

	for (size_t i = 0; i < m_pillarPositions.size(); ++i)
	{
		const float phase = static_cast<float>(i) * 0.5f;
		const float height = 3.2f + std::sinf(phase) * 0.8f;
		const Vector3 pos = m_pillarPositions[i];
		const Matrix pillarWorld =
			Matrix::CreateScale(0.64f, height, 0.64f) *
			Matrix::CreateTranslation(pos.x, height * 0.5f - 0.2f, pos.z);
		m_pillarMesh->Draw(pillarWorld, m_view, m_proj, Color(0.17f, 0.24f, 0.31f, 1.0f));
	}

	// 剣アクションと分かるよう、対峙する2体のシルエットを中央に配置する。
	const float strike = 0.68f;
	const Matrix heroRoot = Matrix::CreateRotationY(0.6f) * Matrix::CreateTranslation(-1.35f, 0.0f, 0.0f);
	const Matrix foeRoot = Matrix::CreateRotationY(3.14159f + 0.45f) * Matrix::CreateTranslation(1.35f, 0.0f, 0.0f);

	const auto DrawFighter = [this](const Matrix& root, const Color& bodyColor, const Color& legColor)
	{
		m_coreMesh->Draw(Matrix::CreateScale(0.48f, 0.7f, 0.4f) * Matrix::CreateTranslation(0.0f, 1.04f, 0.0f) * root, m_view, m_proj, bodyColor);
		m_coreMesh->Draw(Matrix::CreateScale(0.22f) * Matrix::CreateTranslation(0.0f, 1.76f, 0.0f) * root, m_view, m_proj, Color(0.94f, 0.84f, 0.74f, 1.0f));
		m_pillarMesh->Draw(Matrix::CreateScale(0.12f, 0.5f, 0.12f) * Matrix::CreateTranslation(-0.36f, 1.0f, 0.0f) * root, m_view, m_proj, bodyColor);
		m_pillarMesh->Draw(Matrix::CreateScale(0.12f, 0.5f, 0.12f) * Matrix::CreateTranslation(0.36f, 1.0f, 0.0f) * root, m_view, m_proj, bodyColor);
		m_pillarMesh->Draw(Matrix::CreateScale(0.14f, 0.6f, 0.14f) * Matrix::CreateTranslation(-0.16f, 0.3f, 0.0f) * root, m_view, m_proj, legColor);
		m_pillarMesh->Draw(Matrix::CreateScale(0.14f, 0.6f, 0.14f) * Matrix::CreateTranslation(0.16f, 0.3f, 0.0f) * root, m_view, m_proj, legColor);
	};

	DrawFighter(heroRoot, Color(0.26f, 0.32f, 0.39f, 1.0f), Color(0.72f, 0.33f, 0.24f, 1.0f));
	DrawFighter(foeRoot, Color(0.36f, 0.21f, 0.2f, 1.0f), Color(0.58f, 0.15f, 0.15f, 1.0f));

	const Vector3 heroHand = Vector3::Transform(Vector3(0.44f, 1.1f, 0.04f), heroRoot);
	const Vector3 foeHand = Vector3::Transform(Vector3(0.44f, 1.1f, 0.04f), foeRoot);

	const Matrix heroSword =
		Matrix::CreateScale(0.88f) *
		Matrix::CreateRotationX(-0.64f + strike * 0.82f) *
		Matrix::CreateRotationY(0.54f) *
		Matrix::CreateTranslation(heroHand + Vector3(0.1f + strike * 0.52f, 0.12f + strike * 0.15f, 0.22f - strike * 0.35f));
	const Matrix foeSword =
		Matrix::CreateScale(0.88f) *
		Matrix::CreateRotationX(-0.64f + (1.0f - strike) * 0.82f) *
		Matrix::CreateRotationY(3.68f) *
		Matrix::CreateTranslation(foeHand + Vector3(-0.1f - strike * 0.52f, 0.12f + (1.0f - strike) * 0.15f, -0.22f + strike * 0.35f));
	m_bladeMesh->Draw(heroSword, m_view, m_proj, Color(0.9f, 0.94f, 1.0f, 1.0f));
	m_bladeMesh->Draw(foeSword, m_view, m_proj, Color(0.96f, 0.86f, 0.72f, 1.0f));

	// 球体の衝突点は廃止し、斬撃プレートを交差させる。
	const float clash = 0.62f;
	const Matrix slashA =
		Matrix::CreateScale(1.0f + clash * 0.35f, 0.08f, 0.16f) *
		Matrix::CreateRotationY(0.82f) *
		Matrix::CreateTranslation(0.02f, 1.36f, -0.02f);
	const Matrix slashB =
		Matrix::CreateScale(0.94f + clash * 0.32f, 0.08f, 0.16f) *
		Matrix::CreateRotationY(DirectX::XM_PIDIV2 + 0.82f) *
		Matrix::CreateTranslation(-0.04f, 1.34f, 0.03f);
	m_bladeMesh->Draw(slashA, m_view, m_proj, Color(1.0f, 0.87f, 0.45f, 0.9f));
	m_bladeMesh->Draw(slashB, m_view, m_proj, Color(1.0f, 0.73f, 0.34f, 0.84f));
}

// タイトルUIとメニューを描画する。
void TitleScene::DrawUI()
{
	System::UIShaderText* ui = EnsureTextRenderer();
	if (ui == nullptr)
	{
		return;
	}

	BeginSpriteLayer();
	DirectX::SpriteBatch* batch = System::DrawManager::GetInstance().GetSprite();
	if (batch == nullptr)
	{
		EndSpriteLayer();
		return;
	}

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const TitleMenuLayout layout = BuildTitleMenuLayout(width, height);

	System::UIShaderStyle titleStyle;
	titleStyle.baseColor = Color(0.92f, 0.97f, 1.0f, 1.0f);
	titleStyle.outlineColor = Color(0.02f, 0.04f, 0.07f, 1.0f);
	titleStyle.pulseAmount = 0.0f;
	titleStyle.pulseSpeed = 0.0f;

	System::UIShaderStyle selectedStyle;
	selectedStyle.baseColor = Color(0.72f, 0.92f, 1.0f, 1.0f);
	selectedStyle.outlineColor = Color(0.02f, 0.08f, 0.14f, 1.0f);
	selectedStyle.pulseAmount = 0.0f;

	System::UIShaderStyle normalStyle;
	normalStyle.baseColor = Color(0.84f, 0.89f, 0.95f, 1.0f);
	normalStyle.outlineColor = Color(0.05f, 0.06f, 0.08f, 1.0f);

	System::UIShaderStyle helpStyle;
	helpStyle.baseColor = Color(1.0f, 0.9f, 0.28f, 1.0f);
	helpStyle.outlineColor = Color(0.12f, 0.09f, 0.0f, 1.0f);
	helpStyle.blink = false;
	helpStyle.blinkPeriod = 0.0f;

	System::UIShaderStyle demoStyle = helpStyle;
	demoStyle.baseColor = Color(0.72f, 0.92f, 1.0f, 1.0f);
	demoStyle.outlineColor = Color(0.02f, 0.06f, 0.12f, 1.0f);
	demoStyle.blink = false;

	// 背景を単色に見せないため、多層グラデーションを敷く。
	const int bandCount = 22;
	for (int i = 0; i < bandCount; ++i)
	{
		const float t = static_cast<float>(i) / static_cast<float>(bandCount - 1);
		const float y = height * t;
		const float h = (height / static_cast<float>(bandCount)) + 2.0f;
		DrawSolidRect(
			batch,
			Vector2(0.0f, y),
			Vector2(width, h),
			Color(
				0.015f + t * 0.08f,
				0.03f + t * 0.11f,
				0.07f + t * 0.16f,
				0.92f));
	}

	// 斜めの光条は固定配置にし、脈動ではなく構図で見せる。
	for (int i = 0; i < 3; ++i)
	{
		const float x = width * (0.14f + static_cast<float>(i) * 0.23f);

		const DirectX::XMFLOAT2 beamScale(220.0f, height * 1.4f);
		const Color beamColor = (i == 1) ? Color(0.2f, 0.55f, 0.95f, 0.14f) : Color(0.1f, 0.35f, 0.75f, 0.1f);
		batch->Draw(m_uiSolidTexture.Get(), Vector2(x, -height * 0.18f), nullptr, beamColor, 0.35f, Vector2::Zero, beamScale);
	}

	// 上空の星粒ノイズを追加して奥行きを出す。
	for (int i = 0; i < 52; ++i)
	{
		const float fi = static_cast<float>(i);
		float sx = std::fmod(71.0f * fi + std::sinf(fi * 13.37f) * 170.0f, width);
		if (sx < 0.0f)
		{
			sx += width;
		}
		float sy = std::fmod(43.0f * fi + std::cosf(fi * 7.11f) * 90.0f, height * 0.64f);
		if (sy < 0.0f)
		{
			sy += height * 0.64f;
		}
		const float starSize = (i % 9 == 0) ? 2.2f : ((i % 3 == 0) ? 1.6f : 1.0f);
		DrawSolidRect(batch, Vector2(sx, sy), Vector2(starSize, starSize), Color(0.88f, 0.94f, 1.0f, 0.62f));
	}

	// タイトル帯と周辺の陰影。
	DrawSolidRect(batch, Vector2(width * 0.16f, 24.0f), Vector2(width * 0.68f, 116.0f), Color(0.02f, 0.05f, 0.09f, 0.8f));
	DrawSolidRect(batch, Vector2(width * 0.16f, 24.0f), Vector2(width * 0.68f, 2.0f), Color(0.78f, 0.9f, 1.0f, 0.96f));
	DrawSolidRect(batch, Vector2(width * 0.16f, 138.0f), Vector2(width * 0.68f, 2.0f), Color(0.18f, 0.5f, 0.82f, 0.74f));
	DrawSolidRect(batch, Vector2(0.0f, 0.0f), Vector2(width * 0.1f, height), Color(0.0f, 0.0f, 0.0f, 0.22f));
	DrawSolidRect(batch, Vector2(width * 0.9f, 0.0f), Vector2(width * 0.1f, height), Color(0.0f, 0.0f, 0.0f, 0.22f));

	ui->Draw(batch, L"STEEL REVENANT", Vector2(width * 0.5f - 168.0f, 46.0f), titleStyle, 1.9f);
	ui->Draw(batch, L"\u5263\u30a2\u30af\u30b7\u30e7\u30f3\u8a66\u4f5c", Vector2(width * 0.5f - 78.0f, 102.0f), normalStyle, 0.92f);

	// 縦メニューの背景レール。
	const float railBorder = std::max(1.0f, 2.0f * layout.uiScale);
	const Vector2 panelPos(layout.center.x - layout.panelWidth * 0.5f, layout.panelTop);
	DrawSolidRect(batch, panelPos, Vector2(layout.panelWidth, layout.panelHeight), Color(0.02f, 0.06f, 0.1f, 0.56f));
	DrawSolidRect(batch, panelPos, Vector2(layout.panelWidth, railBorder), Color(0.58f, 0.78f, 0.98f, 0.88f));
	DrawSolidRect(batch, panelPos + Vector2(0.0f, layout.panelHeight - railBorder), Vector2(layout.panelWidth, railBorder), Color(0.58f, 0.78f, 0.98f, 0.88f));

	struct MenuEntry
	{
		const wchar_t* label;
		const wchar_t* detail;
	};

	const std::array<MenuEntry, kMenuCount> menu =
	{
		MenuEntry{ L"01 任務開始", L"任務を選んで出撃する" },
		MenuEntry{ L"02 設定", L"感度と操作を調整" },
		MenuEntry{ L"03 終了", L"アプリケーションを終了" }
	};

	for (int i = 0; i < kMenuCount; ++i)
	{
		const bool selected = (i == m_selectedMenu);
		const float y = layout.listStartY + static_cast<float>(i) * (layout.cardHeight + layout.cardGap);
		const Vector2 cardPos(layout.center.x - layout.cardWidth * 0.5f, y);
		const float edgeAlpha = selected ? 0.92f : 0.74f;
		const Color cardColor = selected ? Color(0.08f, 0.13f, 0.18f, 0.92f) : Color(0.04f, 0.07f, 0.10f, 0.82f);
		const Color edgeColor = selected ? Color(0.56f, 0.84f, 1.0f, edgeAlpha) : Color(0.48f, 0.62f, 0.78f, edgeAlpha);

		DrawSolidRect(batch, cardPos, Vector2(layout.cardWidth, layout.cardHeight), cardColor);
		DrawSolidRect(batch, cardPos, Vector2(layout.cardWidth, 1.5f), edgeColor);
		DrawSolidRect(batch, cardPos + Vector2(0.0f, layout.cardHeight - 1.5f), Vector2(layout.cardWidth, 1.5f), edgeColor);

		const float labelScale = 0.8f * layout.uiScale;
		const float detailBaseScale = 0.44f * layout.uiScale;
		float detailScale = detailBaseScale;
		const float detailMaxWidth = layout.cardWidth - 34.0f * layout.uiScale;
		const Vector2 detailSize = ui->Measure(menu[static_cast<size_t>(i)].detail, detailBaseScale);
		if (detailSize.x > detailMaxWidth && detailSize.x > 0.01f)
		{
			detailScale = std::max(0.32f * layout.uiScale, detailBaseScale * (detailMaxWidth / detailSize.x));
		}

		ui->Draw(batch, menu[static_cast<size_t>(i)].label, cardPos + Vector2(16.0f * layout.uiScale, 8.0f * layout.uiScale), selected ? selectedStyle : normalStyle, labelScale);
		ui->Draw(batch, menu[static_cast<size_t>(i)].detail, cardPos + Vector2(16.0f * layout.uiScale, 32.0f * layout.uiScale), selected ? selectedStyle : normalStyle, detailScale);
	}

	EndSpriteLayer();
}

// SpriteBatch上で塗りつぶし矩形を描画する。
void TitleScene::DrawSolidRect(
	DirectX::SpriteBatch* batch,
	const Vector2& position,
	const Vector2& size,
	const Color& color) const
{
	if (batch == nullptr || m_uiSolidTexture == nullptr)
	{
		return;
	}
	if (size.x <= 0.0f || size.y <= 0.0f)
	{
		return;
	}

	const DirectX::XMFLOAT2 scale(size.x, size.y);
	batch->Draw(m_uiSolidTexture.Get(), position, nullptr, color, 0.0f, Vector2::Zero, scale);
}

