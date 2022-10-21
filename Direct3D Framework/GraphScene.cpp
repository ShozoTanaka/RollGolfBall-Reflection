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

// 壁(右下, 右上, 左上, 左下)
const DirectX::SimpleMath::Vector3 GraphScene::WALL[4] =
{
	DirectX::SimpleMath::Vector3(80.0f,   0.0f, -60.0f),
	DirectX::SimpleMath::Vector3(80.0f, 20.0f, -60.0f),
	DirectX::SimpleMath::Vector3(80.0f, 20.0f,  60.0f),
	DirectX::SimpleMath::Vector3(80.0f,   0.0f,  60.0f)
};

// コンストラクタ
GraphScene::GraphScene(Game* game)
	:
	m_game(game),									// Gameクラス
	m_graphics(nullptr),							// DirectXGraphicsクラス
	m_device(nullptr),								// Deviceクラス
	m_context(nullptr),								// DeviceContextクラス
	m_keyboardState{},								// キーボードステート
	m_mouseCursorPosition{},					// マウスカーソル位置
	m_mouseState{},									// マウスステート
	m_mouseStateTracker{},						// マウスステートトラッカー
	m_worldMatrix{},									// ワールド
	m_viewMatrix{},									// ビュー
	m_projectionMatrix{},							// プロジェクション
	m_cameraRotation{},							// カメラ回転
	m_cameraPosition{},							// カメラ位置
	m_cameraFocus{},								// カメラフォーカス
	m_rotaionAngle(0.0f),							// 角度
	m_distance(10.0f),								// 注視点から視点までの距離
	m_zoom(1.0f),										// ズーム
	m_fov(DirectX::XM_PI / 4.0f),				// フィールドオブビュー
	m_nearPlane(0.1f),								// ニアクリップ
	m_farPlane(0.0f),								// ファークリップ
	m_scale(1.0f),										// スケール
	m_golfBallModel(nullptr),					// ゴルフボールモデル
	m_golfBall(nullptr),								// ゴルフボール
	m_rollAngle(0.0f),								// 角度
	m_rollForce(30.0f)								// 転がす力
{
	// DirectX Graphicsクラスのインスタンスを取得する
	m_graphics = Graphics::GetInstance();
	// デバイスを取得する
	m_device = Graphics::GetInstance()->GetDeviceResources()->GetD3DDevice();
	// デバイスコンテキストを取得する
	m_context = Graphics::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

// デストラクタ
GraphScene::~GraphScene()
{
	// 後処理を行う
	Finalize();
}

// 初期化する
void GraphScene::Initialize()
{ 
	// クォータニオンカメラ回転を初期化する
	m_cameraRotation.CreateFromYawPitchRoll(DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f));
	// CMO形式のゴルフボールのモデルをロードする
	m_golfBallModel = DirectX::Model::CreateFromCMO(m_device, L"resources\\cmo\\GolfBall.cmo", *m_graphics->GetFX());
	// GolfBallクラスのインスタンスを生成する
	m_golfBall = std::make_unique<GolfBall>(this);
}

// 更新する
void GraphScene::Update(const DX::StepTimer& timer)
{
	using namespace DirectX::SimpleMath;

	// 経過時間を取得する
	float elapsedTime = float(timer.GetElapsedSeconds());
	// キーボードの状態を取得する
	m_keyboardState = m_game->GetKeyboard()->GetState();
	// キーボードの状態を更新する
	m_game->GetKeyboardTracker().Update(m_keyboardState);

	// マウスの状態を取得する
	m_mouseState = m_game->GetMouse()->GetState();
	// マウストラッカーを更新する
	m_mouseStateTracker.Update(m_mouseState);

	// 視点ベクトルを取得する
	auto eyePosition = m_game->GetCamera()->GetEyePosition();
	// 視点と注視点の距離を計算する
	m_distance = eyePosition.Length();

	// [Shift]+[←]キーでゴルフボールの転がす方向を変える
	if (m_keyboardState.Left && m_keyboardState.LeftControl)
		m_rollAngle -= 1.0f;
	// [Shift]+[→]キーでゴルフボールの転がす方向を変える
	if (m_keyboardState.Right && m_keyboardState.LeftControl)
		m_rollAngle += 1.0f;

	// [Shift]+[↑]キーでゴルフボールの転がす力を変える
	if (m_keyboardState.Up && m_keyboardState.LeftShift)
		m_rollForce += 0.1f;
	// [Shift][↓]キーでゴルフボールの転がす力を変える
	if (m_keyboardState.Down && m_keyboardState.LeftShift)
		m_rollForce -= 0.1f;


	// [Space]キーでゴルフボールを転がす
	if (m_game->GetKeyboardTracker().IsKeyPressed(DirectX::Keyboard::Space))
	{
		// ゴルフボールの初期位置を設定する
		m_golfBall->SetPosition(START_POSITION);
		// 速度を初期化する
		m_golfBall->SetVelocity(Vector3::Zero);
		// ゴルフを転がす方向を設定する
		Vector3 direction(1.0f, 0.0f, 0.0f);
		// 回転行列を生成する
		Matrix rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rollAngle));
		// 回転後の向きを計算する
		direction = Vector3::Transform(direction, rotationMatrix);
		// ゴルフボールを初期化する
		m_golfBall->Initialize();
		// ゴルフボールを転がす
		m_golfBall->Roll(direction, m_rollForce);
	}
	// ゴルフボールを更新する
	m_golfBall->Update(elapsedTime);

	// 平面を初期化する
	Plane plane(0.0f, 1.0f, 0.0f, 0.0f);
	// マウスカーソルのスクリーン位置を取得する
	m_mouseCursorPosition = Vector2(roundf((float)m_mouseState.x), roundf((float)m_mouseState.y));
	// カメラをコントロールする
	ControlCamera(timer);
}

// 描画する
void GraphScene::Render()
{
	using namespace DirectX::SimpleMath;
	// x軸
	const DirectX::XMVECTORF32 xaxis = { 100.f, 0.0f, 0.0f };
	// y軸
	const DirectX::XMVECTORF32 yaxis = { 0.0f, 0.0f, 100.f };

	// グリッドを描画する
	DrawGrid(xaxis, yaxis, DirectX::g_XMZero, 10, 10, DirectX::Colors::DarkGray);
	// 壁を描画する
	DrawWall();
	// ゴルフボールを描画する
	DrawBall();
	// ゴルフボールを転がす方向を描画する
	DrawRollDirection();
	// 情報を表示する
	DrawInfo();
}

// 後処理を行う
void GraphScene::Finalize()
{
	// ゴルフボールのモデルをリセットする
	m_golfBallModel.reset();
	// ゴルフボールをリセットする
	m_golfBall.reset();
}

// ゴルフボールを描画する
void GraphScene::DrawBall()
{
	// ゴルフボールの将来の位置を描画する
	 m_golfBall->DrawFuturePosition(0.5f);
	// ゴルフボールを描画する
	m_golfBall->Render();
}

// ゴルフボールを転がす方向を描画する
void GraphScene::DrawRollDirection()
{
	using namespace DirectX::SimpleMath;

	// ゴルフボールが停止している場合
	if (m_golfBall->GetVelocity().Length() < 0.05f )
	{
		// ゴルフボールの初期位置を設定する
		m_golfBall->SetPosition(START_POSITION);
		//  ゴルフボールの速度を初期化する
		m_golfBall->SetVelocity(Vector3::Zero);
		// ゴルフボールの方向を設定する
		Vector3 direction(1.0f, 0.0f, 0.0f);
		// Y軸回転を行う回転行列を生成する
		Matrix rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rollAngle));
		//  ゴルフボールを転がす方向を計算する
		direction = Vector3::Transform(direction, rotationMatrix);
		// 描画プリミティブを開始する
		m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
		// ゴルフボールを転がす方向を表すベクトルを描画する
		m_graphics->DrawVector(m_golfBall->GetPosition(), direction * m_rollForce, DirectX::Colors::White);
		// 描画プリミティブを終了する
		m_graphics->DrawPrimitiveEnd();
	}
}

// 壁を描画する
void GraphScene::DrawWall()
{
	using namespace DirectX::SimpleMath;

	// 描画プリミティブを開始する
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());
	// 壁を描画する
	m_graphics->DrawQuad(WALL[0], WALL[1], WALL[2], WALL[3], DirectX::Colors::DarkGray);
	// 法線ベクトルを描画する
	// m_graphics->DrawVector(WALL[0], GetWallNormalVector() * 10.0f, DirectX::Colors::White);
	// 描画プリミティブを終了する
	m_graphics->DrawPrimitiveEnd();
}

// 情報を表示する
void GraphScene::DrawInfo()
{
	wchar_t stringBuffer[128];
	// SpriteString2Dを宣言する
	SpriteString2D spriteString2D;

	// カメラ位置を書式化する
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           Camera position: (%6.1f, %6.1f, %6.1f )", 
		m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 0.0f));
	// カメラ回転角を書式化する
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           Camera rotation: (%6.1f, %6.1f, %6.1f ), %6.1f )",
		m_cameraRotation.x, m_cameraRotation.y, m_cameraRotation.z, m_cameraRotation.w);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 28.0f));
	// ゴルフボールの位置を書式化する
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           GolfBall position: (%6.1f, %6.1f,  %6.1f )",
		m_golfBall->GetPosition().x, m_golfBall->GetPosition().y, m_golfBall->GetPosition().z);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 56.0f));
	// ゴルフボールの速度を書式化する
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"           GolfBall velocity: (%6.1f, %6.1f, %6.1f ), %6.1f",
		m_golfBall->GetVelocity().x, m_golfBall->GetVelocity().y, m_golfBall->GetVelocity().z, m_golfBall->GetVelocity().Length());
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 84.0f));
	// 光線と平面の交差点を書式化する
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"         Intersection point: (%6.1f, %6.1f, %6.1f )",
		m_golfBall->GetIntersectionPoint().x, m_golfBall->GetIntersectionPoint().y, m_golfBall->GetIntersectionPoint().z);
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 112.0f));
	// 交差点までの距離を書式化する
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"Distance to Intersection: (%6.1f )",
		m_golfBall->GetDistanceToIntersection());
		spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 140.0f));
	// すべての情報を描画する
	spriteString2D.Render();
}

// アークボールを使用してカメラをコントロールする
void GraphScene::ControlCamera(const DX::StepTimer& timer)
{
	using namespace DirectX::SimpleMath;

	// 経過時間を取得する
	float elapsedTime = float(timer.GetElapsedSeconds());
	// スケールゲイン
	constexpr float SCALE_GAIN = 0.001f;

	// カメラ移動行列
	Matrix im;
	m_viewMatrix.Invert(im);
	Vector3 move = Vector3::TransformNormal(move, im);

	// マウスの移動が相対マウス移動である場合
	if (m_game->GetMouse()->GetState().positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		Vector3 delta = Vector3(-float(m_mouseState.x), float(m_mouseState.y), 0.f) * m_distance;
		delta = Vector3::TransformNormal(delta, im);
		// カメラフォーカス位置を計算する
		m_cameraFocus += delta * elapsedTime;
	}
	// マウスの右ボタンをドラッグしている場合
	else if (m_ballCamera.IsDragging())
	{
		// マウスの移動
		m_ballCamera.OnMove(m_mouseState.x, m_mouseState.y);
		// ボールカメラの現在のクォータニオンを取得する
		auto q = m_ballCamera.GetQuaternion();
		// カメラ回転の逆コォータニオンを計算する
		q.Inverse(m_cameraRotation);
	}
	else
	{
		// マウスフォイールを回転させた場合のズーム値を計算する
		m_zoom = 1.0f + float(m_mouseState.scrollWheelValue) * SCALE_GAIN;
		// ズーム値を調整する
		m_zoom = std::max(m_zoom, 0.01f);
		// スクロールフォイール値をリセットする
		m_game->GetMouse()->ResetScrollWheelValue();
	}

	// ドラッグ中でない場合
	if (!m_ballCamera.IsDragging())
	{
		// マウスの右ボタンを押し下げている場合
		if (m_mouseStateTracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
		{
			// 左右の[Ctrlキー]を押し下げている場合
			if (m_keyboardState.LeftControl || m_keyboardState.RightControl)
			{
				// ボールカメラを開始する
				m_ballCamera.OnBegin(m_mouseState.x, m_mouseState.y);
			}
		}
	}
	// マウスの右ボタンを解放している場合
	else if (m_mouseStateTracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
	{
		// ボールカメラを終了する
		m_ballCamera.OnEnd();
	}

	// カメラの向きを更新する
	Vector3 direction = Vector3::Transform(Vector3::Backward, m_cameraRotation);
	// カメラ位置を計算する
	m_cameraPosition = m_cameraFocus + (m_distance * m_zoom) * direction;
	// 視線ベクトルを設定する
	m_game->GetCamera()->SetEyePosition(m_cameraPosition);
}

// グリッドを描画する
void GraphScene::DrawGrid(
	const DirectX::FXMVECTOR& xAxis,
	const DirectX::FXMVECTOR& yAxis,
	const DirectX::FXMVECTOR& origin,
	const size_t& xdivs,
	const size_t& ydivs,
	const DirectX::GXMVECTOR& m_color
)
{
	// パフォーマンス開始イベント
	m_graphics->GetDeviceResources()->PIXBeginEvent(L"Draw Grid");
	// プリミティブ描画を開始する
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());

	for (size_t index = 0; index <= xdivs; ++index)
	{
		float percent = ((float(index) / float(xdivs)) * 2.0f) - 1.0f;
		// スケールを計算する
		DirectX::XMVECTOR scale = DirectX::XMVectorScale(xAxis, percent);
		scale = DirectX::XMVectorAdd(scale, origin);
		// 頂点1を設定する
		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, yAxis), m_color);
		// 頂点2を設定する
		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, yAxis), m_color);
		// 線分を描画する
		m_graphics->GetPrimitiveBatch()->DrawLine(v1, v2);
	}

	for (size_t index = 0; index <= ydivs; index++)
	{
		float percent = ((float(index) / float(ydivs)) * 2.0f) - 1.0f;
		// スケールを計算する
		DirectX::XMVECTOR scale = DirectX::XMVectorScale(yAxis, percent);
		scale = DirectX::XMVectorAdd(scale, origin);
		// 頂点1を設定する
		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, xAxis), m_color);
		// 頂点2を設定する
		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, xAxis), m_color);
		// 線分を描画する
		m_graphics->GetPrimitiveBatch()->DrawLine(v1, v2);
	}
	// プリミティブバッチを終了する
	m_graphics->DrawPrimitiveEnd();
	// パフォーマンス終了イベント
	m_graphics->GetDeviceResources()->PIXEndEvent();
}

// プロジェクションを生成する
void GraphScene::CreateProjection()
{
	// ウィンドウサイズを取得する
	auto size = m_graphics->GetDeviceResources()->GetOutputSize();
	// プロジェクションを生成する
	m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(m_fov, float(size.right) / float(size.bottom), m_nearPlane, m_farPlane);
}

// アークボールのためのウィンドウサイズを設定する
void GraphScene::SetWindow(const int& width, const int& height)
{
	// アークボールのウィンドウサイズを設定する
	m_ballCamera.SetWindow(width, height);
}
 