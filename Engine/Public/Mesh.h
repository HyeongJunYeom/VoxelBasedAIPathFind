#pragma once

/* �����̳ʺе��� �������� ������ �ε����� ������ ���������ؼ� ����, �ε������۸� ���j����.  */
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

	/* �� �޽ô� �𵨿��� �ε��س��� ���׸���� �� ���° ���׸����� �̿��ϴ°�? */
	_uint					m_iMaterialIndex = { 0 };

	_uint					m_iNumBones = { 0 };
	/* ���� �ε��� : ���� ������ �ִ� ��ü ���� ��, �޽ð� ����ϰ� �ִ� ���� �ε��� */
	vector<_uint>			m_Bones;

	/* ���� �޽ÿ� ������ �ִ� ������ ������� �������� �����ߴ�. */
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