#include "Light.h"

using namespace DirectX;

Light::Light(const XMFLOAT4& pScale, const XMFLOAT4& pRotation, const XMFLOAT4& pTranslation, const XMFLOAT4& pOrbit, const DirectX::XMFLOAT4& pOrbitTranslation, const XMFLOAT4& pColour) :
	mScale(pScale),
	mRotation(pRotation),
	mTranslation(pTranslation),
	mOrbit(pOrbit),
	mOrbitTranslation(pOrbitTranslation),
	mColour(pColour)
{
	SetTransform();
}

/// <summary>
/// Set the transform matrix based on the translation, rotation and scale
/// </summary>
void Light::SetTransform()
{
	//Reset
	XMStoreFloat4x4(&mTransform, XMMatrixIdentity());
	//Scale
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixScalingFromVector(XMLoadFloat4(&mScale)));
	//Rotate - TODO: Replace with Quaternions
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixRotationX(mRotation.x) * XMMatrixRotationY(mRotation.y) * XMMatrixRotationZ(mRotation.z));
	//OrbitTranslate
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixTranslationFromVector(XMLoadFloat4(&mOrbitTranslation)));
	//Orbit
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixRotationX(mOrbit.x) * XMMatrixRotationY(mOrbit.y) * XMMatrixRotationZ(mOrbit.z));
	//Translate
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixTranslationFromVector(XMLoadFloat4(&mTranslation)));

	mPosition = XMFLOAT4(mTransform._41, mTransform._42, mTransform._43, 1);
}

/// <summary>
/// Translates the shape by the given translation
/// </summary>
/// <param name="pTranslation"> the matrix to translate by </param>
void Light::Translate(const XMFLOAT4& pTranslation)
{
	XMStoreFloat4(&mTranslation, XMLoadFloat4(&mTranslation) + XMLoadFloat4(&pTranslation));
	SetTransform();
}

/// <summary>
/// Rotates the shape by the given rotation
/// </summary>
/// <param name="pRotation"> the matrix to rotate by </param>
void Light::Rotate(const XMFLOAT4& pRotation)
{
	XMStoreFloat4(&mRotation, XMLoadFloat4(&mRotation) + XMLoadFloat4(&pRotation));
	SetTransform();
}

/// <summary>
/// Sets the orbit rotation for the light (used for day-night cycle)
/// </summary>
/// <param name="pRotation"> the rotation of the light in its orbit </param>
void Light::Orbit(const DirectX::XMFLOAT4 & pRotation)
{
	XMStoreFloat4(&mOrbit, XMLoadFloat4(&mOrbit) + XMLoadFloat4(&pRotation));
	SetTransform();
}

/// <summary>
/// Used to offset the light by an amount to orbit
/// </summary>
/// <param name="pTranslation"> the offset away from the orbit origin </param>
void Light::OrbitTranslate(const DirectX::XMFLOAT4 & pTranslation)
{
	XMStoreFloat4(&mOrbitTranslation, XMLoadFloat4(&mOrbitTranslation) + XMLoadFloat4(&pTranslation));
	SetTransform();
}

/// <summary>
/// Scales the shape by the given scale
/// </summary>
/// <param name="pScale"> the matrix to scale by </param>
void Light::Scale(const XMFLOAT4& pScale)
{
	XMStoreFloat4(&mScale, XMLoadFloat4(&mScale) + XMLoadFloat4(&pScale));
	SetTransform();
}

/// <summary>
/// Set the tranlation of the light
/// </summary>
/// <param name="pTranslation"> the translation to move the light by </param>
void Light::SetTranslation(const DirectX::XMFLOAT4 & pTranslation)
{
	mTranslation = pTranslation;
	SetTransform();
}

/// <summary>
/// Get the current position of the light from the transform matrix 
/// </summary>
/// <returns> a float4 representing the lights current position in the world </returns>
const XMFLOAT4 & Light::Position() const
{
	return mPosition;
}

/// <summary>
/// Returns the transform of the shape
/// </summary>
/// <returns> a pointer to the transform of the shape </returns>
const DirectX::XMFLOAT4X4 * const Light::Transform() const
{
	return &mTransform;
}

/// <summary>
/// Set the colour of the light
/// </summary>
/// <param name="pColour"> the colour to set the light to </param>
void Light::SetColour(const XMFLOAT4 & pColour)
{
	mColour = pColour;
}

/// <summary>
/// Gets the current colour of the light
/// </summary>
/// <returns> the colour of the light </returns>
const XMFLOAT4 & Light::Colour() const
{
	return mColour;
}

/// <summary>
/// Gets the current orbit of the light
/// </summary>
/// <returns> the rotation of the light around its orbit point </returns>
const DirectX::XMFLOAT4 & Light::GetOrbit() const
{
	return mOrbit;
}
