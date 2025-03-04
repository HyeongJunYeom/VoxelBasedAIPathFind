#include "Voxel_Convertor.h"
#include "GameInstance.h"
#include "Transform.h"

CVoxel_Convertor::CVoxel_Convertor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_Convertor::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	VOXEL_CONVERTOR_DESC*		pSectorLayerDesc = { static_cast<VOXEL_CONVERTOR_DESC*>(pArg) };
	m_pNumVoxelWorldDim = pSectorLayerDesc->pNumVoxelWorldDim;
	m_pNumVoxelSectorDim = pSectorLayerDesc->pNumVoxelSectorDim;
	m_pNumSectorWorldDim = pSectorLayerDesc->pNumSectorWorldDim;
	m_pVoxelSize = pSectorLayerDesc->pVoxelSize;
	m_pTransformCom = pSectorLayerDesc->pTransform;

	Safe_AddRef(m_pTransformCom);

	if (nullptr == m_pNumVoxelWorldDim ||
		nullptr == m_pNumVoxelSectorDim ||
		nullptr == m_pNumSectorWorldDim ||
		nullptr == m_pVoxelSize ||
		nullptr == m_pTransformCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_WorldPos_From_IndexPos(const _uint3& vIndexPos, _float3& vWorldPos)
{
	if (true == Is_Out_Of_Range_Voxel(vIndexPos))
		return E_FAIL;

	_vector		vLocalPos = { XMLoadUInt3(&vIndexPos) * (*m_pVoxelSize) };
	_matrix		WorldMatrix = { m_pTransformCom->Get_WorldMatrix() };

	_vector		vWorldPosVector = XMVector3TransformCoord(vLocalPos, WorldMatrix);
	XMStoreFloat3(&vWorldPos, vWorldPosVector);

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_LocalPos_From_IndexPos(const _uint3& vIndexPos, _float3& vLocalPos)
{
	if (true == Is_Out_Of_Range_Voxel(vIndexPos))
		return E_FAIL;

	_vector		vLocalPosVector = { XMLoadUInt3(&vIndexPos) * (*m_pVoxelSize) };

	XMStoreFloat3(&vLocalPos, vLocalPosVector);

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_IndexPos_From_WorldPos(const _float3& vWorldPos, _uint3& vIndexPos)
{
	if (true == Is_Out_Of_Range_WorldPos(vWorldPos))
		return E_FAIL;

	_float		fWorldVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };
	_float		fOffset = { fWorldVoxelSize * 0.5f };

	_vector		vWorldPosVector = { XMLoadFloat3(&vWorldPos) + XMVectorSet(fOffset, fOffset, fOffset, 0.f) };
	_matrix		WorldMatrixInv = { m_pTransformCom->Get_WorldMatrix_Inverse() };

	_vector		vLocalPos = { XMVector3TransformCoord(vWorldPosVector, WorldMatrixInv) };
	_vector		vIndexPosVector = { vLocalPos / (*m_pVoxelSize) };

	XMStoreUInt3(&vIndexPos, vIndexPosVector);

	if (true == Is_Out_Of_Range_Voxel(vIndexPos))
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_VoxelIndex_From_IndexPos(const _uint3& vIndexPos, _uint& iIndex)
{
	if (true == Is_Out_Of_Range_Voxel(vIndexPos))
		return E_FAIL;

	iIndex = (vIndexPos.x << g_iShift_X)
		+ (vIndexPos.y << g_iShift_Y)
		+ (vIndexPos.z << g_iShift_Z);

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_VoxelIndex_From_WorldPos(const _float3& vWorldPos, _uint& iIndex)
{
	_uint3			vIndexPos;
	if (FAILED(Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Convert_VoxelIndex_From_IndexPos(vIndexPos, iIndex);
}

HRESULT CVoxel_Convertor::Convert_SectorIndex_From_IndexPos(const _uint3& vIndexPos, _uint& iIndex)
{
	if (true == Is_Out_Of_Range_Voxel(vIndexPos))
		return E_FAIL;

	_uint3		vSectorIndexPos;
	vSectorIndexPos.x = vIndexPos.x / (*m_pNumVoxelSectorDim);
	vSectorIndexPos.y = vIndexPos.y / (*m_pNumVoxelSectorDim);
	vSectorIndexPos.z = vIndexPos.z / (*m_pNumVoxelSectorDim);

	iIndex = vSectorIndexPos.x + (vSectorIndexPos.y * (*m_pNumSectorWorldDim)) + (vSectorIndexPos.z * (*m_pNumSectorWorldDim) * (*m_pNumSectorWorldDim));

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_SectorIndex_From_WorldPos(const _float3& vWorldPos, _uint& iIndex)
{	
	_uint3			vIndexPos;
	if (FAILED(Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Convert_SectorIndex_From_IndexPos(vIndexPos, iIndex);
}

HRESULT CVoxel_Convertor::Convert_SectorIndex_From_VoxelIndex(const _uint iVoxelIndex, _uint& iSectorIndex)
{
	_uint3			vIndexPos;
	if (FAILED(Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	if (FAILED(Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_IndexPos_From_Index(const _uint iIndex, _uint3& vIndexPos)
{
	if (iIndex > g_iVoxelIndexLimit)
		return E_FAIL;

	vIndexPos.x = iIndex & g_iConvertFlagX;
	vIndexPos.y = (iIndex & g_iConvertFlagY) >> g_iShift_Y;
	vIndexPos.z = (iIndex & g_iConvertFlagZ) >> g_iShift_Z;

	return S_OK;
}

HRESULT CVoxel_Convertor::Convert_LocalPos_From_Index(const _uint iIndex, _float3& vLocalPos)
{
	_uint3			vIndexPos;
	if (FAILED(Convert_IndexPos_From_Index(iIndex, vIndexPos)))
		return E_FAIL;

	return Convert_LocalPos_From_IndexPos(vIndexPos, vLocalPos);
}

HRESULT CVoxel_Convertor::Convert_WorldPos_From_Index(const _uint iIndex, _float3& vWorldPos)
{
	_uint3			vIndexPos;
	if (FAILED(Convert_IndexPos_From_Index(iIndex, vIndexPos)))
		return E_FAIL;

	return Convert_WorldPos_From_IndexPos(vIndexPos, vWorldPos);
}



_bool CVoxel_Convertor::Is_Out_Of_Range_WorldPos(const _float3& vWorldPos)
{
	if ((*m_pVoxelSize) * -0.5f > min(vWorldPos.x, min(vWorldPos.y, vWorldPos.z)))
		return true;

	if ((*m_pVoxelSize) * ((static_cast<_float>(*m_pNumVoxelWorldDim) - 1.f) + 0.5f) < max(vWorldPos.x, (max(vWorldPos.y, vWorldPos.z))))
		return true;

	return false;
}

_bool CVoxel_Convertor::Is_Out_Of_Range_LocalPos(const _float3& vLocalPos)
{
	if (vLocalPos.x < 0.f ||
		vLocalPos.x >= static_cast<_float>(g_iWorldLength) - 1.f ||
		vLocalPos.y < 0.f ||
		vLocalPos.y >= static_cast<_float>(g_iWorldLength) - 1.f ||
		vLocalPos.z < 0.f ||
		vLocalPos.z >= static_cast<_float>(g_iWorldLength) - 1.f)
		return true;

	return false;
}

_bool CVoxel_Convertor::Is_Out_Of_Range_Voxel(const _uint3& vIndexPos)
{
	if (g_iVoxelPosLimit < max(vIndexPos.x, max(vIndexPos.y, vIndexPos.z)))
		return true;

	return false;
}

_bool CVoxel_Convertor::Is_Out_Of_Range_Voxel(const _uint iIndex)
{
	if (iIndex > g_iVoxelIndexLimit)
		return true;

	return false;
}

_bool CVoxel_Convertor::Is_Out_Of_Range_Sector(const _uint3& vIndexPos)
{
	if ((*m_pNumSectorWorldDim) <= max(vIndexPos.x, max(vIndexPos.y, vIndexPos.z)))
		return true;

	return false;
}

_bool CVoxel_Convertor::Is_Out_Of_Range_Sector(const _uint iIndex)
{
	if ((*m_pNumSectorWorldDim) * (*m_pNumSectorWorldDim) * (*m_pNumSectorWorldDim) <= iIndex)
		return true;

	return false;
}

CVoxel_Convertor* CVoxel_Convertor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_Convertor*		pInstance = { new CVoxel_Convertor{pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxel_Convertor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Convertor::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pTransformCom);
}
