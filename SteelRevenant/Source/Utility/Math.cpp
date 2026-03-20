//------------------------//------------------------
// Contents(処理内容) 汎用数学補助関数を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "Math.h"

using namespace std;
using namespace Teramoto;

// π
const float Teramoto::Math::PI = 3.14159274f;

// 絶対値を返す。
template<typename T>
// 値の絶対値を返す。
T Teramoto::Math::Abs(T num)
{
	if (num < 0)
	{
		num *= -1;
	}
	return num;
}

// 2点の距離を求める。
template<typename T>
// 2値の差の絶対値を返す。
inline T Math::Difference(T p1, T p2)
{
	auto length = p1 - p2;
	return Abs<T>(length);
}

// 自分からターゲットへの方向(角度)を求める。
float Teramoto::Math::TargetAngle(float myPos1, float myPos2, float targetPos1, float targetPos2)
{
	float dPos1;
	float dPos2;
	float angle;

	dPos1 = myPos1 - targetPos1;
	dPos2 = myPos2 - targetPos2;

	angle = float(atan2(dPos2, dPos1));

	return angle;	
}
	

template<typename T>
// 値が2つの境界値の間に収まっているかを返す。
bool Teramoto::Math::Clamp(T value, T num1, T num2)
{
	if (num1 < num2)
	{
		if (value > num2) return false;
		if (value < num1) return false;

		return true;
	}

	if (num1 > num2)
	{
		if (value < num2) return false;
		if (value > num1) return false;

		return true;
	}

	return true;
}
//bool MyLibrary::Math::Clamp(T value, T lo, T hi)
//{
//	if(value < hi)  return false;
//	if(value > lo)	return false;
//
//	return true;
//}

// 乱数の取得(int型)。
int Teramoto::Math::GetRand(int min, int max)
{
	int random;

	srand((unsigned)time(NULL));
	random = rand() % (max - min + 1) + min;

	return random;
}

// デグリーをラジアンに変換。
float Teramoto::Math::ToRad(float deg)
{
	return deg * (PI / 180.f);
}

// ラジアンをデグリーに変換。
float Teramoto::Math::ToDeg(float rad)
{
	return rad * (180.f * PI);
}

