#include "VIBuffer_Line.h"
#include "GameInstance.h"

CVIBuffer_Line::CVIBuffer_Line(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Line::CVIBuffer_Line(const CVIBuffer_Line& rhs)
	: CVIBuffer{ rhs }
{
}

HRESULT CVIBuffer_Line::Initialize_Prototype()
{
	m_iNumVertices = 2;
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumIndices = 2;
	m_iIndexStride = sizeof(_ubyte);
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R8_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	m_pVerticesPos = new _float3[m_iNumVertices];

#pragma region VERTEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXPOS*			pVertices = { new VTXPOS[m_iNumVertices] };
	ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);

	pVertices[0].vPosition = { 0.f, 0.f, 0.f };
	pVertices[1].vPosition = { 0.f, 0.f, 1.f };

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
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

	_ubyte*			pIndices = { new _ubyte[m_iNumIndices] };
	ZeroMemory(pIndices, sizeof(_ubyte) * m_iNumIndices);

	pIndices[0] = 0;
	pIndices[1] = 1;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Line::Initialize(void* pArg)
{
	return S_OK;
}

_float4x4 CVIBuffer_Line::Compute_Look_At(_fvector vLookPos, _fvector vAtPos)
{
	_vector		vDirectionAt = { vAtPos - vLookPos };
	_vector		vLookOriginDir = { XMVectorSet(0.f, 0.f, 1.f, 0.f) };
	_float		fDist = { XMVectorGetX(XMVector3Length(vDirectionAt)) };

	_vector		vRotateAxis;
	_float		fDot = { XMVectorGetX(XMVector3Dot(XMVector3Normalize(vLookOriginDir), XMVector3Normalize(vDirectionAt))) };
	_float		fAngle = { acosf(fDot) };



	if (fDot <= -1.f + 0.000005f || 
		fDot >= 1.f - 0.000005f)
	{
		vRotateAxis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	}
	else
	{
		vRotateAxis = XMVector3Cross(XMVector3Normalize(vLookOriginDir), XMVector3Normalize(vDirectionAt));
	}

	_matrix		ScaleMatrix = { XMMatrixScaling(fDist, fDist, fDist) };
	_matrix		RoatationMatrix = { XMMatrixRotationAxis(vRotateAxis, fAngle) };
	_matrix		TranslationMarix = { XMMatrixTranslation(XMVectorGetX(vLookPos), XMVectorGetY(vLookPos), XMVectorGetZ(vLookPos)) };

	_matrix		WorldMatrix = { ScaleMatrix * RoatationMatrix * TranslationMarix };

	_float4x4	WorldMatrixFloat4x4;
	XMStoreFloat4x4(&WorldMatrixFloat4x4, WorldMatrix);

	return WorldMatrixFloat4x4;
}

CVIBuffer_Line* CVIBuffer_Line::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Line* pInstance = new CVIBuffer_Line(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CVIBuffer_Line"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Line::Clone(void* pArg)
{
	CVIBuffer_Line* pInstance = new CVIBuffer_Line(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CVIBuffer_Line"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Line::Free()
{
	__super::Free();
}
