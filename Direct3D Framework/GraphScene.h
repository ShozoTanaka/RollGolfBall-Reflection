#pragma once
#ifndef GRAPH_SCENE_DEFINED
#define GRAPH_SCENE_DEFINED
#include "DeviceResources.h"
#include "ArcBall.h"
#include "Game.h"
#include "GolfBall.h"
                                        
class Game;

class GraphScene
{                                   
public:
	// �J�n�ʒu
	const DirectX::SimpleMath::Vector3 START_POSITION = DirectX::SimpleMath::Vector3(0.0f, 0.0, 0.0f);

public:
	// ��(�E��, �E��, ����, ����)
	static const DirectX::SimpleMath::Vector3 WALL[4];

	// �ǖ@���x�N�g�����擾����
	const DirectX::SimpleMath::Vector3 GetWallNormalVector()
	{
		using namespace DirectX::SimpleMath;

		Vector3 wallVector = WALL[3] - WALL[0];
		Vector3 wallVector1 = WALL[1] - WALL[0];
		// �@�����擾����
		Vector3 wallNormal = wallVector.Cross(wallVector1);
		// ���K������
		wallNormal.Normalize();
		// �@���x�N�g����Ԃ�
		return wallNormal;
	}

	// �S���t���f�����擾����
	DirectX::Model* const GetGolfBallModel()
	{
		return m_golfBallModel.get();
	}

public:
	// �R���X�g���N�^
	GraphScene(Game* game);
	// �f�X�g���N�^
	~GraphScene();
	// ����������
	void Initialize();
	// �X�V����
	void Update(const DX::StepTimer& timer);
	// �`�悷��
	void Render();
	// �㏈�����s��
	void Finalize();
	// �A�[�N�{�[���̃E�B���h�E�T�C�Y��ݒ肷��
	void SetWindow(const int& width, const int& height);

private:
	// ����\������
	void DrawInfo();
	// �{�[����`�悷��
	void DrawBall();
	// �R������x�N�g����`�悷��
	void DrawRollDirection();
	// �ǂ�`�悷��
	void DrawWall();
	// �A�[�N�{�[�����g�p���ăJ�������R���g���[������
	void ControlCamera(const DX::StepTimer& timer);
	// �O���b�h��`�悷��
	void DrawGrid(
		const DirectX::FXMVECTOR& xAxis, 
		const DirectX::FXMVECTOR& yAxis, 
		const DirectX::FXMVECTOR& origin, 
		const size_t& xdivs, 
		const size_t& ydivs, 
		const DirectX::GXMVECTOR& m_color
	);
	// �v���W�F�N�V�����𐶐�����
	void CreateProjection();

private:
	// Game�N���X�̃C���X�^���X�ւ̃|�C���^
	Game* m_game;
	// DirectX Graphics�N���X�̃C���X�^���X�ւ̃|�C���^
	Graphics* m_graphics;
	// Device�N���X�̃C���X�^���X�ւ̃|�C���^
	ID3D11Device* m_device;
	// DeviceContext�N���X�̃C���X�^���X�ւ̃|�C���^
	ID3D11DeviceContext* m_context;
	// �}�E�X�J�[�\���ʒu
	DirectX::SimpleMath::Vector2 m_mouseCursorPosition;
	// �L�[�{�[�h�X�e�[�g
	DirectX::Keyboard::State m_keyboardState;
	// �}�E�X�X�e�[�g
	DirectX::Mouse::Mouse::State m_mouseState;
	// �}�E�X�X�e�[�g�g���b�J�[
	DirectX::Mouse::ButtonStateTracker m_mouseStateTracker;

	// ���[���h
	DirectX::SimpleMath::Matrix m_worldMatrix;
	// �r���[
	DirectX::SimpleMath::Matrix m_viewMatrix;
	// �v���W�F�N�V����
	DirectX::SimpleMath::Matrix m_projectionMatrix;
	// �J������]
	DirectX::SimpleMath::Quaternion m_cameraRotation;
	// �J�����ʒu
	DirectX::SimpleMath::Vector3 m_cameraPosition;
	// �J�����t�H�[�J�X
	DirectX::SimpleMath::Vector3 m_cameraFocus;
	// �p�x
	float m_rotaionAngle;
	// �����_���王�_�܂ł̋���
	float m_distance;
	// �Y�[��
	float m_zoom;
	// �t�B�[���h�I�u�r���[
	float m_fov;
	// �j�A�N���b�v
	float m_nearPlane;
	// �t�@�[�N���b�v
	float m_farPlane;
	// �X�P�[��
	float m_scale;
	// �A�[�N�{�[��
	ArcBall	 m_ballCamera;
	// �p�x
	float m_angle;

	// �S���t�{�[���̃��f��
	std::unique_ptr<DirectX::Model> m_golfBallModel;
	// �S���t�{�[��
	std::unique_ptr<GolfBall> m_golfBall;

	// ���[���A���O��
	float m_rollAngleRL;
	// ���[���t�H�[�X
	float m_rollForce;
};

#endif		// GRAPH_SCENE_DEFINED
