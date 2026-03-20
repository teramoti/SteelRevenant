//------------------------//------------------------
// Contents(処理内容) シーン共有データ、設定値、サバイバル結果を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#pragma once

#include <array>
#include <string>

#include "../Utility/SingletonBase.h"
#include "../Utility/Sound/AudioTypes.h"

struct BattleResultData
{
	int stageIndex = 1;
	int killCount = 0;
	float survivalTimeSec = 0.0f;
	int damageTaken = 0;
	int relayCaptured = 0;
	int relayRequired = 0;
	int beaconUseCount = 0;
	int totalScore = 0;
	int peakDangerLevel = 1;
	int bestScore = 0;
	bool isNewRecord = false;
};

struct AppSettingsData
{
	float mouseSensitivity = 0.08f;
	GameAudio::AudioVolumeSettings audioVolume = {};
};

class GameSaveData : public Utility::SingletonBase<GameSaveData>
{
public:
	friend class Utility::SingletonBase<GameSaveData>;

public:
	// 現在のステージ番号を設定する。
	void SetStageNum(int num);
	// ステージモデル名を設定する。
	void SetLoadName(std::wstring name);
	// コリジョンモデル名を設定する。
	void SetLoadColName(std::wstring name);
	// 敵モデル名を設定する。
	void SetLoadEnemyName(std::wstring name);

	// リザルト画面で参照する戦績を保存する。
	void SetBattleResult(const BattleResultData& result);
	// ステージ別ベストスコアを更新し、更新有無を返す。
	bool UpdateBestScore(int stageIndex, int score);
	// 保存中の戦績を返す。
	const BattleResultData& GetBattleResult() const;
	// 指定ステージのベストスコアを返す。
	int GetBestScore(int stageIndex) const;

	// アプリ共通設定をまとめて設定する。
	void SetAppSettings(const AppSettingsData& settings);
	// アプリ共通設定を返す。
	const AppSettingsData& GetAppSettings() const;
	// マウス感度設定を更新する。
	void SetMouseSensitivity(float value);
	// マウス感度設定を返す。
	float GetMouseSensitivity() const;
	// 音量設定をまとめて更新する。
	void SetAudioVolumeSettings(const GameAudio::AudioVolumeSettings& settings);
	// 音量設定を返す。
	const GameAudio::AudioVolumeSettings& GetAudioVolumeSettings() const;

	// 現在のステージモデル名を参照で返す。
	std::wstring& GetLoadName()
	{
		return m_name;
	}

	// 現在のコリジョンモデル名を参照で返す。
	std::wstring& GetLoadColName()
	{
		return m_colName;
	}

	// 現在の敵モデル名を参照で返す。
	std::wstring& GetLoadEnemyName()
	{
		return m_enemyName;
	}

	// 現在選択中のステージ番号を参照で返す。
	int& GetStage()
	{
		return m_stageNum;
	}

private:
	// 実行時の保存データを読み込んで初期値へ反映する。
	GameSaveData();
	// ベストスコアと設定を保存ファイルへ書き出す。
	void SaveToDisk() const;
	// 保存ファイルからベストスコアと設定を復元する。
	void LoadFromDisk();

private:
	int m_stageNum = 0;
	std::wstring m_name;
	std::wstring m_colName;
	std::wstring m_enemyName;
	BattleResultData m_battleResult = {};
	std::array<int, 3> m_bestScore = { 0, 0, 0 };
	AppSettingsData m_appSettings = {};
};
