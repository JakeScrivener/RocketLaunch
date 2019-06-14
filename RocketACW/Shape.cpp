#include "Shape.h"

using namespace DirectX;
using namespace std;

/// <summary>
/// Constructor for the shape object
/// </summary>
/// <param name="pTranslation"> the translation of the shape  </param>
/// <param name="pRotation"> the rotation of the shape </param>
/// <param name="pScale"> the scale of the shape </param>
/// <param name="pDiffuseTex"> the diffuse/colour texture of the shape </param>
/// <param name="pNormalMap"> the normal map for the shape </param>
/// <param name="pHeightMap"> the height map for the shape </param>
/// <param name="pShader"> the shader to be used by to draw the shape </param>
/// <param name="pInstances"> the position of all instances of this shape </param>
/// <param name="pIsEnvironment"> bool which marks if this shape is an environment map </param>
/// <param name="pBlended"> bool which marks if this shape should be alpha blended </param>
/// <param name="pGeometryType"> the type of the geometry </param>
Shape::Shape(const std::vector<Instance> * const pInstances, const XMFLOAT4& pScale, const XMFLOAT4& pRotation, const XMFLOAT4& pTranslation,
	const wstring& pDiffuseTex, const wstring& pNormalMap, const wstring& pHeightMap,
	const wstring& pShader, const string& pName, const bool& pIsEnvironment, const bool& pBlended, const GeometryType& pGeometryType) :
	mScale(pScale),
	mRotation(pRotation),
	mTranslation(pTranslation),
	mDiffuseTexture(pDiffuseTex),
	mNormalMap(pNormalMap),
	mHeightMap(pHeightMap),
	mShader(pShader),
	mName(pName),
	mIsEnvironment(pIsEnvironment),
	mBlended(pBlended),
	mGeometryType(pGeometryType)
{
	if (pInstances)
	{
		mInstances = *pInstances;
	}
	SetGeometry(mGeometryType);
	SetTransform();
}

/// <summary>
/// Set the transform matrix based on the translation, rotation and scale
/// </summary>
void Shape::SetTransform()
{
	//Reset
	XMStoreFloat4x4(&mTransform, XMMatrixIdentity());
	//Scale
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixScalingFromVector(XMLoadFloat4(&mScale)));
	//Rotate - TODO: Replace with Quaternions
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixRotationX(mRotation.x) * XMMatrixRotationY(mRotation.y) * XMMatrixRotationZ(mRotation.z));
	//Translate
	XMStoreFloat4x4(&mTransform, XMLoadFloat4x4(&mTransform) * XMMatrixTranslationFromVector(XMLoadFloat4(&mTranslation)));
}

/// <summary>
/// Translates the shape by the given translation
/// </summary>
/// <param name="pTranslation"> the matrix to translate by </param>
void Shape::Translate(const XMFLOAT4& pTranslation)
{
	XMStoreFloat4(&mTranslation, XMLoadFloat4(&mTranslation) + XMLoadFloat4(&pTranslation));
	SetTransform();
}

/// <summary>
/// Rotates the shape by the given rotation
/// </summary>
/// <param name="pRotation"> the matrix to rotate by </param>
void Shape::Rotate(const XMFLOAT4& pRotation)
{
	XMStoreFloat4(&mRotation, XMLoadFloat4(&mRotation) + XMLoadFloat4(&pRotation));
	SetTransform();
}

/// <summary>
/// Scales the shape by the given scale
/// </summary>
/// <param name="pScale"> the matrix to scale by </param>
void Shape::Scale(const XMFLOAT4& pScale)
{
	XMStoreFloat4(&mScale, XMLoadFloat4(&mScale) + XMLoadFloat4(&pScale));
	SetTransform();
}

/// <summary>
/// Returns the transform of the shape
/// </summary>
/// <returns> a pointer to the transform of the shape </returns>
const DirectX::XMFLOAT4X4 * const Shape::Transform() const
{
	return &mTransform;
}

/// <summary>
/// Gets the vertices of the shape
/// </summary>
/// <returns> a vector of simple vertex which contains all vertex data for the shape </returns>
const std::vector<SimpleVertex>& Shape::Vertices() const
{
	return mVertices;
}

/// <summary>
/// Gets the indices of the shape
/// </summary>
/// <returns> a vector of integer indices for the shape </returns>
const std::vector<WORD>& Shape::Indices() const
{
	return mIndices;
}

/// <summary>
/// Returns a list of instances
/// </summary>
/// <returns> a list of instances - this list is empty if the shape does not have instancing enabled </returns>
const std::vector<Instance>& Shape::Instances() const
{
	return mInstances;
}

/// <summary>
/// Gets the diffuse texture of the shape
/// </summary>
/// <returns> a reference to the wide string holding the textures filename </returns>
const std::wstring& Shape::DiffuseTexture() const
{
	return mDiffuseTexture;
}

/// <summary>
/// Gets the normal map of the shape
/// </summary>
/// <returns> a reference to the wide string holding the textures filename </returns>
const std::wstring & Shape::NormalMap() const
{
	return mNormalMap;
}

/// <summary>
/// Gets the height map of the shape
/// </summary>
/// <returns> a reference to the wide string holding the textures filename </returns>
const std::wstring & Shape::HeightMap() const
{
	return mHeightMap;
}

/// <summary>
/// Gets the geometry type which the object is using
/// </summary>
/// <returns> the geometry of the shape </returns>
const GeometryType & Shape::Geometry() const
{
	return mGeometryType;
}

/// <summary>
/// Gets the name of the shape
/// </summary>
/// <returns> the name of the shape </returns>
const std::string & Shape::Name() const
{
	return mName;
}

/// <summary>
/// Returns whether the shape is a cube mapped environment
/// </summary>
/// <returns> the is environment bool </returns>
const bool Shape::IsEnvironment() const
{
	return mIsEnvironment;
}

/// <summary>
/// Returns whether this shape has blending enabled
/// </summary>
/// <returns> A bool to mark if this shape should have blending enabled </returns>
const bool Shape::IsBlended() const
{
	return mBlended;
}

/// <summary>
/// Removes the instances marked by the parameter from the instance list
/// </summary>
/// <param name="pIndexToDelete"> a vector containing all of the indices of the instances which must be deleted </param>
void Shape::RemoveInstances(const std::vector<Instance>& pIndexToDelete)
{
	for (auto& index : pIndexToDelete)
	{
		mInstances.erase(remove(mInstances.begin(), mInstances.end(), index), mInstances.end());
	}
}

/// <summary>
/// Sets the list of instances for the shape
/// </summary>
/// <param name="pInstances"> the list of instances to set </param>
void Shape::SetInstances(const std::vector<Instance>& pInstances)
{
	mInstances = pInstances;
}

/// <summary>
/// Set rotation of the shape
/// </summary>
/// <param name="pRotation"> The rotation to give the shape </param>
void Shape::SetRotation(const DirectX::XMFLOAT4 & pRotation)
{
	mRotation = pRotation;
	SetTransform();
}

/// <summary>
/// An accessor for the shader variable
/// </summary>
/// <returns> the name of the shader used by the shape </returns>
const wstring& Shape::Shader() const
{
	return mShader;
}

/// <summary>
/// Sets the vertices and indices of the shape based on the geometry type enum
/// </summary>
/// <param name="pGeoType"> the geometry type representing the shape so that vertices and indices can be set correctly</param>
void Shape::SetGeometry(const GeometryType pGeoType)
{
	switch (pGeoType)
	{
	case GeometryType::CUBE:
		SetCube();
		break;
	case GeometryType::CYLINDER:
		SetCylinder();
		break;
	case GeometryType::CONE:
		SetCone();
		break;
	case GeometryType::QUAD:
		SetQuad();
		break;
	}
}

/// <summary>
/// sets the vertices and indices to those appropriate for a cube
/// </summary>
void Shape::SetCube()
{
	//set vertices
	//top
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f),		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f),		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 0.0f) }
		);
	//back
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f),		DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),		DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f),		DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 0.0f) }
		);
	//right
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, -0.5f,-0.5f),		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT2(0.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, 0.5f,-0.5f),		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT2(0.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, 0.5f,0.5f),		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT2(1.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, -0.5f,0.5f),		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),	DirectX::XMFLOAT2(1.0f, 0.0f) }
		);
	//front
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, -0.5f,-0.5f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, 0.5f,-0.5f),		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, 0.5f,-0.5f),		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, -0.5f,-0.5f),		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 0.0f) }
		);
	//left
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, -0.5f,0.5f),		DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT2(0.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, 0.5f,0.5f),		DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT2(0.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, 0.5f,-0.5f),		DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT2(1.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, -0.5f,-0.5f),	DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT2(1.0f, 0.0f) }
		);
	//bottom
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f),	DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0),		DirectX::XMFLOAT2(0.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f),	DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f),		DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f),	DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 0.0f) }
		);

	//set indices
	mIndices.emplace_back(0);  
	mIndices.emplace_back(1);  
	mIndices.emplace_back(2);
	mIndices.emplace_back(0);  
	mIndices.emplace_back(2);  
	mIndices.emplace_back(3);

	mIndices.emplace_back(4); 
	mIndices.emplace_back(5);  
	mIndices.emplace_back(6);
	mIndices.emplace_back(4);  
	mIndices.emplace_back(6);  
	mIndices.emplace_back(7);

	mIndices.emplace_back(8); 
	mIndices.emplace_back(9);  
	mIndices.emplace_back(10);
	mIndices.emplace_back(8);  
	mIndices.emplace_back(10); 
	mIndices.emplace_back(11);

	mIndices.emplace_back(12);
	mIndices.emplace_back(13); 
	mIndices.emplace_back(14);
	mIndices.emplace_back(12);
	mIndices.emplace_back(14);
	mIndices.emplace_back(15);

	mIndices.emplace_back(16);
	mIndices.emplace_back(17);
	mIndices.emplace_back(18);
	mIndices.emplace_back(16);
	mIndices.emplace_back(18);
	mIndices.emplace_back(19);

	mIndices.emplace_back(20);
	mIndices.emplace_back(21); 
	mIndices.emplace_back(22);
	mIndices.emplace_back(20); 
	mIndices.emplace_back(22); 
	mIndices.emplace_back(23);
}

/// <summary>
/// sets the vertices and indices to those appropriate for a cylinder
/// </summary>
void Shape::SetCylinder()
{
	//Cylinder data - Generated in program

	const auto pointsOnCircumference = 50;

	//Centres
	mVertices.emplace_back(
		//Position							Normal							Tangent							Binormal							TexCoord						
		SimpleVertex{ XMFLOAT3(0.0f, 0.5f, 0.0f),			XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),			XMFLOAT2(0.5f, 0.5f) });

	mVertices.emplace_back(
		//Position							Normal							Tangent							Binormal							TexCoord						
		SimpleVertex{ XMFLOAT3(0.0f, -0.5f, 0.0f),		XMFLOAT3(0.0f, -1.0f, 0.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),			XMFLOAT2(0.5f, 0.5f) });

	for (auto i = 0; i < pointsOnCircumference; i++)
	{
		const float fraction = static_cast<float>(i) / (pointsOnCircumference - 1);
		const float theta = 2 * XM_PI * fraction;

		XMFLOAT3 normal = XMFLOAT3(sin(theta), 0.0f, cos(theta));
		XMFLOAT3 tangent = XMFLOAT3(0.0f, 1.0f, 0.0f);
		XMFLOAT3 binormal{};
		XMStoreFloat3(&binormal, XMVector3Cross(XMLoadFloat3(&normal), XMLoadFloat3(&tangent)));

		//Tube
		//Top Edge
		mVertices.emplace_back(
			//Position										Normal			Tangent			Binormal		TexCoord						
			SimpleVertex{ XMFLOAT3(sin(theta), 0.5f, cos(theta)),			normal,			tangent,		binormal,		XMFLOAT2(fraction, 1.0f) });

		//Bottom Edge
		mVertices.emplace_back(
			//Position										Normal			Tangent			Binormal		TexCoord						
			SimpleVertex{ XMFLOAT3(sin(theta), -0.5f, cos(theta)),		normal,			tangent,		binormal,		XMFLOAT2(fraction, 0.0f) });

		//Cylinder Caps
		//Top
		mVertices.emplace_back(
							//Position									Normal								Tangent							Binormal						TexCoord						
			SimpleVertex{ XMFLOAT3(sin(theta), 0.5f, cos(theta)),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2((sin(theta) + 1) / 2, (cos(theta) + 1) / 2) });

		//Bottom
		mVertices.emplace_back(
							//Position									Normal								Tangent							Binormal						TexCoord						
			SimpleVertex{ XMFLOAT3(sin(theta), -0.5f, cos(theta)),	XMFLOAT3(0.0f, -1.0f, 0.0f),		XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),		XMFLOAT2((sin(theta) + 1) / 2, (cos(theta) + 1) / 2) });
	}

	//tube indices
	for (auto i = 4; i < (pointsOnCircumference * 4); i += 4)
	{
		//offsets of each vertex in the order they are added to the vertex list
		//tube
		mIndices.push_back(i - 1);
		mIndices.push_back(i + 3);
		mIndices.push_back(i - 2);

		mIndices.push_back(i - 2);
		mIndices.push_back(i + 3);
		mIndices.push_back(i + 2);

		//top cap
		mIndices.push_back(0);
		mIndices.push_back(i);
		mIndices.push_back(i + 4);

		//bottom cap
		mIndices.push_back(1);
		mIndices.push_back(i + 5);
		mIndices.push_back(i + 1);
	}
}

/// <summary>
/// sets the vertices and indices to those appropriate for a cone
/// </summary>
void Shape::SetCone()
{
	//Cone data - Generated in program

	const auto pointsOnCircumference = 500;

	for (auto i = 0; i < pointsOnCircumference; i++)
	{
		const float fraction = static_cast<float>(i) / (pointsOnCircumference - 1);
		const float theta = 2 * XM_PI * fraction;
		const float lengthOfSlope = 1 / sqrt(1/*height^2*/ + 1/*radius^2*/);
		const XMFLOAT2 crossSectionNormal = XMFLOAT2(-lengthOfSlope, lengthOfSlope);
		XMFLOAT3 normal = XMFLOAT3(sin(theta) * -crossSectionNormal.y, crossSectionNormal.x, cos(theta) * -crossSectionNormal.y);
		XMFLOAT3 tangent = XMFLOAT3(sin(theta), -1.0f, cos(theta));
		XMFLOAT3 binormal{};
		XMStoreFloat3(&binormal, XMVector3Cross(XMLoadFloat3(&normal), XMLoadFloat3(&tangent)));

		//Cone point
		mVertices.emplace_back(
			//Position							Normal				Tangent				Binormal			TexCoord						
			SimpleVertex{ XMFLOAT3(0.0f, 0.5f, 0.0f),			normal,				tangent,			binormal,			XMFLOAT2(fraction, 1.0f) });

		//Cone base
		mVertices.emplace_back(
			//Position										Normal			Tangent			Binormal		TexCoord						
			SimpleVertex{ XMFLOAT3(sin(theta), -0.5f, cos(theta)),			normal,			tangent,		binormal,		XMFLOAT2(fraction, 0.0f) });

		//Cone circle
		mVertices.emplace_back(
			//Position									Normal								Tangent							Binormal							TexCoord						
			SimpleVertex{ XMFLOAT3(sin(theta), -0.5f, cos(theta)),	XMFLOAT3(0.0f, -1.0f, 0.0f),		XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),		XMFLOAT2((sin(theta) + 1) / 2, (cos(theta) + 1) / 2) });

	}

	//Cone circle center
	mVertices.emplace_back(
		//Position							Normal								Tangent								Binormal							TexCoord						
		SimpleVertex{ XMFLOAT3(0.0f, -0.5f, 0.0f),			XMFLOAT3(0.0f, -1.0f, 0.0f),		XMFLOAT3(-1.0f, 0.0f, 0.0f),		XMFLOAT3(0.0f, 0.0f, -1.0f),		XMFLOAT2(0.5f, 0.5f) });

	for (auto i = 0; i < pointsOnCircumference * 3 - 3; i += 3)
	{
		mIndices.push_back(i);
		mIndices.push_back(i + 1);
		mIndices.push_back(i + 4);

		mIndices.push_back(mVertices.size() - 1);
		mIndices.push_back(i + 5);
		mIndices.push_back(i + 2);
	}
}

/// <summary>
/// sets the vertices and indices to those appropriate for a quad
/// </summary>
void Shape::SetQuad()
{
	//set vertices
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0),		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, -0.5f, 0),		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 0.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(0.5f, 0.5f, 0),			DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT2(1.0f, 1.0f) }
		);
	mVertices.emplace_back(		//Position									Normal									Tangent									Binormal								TexCoord
		SimpleVertex{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0),		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),	DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),	DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	DirectX::XMFLOAT2(0.0f, 1.0f) }
		);

	//set indices
	mIndices.emplace_back(0);
	mIndices.emplace_back(2);
	mIndices.emplace_back(1);
	mIndices.emplace_back(0);
	mIndices.emplace_back(3);
	mIndices.emplace_back(2);
}

