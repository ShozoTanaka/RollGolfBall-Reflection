//
// Game.h
//

#pragma once
#ifndef GAME_DEFINED
#define GAME_DEFINED

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Camera.h"
#include "GraphScene.h"

class GraphScene;

class Game final : public DX::IDeviceNotify
{
    HWND GetWindowHandle() { return m_hWnd; }

public:
    // �L�[�{�[�h���擾����
    DirectX::Keyboard* GetKeyboard() const { return m_keyboard.get(); }
    // �L�[�{�[�h�X�e�[�g�g���b�J���擾����
    DirectX::Keyboard::KeyboardStateTracker& GetKeyboardTracker() const { return *m_keyboardTracker.get(); }
    // �}�E�X���擾����
    DirectX::Mouse* GetMouse() const { return m_mouse.get(); }
    // �J�������擾����
    Camera* GetCamera() const { return m_camera.get(); }
public:
    // �R���X�g���N�^
    Game() noexcept(false);
    // �f�X�g���N�^
    ~Game() = default;
    Game(Game&&) = default;
    Game& operator= (Game&&) = default;
    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // ����������
    void Initialize(HWND hWnd, int width, int height);

    // �Q�[�����[�v
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // ���b�Z�[�W�n���h��
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // �v���p�e�B
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:
    // �X�V����
    void Update(DX::StepTimer const& timer);
    // �`�悷��
    void Render();
    // �o�b�N�o�b�t�@���N���A����
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

private:
    // �E�B���h�E�n���h��
    HWND m_hWnd;
    // �f�o�C�X���\�[�X
    std::unique_ptr<DX::DeviceResources> m_deviceResources;
    // �^�C�}�[
    DX::StepTimer m_timer;
    // �}�E�X
    std::unique_ptr<DirectX::Mouse> m_mouse;
    // �L�[�{�[�h
    std::unique_ptr<DirectX::Keyboard> m_keyboard;
    // �L�[�{�[�h�g���b�J�[
    std::unique_ptr<DirectX::Keyboard::KeyboardStateTracker> m_keyboardTracker;
    // �J����
    std::unique_ptr<Camera> m_camera;
    // �J�����̈ʒu
    DirectX::SimpleMath::Vector3 m_eyePosition;

    // GraphScene
    std::unique_ptr<GraphScene> m_graphScene;
};

#endif      // GAME_DEFINED