//------------------------//------------------------
// Contents(処理内容) DirectX 初期化で使う HRESULT 補助関数を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

#include <windows.h>
#include <stdexcept>

namespace DX
{
	// 失敗した HRESULT を例外へ変換する。
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// 失敗箇所で停止できるよう例外を送出する。
			throw std::exception();
		}
	}
}

