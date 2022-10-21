#pragma once
#ifndef GOLF_BALL_DEFINED
#define GOLF_BALL_DEFINED
#include "Geometry.h"
#include "StepTimer.h"

class GraphScene;

class GolfBall
{
public:
	// �ʒu���擾����
	DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
	// �ʒu��ݒ肷��
	void SetPosition(const DirectX::SimpleMath::Vector3& position) { m_position = position; }
	// ���x���擾����
	DirectX::SimpleMath::Vector3 GetVelocity() const { return m_velocity; }
	// ���x��ݒ肷��
	void SetVelocity(const DirectX::SimpleMath::Vector3& velocity) { m_velocity = velocity; }
	// �����ƕ��ʂ̌����_���擾����
	DirectX::SimpleMath::Vector3 GetIntersectionPoint() { return m_intersectionPoint; }
	// �����_�܂ł̋������擾����
	float GetDistanceToIntersection() { return m_distanceToIntersection; 	}

public:
	// �R���X�g���N�^
	GolfBall(DirectX::Model* model);
	// �R���X�g���N�^
	GolfBall(GraphScene* graphScene);
	// �f�X�g���N�^
	~GolfBall();
	// ����������
	void Initialize();
	// �X�V����
	void Update(const float& elapsedTime);
	// �`�悷��
	void Render();
	// �㏈�����s��
	void Finalize();

	// �S���t�{�[����]����
	void Roll(const DirectX::SimpleMath::Vector3& direction, const float& force);
	// �S���t�{�[���ƕǂ̏Փ˔���������Ȃ�
	bool DetectCollisionToWall();
	// �S���t�{�[���̈ʒu��\������
	DirectX::SimpleMath::Vector3 PredictFuturePosition(const float& time) const;
	// �S���t�{�[���̈ړ��ɂ����鎞�Ԃ��v�Z����
	float CalculateTimeToMoveDistance(const DirectX::SimpleMath::Vector3& from, const DirectX::SimpleMath::Vector3& to, const float& force) const;
	// �����̈ʒu��`�悷��
	void DrawFuturePosition(const float& time);

public:
	// ���C�W��
	static const float FRICTION;
	// ����
	static const float MASS;
	// ���a
	static const float RADIUS;
	// �ő�ړ����x
	static const float MAX_SPEED;
	// �������苗��
	static const  float INTERSECT_JUDGEMENT_DISTANCE;

private:
	// DirectX�O���t�B�b�N�X
	Graphics* m_graphics;
	// �O���t�V�[��
	GraphScene* m_graphScene;
	// �S���t�{�[�����f��
	DirectX::Model* m_model;
	// �S���t�{�[���̉�]�N�H�[�^�j�I��
	DirectX::SimpleMath::Quaternion m_quaternion;
	// �S���t�{�[���̈ʒu
	DirectX::SimpleMath::Vector3 m_position;
	// �S���t�{�[���̑��x
	DirectX::SimpleMath::Vector3 m_velocity;
	// �S���t�{�[���̕���
	DirectX::SimpleMath::Vector3 m_heading;
	// �S���t�{�[���̎���
	float m_mass;
	// �S���t�{�[���̔��a
	float m_radius;
	// �����ƕ��ʂ̌����_
	DirectX::SimpleMath::Vector3 m_intersectionPoint;
	// �����_�܂ł̋���
	float m_distanceToIntersection;
};

#endif		// GOLF_BALL_DEFINED
