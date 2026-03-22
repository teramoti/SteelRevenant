#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

//------------------------
// ポーズ画面の状態管理と入力処理を担当する。
//------------------------

class GameScenePauseController
{
public:
    // ポーズ状態をリセットする
    void Reset();

    // Escape キーによるポーズ切替を処理する。戻り値: ポーズ中なら true
    bool UpdateToggle();

    // ポーズメニュー内のカーソル移動・決定操作を処理する。
    // 戻り値: シーン遷移要求が発生した場合の遷移先 ID (-1 = 遷移なし, -2 = ゲーム終了要求)
    int UpdateMenuInput(int screenWidth, int screenHeight);

    bool IsPaused() const { return m_isPaused; }
    int  GetSelectedIndex() const { return m_selectedIndex; }

private:
    // マウス座標からメニュー項目のホバー判定を行う
    void DetectMouseHover(int screenWidth, int screenHeight);

    // キーボードによるメニュー項目の上下選択を処理する
    void NavigateMenuByKeyboard();

    bool m_isPaused      = false;
    int  m_selectedIndex = 0;

    // ポーズメニューのレイアウト定数
    static constexpr float kBoxWidth      = 420.0f;
    static constexpr float kBoxHeight     = 220.0f;
    static constexpr float kItemTopOffset = 68.0f;
    static constexpr float kItemSpacing   = 36.0f;
    static constexpr float kItemPaddingX  = 16.0f;
    static constexpr float kItemPaddingY  = 6.0f;
    static constexpr float kItemHeight    = 28.0f;
    static constexpr int   kMenuItemCount = 4;

    // メニュー項目インデックス
    static constexpr int kMenuResume   = 0;
    static constexpr int kMenuTitle    = 1;
    static constexpr int kMenuSettings = 2;
    static constexpr int kMenuQuit     = 3;
};
