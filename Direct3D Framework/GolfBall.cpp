#include "pch.h"
#include "GolfBall.h"
#include "Common.h"
#include "GraphScene.h"

class GraphScene;
// �d�͉����x m/s^2
const float GolfBall::GRAVITATION_ACCELERATION = -9.80665f;
// ���C�W��
const float GolfBall::FRICTION_COEFFICIENT = 0.2f;
// ���� kg
const float GolfBall::MASS = 0.04593f;
// ���a m
const float GolfBall::RADIUS = 0.02133f;
// �ő�ړ����x
const float GolfBall::MAX_SPEED = 3.0f;
// �������苗��
const float GolfBall::INTERSECT_JUDGEMENT_DISTANCE = 40.0f;

// �R���X�g���N�^
GolfBall::GolfBall(GraphScene* graphScene)
	:
	m_graphScene(graphScene),													// �O���t�V�[��
	m_quaternion{},																		// �N�H�[�^�j�I��
	m_position(DirectX::SimpleMath::Vector3::Zero),					// �ʒu
	m_velocity(DirectX::SimpleMath::Vector3::Zero),					// ���x
	m_acceralation(DirectX::SimpleMath::Vector3::Zero),			// �����x
	m_heading(DirectX::SimpleMath::Vector3::Zero),					// ����
	m_intersectionPoint(DirectX::SimpleMath::Vector3::Zero),	// �����ƕ��ʂ̌����_
	m_distanceToIntersection(0.0f),												// �����_�܂ł̋���
	m_totalSeconds(0.0f),																// ���v�b��
	m_initialVelocity(0.0f),																// �������x
	m_mass(MASS),																		// ����:45.93g
	m_radius(RADIUS)																	// ���a:2.13cm
{
	// DirectX�O���t�B�b�N�X���擾����
	m_graphics = Graphics::GetInstance();
	// �S���t�{�[�����f�����擾����
	m_model = graphScene->GetGolfBallModel();
}

// �f�X�g���N�^
GolfBall::~GolfBall()
{
}

// ����������
void GolfBall::Initialize()
{
	using namespace DirectX::SimpleMath;
	// ���x������������
	m_velocity = Vector3::Zero;
	// �����x������������
	// m_acceralation = GRAVITATION_ACCELERATION;
	// ���b��������������
	m_totalSeconds = 0.0f;
	//  �����ƕ��ʂ̌����_������������
	m_intersectionPoint = DirectX::SimpleMath::Vector3::Zero;
	// �����_�܂ł̋���������������
	m_distanceToIntersection = 0.0f;
}

// �X�V����
void GolfBall::Update(const float& elapsedTime)
{
	using namespace DirectX::SimpleMath;
	UNREFERENCED_PARAMETER(elapsedTime);

	// �d�͉����x���ݒ肳��Ă��Ȃ��ꍇ�͍X�V���Ȃ�
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

	// ���b�����v�Z����
	m_totalSeconds += elapsedTime;
	
	// ���x���v�Z����
	//m_velocity += m_velocity * GolfBall::FRICTION_COEFFICIENT * m_acceralation * elapsedTime;
	m_velocity += 0.714f * m_velocity * GolfBall::FRICTION_COEFFICIENT * m_acceralation * elapsedTime;
	// ������ݒ肷��
	m_heading = m_velocity;
	// �ʒu���X�V����
	m_position += m_velocity * elapsedTime;

	if (m_velocity.Length() < 0.01f)
	{
		m_acceralation = Vector3::Zero;
		Initialize();
	}
	// �S���t�{�[���̉�]����ݒ肷��
	Vector3 axis = Vector3(m_heading.x, 0.0f, -m_heading.z);
	// �S���t�{�[�����ړ����Ă���ꍇ
	if (m_velocity.Length())
	{
		// �N�H�[�^�j�I���𐶐�����
		m_quaternion *= Quaternion::CreateFromAxisAngle(axis, m_velocity.Length() / m_radius);
	}
	
	// �S���t�{�[���ƕǂ̏Փ˔���������Ȃ�
	DetectCollisionToWall();
}

// �`�悷��
void GolfBall::Render()
{
	using namespace DirectX::SimpleMath;

	// �X�P�[���s��𐶐�����
	Matrix scaleMatrix = Matrix::CreateScale(0.5f, 0.5f, 0.5f);
	// �ړ��s��𐶐�����
	Matrix translationMatrix = Matrix::CreateTranslation(m_position.x, m_position.y + m_radius * 44.0f, m_position.z);
	// ��]�N�H�[�^�j�I�������]�s��𐶐�����
	Matrix rotationMatrix = Matrix::CreateFromQuaternion(m_quaternion);
	// ���[���h�s����v�Z����
	Matrix worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	// �v���~�e�B�u�`����J�n����
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
	// �S���t�{�[����`�悷��
	m_graphics->DrawModel(m_model, worldMatrix);
	// �v���~�e�B�u�`����I������
	m_graphics->DrawPrimitiveEnd();
}

// �㏈�����s��
void GolfBall::Finalize()
{
}

// �S���t�{�[����]����
void GolfBall::Roll(const DirectX::SimpleMath::Vector3& direction, const float& impulsiveForce)
{
	using namespace DirectX::SimpleMath;

	// �S���t�{�[�����]��������x�N�g���𐳋K������
	Vector3 rollDirection = direction;
	// ���K������
	rollDirection.Normalize();
	// �^�����������瑬�x���v�Z����
	m_velocity = (rollDirection * impulsiveForce) * 0.1f / m_mass;
	// �����x���v�Z����
	m_initialVelocity = m_velocity.Length();
}

// �S���t�{�[���ƕǂ̏Փ˔���������Ȃ�
bool GolfBall::DetectCollisionToWall()
{
	using namespace DirectX::SimpleMath;

	// �P�ʑ��x�x�N�g�����v�Z����
	Vector3 normalVelocity = m_velocity;
	// ���x�𐳋K������
	normalVelocity.Normalize();
	// �ǂ̖@�����擾����
	Vector3 wallNormalVector = m_graphScene->GetWallNormalVector();

	// �S���t�{�[���̈ړ���������ƕǂƂ̌���������s��(�����Ɛ����̌�������)
	if (IntersectLines2D(
		Vector3ToVector2(m_position),
		Vector3ToVector2(m_position + ( -wallNormalVector * m_radius) * INTERSECT_JUDGEMENT_DISTANCE),
		Vector3ToVector2(GraphScene::WALL[0]),
		Vector3ToVector2(GraphScene::WALL[3]))
	)
	{
		// �S���t�{�[���̐i�s�����ƕǂƂ̌����_���v�Z����
		Vector2 intersectionPosition = IntersectPointLines2D(Vector3ToVector2(m_position),
			Vector3ToVector2(m_position + (-wallNormalVector * m_radius) * INTERSECT_JUDGEMENT_DISTANCE),
			Vector3ToVector2(GraphScene::WALL[0]),
			Vector3ToVector2(GraphScene::WALL[3]));
		// �����_�܂ł̋������v�Z����
		m_distanceToIntersection = (Vector3ToVector2(m_position) - intersectionPosition).Length();
		// �����_�ɓ�������܂ł̎��Ԃ��v�Z����
		float timeToIntersection = (m_distanceToIntersection - m_radius) / m_velocity.Length() ;
		// �����_�ɓ�������܂ł̎��Ԃ��w�莞�Ԃ�菬�����ꍇ
		if(timeToIntersection < 0.5f)
		{
			// �S���t�{�[���ƕǂ����������Ă���ꍇ
			if (normalVelocity.Dot(wallNormalVector) < 0.0f)
			{
				// �S���t�{�[���𔽎˂�����
				m_velocity = Vector3::Reflect(m_velocity, wallNormalVector);
			}
			// �������Ă���
			return true;
		}
	}
	// �������Ă��Ȃ�
	return false;
}

// �����̃S���t�{�[���̈ʒu��\������
// �������x�����^���̌����F distance = (velocity * time) + (1/2 * acceralation * time^2)
DirectX::SimpleMath::Vector3 GolfBall::PredictFuturePosition(const float& time) const
{
	using namespace DirectX::SimpleMath;

	// �����̈ړ��x�N�g�����v�Z����
	Vector3 initialVelocity = m_velocity * time;
	// ���x�𕡐�����
	Vector3 velocity = m_velocity;
	// ���x�𐳋K������
	velocity.Normalize();
	// �����������̈ړ��������v�Z����
	float distance = 0.5f * GolfBall::FRICTION_COEFFICIENT * time * time;
	// �ʒu��Ԃ�
	return m_position + initialVelocity + velocity * distance;
}

// �����̃S���t�{�[���̈ʒu��`�悷��
void GolfBall::DrawFuturePosition(const float& time)
{
	// �v���~�e�B�u�`����J�n����
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
	// 10�b��̈ʒu��\������
	m_graphics->DrawCircle(PredictFuturePosition(time), 1.0f);
	// �v���~�e�B�u�`����I������
	m_graphics->DrawPrimitiveEnd();
}

// �S���t�{�[���̈ړ��Ɋ|���鎞�Ԃ��v�Z����
	// �^��������: F(force) = M(mass) * A(accelaration)
float GolfBall::CalculateTimeToMoveDistance(const DirectX::SimpleMath::Vector3& from, const DirectX::SimpleMath::Vector3& to, const float& force) const
{
	// �������v�Z����
	float initialVelocity = force / m_mass;
	// �S���t�{�[���܂ł̋������v�Z����
	float distanceToGolfBall = (from - to).Length();
	// �S���t�{�[���̈ʒu�ɓ��B�������̑��x���v�Z����
	float velocity = sqrtf(initialVelocity * initialVelocity + 2.0f * distanceToGolfBall * GolfBall::FRICTION_COEFFICIENT);
	// �S���t�{�[���܂ł̓��B���Ԃ�Ԃ�
	return (velocity - initialVelocity) / GolfBall::FRICTION_COEFFICIENT;
}
