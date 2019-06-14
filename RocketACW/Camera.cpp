#include "Camera.h"

using namespace DirectX;
using namespace std;

/// <summary>
/// Constructor for the camera
/// </summary>
/// <param name="pEye"> the eye/camera position </param>
/// <param name="pAt"> the position which the camera is looking at </param>
/// <param name="pUp"> the up vector of the camera </param>
/// <param name="pWidth"> the width of the camera view </param>
/// <param name="pHeight"> the height of the camera view </param>
/// <param name="pControllable"> is the camera controllable </param>
Camera::Camera(const XMFLOAT4& pEye, const XMFLOAT4& pRotation, const float pWidth, const float pHeight, const bool& pControllable, const string& pName) : mEye(pEye), mName(pName), mControllable(pControllable)
{
	XMMATRIX transform = XMMatrixTranslationFromVector(XMLoadFloat4(&mEye));
	transform *= XMMatrixRotationX(pRotation.x)  * XMMatrixRotationY(pRotation.y);
	XMStoreFloat4x4(&mTransform, transform);
	SetDirections();
	SetView();
	SetProj(pWidth, pHeight);
}

/// <summary>
/// the default destructor 
/// </summary>
Camera::~Camera() = default;

/// <summary>
/// Sets the view matrix of the camera based on the eye, at and up vector
/// </summary>
void Camera::SetView()
{
	if (!mControllable)
	{
		XMStoreFloat4x4(&mView, XMMatrixLookAtLH(XMLoadFloat4(&mEye), XMLoadFloat4(&mLookAt), XMLoadFloat4(&mUp)));
	}
	else
	{
		XMStoreFloat4x4(&mView, XMMatrixLookAtLH(XMLoadFloat4(&mEye), XMLoadFloat4(&mEye) + XMLoadFloat4(&mForward), XMLoadFloat4(&mUp)));
	}
}

/// <summary>
/// Sets the direction vectors of the camera for movement purposes
/// </summary>
void Camera::SetDirections()
{
	const auto right = XMFLOAT4(mTransform._11, mTransform._12, mTransform._13, 1.0f);
	XMStoreFloat4(&mRight, XMVector4Normalize(XMLoadFloat4(&right)));

	const auto up = XMFLOAT4(mTransform._21, mTransform._22, mTransform._23, 1.0f);
	XMStoreFloat4(&mUp, XMVector4Normalize(XMLoadFloat4(&up)));

	const auto forward = XMFLOAT4(mTransform._31, mTransform._32, mTransform._33, 1.0f);
	XMStoreFloat4(&mForward, XMVector4Normalize(XMLoadFloat4(&forward)));
}

/// <summary>
/// Sets the project matrix of the camera
/// </summary>
/// <param name="pWidth"> the width of the window to fit the camera to </param>
/// <param name="pHeight"> the height of the window to fit the camera to </param>
void Camera::SetProj(const float pWidth, const float pHeight)
{
	XMStoreFloat4x4(&mProjection, XMMatrixPerspectiveFovLH(XM_PIDIV2, pWidth / pHeight, 0.01f, 100.0f));
}

/// <summary>
/// Rotate the camera around on its orgin
/// </summary>
/// <param name="pRotation"> a float3 representing the roation around the x,y and z axis </param>
void Camera::RotateCam(const DirectX::XMFLOAT3 & pRotation)
{
	const auto rot = XMMatrixRotationAxis(XMLoadFloat4(&mRight), pRotation.x) * XMMatrixRotationY(pRotation.y);
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * rot);

	SetDirections();
	SetView();
}

/// <summary>
/// Translate the eye position and at position for a free moving camera
/// </summary>
/// <param name="pTranslation"> The translation by which to move the camera </param>
void Camera::TranslateCam(const DirectX::XMFLOAT4 & pTranslation)
{
	const auto trans = XMLoadFloat3(&XMFLOAT3(pTranslation.x, pTranslation.y, pTranslation.z));
	XMStoreFloat4(&mEye, XMLoadFloat4(&mEye) + trans);
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixTranslationFromVector(XMLoadFloat4(&mEye)));

	SetView();
}

/// <summary>
/// Sets the target which the camera should look at
/// </summary>
/// <param name="pLookAt"> a vector representing the point in space which the camera should look at </param>
void Camera::LookAt(const XMFLOAT4& pLookAt)
{
	mLookAt = pLookAt;
	SetView();
}

/// <summary>
/// Gets the view matrix of the camera
/// </summary>
/// <returns> a reference to the view matrix </returns>
const XMFLOAT4X4& Camera::View() const
{
	return mView;
}

/// <summary>
/// Gets the projection matrix of the camera
/// </summary>
/// <returns> a reference to the projection matrix </returns>
const XMFLOAT4X4& Camera::Proj() const
{
	return mProjection;
}

/// <summary>
/// Gets the bool which flags the camera as controllable
/// </summary>
/// <returns> a bool to mark if the camera is controllable </returns>
const bool& Camera::Controllable() const
{
	return mControllable;
}

/// <summary>
/// Gets the name of the shape
/// </summary>
/// <returns> the name given to the shape </returns>
const std::string & Camera::Name() const
{
	return mName;
}

/// <summary>
/// Gets the eye position of the camera
/// </summary>
/// <returns> A reference to a vector holding the eye position </returns>
const XMFLOAT4& Camera::Eye() const
{
	return mEye;
}

/// <summary>
/// Gets the forward vector for the camera
/// </summary>
/// <returns> the forward vector</returns>
const DirectX::XMFLOAT4& Camera::Forward() const
{
	return mForward;
}

/// <summary>
/// Gets the up vector for the camera
/// </summary>
/// <returns> the up vector </returns>
const DirectX::XMFLOAT4& Camera::Up() const
{
	return mUp;
}

/// <summary>
/// Gets the right vector for the camera
/// </summary>
/// <returns> the right vector for the camera </returns>
const DirectX::XMFLOAT4& Camera::Right() const
{
	return mRight;
}
