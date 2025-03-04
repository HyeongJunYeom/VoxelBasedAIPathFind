#include "Model_Extractor.h"

map<const string, _uint>	CModel_Extractor::BoneIndices;
ofstream					CModel_Extractor::ofs;


HRESULT CModel_Extractor::Extract_FBX(CModel::MODEL_TYPE eType, const string& strModelFilePath, _fmatrix TransformationMatrix)
{
	_uint		iOption = { aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded };

	iOption = eType == CModel::TYPE_NONANIM ? iOption | aiProcess_PreTransformVertices : iOption | aiProcess_LimitBoneWeights;
	//	iOption = eType == CModel::TYPE_NONANIM ? iOption | aiProcess_PreTransformVertices : iOption;

	Assimp::Importer Importer;

	const aiScene* pAIScene = Importer.ReadFile(strModelFilePath.c_str(), iOption);

	if (nullptr == pAIScene)
		return E_FAIL;

	if (FAILED(Open_File(strModelFilePath.c_str())))
		return E_FAIL;


	//	첫 번째 인자는 데이터의 주소를 가리키는 포인터이고, 두 번째 인자는 작성할 바이트 수입니다. 
	//	ofs.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(int));
	Write_File(&eType, sizeof(CModel::MODEL_TYPE));

	//	재귀형태로 구현하여 내부에서 기존 fbx읽는방식과 동일하게 파일 작성
	if (FAILED(Write_Bones(pAIScene->mRootNode)))
		return E_FAIL;

	if (FAILED(Write_Meshes(pAIScene, eType, TransformationMatrix)))
		return E_FAIL;

	if (FAILED(Write_Materials(pAIScene, strModelFilePath.c_str())))
		return E_FAIL;

	if (FAILED(Write_Animations(pAIScene)))
		return E_FAIL;

	Close_File();
	Importer.FreeScene();

	return S_OK;
}

HRESULT CModel_Extractor::Extract_FBX_AnimOnly(const string& strAnimFilePath)
{
	_uint		iOption = { aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded | aiProcess_LimitBoneWeights };

	Assimp::Importer Importer;

	const aiScene* pAIScene = Importer.ReadFile(strAnimFilePath.c_str(), iOption);
	if (nullptr == pAIScene)
		return E_FAIL;

	if (FAILED(Open_File(strAnimFilePath.c_str())))
		return E_FAIL;

	if (FAILED(Non_Write_Bones(pAIScene->mRootNode)))
		return E_FAIL;

	if (FAILED(Write_Animations(pAIScene)))
		return E_FAIL;

	Close_File();
	Importer.FreeScene();

	return S_OK;
}

//	뼈의 수 저장
//	부모 인덱스 저장
//	이름 저장
//	트랜스포메이션 행렬 저장

HRESULT CModel_Extractor::Write_Bones(aiNode* pAINode, _int iParentIndex)
{
	vector<tagBoneInfo>		Bones;

	if (FAILED(Ready_Bones(pAINode, Bones)))
		return E_FAIL;

	_uint iNumBones = (_uint)Bones.size();

	Write_File(&iNumBones, sizeof(_uint));

	_uint iIdx = { 0 };
	for (auto& Bone : Bones)
	{
		Write_File(&Bone.iParentIndex, sizeof(_int));
		Write_File(Bone.szName, sizeof(_char) * MAX_PATH);
		Write_File(&Bone.TransformationMatrix, sizeof(_float4x4));

		BoneIndices.emplace(Bone.szName, iIdx++);
	}

	return S_OK;
}

HRESULT CModel_Extractor::Non_Write_Bones(aiNode* pAINode, _int iParentIndex)
{
	vector<tagBoneInfo>		Bones;

	if (FAILED(Ready_Bones(pAINode, Bones)))
		return E_FAIL;

	_uint iNumBones = (_uint)Bones.size();

	_uint iIdx = { 0 };
	for (auto& Bone : Bones)
	{
		BoneIndices.emplace(Bone.szName, iIdx++);
	}

	return S_OK;
}

HRESULT CModel_Extractor::Ready_Bones(aiNode* pAINode, vector<BONE_INFO>& Bones, _int iParentIndex)
{
	BONE_INFO Bone = CModel_Extractor::Create_Bone(pAINode, iParentIndex);

	Bones.push_back(Bone);

	_int iParent = (_int)(Bones.size() - 1);

	for (size_t i = 0; i < pAINode->mNumChildren; ++i)
	{
		Ready_Bones(pAINode->mChildren[i], Bones, iParent);
	}

	return S_OK;
}

CModel_Extractor::BONE_INFO CModel_Extractor::Create_Bone(aiNode* pAINode, _int iParentIndex)
{
	BONE_INFO Bone;

	strcpy_s(Bone.szName, pAINode->mName.data);

	_float4x4 TransformationMatrix;

	memcpy_s(&TransformationMatrix, sizeof(_float4x4), &pAINode->mTransformation, sizeof(_float4x4));
	//	Assimp의 모든 행렬은 전치되어있으므로 다시 전치 하여 저장해줘야한다.
	//	Assimp(Assimp은 오픈 소스 3D 모델 임포트 라이브러리입니다)에서는 모든 행렬을 column-major 형태로 다룹니다
	XMStoreFloat4x4(&Bone.TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&TransformationMatrix)));

	Bone.iParentIndex = iParentIndex;

	return Bone;
}

//	 메쉬의 수 저장


HRESULT CModel_Extractor::Write_Meshes(const aiScene* pAIScene, CModel::MODEL_TYPE eType, _fmatrix TransformationMatrix)
{
	vector<tagMeshInfo>		Meshs;

	if (FAILED(Ready_Meshes(pAIScene, Meshs, eType, TransformationMatrix)))
		return E_FAIL;

	_uint iNumMesh = pAIScene->mNumMeshes;
	Write_File(&iNumMesh, sizeof(_uint));
	Write_File(&eType, sizeof(CModel::MODEL_TYPE));

	for (auto& Mesh : Meshs)
	{
		Write_File(&Mesh.szName, sizeof(_char) * MAX_PATH);
		Write_File(&Mesh.iMaterialIndex, sizeof(_uint));

		Write_File(&Mesh.iNumVertices, sizeof(_uint));
		for (auto& Vertex : Mesh.Vertices)
		{
			Write_File(&Vertex.vPosition, sizeof(_float3));
			Write_File(&Vertex.vNormal, sizeof(_float3));
			Write_File(&Vertex.vTexcoord, sizeof(_float2));
			Write_File(&Vertex.vTangent, sizeof(_float3));

			if (string(Mesh.szName) == string("Boss_MantisShrimp01_Eye"))
			{
				if (Vertex.vPosition.y < 0.f)
				{
					XMUINT4			vIndices = { 0, 0, 0, 0 };
					_float4			vBlendWeights = { 1.f, 0.f, 0.f, 0.f };


					memcpy_s(&Vertex.vBlendIndices, sizeof(XMUINT4), &vIndices, sizeof(XMUINT4));
					memcpy_s(&Vertex.vBlendWeights, sizeof(_float4), &vBlendWeights, sizeof(_float4));
				}

				else
				{
					XMUINT4			vIndices = { 1, 0, 0, 0 };
					_float4			vBlendWeights = { 1.f, 0.f, 0.f, 0.f };

					memcpy_s(&Vertex.vBlendIndices, sizeof(XMUINT4), &vIndices, sizeof(XMUINT4));
					memcpy_s(&Vertex.vBlendWeights, sizeof(_float4), &vBlendWeights, sizeof(_float4));
				}
			}

			if (CModel::TYPE_ANIM == eType)
			{
				Write_File(&Vertex.vBlendIndices, sizeof(XMUINT4));
				Write_File(&Vertex.vBlendWeights, sizeof(_float4));
			}
		}

		Write_File(&Mesh.iNumIndices, sizeof(_uint));
		for (auto& Index : Mesh.Indices)
			Write_File(&Index, sizeof(_uint));

		Write_File(&Mesh.iNumBones, sizeof(_uint));
		for (auto& Bone : Mesh.Bones)
			Write_File(&Bone, sizeof(_uint));

		_uint iNumOffsetMatrices = (_uint)Mesh.OffsetMatrices.size();
		Write_File(&iNumOffsetMatrices, sizeof(_uint));
		for (auto& OffsetMatrix : Mesh.OffsetMatrices)
			Write_File(&OffsetMatrix, sizeof(_float4x4));
	}

	return S_OK;
}

HRESULT	CModel_Extractor::Ready_Meshes(const aiScene* pAIScene, vector<MESH_INFO>& Meshs, CModel::MODEL_TYPE eType, _fmatrix TransformationMatrix)
{
	_uint iNumMesh = pAIScene->mNumMeshes;

	MESH_INFO Mesh;

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		Mesh = Create_Mesh(pAIScene->mMeshes[i], eType, TransformationMatrix);

		if (0 == Mesh.iNumVertices)
			return E_FAIL;

		Meshs.push_back(Mesh);
	}

	return S_OK;
}

CModel_Extractor::MESH_INFO CModel_Extractor::Create_Mesh(const aiMesh* pAIMesh, CModel::MODEL_TYPE eType, _fmatrix TransformationMatrix)
{
	MESH_INFO Mesh;

	strcpy_s(Mesh.szName, pAIMesh->mName.data);
	Mesh.iMaterialIndex = pAIMesh->mMaterialIndex;
	Mesh.iNumVertices = pAIMesh->mNumVertices;
	Mesh.iNumIndices = pAIMesh->mNumFaces * 3;

	if (CModel::MODEL_TYPE::TYPE_ANIM == eType)
	{
		if (FAILED(Create_AnimMesh(pAIMesh, Mesh)))
			return MESH_INFO{};
	}
	else if (CModel::MODEL_TYPE::TYPE_NONANIM == eType)
	{
		if (FAILED(Create_NonAnimMesh(pAIMesh, TransformationMatrix, Mesh)))
			return MESH_INFO{};
	}

	Mesh.Indices.resize(Mesh.iNumIndices);
	_uint		iNumIndices = { 0 };

	for (size_t i = 0; i < pAIMesh->mNumFaces; ++i)
	{
		Mesh.Indices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[0];
		Mesh.Indices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[1];
		Mesh.Indices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[2];
	}

	return Mesh;
}

HRESULT CModel_Extractor::Create_AnimMesh(const aiMesh* pAIMesh, MESH_INFO& Mesh)
{
	VTXANIMMESH VtxAnimMesh;
	ZeroMemory(&VtxAnimMesh, sizeof(VTXANIMMESH));

	for (size_t i = 0; i < Mesh.iNumVertices; i++)
	{
		memcpy_s(&VtxAnimMesh.vPosition, sizeof(_float3), &pAIMesh->mVertices[i], sizeof(_float3));
		memcpy_s(&VtxAnimMesh.vNormal, sizeof(_float3), &pAIMesh->mNormals[i], sizeof(_float3));
		memcpy_s(&VtxAnimMesh.vTexcoord, sizeof(_float2), &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		memcpy_s(&VtxAnimMesh.vTangent, sizeof(_float3), &pAIMesh->mTangents[i], sizeof(_float3));

		Mesh.Vertices.push_back(VtxAnimMesh);
	}

	Mesh.iNumBones = pAIMesh->mNumBones;

	for (_uint i = 0; i < Mesh.iNumBones; i++)
	{
		aiBone* pAIBone = pAIMesh->mBones[i];

		_float4x4	OffsetMatrix;
		memcpy_s(&OffsetMatrix, sizeof(_float4x4), &pAIBone->mOffsetMatrix, sizeof(_float4x4));
		//	assimp에서 로드한 행렬은 전부 전치되어있으므로 다시 전치하여 저장해야한다.
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));

		Mesh.OffsetMatrices.push_back(OffsetMatrix);

		auto iter = BoneIndices.find(pAIBone->mName.data);

		if (iter == BoneIndices.end())
			return E_FAIL;

		_uint iBoneIndex = iter->second;

		Mesh.Bones.push_back(iBoneIndex);

		/* 이 뼈는 몇개의 정점들에게 영향을 준다. */
		_uint		iNumWeights = pAIBone->mNumWeights;

		for (_uint j = 0; j < iNumWeights; j++)
		{
			if (0.0f == Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x)
			{
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.x = i;
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x = pAIBone->mWeights[j].mWeight;
			}

			else if (0.0f == Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y)
			{
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.y = i;
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y = pAIBone->mWeights[j].mWeight;
			}

			else if (0.0f == Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z)
			{
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.z = i;
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z = pAIBone->mWeights[j].mWeight;
			}

			else if (0.0f == Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w)
			{
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.w = i;
				Mesh.Vertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w = pAIBone->mWeights[j].mWeight;
			}
		}
	}

	//	뼈가 없던 메쉬라면 강제적으로 뼈를 생성해준다.
	//	보통은 메쉬의 이름과 동일한 이름을 지닌 뼈가 모델 전체의 뼈중에 있으므로 해당뼈를 찾아서 지닌다.
	if (0 == Mesh.iNumBones)
	{
		Mesh.iNumBones = 1;

		auto iterPair = BoneIndices.find(Mesh.szName);
		if (BoneIndices.end() == iterPair)
			return E_FAIL;

		_uint iBoneIndex = iterPair->second;

		Mesh.Bones.push_back(iBoneIndex);

		_float4x4		OffsetMatrix;

		//	부모단계의 컴바인드 매트릭스 단계에서 이미 해당 정보의 기입이끝났다고 가정
		//	별도의 오프셋을 지니지않게한다. ( 항등행렬 1개 삽입 )
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());

		Mesh.OffsetMatrices.push_back(OffsetMatrix);
	}

	return S_OK;
}
HRESULT CModel_Extractor::Create_NonAnimMesh(const aiMesh* pAIMesh, _fmatrix TransformationMatrix, MESH_INFO& Mesh)
{
	VTXANIMMESH VtxAnimMesh;

	_float2			zeroFloat2 = {};
	_float3			zeroFloat3 = {};

	for (size_t i = 0; i < Mesh.iNumVertices; i++)
	{
		memcpy_s(&VtxAnimMesh.vPosition, sizeof(_float3), &pAIMesh->mVertices[i], sizeof(_float3));
		XMStoreFloat3(&VtxAnimMesh.vPosition, XMVector3TransformCoord(XMLoadFloat3(&VtxAnimMesh.vPosition), TransformationMatrix));

		memcpy_s(&VtxAnimMesh.vNormal, sizeof(_float3), &pAIMesh->mNormals[i], sizeof(_float3));
		XMStoreFloat3(&VtxAnimMesh.vNormal, XMVector3TransformNormal(XMLoadFloat3(&VtxAnimMesh.vNormal), TransformationMatrix));

		if (pAIMesh->mTextureCoords[0] != nullptr)
			memcpy_s(&VtxAnimMesh.vTexcoord, sizeof(_float2), &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		else
			memcpy_s(&VtxAnimMesh.vTexcoord, sizeof(_float2), &zeroFloat2, sizeof(_float2));

		if (pAIMesh->mTangents != nullptr)
			memcpy_s(&VtxAnimMesh.vTangent, sizeof(_float3), &pAIMesh->mTangents[i], sizeof(_float3));
		else
			memcpy_s(&VtxAnimMesh.vTangent, sizeof(_float3), &zeroFloat3, sizeof(_float3));

		Mesh.Vertices.push_back(VtxAnimMesh);
	}

	return S_OK;
}

HRESULT CModel_Extractor::Write_Materials(const aiScene* pAIScene, const _char* pModelFilePath)
{
	vector<MATERIAL_INFO>	Materials;

	if (FAILED(Ready_Materials(pAIScene, Materials, pModelFilePath)))
		return E_FAIL;

	_uint iNumMaterials = (_uint)Materials.size();

	Write_File(&iNumMaterials, sizeof(_uint));

	for (auto& Material : Materials)
	{
		for (size_t i = aiTextureType_DIFFUSE; i < AI_TEXTURE_TYPE_MAX; ++i)
		{
			Write_File(Material.Textures[i].szPath, sizeof(_tchar) * MAX_PATH);
		}
	}

	return S_OK;
}

HRESULT CModel_Extractor::Ready_Materials(const aiScene* pAIScene, vector<MATERIAL_INFO>& Matrials, const _char* pModelFilePath)
{
	_uint iNumMaterial = pAIScene->mNumMaterials;

	MATERIAL_INFO Material;

	for (_uint i = 0; i < iNumMaterial; ++i)
	{
		aiMaterial* pAIMaterial = pAIScene->mMaterials[i];

		Material = Create_Material(pAIMaterial, pModelFilePath);

		Matrials.push_back(Material);
	}

	return S_OK;
}

CModel_Extractor::MATERIAL_INFO CModel_Extractor::Create_Material(const aiMaterial* pAIMaterial, const _char* pModelFilePath)
{
	MATERIAL_INFO Material;

	for (size_t i = aiTextureType_DIFFUSE; i < AI_TEXTURE_TYPE_MAX; ++i)
	{
		aiString		strTextureFilePath;

		if (FAILED(pAIMaterial->GetTexture(aiTextureType(i), 0, &strTextureFilePath)))
		{
			lstrcpy(Material.Textures[i].szPath, L"");
			continue;
		}

		_char			szDrive[MAX_PATH] = { "" };
		_char			szDirectory[MAX_PATH] = { "" };
		_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDirectory, MAX_PATH, nullptr, 0, nullptr, 0);


		_char			szFileName[MAX_PATH] = { "" };
		_char			szEXT[MAX_PATH] = { "" };

		/* ..\Bin\Resources\Models\Fiona\ */
		_splitpath_s(strTextureFilePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szEXT, MAX_PATH);

		_char			szFullPath[MAX_PATH] = { "" };
		strcpy_s(szFullPath, szDrive);
		strcat_s(szFullPath, szDirectory);
		strcat_s(szFullPath, szFileName);
		strcat_s(szFullPath, szEXT);

		_tchar			szPerfectPath[MAX_PATH] = { L"" };

		MultiByteToWideChar(CP_ACP, 0, szFullPath, (_int)strlen(szFullPath), szPerfectPath, MAX_PATH);

		lstrcpy(Material.Textures[i].szPath, szPerfectPath);
	}

	return Material;
}

HRESULT CModel_Extractor::Write_Animations(const aiScene* pAIScene)
{
	vector<ANIM_INFO>	Anims;

	if (FAILED(Ready_Animations(pAIScene, Anims)))
		return E_FAIL;

	_uint iNumAnims = (_uint)Anims.size();
	Write_File(&iNumAnims, sizeof(_uint));

	for (auto& Anim : Anims)
	{
		Write_File(&Anim.szName, sizeof(_char) * MAX_PATH);
		Write_File(&Anim.fDuration, sizeof(_float));
		Write_File(&Anim.fTickPerSecond, sizeof(_float));

		Write_File(&Anim.iNumChannels, sizeof(_uint));
		for (auto& Channel : Anim.Channels)
		{
			Write_File(&Channel.szName, sizeof(_char) * MAX_PATH);
			Write_File(&Channel.iBoneIndex, sizeof(_uint));

			Write_File(&Channel.iNumKeyFrames, sizeof(_uint));
			for (auto& KeyFrame : Channel.KeyFrames)
			{
				Write_File(&KeyFrame.vScale, sizeof(_float3));
				Write_File(&KeyFrame.vRotation, sizeof(_float4));
				Write_File(&KeyFrame.vTranslation, sizeof(_float3));
				Write_File(&KeyFrame.fTime, sizeof(_float));
			}
		}
	}

	return S_OK;
}

HRESULT CModel_Extractor::Ready_Animations(const aiScene* pAIScene, vector<ANIM_INFO>& Anims)
{
	_uint iNumAnimation = { 0 };
	iNumAnimation = pAIScene->mNumAnimations;

	ANIM_INFO Anim;

	for (size_t i = 0; i < iNumAnimation; ++i)
	{
		Anim = Create_Animation(pAIScene->mAnimations[i]);

		Anims.push_back(Anim);
	}

	return S_OK;
}

CModel_Extractor::ANIM_INFO CModel_Extractor::Create_Animation(const aiAnimation* pAIAnimation)
{
	ANIM_INFO Anim;
	CHANNEL_INFO Channel;

	strcpy_s(Anim.szName, pAIAnimation->mName.data);

	Anim.fDuration = (_float)pAIAnimation->mDuration;
	Anim.fTickPerSecond = (_float)pAIAnimation->mTicksPerSecond;

	/* 이 애니메이션은 몇 개의 뼈를 컨트롤 하는가 */
	Anim.iNumChannels = pAIAnimation->mNumChannels;

	for (size_t i = 0; i < Anim.iNumChannels; ++i)
	{
		Channel = Create_Channel(pAIAnimation->mChannels[i]);

		Anim.Channels.push_back(Channel);
	}

	return Anim;
}

CModel_Extractor::CHANNEL_INFO CModel_Extractor::Create_Channel(const aiNodeAnim* pAIChannel)
{
	CHANNEL_INFO Channel;

	strcpy_s(Channel.szName, pAIChannel->mNodeName.data);

	Channel.iBoneIndex = BoneIndices.find(Channel.szName)->second;

	Channel.iNumKeyFrames = max(max(pAIChannel->mNumPositionKeys, pAIChannel->mNumRotationKeys), pAIChannel->mNumScalingKeys);

	_float3			vScale;
	_float4			vRotation;
	_float3			vTranslation;
	_float			fTime;

	for (size_t i = 0; i < Channel.iNumKeyFrames; ++i)
	{
		KEYFRAME			KeyFrame;
		ZeroMemory(&KeyFrame, sizeof(KEYFRAME));

		if (i < pAIChannel->mNumScalingKeys)
		{
			memcpy_s(&vScale, sizeof(_float3), &pAIChannel->mScalingKeys[i].mValue, sizeof(aiVector3D));
			fTime = (_float)pAIChannel->mScalingKeys[i].mTime;
		}

		if (i < pAIChannel->mNumRotationKeys)
		{
			//	aiQuaternion은 w, x, y, z순으로 저장이되어 그대로 memcopy하면 안됌
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;

			fTime = (_float)pAIChannel->mRotationKeys[i].mTime;
		}

		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy_s(&vTranslation, sizeof(_float3), &pAIChannel->mPositionKeys[i].mValue, sizeof(aiVector3D));
			fTime = (_float)pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vTranslation = vTranslation;
		KeyFrame.fTime = fTime;

		Channel.KeyFrames.push_back(KeyFrame);
	}

	return Channel;
}

void CModel_Extractor::Write_File(void* pValue, _uint iSize)
{
	ofs.write(reinterpret_cast<_char*>(pValue), iSize);
}

HRESULT CModel_Extractor::Open_File(const _char* pFIlePath)
{
	CModel_Extractor::BoneIndices.clear();

	filesystem::path FullPath(pFIlePath);

	//	상대경로에서 경로와 파일이름만 분리
	string strParentsPath = FullPath.parent_path().string();
	string strFileName = FullPath.stem().string();

	//	 동일경로에 동일 파일이름에 확장자만 다르게 새로운 경로 생성
	string strNewPath = strParentsPath + "/" + strFileName + ".bin";

	//	바이너리로 데이터를 작성하기위해서 바이너리 플래그를 포함하였다.
	ofs = ofstream(strNewPath.c_str(), ios::binary | ios::out);
	if (true == ofs.fail())
	{
		MSG_BOX(TEXT("Failed To OpenFile"));

		return E_FAIL;
	}

	return S_OK;
}

void CModel_Extractor::Close_File()
{
	CModel_Extractor::BoneIndices.clear();
	CModel_Extractor::ofs.close();
}
