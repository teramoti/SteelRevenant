//------------------------//------------------------
// Contents(処理内容) シーン生成、切替、復帰処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "SceneManager.h"

#include "../Base/SceneBase.h"
#include "../TitleScene/TitleScene.h"
#include "../StageSelectScene/StageSelectScene.h"
#include "../SettingsScene/SettingsScene.h"
#include "../GameScene/GameScene.h"
#include "../ResultScene/ResultScene.h"
#include "../../GameSystem/GameSaveData.h"
#include "../../Utility/Sound/AudioSystem.h"

#include <algorithm>

namespace
{
	// 現在のステージ番号から本編 BGM を決める。
	GameAudio::BgmId ResolveGameStageBgm()
	{
		const int stageIndex = std::max(1, std::min(3, GameSaveData::GetInstance().GetStage()));
		return (stageIndex >= 3) ? GameAudio::BgmId::CoreSector : GameAudio::BgmId::DefenseCorridor;
	}

	// アクティブシーンに対応する BGM を適用する。
	void ApplySceneBgm(SceneId sceneId)
	{
		GameAudio::AudioSystem& audio = GameAudio::AudioSystem::GetInstance();
		if (!audio.IsInitialized())
		{
			return;
		}

		switch (sceneId)
		{
		case TITLE_SCENE:
			audio.PlayBgm(GameAudio::BgmId::Title, false);
			break;
		case SELECT_SCENE:
			audio.PlayBgm(GameAudio::BgmId::StageSelect, false);
			break;
		case GAME_SCENE:
			audio.PlayBgm(ResolveGameStageBgm(), false);
			break;
		case RESULT_SCENE:
		{
			const BattleResultData& result = GameSaveData::GetInstance().GetBattleResult();
			audio.PlayBgm(result.isNewRecord ? GameAudio::BgmId::ResultClear : GameAudio::BgmId::ResultFail, false);
			break;
		}
		case SETTINGS_SCENE:
		default:
			break;
		}
	}
}

// シーン管理に必要な参照と状態を初期化する。
SceneManager::SceneManager()
	: m_activeScene(nullptr)
	, m_requestedScene(nullptr)
	, m_suspendedScene(nullptr)
	, m_activeSceneId(TITLE_SCENE)
	, m_previousSceneId(TITLE_SCENE)
	, m_requestedSceneId(TITLE_SCENE)
	, m_suspendedSceneId(TITLE_SCENE)
{
	for (int i = 0; i < NUM_SCENES; ++i)
	{
		m_scenes[i] = nullptr;
	}
}

// 管理中シーンの終了処理と解放を行う。
SceneManager::~SceneManager()
{
	FinalizeActiveScene();
}

// 指定されたシーン ID に対応するシーンインスタンスを生成する。
SceneBase* SceneManager::CreateScene(SceneId sceneId)
{
	switch (sceneId)
	{
	case TITLE_SCENE:
		return new TitleScene(this);
	case SELECT_SCENE:
		return new StageSelectScene(this);
	case SETTINGS_SCENE:
		return new SettingsScene(this);
	case GAME_SCENE:
		return new GameScene(this);
	case RESULT_SCENE:
		return new ResultScene(this);
	default:
		return nullptr;
	}
}

// 保留中のシーン切り替え要求を現在状態へ反映する。
void SceneManager::ApplyRequestedScene()
{
	if (m_requestedScene == nullptr)
	{
		return;
	}
	if (m_activeScene != nullptr)
	{
		m_activeScene->Finalize();
		delete m_activeScene;
		m_activeScene = nullptr;
	}
	m_previousSceneId = m_activeSceneId;
	m_activeSceneId = m_requestedSceneId;
	m_activeScene = m_requestedScene;
	m_requestedScene = nullptr;
	if (m_activeScene != nullptr)
	{
		m_activeScene->Initialize();
	}
	ApplySceneBgm(m_activeSceneId);
}

// 現在のアクティブシーンを初期化する。
void SceneManager::InitilizeActiveScene()
{
	if (m_activeScene == nullptr && m_requestedScene != nullptr)
	{
		ApplyRequestedScene();
	}
}

// 現在のアクティブシーンを更新する。
void SceneManager::UpdateActiveScene(const DX::StepTimer& stepTimer)
{
	if (m_activeScene == nullptr && m_requestedScene != nullptr)
	{
		ApplyRequestedScene();
	}
	if (m_activeScene != nullptr)
	{
		m_activeScene->Update(stepTimer);
	}
	if (m_requestedScene != nullptr)
	{
		ApplyRequestedScene();
	}
}

// 現在のアクティブシーンを描画する。
void SceneManager::RenderActiveSceneRender()
{
	if (m_activeScene != nullptr)
	{
		m_activeScene->Render();
	}
}

// 指定シーンへの遷移要求を設定する。
void SceneManager::SetScene(SceneId startSceneId)
{
	ReleaseSuspendedScene();
	m_requestedSceneId = startSceneId;
	if (m_requestedScene != nullptr)
	{
		m_requestedScene->Finalize();
		delete m_requestedScene;
		m_requestedScene = nullptr;
	}
	m_requestedScene = CreateScene(startSceneId);
}

// 現在のシーンを退避しつつオーバーレイシーンへの遷移要求を設定する。
void SceneManager::PushScene(SceneId overlaySceneId)
{
	if (m_activeScene == nullptr || m_suspendedScene != nullptr)
	{
		SetScene(overlaySceneId);
		return;
	}

	if (m_requestedScene != nullptr)
	{
		m_requestedScene->Finalize();
		delete m_requestedScene;
		m_requestedScene = nullptr;
	}

	m_suspendedScene = m_activeScene;
	m_suspendedSceneId = m_activeSceneId;
	m_previousSceneId = m_suspendedSceneId;
	m_activeScene = CreateScene(overlaySceneId);
	m_activeSceneId = overlaySceneId;
	m_requestedSceneId = overlaySceneId;
	if (m_activeScene != nullptr)
	{
		m_activeScene->Initialize();
	}
}

// 退避していたシーンへ復帰し、復帰できたかを返す。
bool SceneManager::PopScene()
{
	if (m_suspendedScene == nullptr)
	{
		return false;
	}

	if (m_requestedScene != nullptr)
	{
		m_requestedScene->Finalize();
		delete m_requestedScene;
		m_requestedScene = nullptr;
	}

	if (m_activeScene != nullptr)
	{
		m_activeScene->Finalize();
		delete m_activeScene;
		m_activeScene = nullptr;
	}

	m_previousSceneId = m_activeSceneId;
	m_activeScene = m_suspendedScene;
	m_activeSceneId = m_suspendedSceneId;
	m_suspendedScene = nullptr;
	ApplySceneBgm(m_activeSceneId);
	return true;
}

// 現在のアクティブシーンを終了処理する。
void SceneManager::FinalizeActiveScene()
{
	if (m_activeScene != nullptr)
	{
		m_activeScene->Finalize();
		delete m_activeScene;
		m_activeScene = nullptr;
	}
	ReleaseSuspendedScene();
	if (m_requestedScene != nullptr)
	{
		m_requestedScene->Finalize();
		delete m_requestedScene;
		m_requestedScene = nullptr;
	}
}

// 管理対象のアクティブシーン参照を差し替える。
void SceneManager::ChangeScene(SceneBase* nextScene)
{
	if (m_requestedScene != nullptr)
	{
		m_requestedScene->Finalize();
		delete m_requestedScene;
	}
	m_requestedScene = nextScene;
	m_requestedSceneId = m_activeSceneId;
}

// 退避中のシーンを解放する。
void SceneManager::ReleaseSuspendedScene()
{
	if (m_suspendedScene != nullptr)
	{
		m_suspendedScene->Finalize();
		delete m_suspendedScene;
		m_suspendedScene = nullptr;
	}
}

// 退避中のシーンが存在するかを返す。
bool SceneManager::HasSuspendedScene() const
{
	return m_suspendedScene != nullptr;
}

// 退避中シーンの ID を返す。
SceneId SceneManager::GetSuspendedSceneId() const
{
	return m_suspendedSceneId;
}

// 現在のアクティブシーン ID を返す。
SceneId SceneManager::GetActiveSceneId() const
{
	return m_activeSceneId;
}

// 直前に表示していたシーン ID を返す。
SceneId SceneManager::GetPreviousSceneId() const
{
	return m_previousSceneId;
}

