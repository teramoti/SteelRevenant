//------------------------//------------------------
// Contents(処理内容) SimpleMath 拡張補助関数を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once

//-----------------------------------------------------------------------------
// SimpleMathEx
//-----------------------------------------------------------------------------
// ゲームプレイ側で共通利用する補助計算をまとめる。
// 依存を増やさないため、単純な関数だけをここへ置く。
// DirectXTK の要件に合わせて SimpleMath.h より先に d3d11.h を読む。
//-----------------------------------------------------------------------------

#include <d3d11.h>
#include <SimpleMath.h>
#include <cmath>

namespace Utility
{
	namespace MathEx
	{
		// 値を指定した最小値と最大値の範囲へ収める。
		template <typename T>
		inline T Clamp(const T& value, const T& minValue, const T& maxValue)
		{
			if (value < minValue) return minValue;
			if (value > maxValue) return maxValue;
			return value;
		}

		// 0.0 から 1.0 の範囲へ収める。
		inline float Clamp01(float value)
		{
			return Clamp<float>(value, 0.0f, 1.0f);
		}

		// 長さが十分あるベクトルだけを正規化し、短い場合はゼロベクトルを返す。
		inline DirectX::SimpleMath::Vector3 SafeNormalize(
			const DirectX::SimpleMath::Vector3& value,
			float epsilon = 0.00001f)
		{
			if (value.LengthSquared() <= epsilon)
			{
				return DirectX::SimpleMath::Vector3::Zero;
			}

			DirectX::SimpleMath::Vector3 out = value;
			out.Normalize();
			return out;
		}

		// 角度を -PI から PI の範囲へ折り返す。
		inline float WrapRadians(float value)
		{
			const float kPi = DirectX::XM_PI;
			const float kTwoPi = DirectX::XM_PI * 2.0f;
			while (value > kPi) value -= kTwoPi;
			while (value < -kPi) value += kTwoPi;
			return value;
		}
	}
}


