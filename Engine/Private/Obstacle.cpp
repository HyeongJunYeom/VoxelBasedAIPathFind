#include "Obstacle.h"
#include "GameInstance.h"
#include "Transform.h"
#include "Bounding.h"

CObstacle::CObstacle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CObstacle::CObstacle(const CObstacle& rhs)
	: CComponent{ rhs }
{
}

HRESULT CObstacle::Initialize_Prototype()
{
	if (FAILED(Sample_Point_Cloud()))
		return E_FAIL;
	if (FAILED(Cull_Point_Cloud_Shape()))
		return E_FAIL;

	return S_OK;
}

HRESULT CObstacle::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	OBSTACLE_DESC*			pDesc = { static_cast<OBSTACLE_DESC*>(pArg) };
	m_pTransformCom = pDesc->pTransform;

	if (nullptr == m_pTransformCom)
		return E_FAIL;

	Safe_AddRef(m_pTransformCom);

	if (FAILED(Update_VoxelWorld()))
		return E_FAIL;
	
	return S_OK;
}

void CObstacle::Tick(_float fTimeDelta)
{
	if (FAILED(Update_VoxelWorld()))
		return;
}

HRESULT CObstacle::Render()
{
	return S_OK;
}

HRESULT CObstacle::Sample_Point_Cloud()
{
	const _float			fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };
	const _int				iNumSample = { static_cast<_int>(m_fRadius / (fVoxelSize * 0.5f)) };
	for (_int iOffsetZ = -iNumSample; iOffsetZ <= iNumSample; ++iOffsetZ)
	{
		for (_int iOffsetY = -iNumSample; iOffsetY <= iNumSample; ++iOffsetY)
		{
			for (_int iOffsetX = -iNumSample; iOffsetX <= iNumSample; ++iOffsetX)
			{
				_int3			vOffsetInts = { iOffsetX, iOffsetY, iOffsetZ };
				_vector			vOffset = { XMLoadSInt3(&vOffsetInts) };

				_vector			vResult = { vOffset / static_cast<_float>(iNumSample) * m_fRadius };
				_float3			vResultFloat3;

				XMStoreFloat3(&vResultFloat3, vResult);

				m_LocalPointCloud.push_back(vResultFloat3);
			}
		}
	}	

	return S_OK;
}

HRESULT CObstacle::Update_VoxelWorld()
{
	_matrix					WorldMatrix = { m_pTransformCom->Get_WorldMatrix() };
	vector<_uint3>			CurIndexPoses;
	unordered_set<_uint>	CurIndicies;

	for (auto& vLocalPosFloat3 : m_LocalPointCloud)
	{
		_vector				vWorldPos = { XMVector3TransformCoord(XMLoadFloat3(&vLocalPosFloat3), WorldMatrix) };
		_float3				vWorldPosFloat3;

		XMStoreFloat3(&vWorldPosFloat3, vWorldPos);

		_uint				iIndex;
		if (FAILED(m_pGameInstance->Get_Index_Voxel(vWorldPosFloat3, iIndex)))
			continue;

		CurIndicies.insert(iIndex);
	}

	for (auto iterSrc = m_ActiveIndices.begin(); iterSrc != m_ActiveIndices.end(); )
	{
		_uint			iIndex = { *iterSrc };
		auto iterDst{ CurIndicies.find(iIndex) };
		if (iterDst == CurIndicies.end())
		{
			_uint3			vIndexPos;
			if (FAILED(m_pGameInstance->Get_IndexPosition_Voxel(iIndex, vIndexPos)))
				continue;
			m_pGameInstance->Erase_Voxel(vIndexPos, VOXEL_LAYER::_OBSTACLE_STATIC);
			iterSrc = m_ActiveIndices.erase(iterSrc);
		}

		else
			++iterSrc;
	}

	for (auto iterSrc = CurIndicies.begin(); iterSrc != CurIndicies.end(); ++iterSrc)
	{
		_uint			iIndex = { *iterSrc };
		auto iterDst{ m_ActiveIndices.find(iIndex) };
		if (iterDst == m_ActiveIndices.end())
		{
			_uint3			vIndexPos;
			if (FAILED(m_pGameInstance->Get_IndexPosition_Voxel(iIndex, vIndexPos)))
				continue;
			m_pGameInstance->Add_Voxel(vIndexPos, VOXEL_LAYER::_OBSTACLE_STATIC, VOXEL_ID::_OBSTACLE);
			m_ActiveIndices.insert(iIndex);
		}
	}

	return S_OK;
}

void CObstacle::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);

	for (const auto iIndex: m_ActiveIndices)
	{
		m_pGameInstance->Erase_Voxel(iIndex, VOXEL_LAYER::_OBSTACLE_STATIC);
	}
}
