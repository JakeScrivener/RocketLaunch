#include "AntTweakManager.h"
#include <directxmath.h>

using namespace std;
using namespace DirectX;

/// <summary>
/// Initialise anttweak by giving it the device and window dimensions
/// </summary>
/// <param name="pDevice"> the device to set the anttweakbars with </param>
/// <param name="pWidth"> the width of the window </param>
/// <param name="pHeight"> the height of the window </param>
void AntTweakManager::Init(ID3D11Device* const pDevice, const UINT pWidth, const UINT pHeight) const
{
	TwInit(TW_DIRECT3D11, pDevice);
	TwWindowSize(pWidth, pHeight);
}

/// <summary>
/// Cleanup and delete the bars
/// </summary>
void AntTweakManager::Cleanup()
{
	//uninitialise and delete the bar
	mBars.clear();
	TwDeleteAllBars();
	TwTerminate();
}

/// <summary>
/// Add an anttweak bar
/// </summary>
/// <param name="pName"> the name to be given to the new bar </param>
void AntTweakManager::AddBar(const string& pName)
{
	//Add to the dictionary
	mBars.insert(pair<string, TwBar*>(pName, TwNewBar(pName.c_str())));
	const auto str = pName + " iconified=true";
	TwDefine(str.c_str());
}

/// <summary>
/// Add a variable to the bar, the type is inferred from what is passed, overloaded for different types
/// </summary>
/// <param name="pBarName"> the name of the bar to add a variable to </param>
/// <param name="pVarName"> the name of the new variable </param>
/// <param name="pVariable"> a reference to the variable to add </param>
/// <param name="pParameters"> a string containing various anttweak parameters </param>
void AntTweakManager::AddVariable(const std::string & pBarName, const std::string & pVarName, const float & pVariable, const std::string & pParameters)
{
	const auto it = mBars.find(pBarName);

	if (it != mBars.end())
	{
		TwAddVarRO(it->second, pVarName.c_str(), TW_TYPE_FLOAT, reinterpret_cast<void*>(&const_cast<float&>(pVariable)), pParameters.c_str());
	}
}
void AntTweakManager::AddVariable(const std::string & pBarName, const std::string & pVarName, const int & pVariable, const std::string & pParameters)
{
	const auto it = mBars.find(pBarName);

	if (it != mBars.end())
	{
		TwAddVarRO(it->second, pVarName.c_str(), TW_TYPE_INT32, reinterpret_cast<void*>(&const_cast<int&>(pVariable)), pParameters.c_str());
	}
}

/// <summary>
/// Add a editable variable to the bar, the type is inferred from what is passed, overloaded for different types
/// </summary>
/// <param name="pBarName"> the name of the bar to add a variable to </param>
/// <param name="pVarName"> the name of the new variable </param>
/// <param name="pVariable"> a reference to the variable to add </param>
/// <param name="pParameters"> a string containing various anttweak parameters </param>
void AntTweakManager::AddWritableVariable(const std::string & pBarName, const std::string & pVarName, const float & pVariable, const std::string & pParameters)
{
	const auto it = mBars.find(pBarName);

	if (it != mBars.end())
	{
		TwAddVarRW(it->second, pVarName.c_str(), TW_TYPE_FLOAT, reinterpret_cast<void*>(&const_cast<float&>(pVariable)), pParameters.c_str());
	}
}
void AntTweakManager::AddWritableVariable(const std::string & pBarName, const std::string & pVarName, const int & pVariable, const std::string & pParameters)
{
	const auto it = mBars.find(pBarName);

	if (it != mBars.end())
	{
		TwAddVarRW(it->second, pVarName.c_str(), TW_TYPE_INT32, reinterpret_cast<void*>(&const_cast<int&>(pVariable)), pParameters.c_str());
	}
}
void AntTweakManager::AddWritableVariable(const std::string & pBarName, const std::string & pVarName, const XMFLOAT4 & pVariable, const std::string & pParameters)
{
	const auto it = mBars.find(pBarName);

	if (it != mBars.end())
	{
		TwAddVarRW(it->second, pVarName.c_str(), TW_TYPE_COLOR4F, reinterpret_cast<void*>(&const_cast<XMFLOAT4&>(pVariable)), pParameters.c_str());
	}
}

void AntTweakManager::ToggleVisible()
{
	mHide = !mHide;
	for (auto& bar : mBars)
	{
		string hiddenBool;
		if (mHide)
		{
			hiddenBool = "true";
		}
		else
		{
			hiddenBool = "false";
		}
		const auto str = bar.first + " iconified=" + hiddenBool;
		TwDefine(str.c_str());
	}
}

/// <summary>
/// Draw the anttweak bars
/// </summary>
void AntTweakManager::DrawBars()
{
	for (auto& bar : mBars)
	{
		TwRefreshBar(bar.second);
		TwDraw();
	}
}
