#include "pch.h"
#include <memory>
#include <utility>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include "GraphScene.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Model.h"
#include "Game.h"
#include "Common.h"
#include "SpriteString2D.h"
#include "Common.h"
#include "Geometry.h"

// ��(�E��, �E��, ����, ����)
const DirectX::SimpleMath::Vector3 GraphScene::WALL[4] =
{
	DirectX::SimpleMath::Vector3(80.0f,   0.0f, -60.0f),
	DirectX::SimpleMath::Vector3(80.0f, 20.0f, -60.0f),
	DirectX::SimpleMath::Vector3(80.0f, 20.0f,  60.0f),
	DirectX::SimpleMath::Vector3(80.0f,   0.0f,  60.0f)
};

// �R���X�g���N�^
GraphScene::GraphScene(Game* game)
	:
	m_game(game),									// Game�N���X
	m_graphics(nullptr),							// DirectXGraphics�N���X
	m_device(nullptr),								// Device�N���X
	m_context(nullptr),								// DeviceContext�N���X
	m_keyboardState{},								// �L�[�{�[�h�X�e�[�g
	m_mouseCursorPosition{},					// �}�E�X�J�[�\���ʒu
	m_mouseState{},									// �}�E�X�X�e�[�g
	m_mouseStateTracker{},						// �}�E�X�X�e�[�g�g���b�J�[
	m_worldMatrix{},									// ���[���h
	m_viewMatrix{},									// �r���[
	m_projectionMatrix{},							// �v���W�F�N�V����
	m_cameraRotation{},							// �J������]
	m_cameraPosition{},							// �J�����ʒu
	m_cameraFocus{},								// �J�����t�H�[�J�X
	m_rotaionAngle(0.0f),							// �p�x
	m_distance(10.0f),								// �����_���王�_�܂ł̋���
	m_zoom(1.0f),										// �Y�[��
	m_fov(DirectX::XM_PI / 4.0f),				// �t�B�[���h�I�u�r���[
	m_nearPlane(0.1f),								// �j�A�N���b�v
	m_farPlane(0.0f),								// �t�@�[�N���b�v
	m_scale(1.0f),										// �X�P�[��
	m_golfBallModel(nullptr),					// �S���t�{�[�����f��
	m_golfBall(nullptr),								// �S���t�{�[��
	m_rollAngle(0.0f),								// �p�x
	m_rollForce(30.0f)								// �]������
{
	// DirectX Graphics�N���X�̃C���X�^���X���擾����
	m_graphics = Graphics::GetInstance();
	// �f�o�C�X���擾����
	m_device = Graphics::GetInstance()->GetDeviceResources()->GetD3DDevice();
	// �f�o�C�X�R���e�L�X�g���擾����
	m_context = Graphics::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

// �f�X�g���N�^
GraphScene::~GraphScene()
{
	// �㏈�����s��
	Finalize();
}

// ����������
void GraphScene::Initialize()
{ 
	// �N�H�[�^�j�I���J������]������������
	m_cameraRotation.CreateFromYawPitchRoll(DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f));
	// CMO�`���̃S���t�{�[���̃��f�������[�h����
	m_golfBallModel = DirectX::Model::CreateFromCMO(m_device, L"resources\\cmo\\GolfBall.cmo", *m_graphics->GetFX());
	// GolfBall�N���X�̃C���X�^���X�𐶐�����
	m_golfBall = std::make_unique<GolfBall>(this);
}

// �X�V����
void GraphScene::Update(const DX::StepTimer& timer)
{
	using namespace DirectX::SimpleMath;

	// �o�ߎ��Ԃ��擾����
	float elapsedTime = float(timer.GetElapsedSeconds());
	// �L�[�{�[�h�̏�Ԃ��擾����
	m_keyboardState = m_game->GetKeyboard()->GetState();
	// �L�[�{�[�h�̏�Ԃ��X�V����
	m_game->GetKeyboardTracker().Update(m_keyboardState);

	// �}�E�X�̏�Ԃ��擾����
	m_mouseState = m_game->GetMouse()->GetState();
	// �}�E�X�g���b�J�[���X�V����
	m_mouseStateTracker.Update(m_mouseState);

	// ���_�x�N�g�����擾����
	auto eyePosition = m_game->GetCamera()->GetEyePosition();
	// ���_�ƒ����_�̋������v�Z����
	m_distance = eyePosition.Length();

	// [Shift]+[��]�L�[�ŃS���t�{�[���̓]����������ς���
	if (m_keyboardState.Left && m_keyboardState.LeftControl)
		m_rollAngle -= 1.0f;
	// [Shift]+[��]�L�[�ŃS���t�{�[���̓]����������ς���
	if (m_keyboardState.Right && m_keyboardState.LeftControl)
		m_rollAngle += 1.0f;

	// [Shift]+[��]�L�[�ŃS���t�{�[���̓]�����͂�ς���
	if (m_keyboardState.Up && m_keyboardState.LeftShift)
		m_rollForce += 0.1f;
	// [Shift][��]�L�[�ŃS���t�{�[���̓]�����͂�ς���
	if (m_keyboardState.Down && m_keyboardState.LeftShift)
		m_rollForce -= 0.1f;


	// [Space]�L�[�ŃS���t�{�[����]����
	if (m_game->GetKeyboardTracker().IsKeyPressed(DirectX::Keyboard::Space))
	{
		// �S���t�{�[���̏����ʒu��ݒ肷��
		m_golfBall->SetPosition(START_POSITION);
		// ���x������������
		m_golfBall->SetVelocity(Vector3::Zero);
		// �S���t��]����������ݒ肷��
		Vector3 direction(1.0f, 0.0f, 0.0f);
		// ��]�s��𐶐�����
		Matrix rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rollAngle));
		// ��]��̌������v�Z����
		direction = Vector3::Transform(direction, rotationMatrix);
		// �S���t�{�[��������������
		m_golfBall->Initialize();
		// �S���t�{�[����]����
		m_golfBall->Roll(direction, m_rollForce);
	}
	// �S���t�{�[�����X�V����
	m_golfBall->Update(elapsedTime);

	// ���ʂ�����������
	Plane plane(0.0f, 1.0f, 0.0f, 0.0f);
	// �}�E�X�J�[�\���̃X�N���[���ʒu���擾����
	m_mouseCursorPosition = Vector2(roundf((float)m_mouseState.x), roundf((float)m_mouseState.y));
	// �J�������R���g���[������
	ControlCamera(timer);
}

// �`�悷��
void GraphScene::Render()
{
	using namespace DirectX::SimpleMath;
	// x��
	const DirectX::XMVECTORF32 xaxis = { 100.f, 0.0f, 0.0f };
	// y��
	const DirectX::XMVECTORF32 yaxis = { 0.0f, 0.0f, 100.f };

	// �O���b�h��`�悷��
	DrawGrid(xaxis, yaxis, DirectX::g_XMZero, 10, 10, DirectX::Colors::DarkGray);
	// �ǂ�`�悷��
	DrawWall();
	// �S���t�{�[����`�悷��
	DrawBall();
	// �S���t�{�[����]����������`�悷��
	DrawRollDirection();
	// ����\������
	DrawInfo();
}

// �㏈�����s��
void GraphScene::Finalize()
{
	// �S���t�{�[���̃��f�������Z�b�g����
	m_golfBallModel.reset();
	// �S���t�{�[�������Z�b�g����
	m_golfBall.reset();
}

// �S���t�{�[����`�悷��
void GraphScene::DrawBall()
{
	// �S���t�{�[���̏����̈ʒu��`�悷��
	 m_golfBall->DrawFuturePosition(0.5f);
	// �S���t�{�[����`�悷��
	m_golfBall->Render();
}

// �S���t�{�[����]����������`�悷��
void GraphScene::DrawRollDirection()
{
	using namespace DirectX::SimpleMath;

	// �S���t�{�[������~���Ă���ꍇ
	if (m_golfBall->GetVelocity().Length() < 0.05f )
	{
		// �S���t�{�[���̏����ʒu��ݒ肷��
		m_golfBall->SetPosition(START_POSITION);
		//  �S���t�{�[���̑��x������������
		m_golfBall->SetVelocity(Vector3::Zero);
		// �S���t�{�[���̕�����ݒ肷��
		Vector3 direction(1.0f, 0.0f, 0.0f);
		// Y����]���s����]�s��𐶐�����
		Matrix rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rollAngle));
		//  �S���t�{�[����]�����������v�Z����
		direction = Vector3::Transform(direction, rotationMatrix);
		// �`��v���~�e�B�u���J�n����
		m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
		// �S���t�{�[����]����������\���x�N�g����`�悷��
		m_graphics->DrawVector(m_golfBall->GetPosition(), direction * m_rollForce, DirectX::Colors::White);
		// �`��v���~�e�B�u���I������
		m_graphics->DrawPrimitiveEnd();
	}
}

// �ǂ�`�悷��
void GraphScene::DrawWall()
{
	using namespace DirectX::SimpleMath;

	// �`��v���~�e�B�u���J�n����
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
	// �ǂ�`�悷��
	m_graphics->DrawQuad(WALL[0], WALL[1], WALL[2], WALL[3], DirectX::Colors::DarkGray);
	// �@���x�N�g����`�悷��
	// m_graphics->DrawVector(WALL[0], GetWallNormalVector() * 10.0f, DirectX::Colors::White);
	// �`��v���~�e�B�u���I������
	m_graphics->DrawPrimitiveEnd();
}

// ����\������
void GraphScene::DrawInfo()
{
	wchar_t stringBuffer[128];
	// SpriteString2D��錾����
	SpriteString2D spriteString2D;

	// �J�����ʒu������������
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           Camera position: (%6.1f, %6.1f, %6.1f )", 
		m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 0.0f));
	// �J������]�p������������
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           Camera rotation: (%6.1f, %6.1f, %6.1f ), %6.1f )",
		m_cameraRotation.x, m_cameraRotation.y, m_cameraRotation.z, m_cameraRotation.w);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 28.0f));
	// �S���t�{�[���̈ʒu������������
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           GolfBall position: (%6.1f, %6.1f,  %6.1f )",
		m_golfBall->GetPosition().x, m_golfBall->GetPosition().y, m_golfBall->GetPosition().z);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 56.0f));
	// �S���t�{�[���̑��x������������
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           GolfBall velocity: (%6.1f, %6.1f, %6.1f ), %6.1f",
		m_golfBall->GetVelocity().x, m_golfBall->GetVelocity().y, m_golfBall->GetVelocity().z, m_golfBall->GetVelocity().Length());
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 84.0f));
	// �����ƕ��ʂ̌����_������������
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"         Intersection point: (%6.1f, %6.1f, %6.1f )",
		m_golfBall->GetIntersectionPoint().x, m_golfBall->GetIntersectionPoint().y, m_golfBall->GetIntersectionPoint().z);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 112.0f));
	// �����_�܂ł̋���������������
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"Distance to Intersection: (%6.1f )",
		m_golfBall->GetDistanceToIntersection());
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 140.0f));
	// ���ׂĂ̏���`�悷��
	spriteString2D.Render();
}

// �A�[�N�{�[�����g�p���ăJ�������R���g���[������
void GraphScene::ControlCamera(const DX::StepTimer& timer)
{
	using namespace DirectX::SimpleMath;

	// �o�ߎ��Ԃ��擾����
	float elapsedTime = float(timer.GetElapsedSeconds());
	// �X�P�[���Q�C��
	constexpr float SCALE_GAIN = 0.001f;

	// �J�����ړ��s��
	Matrix im;
	m_viewMatrix.Invert(im);
	Vector3 move = Vector3::TransformNormal(move, im);

	// �}�E�X�̈ړ������΃}�E�X�ړ��ł���ꍇ
	if (m_game->GetMouse()->GetState().positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		Vector3 delta = Vector3(-float(m_mouseState.x), float(m_mouseState.y), 0.f) * m_distance;
		delta = Vector3::TransformNormal(delta, im);
		// �J�����t�H�[�J�X�ʒu���v�Z����
		m_cameraFocus += delta * elapsedTime;
	}
	// �}�E�X�̉E�{�^�����h���b�O���Ă���ꍇ
	else if (m_ballCamera.IsDragging())
	{
		// �}�E�X�̈ړ�
		m_ballCamera.OnMove(m_mouseState.x, m_mouseState.y);
		// �{�[���J�����̌��݂̃N�H�[�^�j�I�����擾����
		auto q = m_ballCamera.GetQuaternion();
		// �J������]�̋t�R�H�[�^�j�I�����v�Z����
		q.Inverse(m_cameraRotation);
	}
	else
	{
		// �}�E�X�t�H�C�[������]�������ꍇ�̃Y�[���l���v�Z����
		m_zoom = 1.0f + float(m_mouseState.scrollWheelValue) * SCALE_GAIN;
		// �Y�[���l�𒲐�����
		m_zoom = std::max(m_zoom, 0.01f);
		// �X�N���[���t�H�C�[���l�����Z�b�g����
		m_game->GetMouse()->ResetScrollWheelValue();
	}

	// �h���b�O���łȂ��ꍇ
	if (!m_ballCamera.IsDragging())
	{
		// �}�E�X�̉E�{�^�������������Ă���ꍇ
		if (m_mouseStateTracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
		{
			// ���E��[Ctrl�L�[]�����������Ă���ꍇ
			if (m_keyboardState.LeftControl || m_keyboardState.RightControl)
			{
				// �{�[���J�������J�n����
				m_ballCamera.OnBegin(m_mouseState.x, m_mouseState.y);
			}
		}
	}
	// �}�E�X�̉E�{�^����������Ă���ꍇ
	else if (m_mouseStateTracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
	{
		// �{�[���J�������I������
		m_ballCamera.OnEnd();
	}

	// �J�����̌������X�V����
	Vector3 direction = Vector3::Transform(Vector3::Backward, m_cameraRotation);
	// �J�����ʒu���v�Z����
	m_cameraPosition = m_cameraFocus + (m_distance * m_zoom) * direction;
	// �����x�N�g����ݒ肷��
	m_game->GetCamera()->SetEyePosition(m_cameraPosition);
}

// �O���b�h��`�悷��
void GraphScene::DrawGrid(
	const DirectX::FXMVECTOR& xAxis,
	const DirectX::FXMVECTOR& yAxis,
	const DirectX::FXMVECTOR& origin,
	const size_t& xdivs,
	const size_t& ydivs,
	const DirectX::GXMVECTOR& m_color
)
{
	// �p�t�H�[�}���X�J�n�C�x���g
	m_graphics->GetDeviceResources()->PIXBeginEvent(L"Draw Grid");
	// �v���~�e�B�u�`����J�n����
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());

	for (size_t index = 0; index <= xdivs; ++index)
	{
		float percent = ((float(index) / float(xdivs)) * 2.0f) - 1.0f;
		// �X�P�[�����v�Z����
		DirectX::XMVECTOR scale = DirectX::XMVectorScale(xAxis, percent);
		scale = DirectX::XMVectorAdd(scale, origin);
		// ���_1��ݒ肷��
		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, yAxis), m_color);
		// ���_2��ݒ肷��
		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, yAxis), m_color);
		// ������`�悷��
		m_graphics->GetPrimitiveBatch()->DrawLine(v1, v2);
	}

	for (size_t index = 0; index <= ydivs; index++)
	{
		float percent = ((float(index) / float(ydivs)) * 2.0f) - 1.0f;
		// �X�P�[�����v�Z����
		DirectX::XMVECTOR scale = DirectX::XMVectorScale(yAxis, percent);
		scale = DirectX::XMVectorAdd(scale, origin);
		// ���_1��ݒ肷��
		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, xAxis), m_color);
		// ���_2��ݒ肷��
		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, xAxis), m_color);
		// ������`�悷��
		m_graphics->GetPrimitiveBatch()->DrawLine(v1, v2);
	}
	// �v���~�e�B�u�o�b�`���I������
	m_graphics->DrawPrimitiveEnd();
	// �p�t�H�[�}���X�I���C�x���g
	m_graphics->GetDeviceResources()->PIXEndEvent();
}

// �v���W�F�N�V�����𐶐�����
void GraphScene::CreateProjection()
{
	// �E�B���h�E�T�C�Y���擾����
	auto size = m_graphics->GetDeviceResources()->GetOutputSize();
	// �v���W�F�N�V�����𐶐�����
	m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(m_fov, float(size.right) / float(size.bottom), m_nearPlane, m_farPlane);
}

// �A�[�N�{�[���̂��߂̃E�B���h�E�T�C�Y��ݒ肷��
void GraphScene::SetWindow(const int& width, const int& height)
{
	// �A�[�N�{�[���̃E�B���h�E�T�C�Y��ݒ肷��
	m_ballCamera.SetWindow(width, height);
}
 