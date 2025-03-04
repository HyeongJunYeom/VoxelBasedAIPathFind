#include "..\Public\VIBuffer_Instance.h"

CVIBuffer_Instance::CVIBuffer_Instance(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer{pDevice, pContext }
{
	
}

CVIBuffer_Instance::CVIBuffer_Instance(const CVIBuffer_Instance & rhs)
	: CVIBuffer{ rhs }
	, m_pVBInstance{ rhs.m_pVBInstance }
	, m_iInstanceStride{ rhs.m_iInstanceStride }
	, m_iNumInstance{ rhs.m_iNumInstance }
	, m_iIndexCountPerInstance{rhs.m_iIndexCountPerInstance }
	, m_pInstanceVertices{ rhs.m_pInstanceVertices }
	, m_InstanceBufferDesc{ rhs.m_InstanceBufferDesc }
	, m_InstanceSubResourceData { rhs.m_InstanceSubResourceData }	
	, m_pSpeeds{ rhs.m_pSpeeds }
	, m_InstanceDesc { rhs.m_InstanceDesc }
{
	m_pLifeTimes = new _float2[m_iNumInstance];
	memcpy(m_pLifeTimes, rhs.m_pLifeTimes, sizeof(_float2) * m_iNumInstance);

	Safe_AddRef(m_pVBInstance);
}

HRESULT CVIBuffer_Instance::Initialize_Prototype(const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	m_RandomNumber = mt19937_64(m_RandomDevice());

	m_InstanceDesc = InstanceDesc;

	return S_OK;
}

HRESULT CVIBuffer_Instance::Initialize(void * pArg)
{

	if (FAILED(m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &m_InstanceSubResourceData, &m_pVBInstance)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Instance::Bind_Buffers()
{
	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance, 
	};

	_uint				iVertexStrides[] = {
		m_iVertexStride, 
		m_iInstanceStride
	};

	_uint				iOffsets[] = {
		0, 
		0
	};

	/* �������۵��� ��ġ�� ���ε��Ѵ�. */
	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);

	/* �ε������۵��� ��ġ�� ���ε��Ѵ�. */
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	/* �׸��� � ���·� �������� �̾� �׸�����. */
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology); 

	return S_OK;
}

HRESULT CVIBuffer_Instance::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	/* �ε��������� ���� �̿��Ͽ� ���� ������ �������� �׷�����. */
	m_pContext->DrawIndexedInstanced(m_iIndexCountPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

void CVIBuffer_Instance::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX*		pVertices = ((VTXMATRIX*)SubResource.pData);

	for (_uint i = 0; i < m_iNumInstance; i++)
	{
		pVertices[i].vPosition.y -= m_pSpeeds[i] * fTimeDelta;

		Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Spread(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX*		pVertices = ((VTXMATRIX*)SubResource.pData);	

	for (_uint i = 0; i < m_iNumInstance; i++)
	{
		_vector		vDir = XMVectorSetW(XMLoadFloat4(&pVertices[i].vPosition) - XMLoadFloat3(&m_InstanceDesc.vPivot), 0.f);
		
		XMStoreFloat4(&pVertices[i].vPosition,
			XMLoadFloat4(&pVertices[i].vPosition) + XMVector3Normalize(vDir) * m_pSpeeds[i] * fTimeDelta);

		Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Compute_LifeTime(VTXMATRIX* pVertices, _uint iInstanceIndex, _float fTimeDelta)
{
	m_pLifeTimes[iInstanceIndex].x += fTimeDelta;

	if (m_pLifeTimes[iInstanceIndex].x > m_pLifeTimes[iInstanceIndex].y)
	{

		if(false == m_InstanceDesc.isLoop)
			pVertices[iInstanceIndex].isLived = false;

		else
		{
			m_pLifeTimes[iInstanceIndex].x = 0.f;
			pVertices[iInstanceIndex].vPosition = Compute_RandPosition();
		}
	}

}



void CVIBuffer_Instance::Free()
{
	__super::Free();


	if (false == m_isCloned)
	{
		Safe_Delete_Array(m_pSpeeds);
		Safe_Delete_Array(m_pInstanceVertices);
	}

	Safe_Delete_Array(m_pLifeTimes);
	
	Safe_Release(m_pVBInstance);
}
