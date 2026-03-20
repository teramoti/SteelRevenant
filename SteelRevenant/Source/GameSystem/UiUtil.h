//------------------------//------------------------
// Contents(処理内容) UI描画補助関数群を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//--------------------------------------------------------------------------------------
// File: UiUtil.h
//
// UI描画まわりで複数シーンから共通利用する小規模ユーティリティ群。
// - SpriteBatch用の1x1単色テクスチャ生成
// - 画面矩形の当たり判定
// - 小数点付き文字列整形
//
// 方針:
// - Sceneごとに重複しやすい処理だけを集約する
// - 依存は DirectX / 標準ライブラリ の最小限に留める
//--------------------------------------------------------------------------------------

#include <cstdint>
#include <string>

#include <d3d11.h>
#include <wrl/client.h>
#include <SimpleMath.h>

namespace UiUtil
{
	// 1x1単色テクスチャを生成し、SpriteBatchの矩形描画に利用できるSRVを返す。
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateSolidTexture(
		ID3D11Device* device,
		std::uint32_t rgba = 0xffffffffu);

	// 点pが [pos, pos + size] の矩形内にあるかを判定する。
	bool IsInsideRect(
		const DirectX::SimpleMath::Vector2& p,
		const DirectX::SimpleMath::Vector2& pos,
		const DirectX::SimpleMath::Vector2& size);

	// 小数値を固定小数点のwstringへ整形する。
	std::wstring ToWStringFixed(float value, int precision);
}

