#include "pch.h"
#include "GolfBall.h"
#include "Common.h"
#include "GraphScene.h"

class GraphScene;

// 摩擦係数
const float GolfBall::FRICTION = -0.02f;
// 最大移動速度
const float GolfBall::MAX_SPEED = 3.0f;
// 交差判定距離
const float GolfBall::INTERSECT_JUDGEMENT_DISTANCE = 20.0f;

// コンストラクタ
GolfBall::GolfBall(DirectX::Model* model)
	:
	m_graphScene(nullptr),															// グラフシーン
	m_model(model),																	// モデル
	m_quaternion{},																		// クォータニオン
	m_position(DirectX::SimpleMath::Vector3::Zero),					// 位置
	m_velocity(DirectX::SimpleMath::Vector3::Zero),					// 速度
	m_heading(DirectX::SimpleMath::Vector3::Zero),					// 向き
	m_intersectionPoint(DirectX::SimpleMath::Vector3::Zero),	// 光線と平面の交差点
	m_distanceToIntersection(0.0f),												// 交差点までの距離
	m_mass(0.45f),																		// 質量
	m_radius(2.13f)																		// 半径
{
	// DirectXグラフィックスを取得する
	m_graphics = Graphics::GetInstance();
}

// コンストラクタ
GolfBall::GolfBall(GraphScene* graphScene)
	:
	m_graphScene(graphScene),													// グラフシーン
	m_quaternion{},																		// クォータニオン
	m_position(DirectX::SimpleMath::Vector3::Zero),					// 位置
	m_velocity(DirectX::SimpleMath::Vector3::Zero),					// 速度
	m_heading(DirectX::SimpleMath::Vector3::Zero),					// 向き
	m_intersectionPoint(DirectX::SimpleMath::Vector3::Zero),	// 光線と平面の交差点
	m_distanceToIntersection(0.0f),												// 交差点までの距離
	m_mass(0.45f),																		// 質量
	m_radius(2.13f)																		// 半径
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

	// 摩擦による減速の大きさより速度が速ければ減速させる
	if (m_velocity.Length() > GolfBall::FRICTION)
	{
		// 向きを設定する
		m_heading = m_velocity;
		// 摩擦により減速する
		m_velocity += m_heading * GolfBall::FRICTION;
		// 位置を更新する
		m_position += m_velocity;
		// ゴルフボールの回転軸を設定する
		Vector3 axis = Vector3(m_heading.y, 0.0f, -m_heading.x);
		// ゴルフボールが移動している場合
		if (m_velocity.Length())
		{
			// クォータニオンを生成する
			m_quaternion *= Quaternion::CreateFromAxisAngle(axis, m_velocity.Length() / m_radius);
		}
	}
	// ゴルフボールと壁の衝突判定をおこなう
	DetectCollisionToWall();
}

// 描画する
void GolfBall::Render()
{
	using namespace DirectX::SimpleMath;

	// スケール行列を生成する
	Matrix scaleMatrix = Matrix::CreateScale(1.0f, 1.0f, 1.0f);
	// 移動行列を生成する
	Matrix translationMatrix = Matrix::CreateTranslation(m_position.x, m_position.y + m_radius, m_position.z);
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
void GolfBall::Roll(const DirectX::SimpleMath::Vector3& direction, const float& force)
{
	using namespace DirectX::SimpleMath;

	// ゴルフボールが転がる方向ベクトルを正規化する
	Vector3 rollDirection = direction;
	// 正規化する
	rollDirection.Normalize();
	// 運動方程式から速度を計算する
	m_velocity = rollDirection * force / m_mass;
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
		// 光線を宣言する
		Ray ray(m_position, normalVelocity);
		// 平面を宣言する
		Plane plane(wallNormalVector);
		// 光線と平面の交差点を計算する
		IntersectRayPlane(ray, plane, &m_intersectionPoint);
		// 光線の方向を取得する
		Vector3 direction = ray.direction;
		// 光線と平面が交差しているかどうかを調べ、交差している場合は交差点までの距離を計算する
		if (ray.Intersects(GraphScene::WALL[0], GraphScene::WALL[1], GraphScene::WALL[3], m_distanceToIntersection))
		{
			// 交差点に到着するまでの時間を計算する
			float timeToIntersection = (m_distanceToIntersection - m_radius) / m_velocity.Length() ;
			// 交差点に到着するまでの時間が0より小さい場合
			if(timeToIntersection < 1.0f)
			{
				// ゴルフボールと壁が向き合っている場合
				if (normalVelocity.Dot(wallNormalVector) < 0)
				{
					// ゴルフボールを反射させる
					m_velocity = Vector3::Reflect(m_velocity, wallNormalVector);
				}
				// 交差している
				return true;
			}
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
	float distance = 0.5f * GolfBall::FRICTION * time * time;
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
	float velocity = sqrtf(initialVelocity * initialVelocity + 2.0f * distanceToGolfBall * GolfBall::FRICTION);
	// ゴルフボールまでの到達時間を返す
	return (velocity - initialVelocity) / GolfBall::FRICTION;
}
