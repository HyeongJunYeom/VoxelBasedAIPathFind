#pragma once

#include "Component.h"

/* 각 형태를 구성하기위한 정점 인덱스를 보유하는 개겣들의 부모다. */

BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer(const CVIBuffer& rhs);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render();

public:
	virtual HRESULT Bind_Buffers();

public:
	virtual HRESULT Cooking_Static_Voxel(class CTransform* pTransform);

public:
	HRESULT Sampling_Poses(_fmatrix WorldMatrix, vector<_float3>& SamplePoses, const _float fSampleOffset);

public:
	virtual _bool Compute_Picking(const class CTransform* pTransform, _Out_ _float4 * pPickPos);

protected:
	ID3D11Buffer*				m_pVB = { nullptr };
	ID3D11Buffer*				m_pIB = { nullptr };

protected:
	D3D11_BUFFER_DESC			m_BufferDesc;
	D3D11_SUBRESOURCE_DATA		m_InitialData;
	_uint						m_iVertexStride = { 0 };
	_uint						m_iIndexStride = { 0 };
	_uint						m_iNumVertices = { 0 };
	_uint						m_iNumIndices = { 0 };
	_uint						m_iNumVertexBuffers = { 0 };
	DXGI_FORMAT					m_eIndexFormat = {  };
	D3D11_PRIMITIVE_TOPOLOGY	m_ePrimitiveTopology = { };

	_float3*					m_pVerticesPos = { nullptr };
	_uint*						m_pIndices = { nullptr };

protected:
	HRESULT Create_Buffer(_Out_ ID3D11Buffer** ppBuffer);

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END