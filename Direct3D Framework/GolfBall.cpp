#include "pch.h"
#include "GolfBall.h"
#include "Common.h"
#include "GraphScene.h"

class GraphScene;

// ���C�W��
const float GolfBall::FRICTION = -0.02f;
// �ő�ړ����x
const float GolfBall::MAX_SPEED = 3.0f;
// �������苗��
const float GolfBall::INTERSECT_JUDGEMENT_DISTANCE = 20.0f;

// �R���X�g���N�^
GolfBall::GolfBall(DirectX::Model* model)
	:
	m_graphScene(nullptr),															// �O���t�V�[��
	m_model(model),																	// ���f��
	m_quaternion{},																		// �N�H�[�^�j�I��
	m_position(DirectX::SimpleMath::Vector3::Zero),					// �ʒu
	m_velocity(DirectX::SimpleMath::Vector3::Zero),					// ���x
	m_heading(DirectX::SimpleMath::Vector3::Zero),					// ����
	m_intersectionPoint(DirectX::SimpleMath::Vector3::Zero),	// �����ƕ��ʂ̌����_
	m_distanceToIntersection(0.0f),												// �����_�܂ł̋���
	m_mass(0.45f),																		// ����
	m_radius(2.13f)																		// ���a
{
	// DirectX�O���t�B�b�N�X���擾����
	m_graphics = Graphics::GetInstance();
}

// �R���X�g���N�^
GolfBall::GolfBall(GraphScene* graphScene)
	:
	m_graphScene(graphScene),													// �O���t�V�[��
	m_quaternion{},																		// �N�H�[�^�j�I��
	m_position(DirectX::SimpleMath::Vector3::Zero),					// �ʒu
	m_velocity(DirectX::SimpleMath::Vector3::Zero),					// ���x
	m_heading(DirectX::SimpleMath::Vector3::Zero),					// ����
	m_intersectionPoint(DirectX::SimpleMath::Vector3::Zero),	// �����ƕ��ʂ̌����_
	m_distanceToIntersection(0.0f),												// �����_�܂ł̋���
	m_mass(0.45f),																		// ����
	m_radius(2.13f)																		// ���a
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

	// ���C�ɂ�錸���̑傫����葬�x��������Ό���������
	if (m_velocity.Length() > GolfBall::FRICTION)
	{
		// ������ݒ肷��
		m_heading = m_velocity;
		// ���C�ɂ�茸������
		m_velocity += m_heading * GolfBall::FRICTION;
		// �ʒu���X�V����
		m_position += m_velocity;
		// �S���t�{�[���̉�]����ݒ肷��
		Vector3 axis = Vector3(m_heading.y, 0.0f, -m_heading.x);
		// �S���t�{�[�����ړ����Ă���ꍇ
		if (m_velocity.Length())
		{
			// �N�H�[�^�j�I���𐶐�����
			m_quaternion *= Quaternion::CreateFromAxisAngle(axis, m_velocity.Length() / m_radius);
		}
	}
	// �S���t�{�[���ƕǂ̏Փ˔���������Ȃ�
	DetectCollisionToWall();
}

// �`�悷��
void GolfBall::Render()
{
	using namespace DirectX::SimpleMath;

	// �X�P�[���s��𐶐�����
	Matrix scaleMatrix = Matrix::CreateScale(1.0f, 1.0f, 1.0f);
	// �ړ��s��𐶐�����
	Matrix translationMatrix = Matrix::CreateTranslation(m_position.x, m_position.y + m_radius, m_position.z);
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
void GolfBall::Roll(const DirectX::SimpleMath::Vector3& direction, const float& force)
{
	using namespace DirectX::SimpleMath;

	// �S���t�{�[�����]��������x�N�g���𐳋K������
	Vector3 rollDirection = direction;
	// ���K������
	rollDirection.Normalize();
	// �^�����������瑬�x���v�Z����
	m_velocity = rollDirection * force / m_mass;
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
		// ������錾����
		Ray ray(m_position, normalVelocity);
		// ���ʂ�錾����
		Plane plane(wallNormalVector);
		// �����ƕ��ʂ̌����_���v�Z����
		IntersectRayPlane(ray, plane, &m_intersectionPoint);
		// �����̕������擾����
		Vector3 direction = ray.direction;
		// �����ƕ��ʂ��������Ă��邩�ǂ����𒲂ׁA�������Ă���ꍇ�͌����_�܂ł̋������v�Z����
		if (ray.Intersects(GraphScene::WALL[0], GraphScene::WALL[1], GraphScene::WALL[3], m_distanceToIntersection))
		{
			// �����_�ɓ�������܂ł̎��Ԃ��v�Z����
			float timeToIntersection = (m_distanceToIntersection - m_radius) / m_velocity.Length() ;
			// �����_�ɓ�������܂ł̎��Ԃ�0��菬�����ꍇ
			if(timeToIntersection < 1.0f)
			{
				// �S���t�{�[���ƕǂ����������Ă���ꍇ
				if (normalVelocity.Dot(wallNormalVector) < 0)
				{
					// �S���t�{�[���𔽎˂�����
					m_velocity = Vector3::Reflect(m_velocity, wallNormalVector);
				}
				// �������Ă���
				return true;
			}
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
	float distance = 0.5f * GolfBall::FRICTION * time * time;
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
	float velocity = sqrtf(initialVelocity * initialVelocity + 2.0f * distanceToGolfBall * GolfBall::FRICTION);
	// �S���t�{�[���܂ł̓��B���Ԃ�Ԃ�
	return (velocity - initialVelocity) / GolfBall::FRICTION;
}
