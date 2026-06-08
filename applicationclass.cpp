////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace
{
	const float BASE_PADDLE_WIDTH = 120.0f;
	const float BASE_PADDLE_HEIGHT = 20.0f;
	const float BASE_BALL_RADIUS = 8.0f;
	const float BASE_BALL_VELOCITY_X = 170.0f;
	const float BASE_BALL_VELOCITY_Y = 260.0f;
	const float BASE_PADDLE_SPEED = 420.0f;
	const float SPEED_UP_PADDLE_SPEED = 620.0f;
	const float SPEED_UP_MULTIPLIER = 1.35f;
	const float MODIFIER_DROP_SPEED = -180.0f;
	const int MODIFIER_SPAWN_PERCENT = 25;
}

ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_RectModel = 0;
	m_CircleModel = 0;
	m_ColorShader = 0;
	m_ScreenWidth = 0;
	m_ScreenHeight = 0;
	m_LastFrameTime = 0;
	m_BallVelocityX = 0.0f;
	m_BallVelocityY = 0.0f;
	m_GameOver = false;

	m_BigBallModifier.active = false;
	m_BigBallModifier.remainingTime = 0.0f;
	m_UpsideDownModifier.active = false;
	m_UpsideDownModifier.remainingTime = 0.0f;
	m_LongPaddleModifier.active = false;
	m_LongPaddleModifier.remainingTime = 0.0f;
	m_SpeedUpModifier.active = false;
	m_SpeedUpModifier.remainingTime = 0.0f;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other) {}
ApplicationClass::~ApplicationClass() {}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	srand(static_cast<unsigned int>(time(0)));

	m_ScreenWidth = screenWidth;
	m_ScreenHeight = screenHeight;

	m_Direct3D = new D3DClass;
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	m_Camera = new CameraClass;
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

	m_RectModel = new ModelClass;
	result = m_RectModel->Initialize(m_Direct3D->GetDevice(), ModelClass::ShapeRectangle);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the rectangle model object.", L"Error", MB_OK);
		return false;
	}

	m_CircleModel = new ModelClass;
	result = m_CircleModel->Initialize(m_Direct3D->GetDevice(), ModelClass::ShapeCircle);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the circle model object.", L"Error", MB_OK);
		return false;
	}

	m_ColorShader = new ColorShaderClass;
	result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
		return false;
	}

	InitializeGame();
	m_LastFrameTime = GetTickCount64();

	return true;
}

void ApplicationClass::Shutdown()
{
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	if (m_CircleModel)
	{
		m_CircleModel->Shutdown();
		delete m_CircleModel;
		m_CircleModel = 0;
	}

	if (m_RectModel)
	{
		m_RectModel->Shutdown();
		delete m_RectModel;
		m_RectModel = 0;
	}

	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
}

bool ApplicationClass::Frame(InputClass* input)
{
	ULONGLONG currentTime = GetTickCount64();
	float deltaTime = (currentTime - m_LastFrameTime) / 1000.0f;
	m_LastFrameTime = currentTime;

	// Evita salti enormi se il debugger mette in pausa il programma.
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	UpdateGame(input, deltaTime);
	return Render();
}

void ApplicationClass::InitializeGame()
{
	m_Paddle.x = 0.0f;
	m_Paddle.y = -240.0f;
	m_Paddle.width = BASE_PADDLE_WIDTH;
	m_Paddle.height = BASE_PADDLE_HEIGHT;
	m_Paddle.color = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	m_Paddle.active = true;

	m_Ball.radius = BASE_BALL_RADIUS;
	m_Ball.color = XMFLOAT4(1.0f, 0.9f, 0.2f, 1.0f);
	m_Ball.active = true;

	ResetModifiers();
	ResetBricks();
	ResetRound();
}

void ApplicationClass::ResetRound()
{
	m_Modifiers.clear();
	ResetModifiers();

	m_Paddle.x = 0.0f;
	m_Paddle.y = -240.0f;
	m_Paddle.width = BASE_PADDLE_WIDTH;
	m_Paddle.height = BASE_PADDLE_HEIGHT;

	m_Ball.x = 0.0f;
	m_Ball.y = -205.0f;
	m_Ball.radius = BASE_BALL_RADIUS;

	m_BallVelocityX = BASE_BALL_VELOCITY_X;
	m_BallVelocityY = BASE_BALL_VELOCITY_Y;
	m_GameOver = false;
}

void ApplicationClass::ResetModifiers()
{
	m_BigBallModifier.active = false;
	m_BigBallModifier.remainingTime = 0.0f;

	m_UpsideDownModifier.active = false;
	m_UpsideDownModifier.remainingTime = 0.0f;

	m_LongPaddleModifier.active = false;
	m_LongPaddleModifier.remainingTime = 0.0f;

	m_SpeedUpModifier.active = false;
	m_SpeedUpModifier.remainingTime = 0.0f;
}

void ApplicationClass::ResetBricks()
{
	m_Bricks.clear();

	const int rows = 5;
	const int columns = 10;
	const float brickWidth = 64.0f;
	const float brickHeight = 24.0f;
	const float gap = 8.0f;
	const float startX = -((columns - 1) * (brickWidth + gap)) / 2.0f;
	const float startY = 210.0f;

	XMFLOAT4 rowColors[rows] =
	{
		XMFLOAT4(0.2f, 0.6f, 1.0f, 1.0f),
		XMFLOAT4(0.2f, 1.0f, 0.5f, 1.0f),
		XMFLOAT4(1.0f, 0.8f, 0.2f, 1.0f),
		XMFLOAT4(1.0f, 0.45f, 0.25f, 1.0f),
		XMFLOAT4(0.8f, 0.35f, 1.0f, 1.0f)
	};

	for (int row = 0; row < rows; row++)
	{
		for (int column = 0; column < columns; column++)
		{
			RectObject brick;
			brick.x = startX + column * (brickWidth + gap);
			brick.y = startY - row * (brickHeight + gap);
			brick.width = brickWidth;
			brick.height = brickHeight;
			brick.color = rowColors[row];
			brick.active = true;
			m_Bricks.push_back(brick);
		}
	}
}

void ApplicationClass::UpdateGame(InputClass* input, float deltaTime)
{
	if (m_GameOver)
	{
		// Restart manuale semplice dopo game over.
		if (input->IsKeyDown(VK_RETURN))
		{
			ResetBricks();
			ResetRound();
		}
		return;
	}

	UpdateActiveModifierTimers(deltaTime);

	const float halfScreenWidth = m_ScreenWidth * 0.5f;
	const float halfScreenHeight = m_ScreenHeight * 0.5f;
	float paddleSpeed = BASE_PADDLE_SPEED;

	if (m_SpeedUpModifier.active)
	{
		paddleSpeed = SPEED_UP_PADDLE_SPEED;
	}

	float movementDirection = 0.0f;

	if (input->IsKeyDown(VK_LEFT) || input->IsKeyDown('A'))
	{
		movementDirection -= 1.0f;
	}

	if (input->IsKeyDown(VK_RIGHT) || input->IsKeyDown('D'))
	{
		movementDirection += 1.0f;
	}

	// Quando lo schermo è sottosopra, invertiamo anche i controlli orizzontali.
	if (m_UpsideDownModifier.active)
	{
		movementDirection *= -1.0f;
	}

	m_Paddle.x += movementDirection * paddleSpeed * deltaTime;

	float paddleLimit = halfScreenWidth - m_Paddle.width * 0.5f;

	if (m_Paddle.x < -paddleLimit)
	{
		m_Paddle.x = -paddleLimit;
	}
	else if (m_Paddle.x > paddleLimit)
	{
		m_Paddle.x = paddleLimit;
	}

	UpdateModifiers(deltaTime);

	float previousBallX = m_Ball.x;

	m_Ball.x += m_BallVelocityX * deltaTime;
	m_Ball.y += m_BallVelocityY * deltaTime;

	float ballRadius = m_Ball.radius;

	// Pareti laterali.
	if (m_Ball.x - ballRadius < -halfScreenWidth)
	{
		m_Ball.x = -halfScreenWidth + ballRadius;
		m_BallVelocityX *= -1.0f;
	}
	else if (m_Ball.x + ballRadius > halfScreenWidth)
	{
		m_Ball.x = halfScreenWidth - ballRadius;
		m_BallVelocityX *= -1.0f;
	}

	// Soffitto.
	if (m_Ball.y + ballRadius > halfScreenHeight)
	{
		m_Ball.y = halfScreenHeight - ballRadius;
		m_BallVelocityY *= -1.0f;
	}

	// Fondo: una sola vita.
	if (m_Ball.y - ballRadius < -halfScreenHeight)
	{
		m_GameOver = true;
		return;
	}

	// Paddle.
	if (m_BallVelocityY < 0.0f && IntersectsCircleRect(m_Ball, m_Paddle))
	{
		m_Ball.y = m_Paddle.y + (m_Paddle.height * 0.5f) + ballRadius;
		m_BallVelocityY = fabsf(m_BallVelocityY);

		// Colpo più angolato se la palla tocca lontano dal centro della racchetta.
		float hitPosition = (m_Ball.x - m_Paddle.x) / (m_Paddle.width * 0.5f);

		if (hitPosition < -1.0f)
		{
			hitPosition = -1.0f;
		}
		else if (hitPosition > 1.0f)
		{
			hitPosition = 1.0f;
		}

		m_BallVelocityX = hitPosition * 320.0f;

		if (m_SpeedUpModifier.active)
		{
			m_BallVelocityX *= SPEED_UP_MULTIPLIER;
		}
	}

	// Mattoncini: tutti distrutti al primo colpo.
	for (size_t i = 0; i < m_Bricks.size(); i++)
	{
		if (!m_Bricks[i].active)
		{
			continue;
		}

		if (IntersectsCircleRect(m_Ball, m_Bricks[i]))
		{
			m_Bricks[i].active = false;

			// Nel 25% dei casi il mattoncino distrutto rilascia un modificatore.
			if ((rand() % 100) < MODIFIER_SPAWN_PERCENT)
			{
				SpawnModifier(m_Bricks[i].x, m_Bricks[i].y);
			}

			// Scelta semplice della direzione di rimbalzo usando la posizione precedente.
			bool wasOutsideHorizontally =
				previousBallX + ballRadius <= m_Bricks[i].x - m_Bricks[i].width * 0.5f ||
				previousBallX - ballRadius >= m_Bricks[i].x + m_Bricks[i].width * 0.5f;

			if (wasOutsideHorizontally)
			{
				m_BallVelocityX *= -1.0f;
			}
			else
			{
				m_BallVelocityY *= -1.0f;
			}

			break;
		}
	}

	// Se finisci i mattoncini, ripristina il livello per una nuova partita.
	if (AreAllBricksDestroyed())
	{
		ResetBricks();
		ResetRound();
	}
}

void ApplicationClass::SpawnModifier(float x, float y)
{
	ModifierObject modifier;

	modifier.x = x;
	modifier.y = y;
	modifier.radius = 16.0f;
	modifier.velocityY = MODIFIER_DROP_SPEED;
	modifier.active = true;

	int randomType = rand() % 4;

	if (randomType == 0)
	{
		modifier.type = ModifierBigBall;
	}
	else if (randomType == 1)
	{
		modifier.type = ModifierUpsideDown;
	}
	else if (randomType == 2)
	{
		modifier.type = ModifierLongPaddle;
	}
	else
	{
		modifier.type = ModifierSpeedUp;
	}

	modifier.color = GetModifierColor(modifier.type);
	m_Modifiers.push_back(modifier);
}

void ApplicationClass::UpdateModifiers(float deltaTime)
{
	const float halfScreenHeight = m_ScreenHeight * 0.5f;

	for (size_t i = 0; i < m_Modifiers.size(); i++)
	{
		if (!m_Modifiers[i].active)
		{
			continue;
		}

		m_Modifiers[i].y += m_Modifiers[i].velocityY * deltaTime;

		if (IntersectsModifierPaddle(m_Modifiers[i], m_Paddle))
		{
			ApplyModifier(m_Modifiers[i].type);
			m_Modifiers[i].active = false;
		}
		else if (m_Modifiers[i].y + m_Modifiers[i].radius < -halfScreenHeight)
		{
			m_Modifiers[i].active = false;
		}
	}
}

void ApplicationClass::ApplyModifier(ModifierType type)
{
	// Durata casuale tra 5 e 10 secondi.
	float duration = 5.0f + static_cast<float>(rand() % 6);

	if (type == ModifierBigBall)
	{
		m_Ball.radius = 14.0f;
		m_BigBallModifier.active = true;
		m_BigBallModifier.remainingTime = duration;
	}
	else if (type == ModifierUpsideDown)
	{
		m_UpsideDownModifier.active = true;
		m_UpsideDownModifier.remainingTime = duration;
	}
	else if (type == ModifierLongPaddle)
	{
		m_Paddle.width = 180.0f;
		m_LongPaddleModifier.active = true;
		m_LongPaddleModifier.remainingTime = duration;
	}
	else if (type == ModifierSpeedUp)
	{
		if (!m_SpeedUpModifier.active)
		{
			m_BallVelocityX *= SPEED_UP_MULTIPLIER;
			m_BallVelocityY *= SPEED_UP_MULTIPLIER;
		}

		m_SpeedUpModifier.active = true;
		m_SpeedUpModifier.remainingTime = duration;
	}
}

void ApplicationClass::UpdateActiveModifierTimers(float deltaTime)
{
	if (m_BigBallModifier.active)
	{
		m_BigBallModifier.remainingTime -= deltaTime;

		if (m_BigBallModifier.remainingTime <= 0.0f)
		{
			DisableModifier(ModifierBigBall);
		}
	}

	if (m_UpsideDownModifier.active)
	{
		m_UpsideDownModifier.remainingTime -= deltaTime;

		if (m_UpsideDownModifier.remainingTime <= 0.0f)
		{
			DisableModifier(ModifierUpsideDown);
		}
	}

	if (m_LongPaddleModifier.active)
	{
		m_LongPaddleModifier.remainingTime -= deltaTime;

		if (m_LongPaddleModifier.remainingTime <= 0.0f)
		{
			DisableModifier(ModifierLongPaddle);
		}
	}

	if (m_SpeedUpModifier.active)
	{
		m_SpeedUpModifier.remainingTime -= deltaTime;

		if (m_SpeedUpModifier.remainingTime <= 0.0f)
		{
			DisableModifier(ModifierSpeedUp);
		}
	}
}

void ApplicationClass::DisableModifier(ModifierType type)
{
	if (type == ModifierBigBall)
	{
		m_Ball.radius = BASE_BALL_RADIUS;
		m_BigBallModifier.active = false;
		m_BigBallModifier.remainingTime = 0.0f;
	}
	else if (type == ModifierUpsideDown)
	{
		m_UpsideDownModifier.active = false;
		m_UpsideDownModifier.remainingTime = 0.0f;
	}
	else if (type == ModifierLongPaddle)
	{
		m_Paddle.width = BASE_PADDLE_WIDTH;
		m_LongPaddleModifier.active = false;
		m_LongPaddleModifier.remainingTime = 0.0f;
	}
	else if (type == ModifierSpeedUp)
	{
		float signX = 1.0f;
		float signY = 1.0f;

		if (m_BallVelocityX < 0.0f)
		{
			signX = -1.0f;
		}

		if (m_BallVelocityY < 0.0f)
		{
			signY = -1.0f;
		}

		m_BallVelocityX = BASE_BALL_VELOCITY_X * signX;
		m_BallVelocityY = BASE_BALL_VELOCITY_Y * signY;

		m_SpeedUpModifier.active = false;
		m_SpeedUpModifier.remainingTime = 0.0f;
	}
}

XMFLOAT4 ApplicationClass::GetModifierColor(ModifierType type) const
{
	if (type == ModifierBigBall)
	{
		return XMFLOAT4(0.2f, 0.6f, 1.0f, 1.0f); // blu
	}
	else if (type == ModifierUpsideDown)
	{
		return XMFLOAT4(1.0f, 0.2f, 0.8f, 1.0f); // viola/rosa
	}
	else if (type == ModifierLongPaddle)
	{
		return XMFLOAT4(0.2f, 1.0f, 0.3f, 1.0f); // verde
	}

	return XMFLOAT4(1.0f, 0.25f, 0.15f, 1.0f); // rosso/arancio
}

bool ApplicationClass::Intersects(const RectObject& a, const RectObject& b) const
{
	return fabsf(a.x - b.x) * 2.0f < (a.width + b.width) &&
		fabsf(a.y - b.y) * 2.0f < (a.height + b.height);
}

bool ApplicationClass::IntersectsCircleRect(const BallObject& ball, const RectObject& rect) const
{
	float rectHalfWidth = rect.width * 0.5f;
	float rectHalfHeight = rect.height * 0.5f;

	float closestX = ball.x;

	if (closestX < rect.x - rectHalfWidth)
	{
		closestX = rect.x - rectHalfWidth;
	}
	else if (closestX > rect.x + rectHalfWidth)
	{
		closestX = rect.x + rectHalfWidth;
	}

	float closestY = ball.y;

	if (closestY < rect.y - rectHalfHeight)
	{
		closestY = rect.y - rectHalfHeight;
	}
	else if (closestY > rect.y + rectHalfHeight)
	{
		closestY = rect.y + rectHalfHeight;
	}

	float distanceX = ball.x - closestX;
	float distanceY = ball.y - closestY;

	return (distanceX * distanceX + distanceY * distanceY) <= (ball.radius * ball.radius);
}

bool ApplicationClass::IntersectsModifierPaddle(const ModifierObject& modifier, const RectObject& paddle) const
{
	BallObject tempBall;
	tempBall.x = modifier.x;
	tempBall.y = modifier.y;
	tempBall.radius = modifier.radius;
	tempBall.color = modifier.color;
	tempBall.active = modifier.active;

	return IntersectsCircleRect(tempBall, paddle);
}

bool ApplicationClass::AreAllBricksDestroyed() const
{
	for (size_t i = 0; i < m_Bricks.size(); i++)
	{
		if (m_Bricks[i].active)
		{
			return false;
		}
	}
	return true;
}

bool ApplicationClass::RenderRect(const RectObject& rect, const XMMATRIX& viewMatrix, const XMMATRIX& orthoMatrix)
{
	if (!rect.active)
	{
		return true;
	}

	XMMATRIX worldMatrix = XMMatrixScaling(rect.width, rect.height, 1.0f) *
		XMMatrixTranslation(rect.x, rect.y, 0.0f);

	if (m_UpsideDownModifier.active)
	{
		worldMatrix = worldMatrix * XMMatrixRotationZ(XM_PI);
	}

	if (!m_RectModel->SetColor(m_Direct3D->GetDeviceContext(), rect.color))
	{
		return false;
	}

	m_RectModel->Render(m_Direct3D->GetDeviceContext());
	return m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_RectModel->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
}

bool ApplicationClass::RenderBall(const BallObject& ball, const XMMATRIX& viewMatrix, const XMMATRIX& orthoMatrix)
{
	if (!ball.active)
	{
		return true;
	}

	float diameter = ball.radius * 2.0f;

	XMMATRIX worldMatrix = XMMatrixScaling(diameter, diameter, 1.0f) *
		XMMatrixTranslation(ball.x, ball.y, 0.0f);

	if (m_UpsideDownModifier.active)
	{
		worldMatrix = worldMatrix * XMMatrixRotationZ(XM_PI);
	}

	if (!m_CircleModel->SetColor(m_Direct3D->GetDeviceContext(), ball.color))
	{
		return false;
	}

	m_CircleModel->Render(m_Direct3D->GetDeviceContext());
	return m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_CircleModel->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
}

bool ApplicationClass::RenderModifier(const ModifierObject& modifier, const XMMATRIX& viewMatrix, const XMMATRIX& orthoMatrix)
{
	if (!modifier.active)
	{
		return true;
	}

	float diameter = modifier.radius * 2.0f;

	XMMATRIX worldMatrix = XMMatrixScaling(diameter, diameter, 1.0f) *
		XMMatrixTranslation(modifier.x, modifier.y, 0.0f);

	if (m_UpsideDownModifier.active)
	{
		worldMatrix = worldMatrix * XMMatrixRotationZ(XM_PI);
	}

	if (!m_CircleModel->SetColor(m_Direct3D->GetDeviceContext(), modifier.color))
	{
		return false;
	}

	m_CircleModel->Render(m_Direct3D->GetDeviceContext());
	return m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_CircleModel->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix);
}

bool ApplicationClass::Render()
{
	XMMATRIX viewMatrix, orthoMatrix;
	bool result;

	if (m_GameOver)
	{
		m_Direct3D->BeginScene(0.18f, 0.02f, 0.02f, 1.0f);
	}
	else if (m_UpsideDownModifier.active)
	{
		m_Direct3D->BeginScene(0.08f, 0.02f, 0.10f, 1.0f);
	}
	else
	{
		m_Direct3D->BeginScene(0.02f, 0.02f, 0.05f, 1.0f);
	}

	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	for (size_t i = 0; i < m_Bricks.size(); i++)
	{
		result = RenderRect(m_Bricks[i], viewMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}
	}

	for (size_t i = 0; i < m_Modifiers.size(); i++)
	{
		result = RenderModifier(m_Modifiers[i], viewMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}
	}

	result = RenderRect(m_Paddle, viewMatrix, orthoMatrix);
	if (!result)
	{
		return false;
	}

	result = RenderBall(m_Ball, viewMatrix, orthoMatrix);
	if (!result)
	{
		return false;
	}

	m_Direct3D->EndScene();
	return true;
}
