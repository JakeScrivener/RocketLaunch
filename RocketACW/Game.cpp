#include "Game.h"

using namespace DirectX;
using namespace std;

/// <summary>
/// constructor for the game class initialises the game and then loads the scene
/// </summary>
/// <param name="pWidth"> the width of the game window </param>
/// <param name="pHeight"> the height of the game window </param>
/// <param name="pAwManager"> a reference to the ant tweak manager </param>
Game::Game(const float pWidth, const float pHeight, AntTweakManager& pAwManager) : mAwManager(&pAwManager), mTimeScale(5.0f), mCameraSpeed(8.0f), mExit(false), mLaunch(false), mWidth(pWidth), mHeight(pHeight)
{
	const vector<float> vec(50, 0);
	mDeltaTimeSamples = vec;

	//initialise keyboard
	mKeyboard = std::make_unique<Keyboard>();

	//Create the scene
	CreateScene();

	//World Stats
	mAwManager->AddBar("WorldStats");

	//Terrain
	mAwManager->AddVariable("WorldStats", "Terrain Scale", mTerrainScale, "group = Terrain");
	mAwManager->AddVariable("WorldStats", "Cubes in X", mTerrainX, "group = Terrain");
	mAwManager->AddVariable("WorldStats", "Cubes in Y", mTerrainY, "group = Terrain");
	mAwManager->AddVariable("WorldStats", "Cubes in Z", mTerrainZ, "group = Terrain");
	mAwManager->AddVariable("WorldStats", "Cube Count", mCubeCount, "group = Terrain");

	//Rocket
	mAwManager->AddWritableVariable("WorldStats", "Rocket Thrust", mRocketSpeed, "group = Rocket step=0.1 min=0 max = 3");
	mAwManager->AddVariable("WorldStats", "X Pos", const_cast<float&>(mRocket->Position().x), "group = Rocket");
	mAwManager->AddVariable("WorldStats", "Y Pos", const_cast<float&>(mRocket->Position().y), "group = Rocket");
	mAwManager->AddVariable("WorldStats", "Z Pos", const_cast<float&>(mRocket->Position().z), "group = Rocket");

	//Game Stats
	mAwManager->AddBar("GameStats");
	mAwManager->AddWritableVariable("GameStats", "Time Scale", mTimeScale, "step=0.1");
	mAwManager->AddVariable("GameStats", "Time", mTime, "");
	mAwManager->AddVariable("GameStats", "FPS", mFrameRate, "");

	//Camera
	mAwManager->AddVariable("GameStats", "Screen Width", mWidth, "group = Camera");
	mAwManager->AddVariable("GameStats", "Screen Height", mHeight, "group = Camera");
	mAwManager->AddVariable("GameStats", "X Pos", const_cast<float&>(mActiveCamera->Eye().x), "group = Camera");
	mAwManager->AddVariable("GameStats", "Y Pos", const_cast<float&>(mActiveCamera->Eye().y), "group = Camera");
	mAwManager->AddVariable("GameStats", "Z Pos", const_cast<float&>(mActiveCamera->Eye().z), "group = Camera");

	//Lights
	mAwManager->AddVariable("GameStats", "SunX", const_cast<float&>(mLights[0].Position().x), "group = Lights");
	mAwManager->AddVariable("GameStats", "SunY", const_cast<float&>(mLights[0].Position().y), "group = Lights");
	mAwManager->AddVariable("GameStats", "SunZ", const_cast<float&>(mLights[0].Position().z), "group = Lights");
	mAwManager->AddVariable("GameStats", "SunOrbit", const_cast<float&>(mLights[0].GetOrbit().z), "group = Lights");
	mAwManager->AddWritableVariable("GameStats", "SunColour", const_cast<XMFLOAT4&>(mLights[0].Colour()), "group = Lights");

	mAwManager->AddVariable("GameStats", "MoonX", const_cast<float&>(mLights[1].Position().x), "group = Lights");
	mAwManager->AddVariable("GameStats", "MoonY", const_cast<float&>(mLights[1].Position().y), "group = Lights");
	mAwManager->AddVariable("GameStats", "MoonZ", const_cast<float&>(mLights[1].Position().z), "group = Lights");
	mAwManager->AddVariable("GameStats", "MoonOrbit", const_cast<float&>(mLights[1].GetOrbit().z), "group = Lights");
	mAwManager->AddWritableVariable("GameStats", "MoonColour", const_cast<XMFLOAT4&>(mLights[1].Colour()), "group = Lights");

	mAwManager->AddVariable("GameStats", "EngineX", const_cast<float&>(mLights[2].Position().x), "group = Lights");
	mAwManager->AddVariable("GameStats", "EngineY", const_cast<float&>(mLights[2].Position().y), "group = Lights");
	mAwManager->AddVariable("GameStats", "EngineZ", const_cast<float&>(mLights[2].Position().z), "group = Lights");
	mAwManager->AddWritableVariable("GameStats", "EngineColour", const_cast<XMFLOAT4&>(mLights[2].Colour()), "group = Lights");
}

/// <summary>
/// Creates the scene which the game will run
/// </summary>
void Game::CreateScene()
{
	InitialiseLights();

#ifdef _DEBUG
	mTerrainX = 100;
	mTerrainY = 20;
	mTerrainZ = 20;
	mTerrainScale = 1.5f;
	mRocketSpeed = 1.0f;
	mExplosionRadius = 5.0f;
#endif

	//reserving space for gameobjects so that memory only needs to be allocated once - better efficiency and avoid pointer invalidation
	mGameObjects.reserve(10);
	mLights.reserve(5);
	//Create environment cube map
	//						Scale						Rotate					Translate
	GameObject env(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1));
	env.AddShape(nullptr, XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), wstring(L"desertSkybox.dds"), wstring(L""), wstring(L""), wstring(L"environmentShader.fx"), "EnvironmentMap", true, false, GeometryType::CUBE);
	mGameObjects.emplace_back(env);

	//launcher
	//							Scale						Rotate					Translate
	GameObject launcher(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(-(mTerrainScale*mTerrainX) * 4 / 10, 0, 0, 1));
	launcher.AddShape(nullptr, XMFLOAT4(4, 2, 4, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, -0.5f, 0, 1), wstring(L"corrugated_metal.dds"), wstring(L""), wstring(L""), wstring(L"defaultShader.fx"), "LauncherBase", false, false, GeometryType::CUBE);
	launcher.AddShape(nullptr, XMFLOAT4(0.2f, 4, 0.2f, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 2.5f, 0, 1), wstring(L"corrugated_metal.dds"), wstring(L""), wstring(L""), wstring(L"defaultShader.fx"), "LauncherPole", false, false, GeometryType::CUBE);
	mGameObjects.emplace_back(launcher);

	//Create Terrain (Instanced)
	//							Scale												Rotate				Translate
	GameObject terrain(XMFLOAT4(mTerrainScale, mTerrainScale, mTerrainScale, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(-(mTerrainScale*mTerrainX) / 2, -(mTerrainScale*mTerrainY), -(mTerrainScale*mTerrainZ) / 2, 1));

	vector<Instance> instances;

	for (auto x = 0; x < mTerrainX; ++x)
	{
		for (auto y = 0; y < mTerrainY; ++y)
		{
			for (auto z = 0; z < mTerrainZ; ++z)
			{
				instances.emplace_back(Instance{ XMFLOAT3(x,y,z) });
			}
		}
	}

	//Instanced Cube
	//									Scale			Rotate				Translate
	terrain.AddShape(&instances, XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), wstring(L"desert.dds"), wstring(L"desert_norm.dds"), wstring(L"desert_height.dds"), wstring(L"instanceParallaxShader.fx"), "TerrainCube", false, false, GeometryType::CUBE);
	mGameObjects.emplace_back(terrain);

	//Rocket object
	//							Scale				Rotate				Translate
	GameObject rocket(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(-(mTerrainScale*mTerrainX) * 4 / 10, 3, 0, 1));
	//body
	rocket.AddShape(nullptr, XMFLOAT4(0.5f, 5, 0.5f, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), wstring(L"corrugated_metal.dds"), wstring(L"corrugated_metal_norm.dds"), wstring(L"corrugated_metal_height.dds"), wstring(L"parallaxShader.fx"), "RocketBody", false, false, GeometryType::CYLINDER);
	//cone
	rocket.AddShape(nullptr, XMFLOAT4(0.75f, 2, 0.75f, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 3, 0, 1), wstring(L"desertSkybox.dds"), wstring(L""), wstring(L""), wstring(L"chromeShader.fx"), "RocketCone", false, false, GeometryType::CONE);
	
	//Create particles
	//Create instances
	vector<Instance> engineParticles;
	for (auto i = 0; i < 2000; ++i)
	{
		engineParticles.emplace_back(Instance{ XMFLOAT3(0,0,i) });
	}
	//Create object + shape
	rocket.AddShape(&engineParticles, XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), wstring(L"stones.dds"), wstring(L""), wstring(L""), wstring(L"engineParticleShader.fx"), "Particles", false, true, GeometryType::QUAD);
	mGameObjects.emplace_back(rocket);

	//This pointer will be invalidated if the vector is ever reallocated (must not add over the capacity or the memory will be reallocated)
	mEnvironment = &mGameObjects[0];
	mLauncher = &mGameObjects[1];
	mTerrain = &mGameObjects[2];
	mRocket = &mGameObjects[3];

	InitialiseCameras();
}

/// <summary>
/// Uses directxtk keyboard to handle input controls
/// </summary>
void Game::HandleInput(const double& pDt)
{
	const auto state = mKeyboard->GetState();
	mTracker.Update(state);
	//‘ESC’ exits the application 
	if (state.Escape)
	{
		mExit = true;
	}
	//‘r’ resets the application to its initial state 
	if (state.R)
	{
		ResetGame();
	}
	if (mActiveCamera->Controllable())
	{
		//CTRL + ‘left’ / ‘right’ / ’up’ / ’down’ / ’page up’ / ’page down’ panning to left / right / forward / backward / up / down, respectively 
		if (state.LeftControl || state.RightControl)
		{
			if (state.Up)
			{
				auto forward = XMFLOAT4(0, 0, 0, 1);
				XMStoreFloat4(&forward, XMLoadFloat4(&mActiveCamera->Forward()) * mCameraSpeed * pDt);
				mActiveCamera->TranslateCam(forward);
			}
			if (state.Down)
			{
				auto forward = XMFLOAT4(0, 0, 0, 1);
				XMStoreFloat4(&forward, XMLoadFloat4(&mActiveCamera->Forward()) * mCameraSpeed * pDt);
				mActiveCamera->TranslateCam(XMFLOAT4(-forward.x, -forward.y, -forward.z, forward.w));
			}
			if (state.Right)
			{
				auto right = XMFLOAT4(0, 0, 0, 1);
				XMStoreFloat4(&right, XMLoadFloat4(&mActiveCamera->Right()) * mCameraSpeed * pDt);
				mActiveCamera->TranslateCam(right);
			}
			if (state.Left)
			{
				auto right = XMFLOAT4(0, 0, 0, 1);
				XMStoreFloat4(&right, XMLoadFloat4(&mActiveCamera->Right()) * mCameraSpeed * pDt);
				mActiveCamera->TranslateCam(XMFLOAT4(-right.x, -right.y, -right.z, right.w));
			}

			if (state.PageUp)
			{
				auto up = XMFLOAT4(0, 0, 0, 1);
				XMStoreFloat4(&up, XMLoadFloat4(&mActiveCamera->Up()) * mCameraSpeed * pDt);
				mActiveCamera->TranslateCam(up);
			}
			if (state.PageDown)
			{
				auto up = XMFLOAT4(0, 0, 0, 1);
				XMStoreFloat4(&up, XMLoadFloat4(&mActiveCamera->Up()) * mCameraSpeed * pDt);
				mActiveCamera->TranslateCam(XMFLOAT4(-up.x, -up.y, -up.z, up.w));
			}
		}
		//Cameras are controlled by the cursor keys : ‘left’ / ‘right’ / ’up’ / ’down’ rotate left / right / up / down, respectively 
		else
		{
			if (state.Up)
			{
				auto rotation = XMFLOAT3(XMConvertToRadians(-10), 0.0f, 0.0f);
				XMStoreFloat3(&rotation, XMLoadFloat3(&rotation) * mCameraSpeed * pDt);
				mActiveCamera->RotateCam(rotation);
			}
			if (state.Down)
			{
				auto rotation = XMFLOAT3(XMConvertToRadians(10), 0.0f, 0.0f);
				XMStoreFloat3(&rotation, XMLoadFloat3(&rotation) * mCameraSpeed * pDt);
				mActiveCamera->RotateCam(rotation);
			}
			if (state.Left)
			{
				auto rotation = XMFLOAT3(0.0f, XMConvertToRadians(-10), 0.0f);
				XMStoreFloat3(&rotation, XMLoadFloat3(&rotation) * mCameraSpeed * pDt);
				mActiveCamera->RotateCam(rotation);
			}
			if (state.Right)
			{
				auto rotation = XMFLOAT3(0.0f, XMConvertToRadians(10), 0.0f);
				XMStoreFloat3(&rotation, XMLoadFloat3(&rotation) * mCameraSpeed * pDt);
				mActiveCamera->RotateCam(rotation);
			}
		}
	}
	//s to toggle anttweakUI
	if (mTracker.pressed.S)
	{
		mAwManager->ToggleVisible();
	}
	//Function keys F1 to F5 will select cameras C1 to C5, respectively 
	if (state.F1)
	{
		mActiveCamera = &mCameras[0];
	}
	else if (state.F2)
	{
		mActiveCamera = &mCameras[1];
	}
	else if (state.F3)
	{
		mActiveCamera = &mCameras[2];
	}
	else if (state.F4)
	{
		mActiveCamera = &mCameras[3];
	}
	else if (state.F5)
	{
		mActiveCamera = &mCameras[4];
	}
	//‘<’ / ‘>’ to decrease / increase the pitch of the launcher.
	if (!mLaunch)
	{
		if (state.LeftShift || state.RightShift)
		{
			if (state.OemComma)
			{
				auto rotation = XMFLOAT4(0, 0, XMConvertToRadians(5), 1);
				XMStoreFloat4(&rotation, XMLoadFloat4(&rotation) * mCameraSpeed * pDt);
				mRocket->Rotate(rotation);
				mLauncher->RotateShape(1, rotation);
			}
			if (state.OemPeriod)
			{
				auto rotation = XMFLOAT4(0, 0, XMConvertToRadians(-5), 1);
				XMStoreFloat4(&rotation, XMLoadFloat4(&rotation) * mCameraSpeed * pDt);
				mRocket->Rotate(rotation);
				mLauncher->RotateShape(1, rotation);
			}
		}
	}
	//Function key F11, to launch the rocket.
	if (state.F11)
	{
		mLaunch = true;
	}
	//Keys 't' / 'T' decrease / increase a factor that globally slows / speeds - up time - dependent effects
	if (state.T)
	{
		if (state.LeftShift || state.RightShift)
		{
			mTimeScale += 0.1f;
		}
		else
		{
			mTimeScale -= 0.1f;
		}
	}
}

/// <summary>
/// Checks for collisions between the shape and the terrain
/// </summary>
/// <param name="pShape"></param>
void Game::CheckCollision(const Shape & pShape)
{
	const auto coneRadius = 0.5f;
	const auto cubeRadius = mTerrainScale / 2;

	XMFLOAT4X4 transform{};
	XMStoreFloat4x4(&transform, XMLoadFloat4x4(pShape.Transform()) * XMLoadFloat4x4(mRocket->Transform()));
	const auto conePosition = XMFLOAT4(transform._41, transform._42, transform._43, transform._44);
	const auto terrain = mTerrain->Shapes()[0].Instances();

	for (const auto& instance : terrain)
	{
		const auto cubePosition = XMVector3Transform(XMLoadFloat3(&instance.mPosition), XMLoadFloat4x4(mTerrain->Transform()));
		XMFLOAT4 distance{};
		XMStoreFloat4(&distance, XMVector4Length(XMLoadFloat4(&conePosition) - cubePosition));
		if (distance.x < coneRadius + cubeRadius)
		{
			ResetRocket();
			Explosion(transform);
			return;
		}
	}
}

/// <summary>
/// Delete the cubes in the explosion radius and create at light at the explosion location
/// </summary>
/// <param name="pTransform"> the transform of the point of explosion (the cone transform at collision) </param>
void Game::Explosion(const XMFLOAT4X4& pTransform)
{
	auto conePosition = XMFLOAT4(pTransform._41, pTransform._42, pTransform._43, pTransform._44);
	//Explosion light
	if (mLights.size() > 3)
	{

		mLights[3].SetTranslation(conePosition);
	}
	else
	{
		mLights.emplace_back(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), conePosition, XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0.6f, 0.2f, 0.1f, 1));
		mAwManager->AddVariable("GameStats", "ExplosionX", const_cast<float&>(mLights[3].Position().x), "group = Lights");
		mAwManager->AddVariable("GameStats", "ExplosionY", const_cast<float&>(mLights[3].Position().y), "group = Lights");
		mAwManager->AddVariable("GameStats", "ExplosionZ", const_cast<float&>(mLights[3].Position().z), "group = Lights");
		mAwManager->AddWritableVariable("GameStats", "ExplosionColour", const_cast<XMFLOAT4&>(mLights[3].Colour()), "group = Lights");
	}

	//Create particles
		//Create instances
	vector<Instance> instances;
	for (auto i = 0; i < 2000; ++i)
	{
		instances.emplace_back(Instance{ XMFLOAT3(0,0,i) });
	}
	//Create object + shape
	GameObject particles(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(conePosition.x, conePosition.y-3, conePosition.z, 1));
	particles.AddShape(&instances, XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), wstring(L"flame.dds"), wstring(L""), wstring(L""), wstring(L"explosionParticleShader.fx"), "Particles", false, true, GeometryType::QUAD);
	mGameObjects.emplace_back(particles);
	mParticleTimer = 10.0f;

	//Destroy terrain
	vector<Instance> indexToRemove;
	const auto terrain = mTerrain->Shapes()[0].Instances();
	for (const auto& instance : terrain)
	{
		const auto cubePosition = XMVector3Transform(XMLoadFloat3(&instance.mPosition), XMLoadFloat4x4(mTerrain->Transform()));
		XMFLOAT4 distance{};
		XMStoreFloat4(&distance, XMVector4Length(XMLoadFloat4(&conePosition) - cubePosition));
		if (distance.x < mExplosionRadius)
		{
			indexToRemove.push_back(instance);
		}
	}
	mTerrain->RemoveInstancesFromShape(0, indexToRemove);
}

/// <summary>
/// The game update loop
/// </summary>
/// <param name="pDt"> delta time used to update the scene </param>
void Game::Update(const double& pDt)
{
	mCubeCount = mTerrain->Shapes()[0].Instances().size();
	mTime += pDt;

	//smooth framerate over the a sample of deltatimes
	mAverageDt = 0;
	for (auto i = 0; i < 49; ++i)
	{
		mDeltaTimeSamples[i] = mDeltaTimeSamples[i + 1];
	}
	mDeltaTimeSamples[49] = pDt;

	for (const auto& deltaTimeSample : mDeltaTimeSamples)
	{
		mAverageDt += deltaTimeSample;
	}
	mAverageDt /= 50;
	mFrameRate = 1 / mAverageDt;

	if (mLaunch)
	{
		//Launch upwards
		XMFLOAT4 translation{};
		XMStoreFloat4(&translation, XMLoadFloat4(&mRocket->Up()) * mRocketSpeed * mTimeScale * pDt);
		mRocket->Translate(translation);
		//Rotate on the z-axis to curve back to the ground
		if (mRocket->Rotation().z > -(XM_PI * 8 / 10))
		{
			mRocket->Rotate(XMFLOAT4(0, 0, XMConvertToRadians(-2.5)*mTimeScale*pDt, 1));
		}
		else if (mRocket->Rotation().z > -XM_PI)
		{
			mRocket->Rotate(XMFLOAT4(0, 0, XMConvertToRadians(-1)*mTimeScale*pDt, 1));
		}
	}
	XMFLOAT4 enginePos{};
	const auto rocketPos = mRocket->Position();
	XMStoreFloat4(&enginePos, XMLoadFloat4(&rocketPos) - (XMLoadFloat4(&mRocket->Up()) * 5));
	mLights[2].SetTranslation(enginePos);

	HandleInput(pDt);
	if (mActiveCamera->Name() == "RocketConeCam")
	{
		XMFLOAT4X4 transform{};
		XMStoreFloat4x4(&transform, XMLoadFloat4x4(mRocket->Shapes()[1].Transform()) * XMLoadFloat4x4(mRocket->Transform()));
		const auto conePos = XMFLOAT4(transform._41, transform._42, transform._43, 1);
		mActiveCamera->LookAt(conePos);
		mActiveCamera->SetEye(XMFLOAT4(conePos.x + 1, conePos.y, -1.0f, 1.0f));
	}
	else if (mActiveCamera->Name() == "RocketBodyCam")
	{
		mActiveCamera->LookAt(mRocket->Position());
		mActiveCamera->SetEye(XMFLOAT4(mRocket->Position().x, mRocket->Position().y, -2.0f, 1.0f));
	}
	else if (mActiveCamera->Name() == "WideCam")
	{
		mActiveCamera->LookAt(mRocket->Position());
	}

	//Check collisions with the cone
	const auto rocket = mRocket->Shapes();
	for (const auto& shape : rocket)
	{
		if (shape.Name() == "RocketCone")
		{
			CheckCollision(shape);
		}
	}

	if (mParticleTimer > 0)
	{
		mParticleTimer -= pDt * mTimeScale;
	}
	if (mParticleTimer < 0 && mGameObjects.size() > 4)
	{
		mGameObjects.pop_back();
	}

	DayNightCycle(pDt);
}

/// <summary>
/// Gets the list of gameobjects in the game for the renderer
/// </summary>
/// <returns> list of gameobjects from the game </returns>
const std::vector<GameObject>& Game::GameObjects() const
{
	return mGameObjects;
}

/// <summary>
/// Getter for the games currently active camera
/// </summary>
/// <returns> pointer to the active camera </returns>
const Camera * const Game::Cam() const
{
	return mActiveCamera;
}

/// <summary>
/// Getter for the renderer to get all lights in the scene
/// </summary>
/// <returns> the lights in the scene as a vector </returns>
const vector<Light> & Game::Lights() const
{
	return mLights;
}

const float Game::ScaledTime() const
{
	return mTime * mTimeScale;
}

/// <summary>
/// Reset the rocket to its starting position and prepare for another launch
/// </summary>
void Game::ResetRocket()
{
	mLaunch = false;
	mRocket->ResetObject();
	mRocket->Translate(XMFLOAT4(-(mTerrainScale*mTerrainX) * 4 / 10, 3, 0, 1));
	mLauncher->SetShapeRotation(1, XMFLOAT4(0, 0, 0, 1));
}

/// <summary>
/// Setup the initial lights for the scene
/// </summary>
void Game::InitialiseLights()
{
	//The Sun
	mLights.emplace_back(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, (mTerrainScale*mTerrainX / 2) + 10, 0, 1), XMFLOAT4(0.6f, 0.4f, 0.1f, 1));
	//The Moon
	mLights.emplace_back(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, -((mTerrainScale*mTerrainX / 2) + 10), 0, 1), XMFLOAT4(0.2f, 0.2f, 0.7f, 1));

	//Rocket Engine
	mLights.emplace_back(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0.4f, 0.1f, 0.1f, 1));
}

/// <summary>
/// Setup the 5 initial cameras according to spec
/// </summary>
void Game::InitialiseCameras()
{
	mCameras.emplace_back(XMFLOAT4(mLauncher->Position().x, mLauncher->Position().y, -5.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		mWidth,
		mHeight,
		true,
		"LauncherCam");

	mCameras.emplace_back(XMFLOAT4(0, 50, 0.0f, 1.0f),
		XMFLOAT4(XM_PIDIV2, 0, 0, 1),
		mWidth,
		mHeight,
		true,
		"TerrainCam");

	mCameras.emplace_back(XMFLOAT4(0, 1.0f, -20.0f, 1.0f),
		XMFLOAT4(0, 0, 0, 1),
		mWidth,
		mHeight,
		false,
		"WideCam");

	XMFLOAT4X4 transform{};
	XMStoreFloat4x4(&transform, XMLoadFloat4x4(mRocket->Shapes()[1].Transform()) * XMLoadFloat4x4(mRocket->Transform()));
	const auto conePos = XMFLOAT4(transform._41, transform._42, transform._43, 1);
	mCameras.emplace_back(XMFLOAT4(conePos.x + 1, conePos.y, -1.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		mWidth,
		mHeight,
		false,
		"RocketConeCam");

	mCameras.emplace_back(XMFLOAT4(mRocket->Position().x, mRocket->Position().y, -2.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
		mWidth,
		mHeight,
		false,
		"RocketBodyCam");

	// Initialize the camera
	mActiveCamera = &mCameras[0];
}

/// <summary>
/// Orbit the lights to create a day + night cycle
/// </summary>
/// <param name="pDt">delta time used to move the lights </param>
void Game::DayNightCycle(const float pDt)
{
	//Sun
	mLights[0].Orbit(XMFLOAT4(0, 0, -0.05f * mTimeScale * pDt, 1));
	//Moon
	mLights[1].Orbit(XMFLOAT4(0, 0, -0.05f * mTimeScale * pDt, 1));
}

/// <summary>
/// Resets the game to its initial position
/// </summary>
void Game::ResetGame()
{
	ResetRocket();

	vector<Instance> instances;
	for (auto x = 0; x < mTerrainX; ++x)
	{
		for (auto y = 0; y < mTerrainY; ++y)
		{
			for (auto z = 0; z < mTerrainZ; ++z)
			{
				instances.emplace_back(Instance{ XMFLOAT3(x,y,z) });
			}
		}
	}
	mTerrain->SetShapeInstances(0, instances);
	mLights.clear();
	InitialiseLights();
	mCameras.clear();
	InitialiseCameras();
	mTimeScale = 5.0f;
}

/// <summary>
/// Gets a bool which marks if the user has selected to exit the game
/// </summary>
/// <returns> true if the game is to be exit </returns>
const bool & Game::Exit() const
{
	return mExit;
}


