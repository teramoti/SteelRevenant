//------------------------//------------------------
// Contents(処理内容) シーン共有データ、設定値、サバイバル結果の保持と永続化を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 20
//------------------------//------------------------
#include "GameSaveData.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
	namespace fs = std::filesystem;

	constexpr int kSaveDataVersion = 1;

	// 文字列の前後空白を除去する。
	std::wstring Trim(const std::wstring& value)
	{
		const size_t first = value.find_first_not_of(L" \t\r\n");
		if (first == std::wstring::npos)
		{
			return L"";
		}

		const size_t last = value.find_last_not_of(L" \t\r\n");
		return value.substr(first, last - first + 1);
	}

	// 実数値を安全に解析する。
	bool TryParseFloat(const std::wstring& text, float& outValue)
	{
		try
		{
			size_t consumed = 0;
			outValue = std::stof(text, &consumed);
			return consumed == text.size();
		}
		catch (...)
		{
			return false;
		}
	}

	// 整数値を安全に解析する。
	bool TryParseInt(const std::wstring& text, int& outValue)
	{
		try
		{
			size_t consumed = 0;
			outValue = std::stoi(text, &consumed);
			return consumed == text.size();
		}
		catch (...)
		{
			return false;
		}
	}

	// 設定保存先パスを返す。
	fs::path ResolveSaveFilePath()
	{
		wchar_t* localAppData = nullptr;
		size_t length = 0;
		if (_wdupenv_s(&localAppData, &length, L"LOCALAPPDATA") == 0 && localAppData != nullptr)
		{
			const fs::path savePath = (*localAppData != L'\0')
				? (fs::path(localAppData) / L"SteelRevenant" / L"app_state.txt")
				: (fs::current_path() / L"Save" / L"app_state.txt");
			free(localAppData);
			return savePath;
		}

		if (localAppData != nullptr)
		{
			free(localAppData);
		}

		return fs::current_path() / L"Save" / L"app_state.txt";
	}
}

// 保存済みの設定とベストスコアを読み込んで初期化する。
GameSaveData::GameSaveData()
{
	LoadFromDisk();
	SaveToDisk();
}

// 現在のステージ番号を設定する。
void GameSaveData::SetStageNum(int num)
{
	m_stageNum = num;
	SaveToDisk();
}

// ステージモデル名を設定する。
void GameSaveData::SetLoadName(std::wstring name)
{
	m_name = std::move(name);
}

// コリジョンモデル名を設定する。
void GameSaveData::SetLoadColName(std::wstring name)
{
	m_colName = std::move(name);
}

// 敵モデル名を設定する。
void GameSaveData::SetLoadEnemyName(std::wstring name)
{
	m_enemyName = std::move(name);
}

// リザルト画面で参照する戦績を保存する。
void GameSaveData::SetBattleResult(const BattleResultData& result)
{
	m_battleResult = result;
	SaveToDisk();
}

// ステージ別ベストスコアを更新し、更新有無を返す。
bool GameSaveData::UpdateBestScore(int stageIndex, int score)
{
	const int clampedIndex = std::max(1, std::min(3, stageIndex)) - 1;
	if (score <= m_bestScore[static_cast<size_t>(clampedIndex)])
	{
		return false;
	}

	m_bestScore[static_cast<size_t>(clampedIndex)] = score;
	SaveToDisk();
	return true;
}

// 保存中の戦績を返す。
const BattleResultData& GameSaveData::GetBattleResult() const
{
	return m_battleResult;
}

// 指定ステージのベストスコアを返す。
int GameSaveData::GetBestScore(int stageIndex) const
{
	const int clampedIndex = std::max(1, std::min(3, stageIndex)) - 1;
	return m_bestScore[static_cast<size_t>(clampedIndex)];
}

// アプリ共通設定をまとめて設定する。
void GameSaveData::SetAppSettings(const AppSettingsData& settings)
{
	m_appSettings.mouseSensitivity = std::clamp(settings.mouseSensitivity, 0.001f, 1.0f);
	SetAudioVolumeSettings(settings.audioVolume);
	SaveToDisk();
}

// アプリ共通設定を返す。
const AppSettingsData& GameSaveData::GetAppSettings() const
{
	return m_appSettings;
}

// マウス感度設定を更新する。
void GameSaveData::SetMouseSensitivity(float value)
{
	m_appSettings.mouseSensitivity = std::clamp(value, 0.001f, 1.0f);
	SaveToDisk();
}

// マウス感度設定を返す。
float GameSaveData::GetMouseSensitivity() const
{
	return m_appSettings.mouseSensitivity;
}

// 音量設定をまとめて更新する。
void GameSaveData::SetAudioVolumeSettings(const GameAudio::AudioVolumeSettings& settings)
{
	m_appSettings.audioVolume.master = std::clamp(settings.master, 0.0f, 1.0f);
	m_appSettings.audioVolume.bgm = std::clamp(settings.bgm, 0.0f, 1.0f);
	m_appSettings.audioVolume.se = std::clamp(settings.se, 0.0f, 1.0f);
	m_appSettings.audioVolume.ui = std::clamp(settings.ui, 0.0f, 1.0f);
	SaveToDisk();
}

// 音量設定を返す。
const GameAudio::AudioVolumeSettings& GameSaveData::GetAudioVolumeSettings() const
{
	return m_appSettings.audioVolume;
}

// ベストスコアと設定を保存ファイルへ書き出す。
void GameSaveData::SaveToDisk() const
{
	const fs::path savePath = ResolveSaveFilePath();
	std::error_code error;
	fs::create_directories(savePath.parent_path(), error);

	std::wofstream stream(savePath, std::ios::trunc);
	if (!stream.is_open())
	{
		return;
	}

	stream << L"version=" << kSaveDataVersion << L'\n';
	stream << L"stage=" << m_stageNum << L'\n';
	stream << L"best1=" << m_bestScore[0] << L'\n';
	stream << L"best2=" << m_bestScore[1] << L'\n';
	stream << L"best3=" << m_bestScore[2] << L'\n';
	stream << L"mouse=" << m_appSettings.mouseSensitivity << L'\n';
	stream << L"master=" << m_appSettings.audioVolume.master << L'\n';
	stream << L"bgm=" << m_appSettings.audioVolume.bgm << L'\n';
	stream << L"se=" << m_appSettings.audioVolume.se << L'\n';
	stream << L"ui=" << m_appSettings.audioVolume.ui << L'\n';
}

// 保存ファイルからベストスコアと設定を復元する。
void GameSaveData::LoadFromDisk()
{
	const fs::path savePath = ResolveSaveFilePath();
	std::wifstream stream(savePath);
	if (!stream.is_open())
	{
		return;
	}

	std::wstring line;
	while (std::getline(stream, line))
	{
		const size_t separator = line.find(L'=');
		if (separator == std::wstring::npos)
		{
			continue;
		}

		const std::wstring key = Trim(line.substr(0, separator));
		const std::wstring value = Trim(line.substr(separator + 1));
		if (key.empty() || value.empty())
		{
			continue;
		}

		int intValue = 0;
		float floatValue = 0.0f;
		if (key == L"stage" && TryParseInt(value, intValue))
		{
			m_stageNum = intValue;
		}
		else if (key == L"best1" && TryParseInt(value, intValue))
		{
			m_bestScore[0] = std::max(0, intValue);
		}
		else if (key == L"best2" && TryParseInt(value, intValue))
		{
			m_bestScore[1] = std::max(0, intValue);
		}
		else if (key == L"best3" && TryParseInt(value, intValue))
		{
			m_bestScore[2] = std::max(0, intValue);
		}
		else if (key == L"mouse" && TryParseFloat(value, floatValue))
		{
			m_appSettings.mouseSensitivity = std::clamp(floatValue, 0.001f, 1.0f);
		}
		else if (key == L"master" && TryParseFloat(value, floatValue))
		{
			m_appSettings.audioVolume.master = std::clamp(floatValue, 0.0f, 1.0f);
		}
		else if (key == L"bgm" && TryParseFloat(value, floatValue))
		{
			m_appSettings.audioVolume.bgm = std::clamp(floatValue, 0.0f, 1.0f);
		}
		else if (key == L"se" && TryParseFloat(value, floatValue))
		{
			m_appSettings.audioVolume.se = std::clamp(floatValue, 0.0f, 1.0f);
		}
		else if (key == L"ui" && TryParseFloat(value, floatValue))
		{
			m_appSettings.audioVolume.ui = std::clamp(floatValue, 0.0f, 1.0f);
		}
	}
}
