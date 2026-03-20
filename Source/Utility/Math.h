//------------------------//------------------------
// Contents(処理内容) 汎用数学補助関数を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#ifndef  MATH_DEFINED
#define  MATH_DEFINED
// ヘッダファイルの読み込み ================================================
#include <math.h>
//!
// Mathクラス。
/// </summary>
namespace Teramoto
{
	class Math
	{
	private:
		// 円周率
		static const float PI;
	public:
		// 絶対値を求める
		template<typename T>
		// 値の絶対値を返す。
		static T Abs(T num);
		// 二つの点の差分を求める
		template<typename T>
		// 2値の差の絶対値を返す。
		static T Difference(T p1, T p2);
		// 自分からターゲットへの方向(角度)を求める
		static float TargetAngle(float myPos1, float myPos2, float targetPos1, float targetPos2);
		// 指定範囲内かを判定する。
		template<typename T>
		// 値が2つの境界値の間に収まっているかを返す。
		static bool Clamp(T value, T num1, T num2);
		// 乱数
		static int GetRand(int min, int max);
		// デグリーをラジアンに変換
		static float ToRad(float deg);
		// ラジアンをデグリーに変換
		static float ToDeg(float rad);
	};
}
#endif MATH_DEFINED
