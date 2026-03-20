Steel Revenant AudioSystem

概要
- training_zone を除外した再生機構です。
- BGM はループ対応です。
- SE は one-shot 再生です。
- 音量設定は master / bgm / se / ui の 4 系統です。
- ゲーム側の入口は AudioSystem 1 クラスです。

設計意図
- Facade:
  AudioSystem を外部公開の唯一の窓口にしています。
- Repository:
  AudioCueDef の定義と読み込み済み SoundEffect を分けています。
- 呼び出し側は DirectXTK の型を直接触らずに済みます。

同梱ファイル
- AudioTypes.h
- AudioSystem.h
- AudioSystem.cpp
- SampleUsage.cpp

前提
- DirectXTK for Audio を導入済みであること
- #include <Audio.h> が通ること
- wav ファイルが指定パスに存在すること

配置する wav 名
- title_theme.wav
- stage_select.wav
- defense_corridor.wav
- core_sector.wav
- result_clear.wav
- result_fail.wav
- ui_move.wav
- ui_confirm.wav
- ui_back.wav
- player_shot.wav
- enemy_shot.wav
- player_hit.wav
- guard_block.wav
- melee_slash.wav
- enemy_destroy.wav
- relay_start.wav
- relay_secure.wav
- beacon_heal.wav
- warning_alert.wav
- bonus_time.wav

使い方の要点
1. Initialize で wav フォルダを渡す
2. 毎フレーム Update を呼ぶ
3. シーン遷移で PlayBgm を呼ぶ
4. UI / 戦闘イベントで PlaySe を呼ぶ
5. オプション画面から ApplyVolumeSettings を呼ぶ

補足
- DirectXTK の AudioEngine は毎フレーム Update が必要です。
- ループや再生中音量変更は SoundEffectInstance を使います。
- one-shot の SE は SoundEffect::Play を使います。
- SoundEffectInstance は SoundEffect が所有する wave データを参照するため、親の SoundEffect を生かしたままにします。
- ゲーム全体の一時停止には AudioEngine::Suspend / Resume を使います。

注意
- Windows デスクトップアプリでは、プロジェクト構成によっては COM 初期化が別途必要です。
- この実装はフェードやクロスフェードは入れていません。
