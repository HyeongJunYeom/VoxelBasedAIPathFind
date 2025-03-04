#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
private:
	CVIBuffer_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Terrain(const CVIBuffer_Terrain& rhs);
	virtual ~CVIBuffer_Terrain() = default;

public:
	virtual HRESULT Initialize_Prototype(const wstring& strHeightMapFilePath);
	virtual HRESULT Initialize(void* pArg);

public:
	virtual _bool Compute_Picking(const class CTransform* pTransform, _Out_ _float4 * pPickPos) override;
	void Compute_Height(const class CTransform* pTransform, _fvector vPosition, _Out_ _float4 * pPosition);

public:
	_uint Get_Width() { return m_iNumVerticesX - 1; }
	_uint Get_Depth() { return m_iNumVerticesZ - 1; }

public:
	void Culling(_fmatrix WorldMatrixInv);

private:
	_uint				m_iNumVerticesX = { 0 };
	_uint				m_iNumVerticesZ = { 0 };

	class CGameInstance*		m_pGameInstance = { nullptr };
	class CQuadTree*			m_pQuadTree = { nullptr };


public:
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strHeightMapFilePath);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
