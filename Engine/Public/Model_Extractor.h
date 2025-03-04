#pragma once

#include "Model.h"

BEGIN(Engine)

class CModel_Extractor final
{
private:
	typedef struct tagBoneInfo
	{
		_int						iParentIndex = { -1 };
		_char						szName[MAX_PATH] = { "" };
		_float4x4					TransformationMatrix;
	}BONE_INFO;

	typedef struct tagMeshInfo
	{
		_char						szName[MAX_PATH] = { "" };
		CModel::MODEL_TYPE			eType = { CModel::TYPE_END };
		_uint						iMaterialIndex = { 0 };
		_uint						iNumVertices = { 0 };
		vector<VTXANIMMESH>			Vertices;
		_uint						iNumIndices = { 0 };
		vector<_uint>				Indices;
		_uint						iNumBones = { 0 };
		vector<_uint>				Bones;
		vector<_float4x4>			OffsetMatrices;
	}MESH_INFO;

	typedef struct tagTextureInfo
	{
		_tchar						szPath[MAX_PATH] = { TEXT("") };
	}TEXTURE_INFO;

	typedef struct tagMaterialInfo
	{
		TEXTURE_INFO				Textures[AI_TEXTURE_TYPE_MAX] = {};
	}MATERIAL_INFO;

	typedef struct tagChannelInfo
	{
		_char						szName[MAX_PATH] = { "" };
		_uint						iBoneIndex = { 0 };
		_uint						iNumKeyFrames = { 0 };
		vector<KEYFRAME>			KeyFrames;
	}CHANNEL_INFO;

	typedef struct tagAnimationInfo
	{
		_char						szName[MAX_PATH] = { "" };
		_float						fDuration = { 0.f };
		_float						fTickPerSecond = { 0.f };
		_uint						iNumChannels = { 0 };
		vector<CHANNEL_INFO>		Channels;

	}ANIM_INFO;

public:
	static HRESULT		Extract_FBX(CModel::MODEL_TYPE eType, const string& strModelFilePath, _fmatrix TransformationMatrix);
	static HRESULT		Extract_FBX_AnimOnly(const string& strAnimFilePath);

private:	/* For.Bones */
	static HRESULT		Write_Bones(aiNode* pAINode, _int iParentIndex = -1);
	static HRESULT		Non_Write_Bones(aiNode* pAINode, _int iParentIndex = -1);
	static HRESULT		Ready_Bones(aiNode* pAINode, vector<BONE_INFO>& Bones, _int iParentIndex = -1);

	static BONE_INFO	Create_Bone(aiNode* pAINode, _int iParentIndex = -1);

private:	/* For.Meshs */
	static HRESULT		Write_Meshes(const aiScene* pAIScene, CModel::MODEL_TYPE eType, _fmatrix TransformationMatrix);
	static HRESULT		Ready_Meshes(const aiScene* pAIScene, vector<MESH_INFO>& Meshs, CModel::MODEL_TYPE eType, _fmatrix TransformationMatrix);

	static MESH_INFO	Create_Mesh(const aiMesh* pAIMesh, CModel::MODEL_TYPE eType, _fmatrix TransformationMatrix);
	static HRESULT		Create_AnimMesh(const aiMesh* pAIMesh, MESH_INFO& Mesh);
	static HRESULT		Create_NonAnimMesh(const aiMesh* pAIMesh, _fmatrix TransformationMatrix, MESH_INFO& Mesh);

private:	/* For.Materials */
	static HRESULT		Write_Materials(const aiScene* pAIScene, const _char* pModelFilePath);
	static HRESULT		Ready_Materials(const aiScene* pAIScene, vector<MATERIAL_INFO>& Matrials, const _char* pModelFilePath);

	static MATERIAL_INFO Create_Material(const aiMaterial* pAIMaterial, const _char* pFilePpModelFilePathath);

private:	/* For.Animations */
	static HRESULT		Write_Animations(const aiScene* pAIScene);
	static HRESULT		Ready_Animations(const aiScene* pAIScene, vector<ANIM_INFO>& Anims);

	static ANIM_INFO	Create_Animation(const aiAnimation* pAIAnimation);

	static CHANNEL_INFO	Create_Channel(const aiNodeAnim* pAIChannel);


private:
	static void			Write_File(void* pValue, _uint iSize);
	static HRESULT 		Open_File(const _char* pFIlePath);
	static void			Close_File();

private:
	static map<const string, _uint>		BoneIndices;
	static ofstream						ofs;
};

END