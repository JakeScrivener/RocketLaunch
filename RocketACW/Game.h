#pragma once
#include <vector>
#include "GameObject.h"
#include "Light.h"
#include "Camera.h"
#include <Keyboard.h>
#include "AntTweakManager.h"

class Game
{
	std::vector<GameObject> mGameObjects;
	std::vector<Light> mLights;
	std::vector<Camera> mCameras;
	Camera* mActiveCamera = nullptr;
	std::unique_ptr<DirectX::Keyboard> mKeyboard;
	DirectX::Keyboard::KeyboardStateTracker mTracker;
	AntTweakManager* mAwManager;
	float mTimeScale;
	float mCameraSpeed;
	bool mExit;
	bool mLaunch;
	GameObject* mEnvironment = nullptr;
	GameObject* mLauncher = nullptr;
	GameObject* mRocket = nullptr;
	GameObject* mTerrain = nullptr;
	GameObject* mSun = nullptr;
	GameObject* mMoon = nullptr;
	float mTerrainScale = 1;
	int mTerrainX = 120;
	int mTerrainY = 40;
	int mTerrainZ = 40;
	int mCubeCount = 0;
	float mWidth;
	float mHeight;
	float mTime = 0;
	float mAverageDt = 0;
	std::vector<float> mDeltaTimeSamples;
	float mFrameRate = 0;
	float mRocketSpeed = 2;
	float mExplosionRadius = 7.0f;
	float mParticleTimer = 0.0f;

	void CreateScene();
	void HandleInput(const double& pDt);
	void CheckCollision(const Shape& pShape);
	void Explosion(const DirectX::XMFLOAT4X4& pTransform); 
	void ResetRocket();
	void InitialiseLights();
	void InitialiseCameras();
	void DayNightCycle(const float pDt);
	void ResetGame();

public:
	Game(const float pWidth, const float pHeight, AntTweakManager& pAwManager);
	~Game() = default;

	Game& operator=(const Game& pGame) = delete;
	Game(const Game& pGame) = delete;
	const bool& Exit() const;

	void Update(const double& pDt);
	const std::vector<GameObject>& GameObjects() const; //Accessor for the render call to access gameobjects
	const Camera * const Cam() const;
	const std::vector<Light> & Lights() const;
	const float ScaledTime() const;
};

