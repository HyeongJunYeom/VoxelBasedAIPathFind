#include "Boid.h"
#include "GameInstance.h"

_float CBoid::ms_fRatio_FlowField = { 1.f };
_float CBoid::ms_fRatio_Alignment = { 1.f };
_float CBoid::ms_fRatio_Cohesion = { 1.f };
_float CBoid::ms_fRatio_Sepration = { 1.f };
_float CBoid::ms_fRatio_Avoidence = { 1.f };
_float CBoid::ms_fArriveRangeFromGoal = { 1.f };


CBoid::CBoid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CBoid::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	BOID_DESC* pDesc = { static_cast<BOID_DESC*>(pArg) };
	m_pTransform = pDesc->pTransform;
	m_pMove = pDesc->pMove;
	m_pArrived = pDesc->pArrived;

	if (nullptr == m_pTransform ||
		nullptr == m_pMove ||
		nullptr == m_pArrived)
		return E_FAIL;

	Safe_AddRef(m_pTransform);

	return S_OK;
}

void CBoid::Tick(_float fTimeDelta)
{

}

HRESULT CBoid::Compute_ResultDirection_BoidMove(_fvector vSrcPos, _float3& vResultDirection)
{
	vector<_uint>	NeighborIndices = { move(Get_NeighborIndices()) };
	_float3			vAlignmentDirFloat3, vCohesionDirFloat3, vSeparationDirFloat3, vAvoidenceDirFloat3;

	/*if (FAILED(m_pGameInstance->Get_Direction_FlowField(vSrcPosFloat3, vFlowDirFloat3)))
	{
		vSrcPosFloat3.y -= m_pGameInstance->Get_WorldSize_Voxel();
		if (FAILED(m_pGameInstance->Get_Direction_FlowField(vSrcPosFloat3, vFlowDirFloat3)))
			return E_FAIL;
	}*/
	if (FAILED(Compute_Separation_Direction(NeighborIndices, vSrcPos, vSeparationDirFloat3)))
		return E_FAIL;
	if (FAILED(Compute_Avoidence_Direction(NeighborIndices, vSrcPos, vAvoidenceDirFloat3)))
		return E_FAIL;

	_vector			vSeparationDir = { XMLoadFloat3(&vSeparationDirFloat3) * ms_fRatio_Sepration };
	_vector			vAvoidenceDir = { XMLoadFloat3(&vAvoidenceDirFloat3) * ms_fRatio_Avoidence };

	if (true == (*m_pMove))
	{
		if (FAILED(Compute_Alignment_Direction(NeighborIndices, vAlignmentDirFloat3)))
			return E_FAIL;
		if (FAILED(Compute_Cohesion_Direction(NeighborIndices, vSrcPos, vCohesionDirFloat3)))
			return E_FAIL;


		_vector			vFlowDir = { XMVector3Normalize(XMLoadFloat3(&m_vNextPos) - vSrcPos) * ms_fRatio_FlowField };
		_vector			vAlignmentDir = { XMLoadFloat3(&vAlignmentDirFloat3) * ms_fRatio_Alignment };
		_vector			vCohesionDir = { XMLoadFloat3(&vCohesionDirFloat3) * ms_fRatio_Cohesion };

		_vector			vReulstDirectionVector = { XMVector3Normalize(vFlowDir + vAlignmentDir + vCohesionDir + vSeparationDir + vAvoidenceDir) };
		vReulstDirectionVector = XMVector3Normalize(XMVectorSetY(vReulstDirectionVector, XMVectorGetY(vFlowDir)));
		XMStoreFloat3(&vResultDirection, vReulstDirectionVector);
	}

	else
	{
		_vector			vReulstDirectionVector = { XMVector3Normalize(vSeparationDir + vAvoidenceDir) };
		vReulstDirectionVector = XMVector3Normalize(XMVectorSetY(vReulstDirectionVector,0.f));
		XMStoreFloat3(&vResultDirection, vReulstDirectionVector);
	}

	return S_OK;
}

vector<_uint> CBoid::Get_NeighborIndices()
{
	vector<_uint>			NeighborIndices;
	if (FAILED(m_pGameInstance->Get_Neighbor_Indices_Boid(this, m_fMaxDist, NeighborIndices)))
		return vector<_uint>();
	else
		return NeighborIndices;
}

HRESULT CBoid::Compute_Alignment_Direction(const vector<_uint>& NeighborIndices, _float3& vResultDir)
{
	_vector			vAlignmentDir = { XMVectorZero() };
	_uint			iNumAlignment = {};

	for (auto iNeighborIndex : NeighborIndices)
	{
		_uint		iNeighborFlowFieldIndexTag;
		if (FAILED(m_pGameInstance->Get_Neighbor_FlowFieldIndex_Boid(iNeighborIndex, iNeighborFlowFieldIndexTag)) ||
			iNeighborFlowFieldIndexTag != m_iCurFlowFieldIndex)
			continue;

		_float3				vNeighborPosFloat3;
		if (FAILED(m_pGameInstance->Get_Neighbor_Position_Boid(iNeighborIndex, vNeighborPosFloat3)))
			return E_FAIL;

		/*_float3				vNeighborDirFloat3;
		if (FAILED(m_pGameInstance->Get_Direction_FlowField(vNeighborPosFloat3, vNeighborDirFloat3)))
		{
			continue;
		}*/

		_uint			iParentVoxelIndex;
		_float3			vNeighborNextPos;
		if (!(FAILED(m_pGameInstance->Get_ParentIndex_FlowField(vNeighborPosFloat3, m_iCurFlowFieldIndex, iParentVoxelIndex))))
		{
			m_pGameInstance->Get_WorldPosition_Voxel(iParentVoxelIndex, vNeighborNextPos);
			_vector			vNeighborDir = { XMVector3Normalize(XMLoadFloat3(&vNeighborNextPos) - XMLoadFloat3(&vNeighborPosFloat3)) };
			vAlignmentDir += vNeighborDir;
			++iNumAlignment;
		}



		//	vAlignmentDir += XMLoadFloat3(&vNeighborDirFloat3);
		//	++iNumAlignment;
	}

	if (0 < iNumAlignment)
	{
		vAlignmentDir = XMVector3Normalize(vAlignmentDir);
	}

	XMStoreFloat3(&vResultDir, vAlignmentDir);
	return S_OK;
}

HRESULT CBoid::Compute_Cohesion_Direction(const vector<_uint>& NeighborIndices, _fvector vSrcPos, _float3& vResultDir)
{
	_vector			vCenterPos = { XMVectorZero() };
	_vector			vToCenterDir = { XMVectorZero() };
	_uint			iNumCohesion = {};

	for (auto iNeighborIndex : NeighborIndices)
	{
		_uint		iNeighborFlowFieldIndexTag;
		if (FAILED(m_pGameInstance->Get_Neighbor_FlowFieldIndex_Boid(iNeighborIndex, iNeighborFlowFieldIndexTag)) ||
			iNeighborFlowFieldIndexTag != m_iCurFlowFieldIndex)
			continue;

		_float3				vNeighborPosFloat3;
		if (FAILED(m_pGameInstance->Get_Neighbor_Position_Boid(iNeighborIndex, vNeighborPosFloat3)))
			return E_FAIL;

		vCenterPos += XMLoadFloat3(&vNeighborPosFloat3);
		++iNumCohesion;
	}

	if (0 < iNumCohesion)
	{
		vCenterPos /= iNumCohesion;
		vToCenterDir = XMVector3Normalize(vCenterPos - vSrcPos);
	}

	XMStoreFloat3(&vResultDir, vToCenterDir);
	return S_OK;
}

HRESULT CBoid::Compute_Separation_Direction(const vector<_uint>& NeighborIndices, _fvector vSrcPos, _float3& vResultDir)
{
	_vector			vSeparationForceDir = { XMVectorZero() };

	for (auto iNeighborIndex : NeighborIndices)
	{
		_float3				vNeighborPosFloat3;
		if (FAILED(m_pGameInstance->Get_Neighbor_Position_Boid(iNeighborIndex, vNeighborPosFloat3)))
			return E_FAIL;

		_float				fNeighborDist = { XMVectorGetX(XMVector3Length(XMLoadFloat3(&vNeighborPosFloat3) - vSrcPos)) };

		_float				fMinDist = { m_fMinDist };

		if (false == *m_pMove)
			fMinDist *= 0.75f;

		if (fMinDist < fNeighborDist ||
			0.f >= fNeighborDist)
			continue;

		vSeparationForceDir += XMVector3Normalize(vSrcPos - XMLoadFloat3(&vNeighborPosFloat3)) / fNeighborDist;
	}

	XMStoreFloat3(&vResultDir, vSeparationForceDir);
	return S_OK;
}

HRESULT CBoid::Compute_Avoidence_Direction(const vector<_uint>& NeighborIndices, _fvector vSrcPos, _float3& vResultDir)
{
	_vector			vAvoidenceDir = { XMVectorZero() };
	_uint3			vVoxelIndexPosUInts;
	_float3			vSrcPosFloat3;
	XMStoreFloat3(&vSrcPosFloat3, vSrcPos);

	if (FAILED(m_pGameInstance->Get_IndexPosition_Voxel(vSrcPosFloat3, vVoxelIndexPosUInts)))
		return E_FAIL;
	_vector			vVoxelIndexPos = { XMLoadUInt3(&vVoxelIndexPosUInts) };
	_uint			iNumAvoidence = {};

	for (_int iOffsetZ = -1; iOffsetZ <= 1; ++iOffsetZ)
	{
		for (_int iOffsetY = -1; iOffsetY <= 1; ++iOffsetY)
		{
			for (_int iOffsetX = -1; iOffsetX <= 1; ++iOffsetX)
			{
				_uint3		vNewVoxelIndexPos = {
					vVoxelIndexPosUInts.x + iOffsetX,
					vVoxelIndexPosUInts.y + iOffsetY,
					vVoxelIndexPosUInts.z + iOffsetZ
				};

				VOXEL_ID			eVoxelID;
				if (FAILED(m_pGameInstance->Get_VoxelID(vNewVoxelIndexPos, eVoxelID, VOXEL_LAYER::_OBSTACLE_STATIC)))
					continue;

				_vector				vDirection = { XMVector3Normalize(vVoxelIndexPos - XMLoadUInt3(&vNewVoxelIndexPos)) };
				vAvoidenceDir += vDirection;
				++iNumAvoidence;
			}
		}
	}

	if (0 < iNumAvoidence)
		vAvoidenceDir = XMVector3Normalize(vAvoidenceDir);

	XMStoreFloat3(&vResultDir, vAvoidenceDir);

	return S_OK;
}

CBoid* CBoid::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CBoid* pInstance = { new CBoid(pDevice, pContext) };
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CBoid"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBoid::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pTransform);

}
