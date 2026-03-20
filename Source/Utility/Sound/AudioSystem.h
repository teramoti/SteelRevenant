//------------------------//------------------------
// Contents(処理内容) ゲーム全体で共有する音声再生サービスを宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 20
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#pragma once

#include "AudioTypes.h"
#include "../../Utility/SingletonBase.h"

#include <Audio.h>

#include <map>
#include <memory>
#include <optional>
#include <string>

namespace GameAudio
{
	class AudioSystem final : public Utility::SingletonBase<AudioSystem>
	{
	public:
		friend class Utility::SingletonBase<AudioSystem>;

	private:
		AudioSystem() = default;
		~AudioSystem() = default;

	public:
		// wav 群を置いたルートを受け取り、音声サービスを初期化する。
		bool Initialize(const std::wstring& audioRoot);
		// 音声サービスを停止し、読み込み済みリソースを解放する。
		void Shutdown();
		// 毎フレーム AudioEngine を更新する。
		void Update();
		// アプリ全体の一時停止に合わせて音声更新を止める。
		void SuspendAll();
		// 一時停止から復帰した音声更新を再開する。
		void ResumeAll();

		// BGM を再生する。
		void PlayBgm(BgmId id, bool restart = true);
		// 現在再生中の BGM を停止する。
		void StopBgm(bool immediate = true);
		// SE を one-shot 再生する。
		void PlaySe(SeId id, float volumeScale = 1.0f, float pitch = 0.0f, float pan = 0.0f);

		// 音量設定を更新する。
		void SetMasterVolume(float value);
		void SetBusVolume(AudioBus bus, float value);
		void ApplyVolumeSettings(const AudioVolumeSettings& settings);

		// 現在の音量設定を返す。
		const AudioVolumeSettings& GetVolumeSettings() const noexcept { return m_volume; }
		// 初期化済みか返す。
		bool IsInitialized() const noexcept { return m_initialized; }

	private:
		float Clamp01(float value) const noexcept;
		float GetBusVolume(AudioBus bus) const noexcept;
		float GetEffectiveOneShotVolume(const AudioCueDef& cue, float volumeScale) const noexcept;
		float GetEffectiveInstanceVolume(const AudioCueDef& cue) const noexcept;

		void BuildCatalog(const std::wstring& audioRoot);
		bool LoadAll();
		void ApplyCurrentBgmVolume();

	private:
		std::unique_ptr<DirectX::AudioEngine> m_engine;
		std::map<BgmId, AudioCueDef> m_bgmDefs;
		std::map<SeId, AudioCueDef> m_seDefs;
		std::map<BgmId, std::unique_ptr<DirectX::SoundEffect>> m_bgmEffects;
		std::map<SeId, std::unique_ptr<DirectX::SoundEffect>> m_seEffects;
		std::unique_ptr<DirectX::SoundEffectInstance> m_currentBgm;
		std::optional<BgmId> m_currentBgmId;
		AudioVolumeSettings m_volume{};
		bool m_initialized = false;
	};
}
