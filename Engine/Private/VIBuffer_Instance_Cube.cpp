#include "VIBuffer_Instance_Cube.h"
#include "GameInstance.h"

CVIBuffer_Instance_Cube::CVIBuffer_Instance_Cube(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Instance_Cube::CVIBuffer_Instance_Cube(const CVIBuffer_Instance_Cube& rhs)
	: CVIBuffer{ rhs }
	, m_iInstanceStride{ rhs.m_iInstanceStride }
	, m_iNumMaxInstance{ rhs.m_iNumMaxInstance }
	, m_iIndexCountPerInstance{ rhs.m_iIndexCountPerInstance }
	, m_InstanceBufferDesc{ rhs.m_InstanceBufferDesc }
	, m_InstanceSubResourceData{ rhs.m_InstanceSubResourceData }
{
}

HRESULT CVIBuffer_Instance_Cube::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_iNumMaxInstance = 1024;
	m_iNumCurInstance = 0;
	m_iInstanceStride = sizeof(VTXINSTANCE_CUBE);
	m_iIndexCountPerInstance = 1;

	m_iNumVertices = 1;
	m_iVertexStride = sizeof(VTXPOS);

	m_iNumIndices = m_iIndexCountPerInstance * m_iNumMaxInstance;
	m_iIndexStride = sizeof(_ushort);
	m_iNumVertexBuffers = 2;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

#pragma region VERTEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXPOS*		pVertices = { new VTXPOS[m_iNumVertices] };
	ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);

	pVertices[0].vPosition = _float3(0.f, 0.f, 0.f);

	ZeroMemory(&m_InitialData, sizeof(m_InitialData));
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX_BUFFER

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 인덱스 버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;

	_ushort*		pIndices = { new _ushort[m_iNumIndices] };
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

#pragma region INSTANCE_BUFFER

	ZeroMemory(&m_InstanceBufferDesc, sizeof m_InstanceBufferDesc);

	/* 인덱스 버퍼의 byte크기 */
	m_InstanceBufferDesc.ByteWidth = m_iInstanceStride * m_iNumMaxInstance;
	m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_InstanceBufferDesc.MiscFlags = 0;
	m_InstanceBufferDesc.StructureByteStride = m_iInstanceStride;

	m_pInstanceVertices = new VTXINSTANCE_CUBE[m_iNumMaxInstance];
	ZeroMemory(m_pInstanceVertices, sizeof(VTXINSTANCE_CUBE) * m_iNumMaxInstance);

	for (size_t i = 0; i < m_iNumMaxInstance; i++)
	{
		XMStoreFloat3(&m_pInstanceVertices[i].vPosition, XMVectorSet(0.f, 0.f, 0.f, 1.f));
		m_pInstanceVertices[i].iID = static_cast<_int>(VOXEL_ID::_END);
		m_pInstanceVertices[i].iState = static_cast<_int>(VOXEL_STATE::_END);
		m_pInstanceVertices[i].fScale = 1.f;
	}

	ZeroMemory(&m_InstanceSubResourceData, sizeof m_InstanceSubResourceData);
	m_InstanceSubResourceData.pSysMem = m_pInstanceVertices;

	if (FAILED(m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &m_InstanceSubResourceData, &m_pVBInstance)))
		return E_FAIL;
	Safe_Delete_Array(m_pInstanceVertices);

#pragma endregion


	

	return S_OK;
}

HRESULT CVIBuffer_Instance_Cube::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Instance_Cube::Bind_Buffers()
{
	if (0 >= m_iNumCurInstance)
		return S_OK;

	ID3D11Buffer* pVertexBuffers[] = {
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

	/* 정점버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);

	/* 인덱스버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	/* 그릴때 어떤 형태로 정점들을 이어 그릴건지. */
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}

HRESULT CVIBuffer_Instance_Cube::Render()
{
	if (0 >= m_iNumCurInstance)
		return S_OK;

	/* 인덱스버퍼의 값을 이용하여 정점 버퍼의 정점들을 그려낸다. */
	m_pContext->DrawIndexedInstanced(m_iIndexCountPerInstance, m_iNumCurInstance, 0, 0, 0);

	return S_OK;
}

void CVIBuffer_Instance_Cube::Set_Positions(const vector<_float3>& Poses, _float fScale)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);

	VTXINSTANCE_CUBE*			pVertices = ((VTXINSTANCE_CUBE*)SubResource.pData);

	m_iNumCurInstance = static_cast<_uint>(Poses.size());
	_uint iIndex{};
	for (auto& vPos : Poses)
	{
		*(reinterpret_cast<_float3*>(&pVertices[iIndex].vPosition)) = vPos;
		pVertices[iIndex].fScale = fScale;
		++iIndex;
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance_Cube::Set_Positions_States_IDs(const vector<_float3>& Poses, const vector<_byte>& States, const vector<_byte>& IDs, const vector<_int>& NeighborFlags, _float fScale)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);

	VTXINSTANCE_CUBE* pVertices = ((VTXINSTANCE_CUBE*)SubResource.pData);

	m_iNumCurInstance = static_cast<_uint>(Poses.size());

	for(_uint iIndex  = 0; iIndex < m_iNumCurInstance; ++iIndex)
	{
		*(reinterpret_cast<_float3*>(&pVertices[iIndex].vPosition)) = Poses[iIndex];
		pVertices[iIndex].fScale = fScale;
		pVertices[iIndex].iID = IDs[iIndex];
		pVertices[iIndex].iState = States[iIndex];
		pVertices[iIndex].iNeighborFlag = NeighborFlags[iIndex];
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

CVIBuffer_Instance_Cube* CVIBuffer_Instance_Cube::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Instance_Cube*		pInstance = { new CVIBuffer_Instance_Cube(pDevice, pContext) };

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CVIBuffer_Instance_Cube"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Instance_Cube::Clone(void* pArg)
{
	CVIBuffer_Instance_Cube*		pInstance = { new CVIBuffer_Instance_Cube(*this) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CVIBuffer_Instance_Cube"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Instance_Cube::Free()
{
	__super::Free();
}
