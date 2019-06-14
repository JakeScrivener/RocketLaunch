#include "GameObject.h"

using namespace DirectX;
using namespace std;

/// <summary>
/// Constructor for the gameobject
/// </summary>
GameObject::GameObject(const XMFLOAT4& pScale, const XMFLOAT4&pRotation, const XMFLOAT4& pTranslation) : mScale(pScale), mRotation(pRotation), mTranslation(pTranslation)
{
	SetTransform();
}

/// <summary>
/// destructor for the gameobject
/// </summary>
GameObject::~GameObject() = default;

/// <summary>
/// Translate the gameobject by the given translation
/// </summary>
/// <param name="pTranslation"> the matrix to translate by  </param>
void GameObject::Translate(const XMFLOAT4& pTranslation)
{
	XMStoreFloat4(&mTranslation, XMLoadFloat4(&mTranslation) + XMLoadFloat4(&pTranslation));
	SetTransform();
}

/// <summary>
/// Sets the translation of the gameobject
/// </summary>
/// <param name="pTranslation"> the translation to give the gameobject </param>
void GameObject::SetTranslation(const DirectX::XMFLOAT4 & pTranslation)
{
	XMStoreFloat4(&mTranslation, XMLoadFloat4(&pTranslation));
	SetTransform();
}

/// <summary>
/// Accessor for the rotation of the gameobject
/// </summary>
/// <returns> the rotation of the gameobject on each axis </returns>
const DirectX::XMFLOAT4 & GameObject::Rotation() const
{
	return mRotation;
}

/// <summary>
/// Gets the position of the gameobject from the transform matrix
/// </summary>
/// <returns> a position vector for the gameobject </returns>
const DirectX::XMFLOAT4 & GameObject::Position() const
{
	return mPosition;

}

/// <summary>
/// Rotate the gameobject by the given rotation
/// </summary>
/// <param name="pRotation"> the matrix to rotate by </param>
void GameObject::Rotate(const XMFLOAT4& pRotation)
{
	XMStoreFloat4(&mRotation, XMLoadFloat4(&mRotation) + XMLoadFloat4(&pRotation));
	SetTransform();
}

/// <summary>
/// Scale the gameobject by the given scale
/// </summary>
/// <param name="pScale"> the matrix to scale by </param>
void GameObject::Scale(const XMFLOAT4& pScale)
{
	XMStoreFloat4(&mScale, XMLoadFloat4(&mScale) + XMLoadFloat4(&pScale));
	SetTransform();
}

/// <summary>
/// Gets the list of shapes for the gameobject
/// </summary>
/// <returns> a reference to the list of shapes in the gameobject </returns>
const std::vector<Shape> & GameObject::Shapes() const
{
	return mShapes;
}

/// <summary>
/// Adds a shape to the gameobjects list
/// </summary>
/// <param name="pInstances"> The position of all of the instances of this shape </param>
/// <param name="pTranslation"> The local translation of the shape</param>
/// <param name="pRotation"> the local rotation of the shape </param>
/// <param name="pScale"> The local scale of the shape </param>
/// <param name="pDiffuseTex"> The diffuse texture of the shape </param>
/// <param name="pNormalMap"> The normal map of the shape </param>
/// <param name="pHeightMap"> The height map of the shape </param>
/// <param name="pShader"> The shader used to draw the shape </param>
/// <param name="pName"> The name of this shape </param>
/// <param name="pGeometryType"> The type of geometry which the shape uses </param>
/// <param name="pEnvironment"> The flag to show if this is a skybox </param>
/// <param name="pBlended"> The flag to show if this shape should be alpha blended </param>
void GameObject::AddShape(const vector<Instance> * const pInstances,
	const XMFLOAT4& pScale,
	const XMFLOAT4& pRotation,
	const XMFLOAT4& pTranslation,
	const wstring& pDiffuseTex,
	const wstring& pNormalMap,
	const wstring& pHeightMap,
	const wstring& pShader,
	const string& pName,
	const bool& pEnvironment,
	const bool& pBlended,
	const GeometryType& pGeometryType )
{
	mShapes.emplace_back(pInstances, pScale, pRotation, pTranslation, pDiffuseTex, pNormalMap, pHeightMap, pShader, pName, pEnvironment, pBlended, pGeometryType);
}

/// <summary>
/// Gets the transform of the gameobject
/// </summary>
/// <returns> a pointer to the transform of the gameobject </returns>
const XMFLOAT4X4 * const GameObject::Transform() const
{
	return &mTransform;
}

/// <summary>
/// Returns the forward vector of the gameobject
/// </summary>
/// <returns> the objects forward vector </returns>
const XMFLOAT4 & GameObject::Forward() const
{
	return mForward;
}

/// <summary>
/// Returns the up vector of the gameobject
/// </summary>
/// <returns> the objects up vector </returns>
const XMFLOAT4 & GameObject::Up() const
{
	return mUp;
}

/// <summary>
/// Returns the right vector of the gameobject
/// </summary>
/// <returns> the objects right vector </returns>
const XMFLOAT4 & GameObject::Right() const
{
	return mRight;
}

/// <summary>
/// Reset the gameobjects transform
/// </summary>
void GameObject::ResetObject()
{
	mRotation = XMFLOAT4(0, 0, 0, 1);
	mScale = XMFLOAT4(1, 1, 1, 1);
	mTranslation = XMFLOAT4(0, 0, 0, 1);
	SetTransform();
}

/// <summary>
/// Rotate the given shape by its index
/// </summary>
/// <param name="pIndex"> the index of the shape to rotate </param>
/// <param name="pRotation"> the rotation to rotate the shape by </param>
void GameObject::RotateShape(const int & pIndex, const DirectX::XMFLOAT4 & pRotation)
{
	mShapes[pIndex].Rotate(pRotation);
}

/// <summary>
/// Sets the rotation of the given shape
/// </summary>
/// <param name="pIndex"> the index of the shape which will have its rotation set </param>
/// <param name="pRotation"> the rotation to set the shape to </param>
void GameObject::SetShapeRotation(const int & pIndex, const DirectX::XMFLOAT4 & pRotation)
{
	mShapes[pIndex].SetRotation(pRotation);
}

/// <summary>
/// Remove instances from the given shape
/// </summary>
/// <param name="pIndex"> the index of the shape to remove instances of </param>
/// <param name="pInstances"> the list of instances to remove </param>
void GameObject::RemoveInstancesFromShape(const int & pIndex, const std::vector<Instance>& pInstances)
{
	mShapes[pIndex].RemoveInstances(pInstances);
}

/// <summary>
/// Set the instance list of a given shape
/// </summary>
/// <param name="pIndex"> the index of the shape which will have its instance list set </param>
/// <param name="pInstances"> the instance list to set in the given shape </param>
void GameObject::SetShapeInstances(const int & pIndex, const std::vector<Instance>& pInstances)
{
	mShapes[pIndex].SetInstances(pInstances);
}

/// <summary>
/// Sets the transform based on the translation, rotation and scale
/// </summary>
void GameObject::SetTransform()
{	
	//Reset
	XMStoreFloat4x4(&mTransform, XMMatrixIdentity());
	//Scale
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixScalingFromVector(XMLoadFloat4(&mScale)));
	//Rotate - TODO: Replace with Quaternions
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixRotationX(mRotation.x) * XMMatrixRotationY(mRotation.y) * XMMatrixRotationZ(mRotation.z));
	//Translate
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixTranslationFromVector(XMLoadFloat4(&mTranslation)));
	//Right
	const auto right = XMFLOAT4(mTransform._11, mTransform._12, mTransform._13, 1.0f);
	XMStoreFloat4(&mRight, XMVector4Normalize(XMLoadFloat4(&right)));
	//Up
	const auto up = XMFLOAT4(mTransform._21, mTransform._22, mTransform._23, 1.0f);
	XMStoreFloat4(&mUp, XMVector4Normalize(XMLoadFloat4(&up)));
	//Forward
	const auto forward = XMFLOAT4(mTransform._31, mTransform._32, mTransform._33, 1.0f);
	XMStoreFloat4(&mForward, XMVector4Normalize(XMLoadFloat4(&forward)));
	//Position
	mPosition = XMFLOAT4(mTransform._41, mTransform._42, mTransform._43, 1);
}


