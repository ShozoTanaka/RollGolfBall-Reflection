#pragma once
#include "SimpleMath.h"

// Vector3をVector2に変換する
inline DirectX::SimpleMath::Vector2 Vector3ToVector2(const DirectX::SimpleMath::Vector3& vector3)
{
	return DirectX::SimpleMath::Vector2(vector3.x, vector3.z);
}

// 2次元の内積を計算する
inline float Dot2D(const DirectX::SimpleMath::Vector2& a, const DirectX::SimpleMath::Vector2& b)
{
	return a.x * b.x + b.y * a.y;
}

// 2次元の外積を計算する
inline float Cross2D(const DirectX::SimpleMath::Vector2& a, const DirectX::SimpleMath::Vector2& b)
{
	return a.x * b.y - a.y * b.x;
}

// ベクトルを正規化する
inline DirectX::SimpleMath::Vector2 Normalize(const DirectX::SimpleMath::Vector2& v1)
{
	// ベクトルを正規化する
	return v1 * (1.0f / v1.Length());
}
