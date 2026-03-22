//------------------------//------------------------
// Contents(処理内容) 基本形状どうしの衝突判定関数群を宣言する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
class Collision
{
public:
//------------------------------//
// 境界ボリュームの構造体		//
//------------------------------//
	// 球の構造体
	struct Sphere
	{
		DirectX::SimpleMath::Vector3 c; // 球の中心
		float r;   // 球の半径
	};
	// カプセルの構造体
	struct Capsule
	{
		DirectX::SimpleMath::Vector3 a; // 中間部の線分の開始点
		DirectX::SimpleMath::Vector3 b; // 中間部の線分の終了点
		float r;   // カプセルの半径
	};
	// ボックスの構造体
	struct Box
	{
		DirectX::SimpleMath::Vector3 c; // BOXの中心点
		DirectX::SimpleMath::Vector3 r; // 幅の半分の範囲
	};
//--------------------------------------//
// 境界ボリューム同士の衝突判定関数		//
//--------------------------------------//
	// 球と球の衝突判定関数
	static bool HitCheck_Sphere2Sphere(const Sphere& sphere1, const Sphere& sphere2)
	{
		// 中心間の距離の平方を計算
		DirectX::SimpleMath::Vector3 d = sphere1.c - sphere2.c;
		// 球中心同士の距離の平方を求める。
		float dist2 = d.Dot(d);
		// 平方した距離が平方した半径の合計よりも小さい場合に球は交差している
		float radiusSum = sphere1.r + sphere2.r;
		return dist2 <= radiusSum * radiusSum;
	}
	// 球とカプセルの衝突判定関数
	static bool HitCheck_Sphere2Capsule(const Sphere& sphere, const Capsule& capsule)
	{
		// 球の中心とカプセルの中心の線分との距離の平方を計算
		float dist2 = SqDistPointSegment(capsule.a, capsule.b, sphere.c);
		float radius = sphere.r + capsule.r;
		return dist2 <= radius * radius;
	}
	// カプセルとカプセルの衝突判定関数
	static bool HitCheck_Capsule2Capsule(const Capsule& capsule1, const Capsule& capsule2)
	{
		float s, t;
		DirectX::SimpleMath::Vector3 c1, c2;
		// カプセルの中心の線分間の距離の平方を計算
		float dist2 = ClosestPtSegmentSegment(capsule1.a, capsule1.b, capsule2.a, capsule2.b, s, t, c1, c2);
		float radius = capsule1.r + capsule2.r;
		return dist2 <= radius * radius;
	}
	// ボックスとボックスの衝突判定関数
	static bool HitCheck_Box2Box(const Box& box1, const Box& box2)
	{
		if (fabsf(box1.c.x - box2.c.x) > (box1.r.x + box2.r.x)) return false;
		if (fabsf(box1.c.y - box2.c.y) > (box1.r.y + box2.r.y)) return false;
		if (fabsf(box1.c.z - box2.c.z) > (box1.r.z + box2.r.z)) return false;
		return true;
	}
	// 球とボックスの衝突判定関数
	static bool HitCheck_Sphere2Box(const Sphere& sphere, const Box& box)
	{
		float sqDist = SqDistPointBox(sphere.c, box);
		return sqDist <= sphere.r * sphere.r;
	}
//------------------------------//
// 衝突判定に必要な補助関数群	//
//------------------------------//
// 点cと線分abの間の距離の平方を返す関数。
	static float SqDistPointSegment(DirectX::SimpleMath::Vector3 a, DirectX::SimpleMath::Vector3 b, DirectX::SimpleMath::Vector3 c)
	{
		DirectX::SimpleMath::Vector3 ab = b - a;
		DirectX::SimpleMath::Vector3 ac = c - a;
		DirectX::SimpleMath::Vector3 bc = c - b;
		float e = ac.Dot(ab);
		if (e <= 0.0f) return ac.Dot(ac);
		float f = ab.Dot(ab);
		if (e >= f) return bc.Dot(bc);
		return ac.Dot(ac) - e * e / f;
	}
	// クランプ関数
	static inline float Clamp(float n, float min, float max)
	{
		if (n < min) return min;
		if (n > max) return max;
		return n;
	}
// ２つの線分の最短距離の平方を返す関数。
	static float ClosestPtSegmentSegment(DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 q1
		, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 q2
		, float &s, float &t
		, DirectX::SimpleMath::Vector3& c1, DirectX::SimpleMath::Vector3& c2)
	{
		DirectX::SimpleMath::Vector3 d1 = q1 - p1;
		DirectX::SimpleMath::Vector3 d2 = q2 - p2;
		DirectX::SimpleMath::Vector3 r = p1 - p2;
		float a = d1.Dot(d1);
		float e = d2.Dot(d2);
		float f = d2.Dot(r);
		if (a <= FLT_EPSILON && e <= FLT_EPSILON)
		{
			s = t = 0.0f;
			c1 = p1;
			c2 = p2;
			return (c1 - c2).Dot(c1 - c2);
		}
		if (a <= FLT_EPSILON)
		{
			s = 0.0f;
			t = f / e;
			t = Clamp(t, 0.0f, 1.0f);
		}
		else
		{
			float c = d1.Dot(r);
			if (e <= FLT_EPSILON)
			{
				t = 0.0f;
				s = Clamp(-c / a, 0.0f, 1.0f);
			}
			else
			{
				float b = d1.Dot(d2);
				float denom = a * e - b * b;
				if (denom != 0.0f)
				{
					s = Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
				}
				else
				{
					s = 0.0f;
				}
				float tnom = (b * s + f);
				if (tnom < 0.0f)
				{
					t = 0.0f;
					s = Clamp(-c / a, 0.0f, 1.0f);
				}
				else if (tnom > e)
				{
					t = 1.0f;
					s = Clamp((b - c) / a, 0.0f, 1.0f);
				}
				else
				{
					t = tnom / e;
				}
			}
		}
		c1 = p1 + d1 * s;
		c2 = p2 + d2 * t;
		return (c1 - c2).Dot(c1 - c2);
	}
	// 点とボックスの間の最短距離の平方を計算する関数
	static float SqDistPointBox(const DirectX::SimpleMath::Vector3& p, const Box& b)
	{
		float point[3] = { p.x, p.y, p.z };
		float min[3] = { b.c.x - b.r.x, b.c.y - b.r.y, b.c.z - b.r.z, };
		float max[3] = { b.c.x + b.r.x, b.c.y + b.r.y, b.c.z + b.r.z, };
		float sqDist = 0.0f;
		for (int i = 0; i < 3; i++)
		{
			float v = point[i];
			if (v < min[i]) sqDist += (min[i] - v) * (min[i] - v);
			if (v > max[i]) sqDist += (v - max[i]) * (v - max[i]);
		}
		return sqDist;
	}
//------------------------------//
// 線分と三角形の交差判定用		//
//------------------------------//
	// 三角形の構造体（線分と三角形の交差判定用）
	struct Triangle
	{
		// 三角形の平面方程式
		DirectX::SimpleMath::Plane p;
		// 辺BCの平面方程式（重心座標の頂点aに対する重みuを与える）
		DirectX::SimpleMath::Plane edgePlaneBC;
		// 辺CAの平面方程式（重心座標の頂点bに対する重みvを与える）
		DirectX::SimpleMath::Plane edgePlaneCA;
		// コンストラクタ内で衝突判定を軽くするために前処理で計算しておく
		Triangle(DirectX::SimpleMath::Vector3 a, DirectX::SimpleMath::Vector3 b, DirectX::SimpleMath::Vector3 c)
		{
			// 三角形平面と重心座標判定用の辺平面を構築する。
			DirectX::SimpleMath::Vector3 n = (c - a).Cross(b - a);
			p = DirectX::SimpleMath::Plane(a, n);
			DirectX::SimpleMath::Plane pp = DirectX::SimpleMath::Plane(b, n);
			edgePlaneBC = DirectX::SimpleMath::Plane(b, n.Cross(b - c));
			edgePlaneCA = DirectX::SimpleMath::Plane(c, n.Cross(c - a));
			p.Normalize(); edgePlaneBC.Normalize(); edgePlaneCA.Normalize();
			// 辺平面が重心座標の重みを直接返すよう係数を正規化する。
			float bc_scale = 1.0f / (a.Dot(edgePlaneBC.Normal()) + edgePlaneBC.D());
			float ca_scale = 1.0f / (b.Dot(edgePlaneCA.Normal()) + edgePlaneCA.D());
			edgePlaneBC.x *= bc_scale; edgePlaneBC.y *= bc_scale; edgePlaneBC.z *= bc_scale; edgePlaneBC.w *= bc_scale;
			edgePlaneCA.x *= ca_scale; edgePlaneCA.y *= ca_scale; edgePlaneCA.z *= ca_scale; edgePlaneCA.w *= ca_scale;
		}
	};
	// 浮動小数点の誤差で当たりぬけするので少し余裕をもつ
	#define EPSILON 1.0e-06F
// 線分と三角形の交差判定。
	static bool IntersectSegmentTriangle(DirectX::SimpleMath::Vector3 p, DirectX::SimpleMath::Vector3 q, Triangle tri, DirectX::SimpleMath::Vector3* s)
	{
		float distp = p.Dot(tri.p.Normal()) + tri.p.D();
		if (distp < 0.0f) return false;
		float distq = q.Dot(tri.p.Normal()) + tri.p.D();
		if (distq >= 0.0f) return false;
		float denom = distp - distq;
		float t = distp / denom;
		*s = p + t * (q - p);
		float u = s->Dot(tri.edgePlaneBC.Normal()) + tri.edgePlaneBC.D();
		if (fabsf(u) < EPSILON) u = 0.0f;
		if (u < 0.0f || u > 1.0f) return false;
		float v = s->Dot(tri.edgePlaneCA.Normal()) + tri.edgePlaneCA.D();
		if (fabsf(v) < EPSILON) v = 0.0f;
		if (v < 0.0f) return false;
		float w = 1.0f - u - v;
		if (fabsf(w) < EPSILON) w = 0.0f;
		if (w < 0.0f) return false;
		return true;
	}
};

