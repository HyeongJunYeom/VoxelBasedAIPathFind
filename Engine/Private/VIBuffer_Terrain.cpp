#include "..\Public\VIBuffer_Terrain.h"

#include "GameInstance.h"
#include "QuadTree.h"

CVIBuffer_Terrain::CVIBuffer_Terrain(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer{ pDevice, pContext }
	, m_pGameInstance { CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

CVIBuffer_Terrain::CVIBuffer_Terrain(const CVIBuffer_Terrain & rhs)
	: CVIBuffer{ rhs }
	, m_iNumVerticesX{ rhs.m_iNumVerticesX }
	, m_iNumVerticesZ{ rhs.m_iNumVerticesZ }
	, m_pGameInstance{ rhs.m_pGameInstance }
	, m_pQuadTree { rhs.m_pQuadTree }
{
	Safe_AddRef(m_pQuadTree);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVIBuffer_Terrain::Initialize_Prototype(const wstring& strHeightMapFilePath)
{
	_ulong		dwByte = { 0 };

	HANDLE		hFile = CreateFile(strHeightMapFilePath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (0 == hFile)
		return E_FAIL;

	BITMAPFILEHEADER			fh;
	BITMAPINFOHEADER			ih;

	if(false == ReadFile(hFile, &fh, sizeof(BITMAPFILEHEADER), &dwByte, nullptr))
		return E_FAIL;
	if(false == ReadFile(hFile, &ih, sizeof(BITMAPINFOHEADER), &dwByte, nullptr))
		return E_FAIL;

	m_iNumVerticesX = ih.biWidth;
	m_iNumVerticesZ = ih.biHeight;

	_uint*			pPixel = new _uint[m_iNumVerticesX * m_iNumVerticesZ];
	ZeroMemory(pPixel, sizeof(_uint) * m_iNumVerticesX * m_iNumVerticesZ);

	if (false == ReadFile(hFile, pPixel, sizeof(_uint) * m_iNumVerticesX * m_iNumVerticesZ, &dwByte, nullptr))
		return E_FAIL;

	CloseHandle(hFile);
	
	m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;
	m_iVertexStride = sizeof(VTXNORTEX);
	m_iNumIndices = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2 * 3;
	m_iIndexStride = sizeof(_uint);
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER
	VTXNORTEX*		pVertices = new VTXNORTEX[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXNORTEX) * m_iNumVertices);

	m_pVerticesPos = new _float3[m_iNumVertices];
	ZeroMemory(m_pVerticesPos, sizeof(_float3) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVerticesZ; i++)
	{
		for (_uint j = 0; j < m_iNumVerticesX; j++)
		{
			_uint	iIndex = i * m_iNumVerticesX + j;

			m_pVerticesPos[iIndex] = pVertices[iIndex].vPosition = 
				_float3(static_cast<_float>(j), (pPixel[iIndex] & 0x000000ff) / 255.0f * g_iTerrainMaxHeight, static_cast<_float>(i));
			pVertices[iIndex].vNormal = _float3(0.0f, 0.f, 0.f);
			pVertices[iIndex].vTexcoord = _float2(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
		}
	}
#pragma endregion

#pragma region INDEX_BUFFER	
	_uint*		pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

	_uint		iNumIndices = { 0 };

	for (_uint i = 0; i < m_iNumVerticesZ - 1; i++)
	{
		for (_uint j = 0; j < m_iNumVerticesX - 1; j++)
		{
			_uint	iIndex = i * m_iNumVerticesX + j;


			_uint	iIndices[] = {
				iIndex + m_iNumVerticesX, 
				iIndex + m_iNumVerticesX + 1,
				iIndex + 1, 
				iIndex
			};

			_vector		vSour, vDest, vNormal;

			/* 오른쪽 위 삼각형의 인덱스 */
			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[1];
			pIndices[iNumIndices++] = iIndices[2];

			vSour = XMLoadFloat3(&pVertices[iIndices[1]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDest = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[1]].vPosition);
			vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));			

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal, XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[1]].vNormal, XMLoadFloat3(&pVertices[iIndices[1]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal, XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);

			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[2];
			pIndices[iNumIndices++] = iIndices[3];

			vSour = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDest = XMLoadFloat3(&pVertices[iIndices[3]].vPosition) - XMLoadFloat3(&pVertices[iIndices[2]].vPosition);
			vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal, XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal, XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[3]].vNormal, XMLoadFloat3(&pVertices[iIndices[3]].vNormal) + vNormal);
		}
	}

	for (_uint i = 0; i < m_iNumVertices; ++i)
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3Normalize(XMLoadFloat3(&pVertices[i].vNormal)));
		
#pragma endregion

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
	
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	/* 인덱스 버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
	Safe_Delete_Array(pPixel);

	m_pQuadTree = CQuadTree::Create(m_iNumVerticesX * m_iNumVerticesZ - m_iNumVerticesX, 
		m_iNumVerticesX * m_iNumVerticesZ - 1, 		
		m_iNumVerticesX - 1, 
		0 
	);
	if (nullptr == m_pQuadTree)
		return E_FAIL;

	m_pQuadTree->SetUp_Neighbors();

	return S_OK;
}

HRESULT CVIBuffer_Terrain::Initialize(void * pArg)
{
	return S_OK;
}

_bool CVIBuffer_Terrain::Compute_Picking(const CTransform* pTransform, _float4* pPickPos)
{
	_float3		vRayDir;
	_float4		vRayPos;

	m_pGameInstance->Transform_PickingToLocalSpace_Perspective(pTransform, &vRayDir, &vRayPos);
	_matrix WorldMatrix = pTransform->Get_WorldMatrix();

	_vector vRayOrigin = XMLoadFloat4(&vRayPos);
	_vector vRayDirection = XMVector3Normalize(XMLoadFloat3(&vRayDir));

	for (size_t i = 0; i < m_iNumVerticesZ - 1; i++)
	{
		for (size_t j = 0; j < m_iNumVerticesX - 1; j++)
		{
			_uint		iIndex = _uint(i * (m_iNumVerticesX)+j);

			_uint		iIndices[4] = {
				iIndex + m_iNumVerticesX,
				iIndex + m_iNumVerticesX + 1,
				iIndex + 1,
				iIndex
			};

			_float fDistance = 0.0f;

			//	우 상단 삼각형과의 검사
			if (true == DirectX::TriangleTests::Intersects(
				vRayOrigin, vRayDirection,
				XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[0]]), 1.f),
				XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[1]]), 1.f),
				XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[2]]), 1.f),
				fDistance))
			{
				_vector vPickPos = vRayOrigin + vRayDirection * fDistance;
				vPickPos = XMVector4Transform(vPickPos, WorldMatrix);

				XMStoreFloat4(pPickPos, vPickPos);

				return true;
			}

			//	좌 하단 삼각형과의 검사
			if (true == DirectX::TriangleTests::Intersects(
				vRayOrigin, vRayDirection,
				XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[0]]), 1.f),
				XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[2]]), 1.f),
				XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[3]]), 1.f),
				fDistance))
			{
				_vector vPickPos = vRayOrigin + vRayDirection * fDistance;
				vPickPos = XMVector4Transform(vPickPos, WorldMatrix);

				XMStoreFloat4(pPickPos, vPickPos);

				return true;
			}
		}
	}

	return false;
}

void CVIBuffer_Terrain::Compute_Height(const CTransform* pTransform, _fvector vPosition, _float4* pPosition)
{
	_matrix			WorldMatrixInv = { pTransform->Get_WorldMatrix_Inverse() };
	_vector			vTargetPos = { XMVector3TransformCoord(vPosition, WorldMatrixInv) };

	if (vTargetPos.m128_f32[0] < 0.f ||
		vTargetPos.m128_f32[2] < 0.f ||
		vTargetPos.m128_f32[0] > m_iNumVerticesX - 1.f ||
		vTargetPos.m128_f32[2] > m_iNumVerticesZ - 1.f)
	{
		XMStoreFloat4(pPosition, vPosition);
		return;
	}

	_uint		iIndex = _uint(XMVectorGetZ(vTargetPos)) * m_iNumVerticesX + _uint(XMVectorGetX(vTargetPos));

	_uint		iIndices[4] = {
		iIndex + m_iNumVerticesX,
		iIndex + m_iNumVerticesX + 1,
		iIndex + 1,
		iIndex
	};

	_float		fWidth = XMVectorGetX(vTargetPos) - m_pVerticesPos[iIndices[0]].x;
	_float		fDepth = m_pVerticesPos[iIndices[0]].z - XMVectorGetZ(vTargetPos);

	_vector			vPlane;

	if (fWidth > fDepth)
		vPlane = { XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[0]]), 1.f), XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[1]]), 1.f), XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[2]]), 1.f)) };
	else
		vPlane = { XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[0]]), 1.f), XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[2]]), 1.f), XMVectorSetW(XMLoadFloat3(&m_pVerticesPos[iIndices[3]]), 1.f)) };

	_float		fHeight = { (-XMVectorGetX(vPlane) * XMVectorGetX(vTargetPos) - XMVectorGetZ(vPlane) * XMVectorGetZ(vTargetPos) - XMVectorGetW(vPlane)) / XMVectorGetY(vPlane) };

	vTargetPos = XMVectorSetY(vTargetPos, fHeight);

	vTargetPos = XMVector3TransformCoord(vTargetPos, pTransform->Get_WorldMatrix());

	XMStoreFloat4(pPosition, vTargetPos);
}

void CVIBuffer_Terrain::Culling(_fmatrix WorldMatrixInv)
{
	m_pGameInstance->TransformFrustum_LocalSpace(WorldMatrixInv);

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);

	_uint*		pIndices = static_cast<_uint*>(SubResource.pData);

	_uint		iNumIndices = { 0 };
	
	m_pQuadTree->Culling(m_pGameInstance, m_pVerticesPos, pIndices, &iNumIndices);

	m_pContext->Unmap(m_pIB, 0);

	m_iNumIndices = iNumIndices;
}

CVIBuffer_Terrain * CVIBuffer_Terrain::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring& strHeightMapFilePath)
{
	CVIBuffer_Terrain*		pInstance = new CVIBuffer_Terrain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strHeightMapFilePath)))
	{
		MSG_BOX(TEXT("Failed To Created : CVIBuffer_Rect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Terrain::Clone(void * pArg)
{
	CVIBuffer_Terrain*		pInstance = new CVIBuffer_Terrain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CVIBuffer_Rect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Terrain::Free()
{
	__super::Free();

	Safe_Release(m_pQuadTree);

	Safe_Release(m_pGameInstance);
}
