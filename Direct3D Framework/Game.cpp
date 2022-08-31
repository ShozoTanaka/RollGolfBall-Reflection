//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

// �R���X�g���N�^
Game::Game() noexcept(false)
    :
    m_hWnd(0),                                                                     // �E�B���h�E�n���h��
    m_deviceResources(nullptr),                                          // �f�o�C�X���\�[�X
    m_timer{},                                                                       // �^�C�}�[
    m_mouse(nullptr),                                                          // �}�E�X
    m_keyboard(nullptr),                                                      // �L�[�{�[�h
    m_keyboardTracker(nullptr),                                          // �L�[�{�[�h�g���b�J�[
    m_camera(nullptr),                                                        // �J����
    m_eyePosition(DirectX::SimpleMath::Vector3::Zero),  // �J�����ʒu
    m_graphScene{}                                                           // GraphScene�N���X
{
    // �f�o�C�X���\�[�X�𐶐�����
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // �f�o�C�X�ʒm��o�^����
    m_deviceResources->RegisterDeviceNotify(this);
}

// ����������
void Game::Initialize(HWND hWnd, int width, int height)
{
    // �E�B���h�E�n���h����ݒ肷��
    m_hWnd = hWnd;
    // �E�B���h�E����ݒ肷��
    m_deviceResources->SetWindow(hWnd, width, height);
    // �f�o�C�X���\�[�X�𐶐�����
    m_deviceResources->CreateDeviceResources();
    // �f�o�C�X�Ɉˑ��������\�[�X�𐶐�����
    CreateDeviceDependentResources();
    // �E�B���h�E�T�C�Y�Ɉˑ��������\�[�X�𐶐�����
    m_deviceResources->CreateWindowSizeDependentResources();
    // �E�B���h�E�T�C�Y�ɉ��������\�[�X�𐶐�����
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
    // �}�E�X�̃C���X�^���X�𐶐�����
    m_mouse = std::make_unique<DirectX::Mouse>();
    // �E�B���h�E�T�C�Y��ݒ肷��
    m_mouse->SetWindow(hWnd);
    // �L�[�{�[�h�̃C���X�^���X�𐶐�����
    m_keyboard = std::make_unique<DirectX::Keyboard>();
    // �L�[�{�[�h�g���b�J�[�̃C���X�^���X�𐶐�����
    m_keyboardTracker = std::make_unique<DirectX::Keyboard::KeyboardStateTracker>();
    // ��ʃT�C�Y
    int screenWidth, screenHeight;
    // ��ʃT�C�Y���擾����
    GetDefaultSize(screenWidth, screenHeight);
    // ��ʃT�C�Y��ݒ肷��
    Graphics::GetInstance()->SetScreenSize(screenWidth, screenHeight);
    // �J�����𐶐�����
    m_camera = std::make_unique<Camera>();
    // �ˉe�s��𐶐�����
    DirectX::SimpleMath::Matrix projection =
        DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
            DirectX::XMConvertToRadians(40.0f),
            screenWidth / (float)screenHeight,
            0.1f,
            1000.0f
        );
    // �ˉe�s���ݒ肷��
    Graphics::GetInstance()->SetProjectionMatrix(projection);

    // GraphScene�N���X�̃C���X�^���X�𐶐�����
    m_graphScene = std::make_unique<GraphScene>(this);
    // GraphScene�N���X�̃C���X�^���X������������
    m_graphScene->Initialize();
}

#pragma region Frame Update
// �Q�[�����[�v�����s����
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        // �X�V����
        Update(m_timer);
    });
    // �`�悷��
    Render();
}

// �X�V����
void Game::Update(DX::StepTimer const& timer)
{
    // GraphScene�N���X�̃C���X�^���X���X�V����
    m_graphScene->Update(timer);
    // Camera�N���X�̃C���X�^���X���X�V����
    m_camera->Update();
}
#pragma endregion

#pragma region Frame Render
// �V�[����`�悷��
void Game::Render()
{
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }
    // �o�b�N�o�b�t�@���N���A����
    Clear();
    // �p�t�H�[�}���X�J�n�C�x���g
    m_deviceResources->PIXBeginEvent(L"Render");
    // GraphScene��`�悷��
    m_graphScene->Render();
    // �p�t�H�[�}���X�I���C�x���g
    m_deviceResources->PIXEndEvent();
    // �V�K�t���[����`�悷��
    m_deviceResources->Present();
}

// �o�b�N�o�b�t�@���N���A����
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // �r���[���N���A����
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, DirectX::Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // �r���[�|�[�g��ݒ肷��
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// ���b�Z�[�W�n���h��
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    // TODO: �f�o�C�X�Ɉˑ������I�u�W�F�N�g������������(�E�B���h�E�T�C�Y����Ɨ�����)
    int width, height;
    // �E�B���h�E�̊���T�C�Y���擾����
    GetDefaultSize(width, height);
    // DirectX������������
    Graphics::GetInstance()->Initialize(m_deviceResources.get(), width, height);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // �o�̓T�C�Y���擾����
    auto m_size = m_deviceResources->GetOutputSize();
    // �A�X�y�N�g�����v�Z����
    float aspectRatio = float(m_size.right) / float(m_size.bottom);
    // ����p��ݒ肷��
    float fovAngleY = 45.0f * DirectX::XM_PI / 180.0f;

    // �A�v�����c�����܂��̓X�i�b�v�r���[�̂Ƃ��ɕύX����
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }
    // �ˉe�s��𐶐�����
    DirectX::SimpleMath::Matrix projection = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView
    (
        fovAngleY,
        aspectRatio,
        0.1f,
        1000.0f
    );
    // �ˉe�s���ݒ肷��
    Graphics::GetInstance()->SetProjectionMatrix(projection);

    if (m_graphScene)
    {
        // �A�[�N�{�[���̃E�B���h�E�T�C�Y��ݒ肷��
        m_graphScene->SetWindow(m_size.right, m_size.bottom);
    }
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
