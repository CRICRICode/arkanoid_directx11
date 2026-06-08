#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

#include <vector>
#include <windows.h>

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"
#include "inputclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

class ApplicationClass
{
private:
	struct RectObject
	{
		float x, y;
		float width, height;
		XMFLOAT4 color;
		bool active;
	};

	struct BallObject
	{
		float x, y;
		float radius;
		XMFLOAT4 color;
		bool active;
	};

	enum ModifierType
	{
		ModifierBigBall,
		ModifierUpsideDown,
		ModifierLongPaddle,
		ModifierSpeedUp
	};

	struct ModifierObject
	{
		float x, y;
		float radius;
		float velocityY;
		XMFLOAT4 color;
		bool active;
		ModifierType type;
	};

	struct ActiveModifier
	{
		bool active;
		float remainingTime;
	};

public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&) = delete;
	ApplicationClass& operator=(const ApplicationClass&) = delete;
	~ApplicationClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(InputClass*);

private:
	void InitializeGame();
	void ResetRound();
	void ResetBricks();
	void ResetModifier(ActiveModifier&);
	void ResetModifiers();
	void UpdateGame(InputClass*, float);

	void SpawnModifier(float, float);
	void UpdateModifiers(float);
	void ApplyModifier(ModifierType);
	void UpdateModifierTimer(ActiveModifier&, ModifierType, float);
	void UpdateActiveModifierTimers(float);
	void DisableModifier(ModifierType);
	XMFLOAT4 GetModifierColor(ModifierType) const;

	bool IntersectsCircleRect(const BallObject&, const RectObject&) const;
	bool IntersectsCircleRect(float, float, float, const RectObject&) const;
	bool IntersectsModifierPaddle(const ModifierObject&, const RectObject&) const;
	bool AreAllBricksDestroyed() const;

	bool Render();
	XMMATRIX BuildWorldMatrix(float, float, float, float) const;
	bool RenderModel(ID3D11DeviceContext*, ModelClass*, int, const XMFLOAT4&, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&);
	bool RenderRect(const RectObject&, ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&);
	bool RenderCircle(float, float, float, const XMFLOAT4&, bool, ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&);

private:
	D3DClass* m_Direct3D;
	CameraClass* m_Camera;
	ModelClass* m_RectModel;
	ModelClass* m_CircleModel;
	ColorShaderClass* m_ColorShader;

	int m_ScreenWidth;
	int m_ScreenHeight;
	ULONGLONG m_LastFrameTime;

	RectObject m_Paddle;
	BallObject m_Ball;
	std::vector<RectObject> m_Bricks;
	std::vector<ModifierObject> m_Modifiers;

	ActiveModifier m_BigBallModifier;
	ActiveModifier m_UpsideDownModifier;
	ActiveModifier m_LongPaddleModifier;
	ActiveModifier m_SpeedUpModifier;

	float m_BallVelocityX;
	float m_BallVelocityY;
	bool m_GameOver;
};

#endif
