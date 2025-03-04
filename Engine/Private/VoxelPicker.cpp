#include "GameInstance.h"
#include "VoxelPicker.h"

CVoxel_Picker::CVoxel_Picker(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_Picker::Initialize(void* pArg)
{
	return S_OK;
}

void CVoxel_Picker::Tick()
{
	_uint		iNewPickIndex{};
	if (!(FAILED(m_pGameInstance->Compute_Picking_Voxel(iNewPickIndex, VOXEL_LAYER::_STATIC))))
	{
		m_iPrePickIndex = m_iCurPickIndex;
		m_iCurPickIndex = iNewPickIndex;
	}
}

_float3 CVoxel_Picker::Get_CurPickWorldPos()
{
	_float3				vPos{};
	m_pGameInstance->Get_WorldPosition_Voxel(m_iCurPickIndex, vPos);

	return vPos;
}

_float3 CVoxel_Picker::Get_PrePickWorldPos()
{
	_float3				vPos{};
	m_pGameInstance->Get_WorldPosition_Voxel(m_iPrePickIndex, vPos);

	return vPos;
}

CVoxel_Picker* CVoxel_Picker::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_Picker*		pInstance = { new CVoxel_Picker{ pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxelPicker"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Picker::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
