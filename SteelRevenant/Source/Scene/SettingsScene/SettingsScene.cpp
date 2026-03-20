//------------------------//------------------------
// Contents(処理内容) 設定画面の更新、音量反映、前画面復帰処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "SettingsScene.h"

#include "../../GameSystem/InputManager.h"
#include "../../GameSystem/DrawManager.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../GameSystem/UiUtil.h"
#include "../../Utility/Sound/AudioSystem.h"
#include "../Base/SceneUiSound.h"
#include "../SceneManager/SceneManager.h"

#include <Mouse.h>
#include <Keyboard.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;

extern void ExitGame();

namespace
{
	constexpr float kSensitivityMin = 0.01f;
	constexpr float kSensitivityMax = 0.35f;
	constexpr float kSensitivityStep = 0.01f;
	constexpr float kVolumeMin = 0.0f;
	constexpr float kVolumeMax = 1.0f;
	constexpr float kVolumeStep = 0.05f;
	constexpr float kClickFxDurationSec = 0.22f;
	constexpr int kSettingControlCount = 5;

	enum SettingControlIndex
	{
		MouseSensitivity = 0,
		MasterVolume,
		BgmVolume,
		SeVolume,
		UiVolume,
	};

	struct SettingsMenuContext
	{
		SceneId backScene = TITLE_SCENE;
		bool returnToSuspendedGame = false;
		bool showBackButton = false;
		int itemCount = kSettingControlCount + 2;
		int backItemIndex = -1;
		int titleItemIndex = kSettingControlCount;
		int quitItemIndex = kSettingControlCount + 1;
		const wchar_t* backLabel = L"前の画面へ戻る";
		const wchar_t* backDetail = L"直前に開いていた画面へ戻る";
		const wchar_t* titleDetail = L"設定を閉じてタイトルメニューへ戻る";
		const wchar_t* escapeHint = L"Escでタイトルへ戻る";
	};

	struct SettingsLayout
	{
		float uiScale = 1.0f;
		Vector2 panelPos = Vector2::Zero;
		Vector2 panelSize = Vector2::Zero;
		Vector2 leftCardPos = Vector2::Zero;
		Vector2 leftCardSize = Vector2::Zero;
		Vector2 rightCardPos = Vector2::Zero;
		Vector2 rightCardSize = Vector2::Zero;
		Vector2 previewPos = Vector2::Zero;
		Vector2 previewSize = Vector2::Zero;
		Vector2 rowBandSize = Vector2::Zero;
		Vector2 minusSize = Vector2::Zero;
		Vector2 plusSize = Vector2::Zero;
		Vector2 sliderSize = Vector2::Zero;
		Vector2 actionButtonSize = Vector2::Zero;
		Vector2 backPos = Vector2::Zero;
		Vector2 titlePos = Vector2::Zero;
		Vector2 quitPos = Vector2::Zero;
		std::array<Vector2, kSettingControlCount> rowBandPos = {};
		std::array<Vector2, kSettingControlCount> titlePosList = {};
		std::array<Vector2, kSettingControlCount> valuePosList = {};
		std::array<Vector2, kSettingControlCount> minusPos = {};
		std::array<Vector2, kSettingControlCount> plusPos = {};
		std::array<Vector2, kSettingControlCount> sliderPos = {};
	};

	SettingsMenuContext BuildSettingsMenuContext(const SceneManager* sceneManager)
	{
		SettingsMenuContext context;
		if (sceneManager == nullptr)
		{
			return context;
		}

		const bool hasSuspendedScene = sceneManager->HasSuspendedScene();
		const SceneId suspendedScene = sceneManager->GetSuspendedSceneId();
		const SceneId previousScene = sceneManager->GetPreviousSceneId();

		if (hasSuspendedScene && suspendedScene == GAME_SCENE)
		{
			context.backScene = GAME_SCENE;
			context.returnToSuspendedGame = true;
			context.showBackButton = true;
			context.itemCount = kSettingControlCount + 3;
			context.backItemIndex = kSettingControlCount;
			context.titleItemIndex = kSettingControlCount + 1;
			context.quitItemIndex = kSettingControlCount + 2;
			context.backLabel = L"一時停止へ戻る";
			context.backDetail = L"中断中のゲームを保持したまま一時停止へ戻る";
			context.titleDetail = L"中断中のゲームを破棄してタイトルへ戻る";
			context.escapeHint = L"Escで一時停止へ戻る";
			return context;
		}

		if (previousScene == SELECT_SCENE)
		{
			context.backScene = SELECT_SCENE;
			context.showBackButton = true;
			context.itemCount = kSettingControlCount + 3;
			context.backItemIndex = kSettingControlCount;
			context.titleItemIndex = kSettingControlCount + 1;
			context.quitItemIndex = kSettingControlCount + 2;
			context.backLabel = L"任務選択へ戻る";
			context.backDetail = L"ステージ選択画面へ戻る";
			context.titleDetail = L"選択中の任務を離れてタイトルへ戻る";
			context.escapeHint = L"Escで任務選択へ戻る";
			return context;
		}

		if (previousScene == GAME_SCENE)
		{
			context.backScene = GAME_SCENE;
			context.showBackButton = true;
			context.itemCount = kSettingControlCount + 3;
			context.backItemIndex = kSettingControlCount;
			context.titleItemIndex = kSettingControlCount + 1;
			context.quitItemIndex = kSettingControlCount + 2;
			context.backLabel = L"ゲームへ戻る";
			context.backDetail = L"ゲーム画面へ戻る";
			context.titleDetail = L"現在の進行を離れてタイトルへ戻る";
			context.escapeHint = L"Escでゲームへ戻る";
			return context;
		}

		return context;
	}

	SettingsLayout BuildSettingsLayout(float width, float height, bool showBackButton)
	{
		SettingsLayout layout;
		layout.uiScale = std::max(0.78f, std::min(1.35f, std::min(width / 1280.0f, height / 720.0f)));

		const float safeMargin = 12.0f * layout.uiScale;
		const float panelWidth = std::min(width * 0.80f, width - safeMargin * 2.0f);
		const float panelHeight = std::min(height * 0.76f, height - safeMargin * 2.0f);
		layout.panelPos = Vector2((width - panelWidth) * 0.5f, std::max(safeMargin, (height - panelHeight) * 0.5f));
		layout.panelSize = Vector2(panelWidth, panelHeight);

		const float leftWidth = layout.panelSize.x * 0.60f;
		layout.leftCardPos = layout.panelPos + Vector2(20.0f * layout.uiScale, 74.0f * layout.uiScale);
		layout.leftCardSize = Vector2(leftWidth - 30.0f * layout.uiScale, layout.panelSize.y - 96.0f * layout.uiScale);
		layout.rightCardPos = Vector2(layout.panelPos.x + leftWidth + 10.0f * layout.uiScale, layout.panelPos.y + 74.0f * layout.uiScale);
		layout.rightCardSize = Vector2(layout.panelSize.x - leftWidth - 30.0f * layout.uiScale, layout.panelSize.y - 96.0f * layout.uiScale);

		layout.previewPos = layout.leftCardPos + Vector2(18.0f * layout.uiScale, 48.0f * layout.uiScale);
		layout.previewSize = Vector2(layout.leftCardSize.x - 36.0f * layout.uiScale, 84.0f * layout.uiScale);
		layout.rowBandSize = Vector2(layout.leftCardSize.x - 36.0f * layout.uiScale, 40.0f * layout.uiScale);
		layout.minusSize = Vector2(34.0f * layout.uiScale, 24.0f * layout.uiScale);
		layout.plusSize = layout.minusSize;

		const float rowsStartY = layout.previewPos.y + layout.previewSize.y + 18.0f * layout.uiScale;
		for (int index = 0; index < kSettingControlCount; ++index)
		{
			const float rowTop = rowsStartY + static_cast<float>(index) * (46.0f * layout.uiScale);
			layout.rowBandPos[static_cast<size_t>(index)] = Vector2(layout.leftCardPos.x + 18.0f * layout.uiScale, rowTop);
			layout.titlePosList[static_cast<size_t>(index)] = Vector2(layout.rowBandPos[static_cast<size_t>(index)].x + 10.0f * layout.uiScale, rowTop + 4.0f * layout.uiScale);
			layout.valuePosList[static_cast<size_t>(index)] = Vector2(layout.leftCardPos.x + layout.leftCardSize.x - 84.0f * layout.uiScale, rowTop + 4.0f * layout.uiScale);
			layout.minusPos[static_cast<size_t>(index)] = Vector2(layout.rowBandPos[static_cast<size_t>(index)].x + 10.0f * layout.uiScale, rowTop + 18.0f * layout.uiScale);
			layout.plusPos[static_cast<size_t>(index)] = Vector2(layout.leftCardPos.x + layout.leftCardSize.x - 18.0f * layout.uiScale - layout.plusSize.x, rowTop + 18.0f * layout.uiScale);
			layout.sliderPos[static_cast<size_t>(index)] = Vector2(layout.minusPos[static_cast<size_t>(index)].x + layout.minusSize.x + 12.0f * layout.uiScale, rowTop + 27.0f * layout.uiScale);
		}

		layout.sliderSize.y = std::max(4.0f, 6.0f * layout.uiScale);
		layout.sliderSize.x = std::max(
			110.0f * layout.uiScale,
			layout.plusPos[0].x - layout.sliderPos[0].x - 12.0f * layout.uiScale);

		const float actionButtonHeight = 54.0f * layout.uiScale;
		const float actionButtonGap = 10.0f * layout.uiScale;
		const float actionButtonLeft = layout.rightCardPos.x + 18.0f * layout.uiScale;
		const float actionStartY = layout.rightCardPos.y + 118.0f * layout.uiScale;
		const int actionCount = showBackButton ? 3 : 2;
		layout.actionButtonSize = Vector2(std::max(0.0f, layout.rightCardSize.x - 36.0f * layout.uiScale), actionButtonHeight);
		layout.backPos = Vector2(actionButtonLeft, actionStartY);
		layout.titlePos = Vector2(actionButtonLeft, actionStartY + (showBackButton ? (actionButtonHeight + actionButtonGap) : 0.0f));
		layout.quitPos = Vector2(actionButtonLeft, actionStartY + (actionButtonHeight + actionButtonGap) * static_cast<float>(actionCount - 1));
		return layout;
	}

	float ClampVolume(float value)
	{
		return std::max(kVolumeMin, std::min(kVolumeMax, value));
	}

	const wchar_t* GetSettingLabel(int index)
	{
		switch (index)
		{
		case MouseSensitivity:
			return L"マウス感度";
		case MasterVolume:
			return L"マスター音量";
		case BgmVolume:
			return L"BGM音量";
		case SeVolume:
			return L"SE音量";
		case UiVolume:
			return L"UI音量";
		default:
			return L"";
		}
	}

	const wchar_t* GetSettingDetail(int index)
	{
		switch (index)
		{
		case MouseSensitivity:
			return L"視点移動にその場で反映";
		case MasterVolume:
			return L"全体音量";
		case BgmVolume:
			return L"シーンBGM";
		case SeVolume:
			return L"戦闘と環境音";
		case UiVolume:
			return L"メニュー操作音";
		default:
			return L"";
		}
	}
}

SettingsScene::SettingsScene(SceneManager* sceneManager)
	: ActionSceneBase(sceneManager, false)
	, m_selectedItem(0)
	, m_sceneTime(0.0f)
	, m_mouseSensitivity(0.08f)
	, m_audioVolume()
	, m_clickFxTimer(0.0f)
	, m_clickFxPos(Vector2::Zero)
{
	m_SceneFlag = false;
}

SettingsScene::~SettingsScene()
{
	Finalize();
}

void SettingsScene::Initialize()
{
	m_selectedItem = 0;
	m_sceneTime = 0.0f;
	m_mouseSensitivity = GameSaveData::GetInstance().GetMouseSensitivity();
	m_audioVolume = GameSaveData::GetInstance().GetAudioVolumeSettings();
	m_clickFxTimer = 0.0f;
	m_clickFxPos = Vector2::Zero;
	m_uiSolidTexture = UiUtil::CreateSolidTexture(m_directX.GetDevice().Get());

	DirectX::Mouse::Get().SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	SetSystemCursorVisible(true);
	EnsureTextRenderer();
}

void SettingsScene::Update(const DX::StepTimer& stepTimer)
{
	const float dt = static_cast<float>(stepTimer.GetElapsedSeconds());
	if (dt <= 0.0f)
	{
		return;
	}

	m_sceneTime += dt;
	m_clickFxTimer = std::max(0.0f, m_clickFxTimer - dt);

	if (m_textRenderer != nullptr)
	{
		m_textRenderer->Update(dt);
	}

	const System::InputManager& input = System::InputManager::GetInstance();
	const DirectX::Keyboard::KeyboardStateTracker tracker = input.GetKeyboardTracker();
	const DirectX::Mouse::ButtonStateTracker mouseTracker = input.GetMouseTracker();
	const DirectX::Mouse::State mouseState = input.GetMouseState();
	const Vector2 mousePoint(static_cast<float>(mouseState.x), static_cast<float>(mouseState.y));
	const bool leftPressed = (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED);
	const SettingsMenuContext menuContext = BuildSettingsMenuContext(m_sceneManager);
	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const SettingsLayout layout = BuildSettingsLayout(width, height, menuContext.showBackButton);

	const int previousSelected = m_selectedItem;
	m_selectedItem = std::max(0, std::min(m_selectedItem, menuContext.itemCount - 1));

	if (tracker.pressed.Up || tracker.pressed.W)
	{
		m_selectedItem = (m_selectedItem + menuContext.itemCount - 1) % menuContext.itemCount;
	}
	if (tracker.pressed.Down || tracker.pressed.S)
	{
		m_selectedItem = (m_selectedItem + 1) % menuContext.itemCount;
	}

	for (int index = 0; index < kSettingControlCount; ++index)
	{
		const size_t rowIndex = static_cast<size_t>(index);
		if (UiUtil::IsInsideRect(mousePoint, layout.minusPos[rowIndex], layout.minusSize) ||
			UiUtil::IsInsideRect(mousePoint, layout.plusPos[rowIndex], layout.plusSize) ||
			UiUtil::IsInsideRect(mousePoint, layout.sliderPos[rowIndex], layout.sliderSize))
		{
			m_selectedItem = index;
		}
	}
	if (menuContext.showBackButton && UiUtil::IsInsideRect(mousePoint, layout.backPos, layout.actionButtonSize))
	{
		m_selectedItem = menuContext.backItemIndex;
	}
	if (UiUtil::IsInsideRect(mousePoint, layout.titlePos, layout.actionButtonSize))
	{
		m_selectedItem = menuContext.titleItemIndex;
	}
	if (UiUtil::IsInsideRect(mousePoint, layout.quitPos, layout.actionButtonSize))
	{
		m_selectedItem = menuContext.quitItemIndex;
	}

	if (m_selectedItem != previousSelected)
	{
		SceneUiSound::PlayMove();
	}

	if (m_selectedItem >= 0 && m_selectedItem < kSettingControlCount)
	{
		const float step = (m_selectedItem == MouseSensitivity) ? kSensitivityStep : kVolumeStep;
		if (tracker.pressed.Left || tracker.pressed.A)
		{
			SetSettingValue(m_selectedItem, GetSettingValue(m_selectedItem) - step);
			SceneUiSound::PlayMove();
		}
		if (tracker.pressed.Right || tracker.pressed.D)
		{
			SetSettingValue(m_selectedItem, GetSettingValue(m_selectedItem) + step);
			SceneUiSound::PlayMove();
		}
	}

	if (leftPressed)
	{
		m_clickFxTimer = kClickFxDurationSec;
		m_clickFxPos = mousePoint;

		bool handledControl = false;
		for (int index = 0; index < kSettingControlCount; ++index)
		{
			const size_t rowIndex = static_cast<size_t>(index);
			const float step = (index == MouseSensitivity) ? kSensitivityStep : kVolumeStep;
			if (UiUtil::IsInsideRect(mousePoint, layout.minusPos[rowIndex], layout.minusSize))
			{
				SetSettingValue(index, GetSettingValue(index) - step);
				SceneUiSound::PlayMove();
				handledControl = true;
				break;
			}
			if (UiUtil::IsInsideRect(mousePoint, layout.plusPos[rowIndex], layout.plusSize))
			{
				SetSettingValue(index, GetSettingValue(index) + step);
				SceneUiSound::PlayMove();
				handledControl = true;
				break;
			}
			if (UiUtil::IsInsideRect(mousePoint, layout.sliderPos[rowIndex], layout.sliderSize))
			{
				const float ratio = std::max(0.0f, std::min(1.0f, (mousePoint.x - layout.sliderPos[rowIndex].x) / std::max(1.0f, layout.sliderSize.x)));
				if (index == MouseSensitivity)
				{
					SetSettingValue(index, kSensitivityMin + (kSensitivityMax - kSensitivityMin) * ratio);
				}
				else
				{
					SetSettingValue(index, ratio);
				}
				SceneUiSound::PlayMove();
				handledControl = true;
				break;
			}
		}

		if (handledControl)
		{
			return;
		}

		if (menuContext.showBackButton && UiUtil::IsInsideRect(mousePoint, layout.backPos, layout.actionButtonSize))
		{
			SceneUiSound::PlayBack();
			ReturnToBackScene(menuContext.returnToSuspendedGame, menuContext.backScene);
			return;
		}
		if (UiUtil::IsInsideRect(mousePoint, layout.titlePos, layout.actionButtonSize))
		{
			SceneUiSound::PlayConfirm();
			m_sceneManager->SetScene(TITLE_SCENE);
			return;
		}
		if (UiUtil::IsInsideRect(mousePoint, layout.quitPos, layout.actionButtonSize))
		{
			SceneUiSound::PlayConfirm();
			ExitGame();
			return;
		}
	}

	if (tracker.pressed.Enter || tracker.pressed.Space)
	{
		if (menuContext.showBackButton && m_selectedItem == menuContext.backItemIndex)
		{
			SceneUiSound::PlayBack();
			ReturnToBackScene(menuContext.returnToSuspendedGame, menuContext.backScene);
			return;
		}
		if (m_selectedItem == menuContext.titleItemIndex)
		{
			SceneUiSound::PlayConfirm();
			m_sceneManager->SetScene(TITLE_SCENE);
			return;
		}
		if (m_selectedItem == menuContext.quitItemIndex)
		{
			SceneUiSound::PlayConfirm();
			ExitGame();
			return;
		}
	}

	if (tracker.pressed.Escape)
	{
		SceneUiSound::PlayBack();
		ReturnToBackScene(menuContext.returnToSuspendedGame, menuContext.backScene);
		return;
	}
}

void SettingsScene::Render()
{
	System::UIShaderText* ui = EnsureTextRenderer();
	if (ui == nullptr)
	{
		return;
	}

	System::UIShaderStyle titleStyle;
	titleStyle.baseColor = Color(0.9f, 0.95f, 1.0f, 1.0f);
	titleStyle.outlineColor = Color(0.05f, 0.05f, 0.08f, 1.0f);
	titleStyle.pulseAmount = 0.0f;
	titleStyle.pulseSpeed = 0.0f;

	System::UIShaderStyle selectedStyle;
	selectedStyle.baseColor = Color(0.72f, 0.92f, 1.0f, 1.0f);
	selectedStyle.outlineColor = Color(0.02f, 0.08f, 0.14f, 1.0f);
	selectedStyle.pulseAmount = 0.0f;

	System::UIShaderStyle normalStyle;
	normalStyle.baseColor = Color(0.85f, 0.9f, 0.96f, 1.0f);
	normalStyle.outlineColor = Color(0.05f, 0.06f, 0.08f, 1.0f);

	System::UIShaderStyle subStyle;
	subStyle.baseColor = Color(0.7f, 0.78f, 0.88f, 1.0f);
	subStyle.outlineColor = Color(0.04f, 0.05f, 0.07f, 1.0f);

	System::UIShaderStyle helpStyle;
	helpStyle.baseColor = Color(0.95f, 0.92f, 0.46f, 1.0f);
	helpStyle.outlineColor = Color(0.16f, 0.12f, 0.03f, 1.0f);

	BeginSpriteLayer();
	DirectX::SpriteBatch* batch = System::DrawManager::GetInstance().GetSprite();
	if (batch == nullptr)
	{
		EndSpriteLayer();
		return;
	}

	const float width = static_cast<float>(std::max(1, m_directX.GetWidth()));
	const float height = static_cast<float>(std::max(1, m_directX.GetHeight()));
	const SettingsMenuContext menuContext = BuildSettingsMenuContext(m_sceneManager);
	const SettingsLayout layout = BuildSettingsLayout(width, height, menuContext.showBackButton);

	for (int band = 0; band < 14; ++band)
	{
		const float t = static_cast<float>(band) / 13.0f;
		const float y = height * t;
		const float h = (height / 14.0f) + 2.0f;
		DrawSolidRect(
			batch,
			Vector2(0.0f, y),
			Vector2(width, h),
			Color(0.012f + t * 0.048f, 0.032f + t * 0.084f, 0.074f + t * 0.122f, 0.92f));
	}

	DrawSolidRect(batch, layout.panelPos, layout.panelSize, Color(0.03f, 0.05f, 0.08f, 0.78f));
	DrawSolidRect(batch, layout.panelPos, Vector2(layout.panelSize.x, std::max(1.0f, 2.0f * layout.uiScale)), Color(0.72f, 0.82f, 1.0f, 0.82f));
	DrawSolidRect(batch, layout.leftCardPos, layout.leftCardSize, Color(0.04f, 0.07f, 0.1f, 0.56f));
	DrawSolidRect(batch, layout.rightCardPos, layout.rightCardSize, Color(0.04f, 0.07f, 0.1f, 0.56f));

	ui->Draw(batch, L"設定", layout.panelPos + Vector2(26.0f * layout.uiScale, 20.0f * layout.uiScale), titleStyle, 1.18f * layout.uiScale);
	ui->Draw(batch, L"視点と音量", layout.panelPos + Vector2(26.0f * layout.uiScale, 50.0f * layout.uiScale), subStyle, 0.80f * layout.uiScale);
	ui->Draw(batch, L"調整項目", layout.leftCardPos + Vector2(18.0f * layout.uiScale, 14.0f * layout.uiScale), normalStyle, 0.86f * layout.uiScale);

	DrawSolidRect(batch, layout.previewPos, layout.previewSize, Color(0.05f, 0.09f, 0.12f, 0.72f));
	DrawSolidRect(batch, layout.previewPos, Vector2(layout.previewSize.x, std::max(1.0f, 1.5f * layout.uiScale)), Color(0.54f, 0.82f, 1.0f, 0.76f));
	ui->Draw(batch, L"AIM PREVIEW", layout.previewPos + Vector2(12.0f * layout.uiScale, 10.0f * layout.uiScale), normalStyle, 0.56f * layout.uiScale);

	const float sensitivityRatio = std::max(0.0f, std::min(1.0f, (m_mouseSensitivity - kSensitivityMin) / (kSensitivityMax - kSensitivityMin)));
	const Vector2 previewCenter(layout.previewPos.x + layout.previewSize.x * 0.5f, layout.previewPos.y + layout.previewSize.y * 0.62f);
	const float crossGap = (4.0f + sensitivityRatio * 8.0f) * layout.uiScale;
	const float crossLen = (9.0f + sensitivityRatio * 12.0f) * layout.uiScale;
	DrawSolidRect(batch, previewCenter + Vector2(-crossLen - crossGap, -1.0f * layout.uiScale), Vector2(crossLen, 2.0f * layout.uiScale), Color(0.92f, 0.96f, 1.0f, 0.96f));
	DrawSolidRect(batch, previewCenter + Vector2(crossGap, -1.0f * layout.uiScale), Vector2(crossLen, 2.0f * layout.uiScale), Color(0.92f, 0.96f, 1.0f, 0.96f));
	DrawSolidRect(batch, previewCenter + Vector2(-1.0f * layout.uiScale, -crossLen - crossGap), Vector2(2.0f * layout.uiScale, crossLen), Color(0.92f, 0.96f, 1.0f, 0.96f));
	DrawSolidRect(batch, previewCenter + Vector2(-1.0f * layout.uiScale, crossGap), Vector2(2.0f * layout.uiScale, crossLen), Color(0.92f, 0.96f, 1.0f, 0.96f));
	DrawSolidRect(batch, previewCenter + Vector2(-1.5f * layout.uiScale, -1.5f * layout.uiScale), Vector2(3.0f * layout.uiScale, 3.0f * layout.uiScale), Color(0.62f, 0.9f, 1.0f, 0.98f));

	for (int band = 0; band < 3; ++band)
	{
		const float fb = static_cast<float>(band);
		const float bandRatio = std::max(0.0f, std::min(1.0f, sensitivityRatio + fb * 0.18f));
		DrawSolidRect(
			batch,
			layout.previewPos + Vector2(12.0f * layout.uiScale, 52.0f * layout.uiScale + fb * 8.0f * layout.uiScale),
			Vector2((layout.previewSize.x - 24.0f * layout.uiScale) * bandRatio, 4.0f * layout.uiScale),
			Color(0.24f + fb * 0.05f, 0.64f + fb * 0.08f, 0.92f + fb * 0.03f, 0.72f));
	}

	const std::array<float, kSettingControlCount> values =
	{
		m_mouseSensitivity,
		m_audioVolume.master,
		m_audioVolume.bgm,
		m_audioVolume.se,
		m_audioVolume.ui
	};

	for (int index = 0; index < kSettingControlCount; ++index)
	{
		const size_t rowIndex = static_cast<size_t>(index);
		const bool selected = (m_selectedItem == index);
		const Color bandColor = selected ? Color(0.08f, 0.15f, 0.21f, 0.92f) : Color(0.06f, 0.09f, 0.12f, 0.82f);
		const Color borderColor = selected ? Color(0.56f, 0.88f, 1.0f, 0.86f) : Color(0.30f, 0.42f, 0.56f, 0.52f);
		DrawSolidRect(batch, layout.rowBandPos[rowIndex], layout.rowBandSize, bandColor);
		DrawSolidRect(batch, layout.rowBandPos[rowIndex], Vector2(layout.rowBandSize.x, std::max(1.0f, 1.0f * layout.uiScale)), borderColor);

		const float ratio = (index == MouseSensitivity)
			? std::max(0.0f, std::min(1.0f, (values[rowIndex] - kSensitivityMin) / (kSensitivityMax - kSensitivityMin)))
			: std::max(0.0f, std::min(1.0f, values[rowIndex]));
		const std::wstring valueText = std::to_wstring(static_cast<int>(std::round(values[rowIndex] * 100.0f))) + L"%";

		ui->Draw(batch, GetSettingLabel(index), layout.titlePosList[rowIndex], selected ? selectedStyle : normalStyle, 0.66f * layout.uiScale);
		ui->Draw(batch, GetSettingDetail(index), layout.titlePosList[rowIndex] + Vector2(126.0f * layout.uiScale, 0.0f), subStyle, 0.46f * layout.uiScale);
		ui->Draw(batch, valueText, layout.valuePosList[rowIndex], selected ? selectedStyle : normalStyle, 0.58f * layout.uiScale);

		DrawSolidRect(batch, layout.sliderPos[rowIndex], layout.sliderSize, Color(0.15f, 0.18f, 0.22f, 0.9f));
		DrawSolidRect(batch, layout.sliderPos[rowIndex], Vector2(layout.sliderSize.x * ratio, layout.sliderSize.y), Color(0.36f, 0.76f, 1.0f, 0.95f));
		DrawSolidRect(batch, layout.sliderPos[rowIndex] + Vector2(layout.sliderSize.x * ratio - 2.0f * layout.uiScale, -5.0f * layout.uiScale), Vector2(std::max(2.0f, 4.0f * layout.uiScale), 16.0f * layout.uiScale), Color(0.96f, 0.98f, 1.0f, 0.95f));

		const Color adjustColor = selected ? Color(0.10f, 0.18f, 0.24f, 0.92f) : Color(0.08f, 0.11f, 0.14f, 0.84f);
		const Color iconColor(0.92f, 0.96f, 1.0f, 0.96f);
		DrawSolidRect(batch, layout.minusPos[rowIndex], layout.minusSize, adjustColor);
		DrawSolidRect(batch, layout.plusPos[rowIndex], layout.plusSize, adjustColor);

		const Vector2 minusBarSize(12.0f * layout.uiScale, std::max(2.0f, 3.0f * layout.uiScale));
		const Vector2 plusHorizontalSize(12.0f * layout.uiScale, std::max(2.0f, 3.0f * layout.uiScale));
		const Vector2 plusVerticalSize(std::max(2.0f, 3.0f * layout.uiScale), 12.0f * layout.uiScale);
		DrawSolidRect(batch, layout.minusPos[rowIndex] + (layout.minusSize - minusBarSize) * 0.5f, minusBarSize, iconColor);
		DrawSolidRect(batch, layout.plusPos[rowIndex] + (layout.plusSize - plusHorizontalSize) * 0.5f, plusHorizontalSize, iconColor);
		DrawSolidRect(batch, layout.plusPos[rowIndex] + (layout.plusSize - plusVerticalSize) * 0.5f, plusVerticalSize, iconColor);
	}

	ui->Draw(batch, L"A / D / クリックで調整", layout.leftCardPos + Vector2(18.0f * layout.uiScale, layout.leftCardSize.y - 34.0f * layout.uiScale), helpStyle, 0.52f * layout.uiScale);

	ui->Draw(batch, L"メニュー", layout.rightCardPos + Vector2(18.0f * layout.uiScale, 16.0f * layout.uiScale), normalStyle, 0.86f * layout.uiScale);
	ui->Draw(batch, menuContext.escapeHint, layout.rightCardPos + Vector2(18.0f * layout.uiScale, 40.0f * layout.uiScale), subStyle, 0.6f * layout.uiScale);

	const auto ResolveActionDetailScale = [ui, &layout](const wchar_t* text)
	{
		const float baseScale = 0.44f * layout.uiScale;
		const float maxWidth = std::max(1.0f, layout.actionButtonSize.x - 40.0f * layout.uiScale);
		const Vector2 measured = ui->Measure(text, baseScale);
		if (measured.x <= maxWidth || measured.x <= 0.01f)
		{
			return baseScale;
		}
		return std::max(0.34f * layout.uiScale, baseScale * (maxWidth / measured.x));
	};

	const Color backColor = (m_selectedItem == menuContext.backItemIndex) ? Color(0.10f, 0.18f, 0.24f, 0.92f) : Color(0.08f, 0.11f, 0.14f, 0.82f);
	const Color titleColor = (m_selectedItem == menuContext.titleItemIndex) ? Color(0.10f, 0.20f, 0.28f, 0.92f) : Color(0.08f, 0.11f, 0.14f, 0.82f);
	const Color quitColor = (m_selectedItem == menuContext.quitItemIndex) ? Color(0.26f, 0.12f, 0.12f, 0.92f) : Color(0.08f, 0.11f, 0.14f, 0.82f);
	const float actionBorderHeight = std::max(1.0f, 1.5f * layout.uiScale);
	const float backDetailScale = ResolveActionDetailScale(menuContext.backDetail);
	const float titleDetailScale = ResolveActionDetailScale(menuContext.titleDetail);
	const float quitDetailScale = ResolveActionDetailScale(L"アプリケーションを終了する");
	if (menuContext.showBackButton)
	{
		DrawSolidRect(batch, layout.backPos, layout.actionButtonSize, backColor);
		DrawSolidRect(batch, layout.backPos, Vector2(layout.actionButtonSize.x, actionBorderHeight), Color(0.58f, 0.84f, 1.0f, 0.86f));
		ui->Draw(batch, menuContext.backLabel, layout.backPos + Vector2(20.0f * layout.uiScale, 10.0f * layout.uiScale), (m_selectedItem == menuContext.backItemIndex) ? selectedStyle : normalStyle, 0.8f * layout.uiScale);
		ui->Draw(batch, menuContext.backDetail, layout.backPos + Vector2(20.0f * layout.uiScale, 31.0f * layout.uiScale), subStyle, backDetailScale);
	}
	DrawSolidRect(batch, layout.titlePos, layout.actionButtonSize, titleColor);
	DrawSolidRect(batch, layout.quitPos, layout.actionButtonSize, quitColor);
	DrawSolidRect(batch, layout.titlePos, Vector2(layout.actionButtonSize.x, actionBorderHeight), Color(0.52f, 0.70f, 0.95f, 0.86f));
	DrawSolidRect(batch, layout.quitPos, Vector2(layout.actionButtonSize.x, actionBorderHeight), Color(0.95f, 0.58f, 0.52f, 0.86f));
	ui->Draw(batch, L"タイトルへ", layout.titlePos + Vector2(20.0f * layout.uiScale, 10.0f * layout.uiScale), (m_selectedItem == menuContext.titleItemIndex) ? selectedStyle : normalStyle, 0.82f * layout.uiScale);
	ui->Draw(batch, L"アプリ終了", layout.quitPos + Vector2(20.0f * layout.uiScale, 10.0f * layout.uiScale), (m_selectedItem == menuContext.quitItemIndex) ? selectedStyle : normalStyle, 0.82f * layout.uiScale);
	ui->Draw(batch, menuContext.titleDetail, layout.titlePos + Vector2(20.0f * layout.uiScale, 31.0f * layout.uiScale), subStyle, titleDetailScale);
	ui->Draw(batch, L"アプリケーションを終了する", layout.quitPos + Vector2(20.0f * layout.uiScale, 31.0f * layout.uiScale), subStyle, quitDetailScale);

	const float footerScale = 0.6f * layout.uiScale;
	const Vector2 escapeHintSize = ui->Measure(menuContext.escapeHint, footerScale);
	ui->Draw(batch, L"Enter/クリック: 決定", layout.panelPos + Vector2(24.0f * layout.uiScale, layout.panelSize.y - 34.0f * layout.uiScale), helpStyle, footerScale);
	ui->Draw(batch, menuContext.escapeHint, layout.panelPos + Vector2(std::max(24.0f * layout.uiScale, layout.panelSize.x - escapeHintSize.x - 24.0f * layout.uiScale), layout.panelSize.y - 34.0f * layout.uiScale), helpStyle, footerScale);

	if (m_clickFxTimer > 0.0f)
	{
		const float progress = 1.0f - (m_clickFxTimer / kClickFxDurationSec);
		const float radius = (7.0f + progress * 36.0f) * layout.uiScale;
		const float alpha = (1.0f - progress) * 0.86f;
		const Color fxColor(0.62f, 0.9f, 1.0f, alpha);
		for (int particle = 0; particle < 12; ++particle)
		{
			const float fp = static_cast<float>(particle);
			const float angle = (DirectX::XM_2PI / 12.0f) * fp + progress * 1.5f;
			const Vector2 dir(std::cosf(angle), std::sinf(angle));
			const Vector2 point = m_clickFxPos + dir * radius;
			const float size = (2.0f + std::fmod(fp, 2.0f)) * layout.uiScale;
			DrawSolidRect(batch, point - Vector2(size * 0.5f, size * 0.5f), Vector2(size, size), fxColor);
		}

		const float coreRadius = (3.0f + (1.0f - progress) * 8.0f) * layout.uiScale;
		DrawSolidRect(batch, m_clickFxPos + Vector2(-coreRadius, -layout.uiScale), Vector2(coreRadius * 2.0f, 2.0f * layout.uiScale), Color(0.96f, 1.0f, 0.98f, alpha * 0.92f));
		DrawSolidRect(batch, m_clickFxPos + Vector2(-layout.uiScale, -coreRadius), Vector2(2.0f * layout.uiScale, coreRadius * 2.0f), Color(0.96f, 1.0f, 0.98f, alpha * 0.92f));
	}

	EndSpriteLayer();
}

void SettingsScene::Finalize()
{
	m_uiSolidTexture.Reset();
	m_textRenderer.reset();
	m_sceneTime = 0.0f;
	m_clickFxTimer = 0.0f;
	m_clickFxPos = Vector2::Zero;
}

void SettingsScene::DrawSolidRect(
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

void SettingsScene::ApplyMouseSensitivity(float value)
{
	m_mouseSensitivity = std::max(kSensitivityMin, std::min(kSensitivityMax, value));
	System::InputManager::GetInstance().SetMouseSensitivity(m_mouseSensitivity);
	GameSaveData::GetInstance().SetMouseSensitivity(m_mouseSensitivity);
}

float SettingsScene::GetSettingValue(int index) const
{
	switch (index)
	{
	case MouseSensitivity:
		return m_mouseSensitivity;
	case MasterVolume:
		return m_audioVolume.master;
	case BgmVolume:
		return m_audioVolume.bgm;
	case SeVolume:
		return m_audioVolume.se;
	case UiVolume:
		return m_audioVolume.ui;
	default:
		return 0.0f;
	}
}

void SettingsScene::SetSettingValue(int index, float value)
{
	switch (index)
	{
	case MouseSensitivity:
		ApplyMouseSensitivity(value);
		break;
	case MasterVolume:
		m_audioVolume.master = ClampVolume(value);
		GameAudio::AudioSystem::GetInstance().SetMasterVolume(m_audioVolume.master);
		GameSaveData::GetInstance().SetAudioVolumeSettings(m_audioVolume);
		break;
	case BgmVolume:
		ApplyAudioBusVolume(GameAudio::AudioBus::Bgm, value);
		break;
	case SeVolume:
		ApplyAudioBusVolume(GameAudio::AudioBus::Se, value);
		break;
	case UiVolume:
		ApplyAudioBusVolume(GameAudio::AudioBus::Ui, value);
		break;
	default:
		break;
	}
}

void SettingsScene::ApplyAudioBusVolume(GameAudio::AudioBus bus, float value)
{
	const float clamped = ClampVolume(value);
	switch (bus)
	{
	case GameAudio::AudioBus::Bgm:
		m_audioVolume.bgm = clamped;
		break;
	case GameAudio::AudioBus::Se:
		m_audioVolume.se = clamped;
		break;
	case GameAudio::AudioBus::Ui:
		m_audioVolume.ui = clamped;
		break;
	default:
		break;
	}

	GameAudio::AudioSystem::GetInstance().SetBusVolume(bus, clamped);
	GameSaveData::GetInstance().SetAudioVolumeSettings(m_audioVolume);
}

void SettingsScene::ReturnToBackScene(bool returnToSuspendedGame, SceneId backScene)
{
	if (returnToSuspendedGame)
	{
		m_sceneManager->PopScene();
		return;
	}

	m_sceneManager->SetScene(backScene);
}
