#pragma once

#include "Base.h"

/* ���� ������ ǥ���ϱ����� Ÿ�� ������ �����ϳ�. */
/* aiNode, aiBone, aiAnimNode */

BEGIN(Engine)

class ENGINE_DLL CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}

public:
	const _float4x4* Get_CombinedTransformationMatrix() const {
		return &m_CombinedTransformationMatrix;
	}

public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentIndex);
	void Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix TransformatrixMatrix);	
	
	_bool Compare_Name(const _char* pBoneName) {
		return !strcmp(m_szName, pBoneName);
	}

private:
	_char				m_szName[MAX_PATH] = { "" };

	/* �� �� ���� ������� */
	_float4x4			m_TransformationMatrix;

	/* ���� �������. */
	_float4x4			m_CombinedTransformationMatrix;

	_int				m_iParentBoneIndex = { -1 };

public:
	static CBone* Create(const aiNode* pAINode, _int  iParentIndex);
	CBone* Clone();
	virtual void Free() override;
};

END