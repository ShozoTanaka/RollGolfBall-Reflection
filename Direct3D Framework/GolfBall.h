#pragma once
#ifndef GOLF_BALL_DEFINED
#define GOLF_BALL_DEFINED
#include "Geometry.h"
#include "StepTimer.h"

class GraphScene;

class GolfBall
{
public:
	// 位置を取得する
	DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
	// 位置を設定する
	void SetPosition(const DirectX::SimpleMath::Vector3& position) { m_position = position; }
	// 速度を取得する
	DirectX::SimpleMath::Vector3 GetVelocity() const { return m_velocity; }
	// 速度を設定する
	void SetVelocity(const DirectX::SimpleMath::Vector3& velocity) { m_velocity = velocity; }
	// 光線と平面の交差点を取得する
	DirectX::SimpleMath::Vector3 GetIntersectionPoint() { return m_intersectionPoint; }
	// 交差点までの距離を取得する
	float GetDistanceToIntersection() { return m_distanceToIntersection; 	}

public:
	// コンストラクタ
	GolfBall(DirectX::Model* model);
	// コンストラクタ
	GolfBall(GraphScene* graphScene);
	// デストラクタ
	~GolfBall();
	// 初期化する
	void Initialize();
	// 更新する
	void Update(const float& elapsedTime);
	// 描画する
	void Render();
	// 後処理を行う
	void Finalize();

	// ゴルフボールを転がす
	void Roll(const DirectX::SimpleMath::Vector3& direction, const float& force);
	// ゴルフボールと壁の衝突判定をおこなう
	bool DetectCollisionToWall();
	// ゴルフボールの位置を予測する
	DirectX::SimpleMath::Vector3 PredictFuturePosition(const float& time) const;
	// ゴルフボールの移動にかかる時間を計算する
	float CalculateTimeToMoveDistance(const DirectX::SimpleMath::Vector3& from, const DirectX::SimpleMath::Vector3& to, const float& force) const;
	// 将来の位置を描画する
	void DrawFuturePosition(const float& time);

public:
	// 摩擦係数
	static const float FRICTION;
	// 質量
	static const float MASS;
	// 半径
	static const float RADIUS;
	// 最大移動速度
	static const float MAX_SPEED;
	// 交差判定距離
	static const  float INTERSECT_JUDGEMENT_DISTANCE;

private:
	// DirectXグラフィックス
	Graphics* m_graphics;
	// グラフシーン
	GraphScene* m_graphScene;
	// ゴルフボールモデル
	DirectX::Model* m_model;
	// ゴルフボールの回転クォータニオン
	DirectX::SimpleMath::Quaternion m_quaternion;
	// ゴルフボールの位置
	DirectX::SimpleMath::Vector3 m_position;
	// ゴルフボールの速度
	DirectX::SimpleMath::Vector3 m_velocity;
	// ゴルフボールの方向
	DirectX::SimpleMath::Vector3 m_heading;
	// ゴルフボールの質量
	float m_mass;
	// ゴルフボールの半径
	float m_radius;
	// 光線と平面の交差点
	DirectX::SimpleMath::Vector3 m_intersectionPoint;
	// 交差点までの距離
	float m_distanceToIntersection;
};

#endif		// GOLF_BALL_DEFINED
