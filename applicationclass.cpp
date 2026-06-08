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
	const int MIN_BRICK_COLOR_COUNT = 5;
	const int BRICK_COLOR_COUNT = 8;
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

void ApplicationClass::ResetModifier(ActiveModifier& modifier)
{
	modifier.active = false;
	modifier.remainingTime = 0.0f;
}

void ApplicationClass::ResetModifiers()
{
	ResetModifier(m_BigBallModifier);
	ResetModifier(m_UpsideDownModifier);
	ResetModifier(m_LongPaddleModifier);
	ResetModifier(m_SpeedUpModifier);
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
	const int totalBricks = rows * columns;

	const XMFLOAT4 brickColors[BRICK_COLOR_COUNT] =
	{
		XMFLOAT4(0.2f, 0.6f, 1.0f, 1.0f),
		XMFLOAT4(0.2f, 1.0f, 0.5f, 1.0f),
		XMFLOAT4(1.0f, 0.8f, 0.2f, 1.0f),
		XMFLOAT4(1.0f, 0.45f, 0.25f, 1.0f),
		XMFLOAT4(0.8f, 0.35f, 1.0f, 1.0f),
		XMFLOAT4(0.1f, 0.9f, 0.9f, 1.0f),
		XMFLOAT4(1.0f, 0.25f, 0.55f, 1.0f),
		XMFLOAT4(0.7f, 1.0f, 0.25f, 1.0f)
	};

	std::vector<int> colorIndices;
	colorIndices.reserve(totalBricks);

	for (int i = 0; i < totalBricks; i++)
	{
		if (i < MIN_BRICK_COLOR_COUNT)
		{
			colorIndices.push_back(i);
		}
		else
		{
			colorIndices.push_back(rand() % BRICK_COLOR_COUNT);
		}
	}

	for (int i = totalBricks - 1; i > 0; i--)
	{
		int randomIndex = rand() % (i + 1);
		int temp = colorIndices[i];
		colorIndices[i] = colorIndices[randomIndex];
		colorIndices[randomIndex] = temp;
	}

	for (int row = 0; row < rows; row++)
	{
		for (int column = 0; column < columns; column++)
		{
			int brickIndex = row * columns + column;
			RectObject brick;
			brick.x = startX + column * (brickWidth + gap);
			brick.y = startY - row * (brickHeight + gap);
			brick.width = brickWidth;
			brick.height = brickHeight;
			brick.color = brickColors[colorIndices[brickIndex]];
			brick.active = true;
			m_Bricks.push_back(brick);
		}
	}
}

void ApplicationClass::UpdateGame(InputClass* input, float deltaTime)
{
	if (m_GameOver)
	{
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

	if (m_Ball.y + ballRadius > halfScreenHeight)
	{
		m_Ball.y = halfScreenHeight - ballRadius;
		m_BallVelocityY *= -1.0f;
	}

	if (m_Ball.y - ballRadius < -halfScreenHeight)
	{
		m_GameOver = true;
		return;
	}

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

	for (size_t i = 0; i < m_Bricks.size(); i++)
	{
		if (!m_Bricks[i].active)
		{
			continue;
		}

		if (IntersectsCircleRect(m_Ball, m_Bricks[i]))
		{
			m_Bricks[i].active = false;

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

void ApplicationClass::UpdateModifierTimer(ActiveModifier& modifier, ModifierType type, float deltaTime)
{
	if (!modifier.active)
	{
		return;
	}

	modifier.remainingTime -= deltaTime;

	if (modifier.remainingTime <= 0.0f)
	{
		DisableModifier(type);
	}
}

void ApplicationClass::UpdateActiveModifierTimers(float deltaTime)
{
	UpdateModifierTimer(m_BigBallModifier, ModifierBigBall, deltaTime);
	UpdateModifierTimer(m_UpsideDownModifier, ModifierUpsideDown, deltaTime);
	UpdateModifierTimer(m_LongPaddleModifier, ModifierLongPaddle, deltaTime);
	UpdateModifierTimer(m_SpeedUpModifier, ModifierSpeedUp, deltaTime);
}

void ApplicationClass::DisableModifier(ModifierType type)
{
	if (type == ModifierBigBall)
	{
		m_Ball.radius = BASE_BALL_RADIUS;
		ResetModifier(m_BigBallModifier);
	}
	else if (type == ModifierUpsideDown)
	{
		ResetModifier(m_UpsideDownModifier);
	}
	else if (type == ModifierLongPaddle)
	{
		m_Paddle.width = BASE_PADDLE_WIDTH;
		ResetModifier(m_LongPaddleModifier);
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

		ResetModifier(m_SpeedUpModifier);
	}
}

XMFLOAT4 ApplicationClass::GetModifierColor(ModifierType type) const
{
	if (type == ModifierBigBall)
	{
		return XMFLOAT4(0.2f, 0.6f, 1.0f, 1.0f);
	}
	else if (type == ModifierUpsideDown)
	{
		return XMFLOAT4(1.0f, 0.2f, 0.8f, 1.0f);
	}
	else if (type == ModifierLongPaddle)
	{
		return XMFLOAT4(0.2f, 1.0f, 0.3f, 1.0f);
	}

	return XMFLOAT4(1.0f, 0.25f, 0.15f, 1.0f);
}

bool ApplicationClass::IntersectsCircleRect(const BallObject& ball, const RectObject& rect) const
{
	return IntersectsCircleRect(ball.x, ball.y, ball.radius, rect);
}

bool ApplicationClass::IntersectsCircleRect(float circleX, float circleY, float radius, const RectObject& rect) const
{
	float rectHalfWidth = rect.width * 0.5f;
	float rectHalfHeight = rect.height * 0.5f;

	float closestX = circleX;

	if (closestX < rect.x - rectHalfWidth)
	{
		closestX = rect.x - rectHalfWidth;
	}
	else if (closestX > rect.x + rectHalfWidth)
	{
		closestX = rect.x + rectHalfWidth;
	}

	float closestY = circleY;

	if (closestY < rect.y - rectHalfHeight)
	{
		closestY = rect.y - rectHalfHeight;
	}
	else if (closestY > rect.y + rectHalfHeight)
	{
		closestY = rect.y + rectHalfHeight;
	}

	float distanceX = circleX - closestX;
	float distanceY = circleY - closestY;

	return (distanceX * distanceX + distanceY * distanceY) <= (radius * radius);
}

bool ApplicationClass::IntersectsModifierPaddle(const ModifierObject& modifier, const RectObject& paddle) const
{
	return IntersectsCircleRect(modifier.x, modifier.y, modifier.radius, paddle);
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

XMMATRIX ApplicationClass::BuildWorldMatrix(float x, float y, float width, float height) const
{
	XMMATRIX worldMatrix = XMMatrixScaling(width, height, 1.0f) *
		XMMatrixTranslation(x, y, 0.0f);

	if (m_UpsideDownModifier.active)
	{
		worldMatrix = worldMatrix * XMMatrixRotationZ(XM_PI);
	}

	return worldMatrix;
}

bool ApplicationClass::RenderModel(ID3D11DeviceContext* deviceContext, ModelClass* model, int indexCount,
	const XMFLOAT4& color, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& orthoMatrix)
{
	if (!model->SetColor(deviceContext, color))
	{
		return false;
	}

	model->Render(deviceContext);
	return m_ColorShader->Render(deviceContext, indexCount, worldMatrix, viewMatrix, orthoMatrix);
}

bool ApplicationClass::RenderRect(const RectObject& rect, ID3D11DeviceContext* deviceContext,
	const XMMATRIX& viewMatrix, const XMMATRIX& orthoMatrix)
{
	if (!rect.active)
	{
		return true;
	}

	XMMATRIX worldMatrix = BuildWorldMatrix(rect.x, rect.y, rect.width, rect.height);
	return RenderModel(deviceContext, m_RectModel, m_RectModel->GetIndexCount(), rect.color, worldMatrix, viewMatrix, orthoMatrix);
}

bool ApplicationClass::RenderCircle(float x, float y, float radius, const XMFLOAT4& color, bool active,
	ID3D11DeviceContext* deviceContext, const XMMATRIX& viewMatrix, const XMMATRIX& orthoMatrix)
{
	if (!active)
	{
		return true;
	}

	float diameter = radius * 2.0f;

	XMMATRIX worldMatrix = BuildWorldMatrix(x, y, diameter, diameter);
	return RenderModel(deviceContext, m_CircleModel, m_CircleModel->GetIndexCount(), color, worldMatrix, viewMatrix, orthoMatrix);
}

bool ApplicationClass::Render()
{
	XMMATRIX viewMatrix, orthoMatrix;
	ID3D11DeviceContext* deviceContext;
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
	deviceContext = m_Direct3D->GetDeviceContext();

	for (size_t i = 0; i < m_Bricks.size(); i++)
	{
		result = RenderRect(m_Bricks[i], deviceContext, viewMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}
	}

	for (size_t i = 0; i < m_Modifiers.size(); i++)
	{
		result = RenderCircle(m_Modifiers[i].x, m_Modifiers[i].y, m_Modifiers[i].radius, m_Modifiers[i].color,
			m_Modifiers[i].active, deviceContext, viewMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}
	}

	result = RenderRect(m_Paddle, deviceContext, viewMatrix, orthoMatrix);
	if (!result)
	{
		return false;
	}

	result = RenderCircle(m_Ball.x, m_Ball.y, m_Ball.radius, m_Ball.color, m_Ball.active,
		deviceContext, viewMatrix, orthoMatrix);
	if (!result)
	{
		return false;
	}

	m_Direct3D->EndScene();
	return true;
}
