#pragma once
#include "SimpleMath.h"

// Vector3��Vector2�ɕϊ�����
inline DirectX::SimpleMath::Vector2 Vector3ToVector2(const DirectX::SimpleMath::Vector3& vector3)
{
	return DirectX::SimpleMath::Vector2(vector3.x, vector3.z);
}

// 2�����̓��ς��v�Z����
inline float Dot2D(const DirectX::SimpleMath::Vector2& a, const DirectX::SimpleMath::Vector2& b)
{
	return a.x * b.x + b.y * a.y;
}

// 2�����̊O�ς��v�Z����
inline float Cross2D(const DirectX::SimpleMath::Vector2& a, const DirectX::SimpleMath::Vector2& b)
{
	return a.x * b.y - a.y * b.x;
}

// �x�N�g���𐳋K������
inline DirectX::SimpleMath::Vector2 Normalize(const DirectX::SimpleMath::Vector2& v1)
{
	// �x�N�g���𐳋K������
	return v1 * (1.0f / v1.Length());
}
