#include "pch.h"
#include "GolfBall.h"
#include "Common.h"
#include "GraphScene.h"

class GraphScene;
// 重力加速度 m/s^2
const float GolfBall::GRAVITATION_ACCELERATION = -9.80665f;
// 摩擦係数
const float GolfBall::FRICTION_COEFFICIENT = 0.2f;
// 質量 kg
const float GolfBall::MASS = 0.04593f;
// 半径 m
const float GolfBall::RADIUS = 0.02133f;
// 最大移動速度
const float GolfBall::MAX_SPEED = 3.0f;
// 交差判定距離
const float GolfBall::INTERSECT_JUDGEMENT_DISTANCE = 40.0f;

// コンストラクタ
GolfBall::GolfBall(GraphScene* graphScene)
	:
	m_graphScene(graphScene),													// グラフシーン
	m_quaternion{},																		// クォータニオン
	m_position(DirectX::SimpleMath::Vector3::Zero),					// 位置
	m_velocity(DirectX::SimpleMath::Vector3::Zero),					// 速度
	m_acceralation(DirectX::SimpleMath::Vector3::Zero),			// 加速度
	m_heading(DirectX::SimpleMath::Vector3::Zero),					// 向き
	m_intersectionPoint(DirectX::SimpleMath::Vector3::Zero),	// 光線と平面の交差点
	m_distanceToIntersection(0.0f),												// 交差点までの距離
	m_totalSeconds(0.0f),																// 合計秒数
	m_initialVelocity(0.0f),																// 初期速度
	m_mass(MASS),																		// 質量:45.93g
	m_radius(RADIUS)																	// 半径:2.13cm
{
	// DirectXグラフィックスを取得する
	m_graphics = Graphics::GetInstance();
	// ゴルフボールモデルを取得する
	m_model = graphScene->GetGolfBallModel();
}

// デストラクタ
GolfBall::~GolfBall()
{
}

// 初期化する
void GolfBall::Initialize()
{
	using namespace DirectX::SimpleMath;
	// 速度を初期化する
	m_velocity = Vector3::Zero;
	// 加速度を初期化する
	// m_acceralation = GRAVITATION_ACCELERATION;
	// 総秒数を初期化する
	m_totalSeconds = 0.0f;
	//  光線と平面の交差点を初期化する
	m_intersectionPoint = DirectX::SimpleMath::Vector3::Zero;
	// 交差点までの距離を初期化する
	m_distanceToIntersection = 0.0f;
}

// 更新する
void GolfBall::Update(const float& elapsedTime)
{
	using namespace DirectX::SimpleMath;
	UNREFERENCED_PARAMETER(elapsedTime);

	// 重力加速度が設定されていない場合は更新しない
	if (m_acceralation.x == 0.0f && m_acceralation.z == 0.0f)
		return;
	wchar_t buffer[128];
	//swprintf(buffer, L"time: %6.4f\t position: (%6.3f, %6.3f, %6.3f)  velocity: (%6.3f, %6.3f, %6.3f)\n",
	swprintf(buffer, L"%6.4f\t %6.3f\t %6.3f\t %6.3f\t %6.3f\t %6.3f\t %6.3f\n",
		m_totalSeconds,
		m_position.x + abs(GraphScene::HOME_POSITION.x),
		m_position.y,
		m_position.z,
		m_velocity.x,
		m_velocity.y,
		m_velocity.z
	);
	OutputDebugString(buffer);

	// 総秒数を計算する
	m_totalSeconds += elapsedTime;
	
	// 速度を計算する
	//m_velocity += m_velocity * GolfBall::FRICTION_COEFFICIENT * m_acceralation * elapsedTime;
	m_velocity += 0.714f * m_velocity * GolfBall::FRICTION_COEFFICIENT * m_acceralation * elapsedTime;
	// 向きを設定する
	m_heading = m_velocity;
	// 位置を更新する
	m_position += m_velocity * elapsedTime;

	if (m_velocity.Length() < 0.01f)
	{
		m_acceralation = Vector3::Zero;
		Initialize();
	}
	// ゴルフボールの回転軸を設定する
	Vector3 axis = Vector3(m_heading.x, 0.0f, -m_heading.z);
	// ゴルフボールが移動している場合
	if (m_velocity.Length())
	{
		// クォータニオンを生成する
		m_quaternion *= Quaternion::CreateFromAxisAngle(axis, m_velocity.Length() / m_radius);
	}
	
	// ゴルフボールと壁の衝突判定をおこなう
	DetectCollisionToWall();
}

// 描画する
void GolfBall::Render()
{
	using namespace DirectX::SimpleMath;

	// スケール行列を生成する
	Matrix scaleMatrix = Matrix::CreateScale(0.5f, 0.5f, 0.5f);
	// 移動行列を生成する
	Matrix translationMatrix = Matrix::CreateTranslation(m_position.x, m_position.y + m_radius * 44.0f, m_position.z);
	// 回転クォータニオンから回転行列を生成する
	Matrix rotationMatrix = Matrix::CreateFromQuaternion(m_quaternion);
	// ワールド行列を計算する
	Matrix worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	// プリミティブ描画を開始する
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
	// ゴルフボールを描画する
	m_graphics->DrawModel(m_model, worldMatrix);
	// プリミティブ描画を終了する
	m_graphics->DrawPrimitiveEnd();
}

// 後処理を行う
void GolfBall::Finalize()
{
}

// ゴルフボールを転がす
void GolfBall::Roll(const DirectX::SimpleMath::Vector3& direction, const float& impulsiveForce)
{
	using namespace DirectX::SimpleMath;

	// ゴルフボールが転がる方向ベクトルを正規化する
	Vector3 rollDirection = direction;
	// 正規化する
	rollDirection.Normalize();
	// 運動方程式から速度を計算する
	m_velocity = (rollDirection * impulsiveForce) * 0.1f / m_mass;
	// 初速度を計算する
	m_initialVelocity = m_velocity.Length();
}

// ゴルフボールと壁の衝突判定をおこなう
bool GolfBall::DetectCollisionToWall()
{
	using namespace DirectX::SimpleMath;

	// 単位速度ベクトルを計算する
	Vector3 normalVelocity = m_velocity;
	// 速度を正規化する
	normalVelocity.Normalize();
	// 壁の法線を取得する
	Vector3 wallNormalVector = m_graphScene->GetWallNormalVector();

	// ゴルフボールの移動する方向と壁との交差判定を行う(線分と線分の交差判定)
	if (IntersectLines2D(
		Vector3ToVector2(m_position),
		Vector3ToVector2(m_position + ( -wallNormalVector * m_radius) * INTERSECT_JUDGEMENT_DISTANCE),
		Vector3ToVector2(GraphScene::WALL[0]),
		Vector3ToVector2(GraphScene::WALL[3]))
	)
	{
		// ゴルフボールの進行方向と壁との交差点を計算する
		Vector2 intersectionPosition = IntersectPointLines2D(Vector3ToVector2(m_position),
			Vector3ToVector2(m_position + (-wallNormalVector * m_radius) * INTERSECT_JUDGEMENT_DISTANCE),
			Vector3ToVector2(GraphScene::WALL[0]),
			Vector3ToVector2(GraphScene::WALL[3]));
		// 交差点までの距離を計算する
		m_distanceToIntersection = (Vector3ToVector2(m_position) - intersectionPosition).Length();
		// 交差点に到着するまでの時間を計算する
		float timeToIntersection = (m_distanceToIntersection - m_radius) / m_velocity.Length() ;
		// 交差点に到着するまでの時間が指定時間より小さい場合
		if(timeToIntersection < 0.5f)
		{
			// ゴルフボールと壁が向き合っている場合
			if (normalVelocity.Dot(wallNormalVector) < 0.0f)
			{
				// ゴルフボールを反射させる
				m_velocity = Vector3::Reflect(m_velocity, wallNormalVector);
			}
			// 交差している
			return true;
		}
	}
	// 交差していない
	return false;
}

// 将来のゴルフボールの位置を予測する
// 等加速度直線運動の公式： distance = (velocity * time) + (1/2 * acceralation * time^2)
DirectX::SimpleMath::Vector3 GolfBall::PredictFuturePosition(const float& time) const
{
	using namespace DirectX::SimpleMath;

	// 初速の移動ベクトルを計算する
	Vector3 initialVelocity = m_velocity * time;
	// 速度を複製する
	Vector3 velocity = m_velocity;
	// 速度を正規化する
	velocity.Normalize();
	// 減速した分の移動距離を計算する
	float distance = 0.5f * GolfBall::FRICTION_COEFFICIENT * time * time;
	// 位置を返す
	return m_position + initialVelocity + velocity * distance;
}

// 将来のゴルフボールの位置を描画する
void GolfBall::DrawFuturePosition(const float& time)
{
	// プリミティブ描画を開始する
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
	// 10秒後の位置を表示する
	m_graphics->DrawCircle(PredictFuturePosition(time), 1.0f);
	// プリミティブ描画を終了する
	m_graphics->DrawPrimitiveEnd();
}

// ゴルフボールの移動に掛かる時間を計算する
	// 運動方程式: F(force) = M(mass) * A(accelaration)
float GolfBall::CalculateTimeToMoveDistance(const DirectX::SimpleMath::Vector3& from, const DirectX::SimpleMath::Vector3& to, const float& force) const
{
	// 初速を計算する
	float initialVelocity = force / m_mass;
	// ゴルフボールまでの距離を計算する
	float distanceToGolfBall = (from - to).Length();
	// ゴルフボールの位置に到達した時の速度を計算する
	float velocity = sqrtf(initialVelocity * initialVelocity + 2.0f * distanceToGolfBall * GolfBall::FRICTION_COEFFICIENT);
	// ゴルフボールまでの到達時間を返す
	return (velocity - initialVelocity) / GolfBall::FRICTION_COEFFICIENT;
}
