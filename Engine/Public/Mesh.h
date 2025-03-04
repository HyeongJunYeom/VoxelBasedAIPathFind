#pragma once

/* 디자이너분들이 저장해준 정점과 인덱스의 정보를 바탕으로해서 정점, 인덱스버퍼를 새엏나다.  */
#include "VIBuffer.h"
#include "Model.h"

BEGIN(Engine)

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& rhs);
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(CModel::MODEL_TYPE eModelType, const aiMesh* pAIMesh, const vector<CBone*>& Bones, _fmatrix TransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Stock_Matrices(const vector<CBone*>& Bones, _float4x4* pMeshBoneMatrices);

private:
	_char					m_szName[MAX_PATH] = { "" };

	/* 이 메시는 모델에서 로드해놓은 머테리얼들 중 몇번째 머테리얼을 이용하는가? */
	_uint					m_iMaterialIndex = { 0 };

	_uint					m_iNumBones = { 0 };
	/* 뼈의 인데스 : 모델이 가지고 있는 전체 뼈들 중, 메시가 사용하고 있는 뼈의 인덱스 */
	vector<_uint>			m_Bones;

	/* 현재 메시에 영향을 주는 뼈들의 순서대로 오프셋을 저장했다. */
	vector<_float4x4>		m_OffsetMatrices;


private:
	HRESULT Ready_Vertices_For_NonAnimModel(const aiMesh* pAIMesh, _fmatrix TransformationMatrix);
	HRESULT Ready_Vertices_For_AnimModel(const aiMesh* pAIMesh, const vector<CBone*>& Bones);

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CModel::MODEL_TYPE eModelType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix TransformMatrix);
	virtual CMesh* Clone(void* pArg);
	virtual void Free() override;
};

END