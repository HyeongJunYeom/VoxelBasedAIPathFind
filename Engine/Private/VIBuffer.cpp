#include "..\Public\VIBuffer.h"
#include "GameInstance.h"
#include "Transform.h"

CVIBuffer::CVIBuffer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent{ pDevice, pContext }
{
}

CVIBuffer::CVIBuffer(const CVIBuffer & rhs) //헤헤호호
	: CComponent{ rhs }
	, m_pVB { rhs.m_pVB }
	, m_pIB { rhs.m_pIB }
	, m_BufferDesc { rhs.m_BufferDesc }
	, m_InitialData { rhs.m_InitialData }
	, m_iVertexStride { rhs.m_iVertexStride }
	, m_iIndexStride { rhs.m_iIndexStride}
	, m_iNumVertices { rhs.m_iNumVertices }
	, m_iNumIndices { rhs.m_iNumIndices }
	, m_iNumVertexBuffers { rhs.m_iNumVertexBuffers }
	, m_eIndexFormat { rhs.m_eIndexFormat }
	, m_ePrimitiveTopology { rhs.m_ePrimitiveTopology }
	, m_pVerticesPos { rhs.m_pVerticesPos }
	, m_pIndices {rhs.m_pIndices}
{
	Safe_AddRef(m_pVB);
	Safe_AddRef(m_pIB);
}

HRESULT CVIBuffer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVIBuffer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CVIBuffer::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	/* 인덱스버퍼의 값을 이용하여 정점 버퍼의 정점들을 그려낸다. */
	m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

	return S_OK;
}

HRESULT CVIBuffer::Bind_Buffers()
{
	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB, 		
		
	
	};

	_uint				iVertexStrides[] = {
		m_iVertexStride
		
	};

	_uint				iOffsets[] = {
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

HRESULT CVIBuffer::Cooking_Static_Voxel(CTransform* pTransform)
{
	if (nullptr == pTransform)
		return E_FAIL;

	const _float		fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };
	const _float		fHalfVoxelSize = { fVoxelSize * 0.5f };
	_matrix				WorldMatrix = { pTransform->Get_WorldMatrix() };

	unordered_set<_uint>			TempVoxelIndices;

	vector<_float3>			Sample_Poses;
	if (FAILED(Sampling_Poses(WorldMatrix, Sample_Poses, fHalfVoxelSize)))
		return E_FAIL;

	for(auto& vSamplePos : Sample_Poses)
	{
		_uint				iResultVoxelIndex;
		m_pGameInstance->Get_Index_Voxel(vSamplePos, iResultVoxelIndex);

		TempVoxelIndices.insert(iResultVoxelIndex);
	}	

	for (auto& iVoxelIndex : TempVoxelIndices)
	{
		_uint3			vIndexPos;
		m_pGameInstance->Get_IndexPosition_Voxel(iVoxelIndex, vIndexPos);
		m_pGameInstance->Add_Voxel(vIndexPos, VOXEL_LAYER::_STATIC, VOXEL_ID::_FLOOR);
	}

	return S_OK;
}

HRESULT CVIBuffer::Sampling_Poses(_fmatrix WorldMatrix, vector<_float3>& SamplePoses, const _float fSampleOffset)
{
	for (_uint i = 0; i < m_iNumIndices; )
	{
		_vector				vPosA = { XMVector3TransformCoord(XMLoadFloat3(&m_pVerticesPos[m_pIndices[i++]]), WorldMatrix) };
		_vector				vPosB = { XMVector3TransformCoord(XMLoadFloat3(&m_pVerticesPos[m_pIndices[i++]]), WorldMatrix) };
		_vector				vPosC = { XMVector3TransformCoord(XMLoadFloat3(&m_pVerticesPos[m_pIndices[i++]]), WorldMatrix) };

		_float				fDistAB = { XMVectorGetX(XMVector3Length(vPosB - vPosA)) };
		_float				fDistBC = { XMVectorGetX(XMVector3Length(vPosC - vPosB)) };
		_float				fDistCA = { XMVectorGetX(XMVector3Length(vPosA - vPosC)) };

		_float				fMaxDist = { max(fDistAB, max(fDistBC, fDistCA)) };

		_vector				vPos1st, vPos2nd, vPos3rd;

		if (fDistAB >= fDistBC &&
			fDistCA >= fDistBC)
		{
			vPos1st = vPosA;

			if (fDistAB >= fDistCA)
			{
				vPos2nd = vPosB;
				vPos3rd = vPosC;
			}
			else
			{
				vPos2nd = vPosC;
				vPos3rd = vPosB;
			}
		}
		else if (fDistAB >= fDistCA &&
			fDistBC >= fDistCA)
		{
			vPos1st = vPosB;

			if (fDistAB >= fDistBC)
			{
				vPos2nd = vPosA;
				vPos3rd = vPosC;
			}
			else
			{
				vPos2nd = vPosC;
				vPos3rd = vPosA;
			}
		}
		else if (fDistCA >= fDistAB &&
			fDistBC >= fDistAB)
		{
			vPos1st = vPosC;

			if (fDistCA >= fDistBC)
			{
				vPos2nd = vPosA;
				vPos3rd = vPosB;
			}
			else
			{
				vPos2nd = vPosB;
				vPos3rd = vPosA;
			}
		}

		_uint				iSampleCnt = { static_cast<_uint>(fMaxDist / fSampleOffset + EPSILON) + 1 };

		for (_uint j = 0; j <= iSampleCnt; ++j)
		{
			_vector				vSamplePosSrc = { XMVectorLerp(vPos1st, vPos2nd, static_cast<_float>(j) / iSampleCnt) };
			_vector				vSamplePosDst = { XMVectorLerp(vPos1st, vPos3rd, static_cast<_float>(j) / iSampleCnt) };

			_float				fDistSample = { XMVectorGetX(XMVector3Length(vSamplePosSrc - vSamplePosDst)) };
			_uint				iSampleCntSrcDst = { static_cast<_uint>(fDistSample / fSampleOffset + EPSILON) + 1 };

			for (_uint k = 0; k <= iSampleCntSrcDst; ++k)
			{
				_vector				vResultSmaplePos = { XMVectorLerp(vSamplePosSrc, vSamplePosDst, static_cast<_float>(k) / iSampleCntSrcDst) };
				_float3				vResultSamplePosFloat3;

				XMStoreFloat3(&vResultSamplePosFloat3, vResultSmaplePos);

				SamplePoses.push_back(vResultSamplePosFloat3);
			}
		}
	}

	return S_OK;
}

_bool CVIBuffer::Compute_Picking(const CTransform* pTransform, _float4* pPickPos)
{
	return false;
}



HRESULT CVIBuffer::Create_Buffer(ID3D11Buffer ** ppBuffer)
{
	return m_pDevice->CreateBuffer(&m_BufferDesc, &m_InitialData, ppBuffer);
}

void CVIBuffer::Free()
{
	__super::Free();

	if (false == m_isCloned)
	{
		Safe_Delete_Array(m_pIndices);
		Safe_Delete_Array(m_pVerticesPos);
	}

	Safe_Release(m_pVB);
	Safe_Release(m_pIB);
}
