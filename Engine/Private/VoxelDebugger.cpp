#include "GameInstance.h"
#include "VoxelDebugger.h"

CVoxel_Debugger::CVoxel_Debugger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_Debugger::Initialize(void* pArg)
{
	return S_OK;
}

void CVoxel_Debugger::Tick()
{
	_uint		iCurPickedIndex = { m_pGameInstance->Get_CurPicked_VoxelIndex(VOXEL_LAYER::_STATIC) };
	_uint		iPrePickedIndex = { m_pGameInstance->Get_PrePicked_VoxelIndex() };

	if (0 != iCurPickedIndex &&
		iCurPickedIndex != iPrePickedIndex)
	{
		VOXEL_STATE			eLastState;
		if (!FAILED(m_pGameInstance->Get_VoxelState(iPrePickedIndex, eLastState, VOXEL_LAYER::_STATIC)) &&
			(VOXEL_STATE::_PICKED == eLastState ||
			VOXEL_STATE::_DEFAULT == eLastState))
		{

			if (FAILED(m_pGameInstance->Set_VoxelState(iPrePickedIndex, m_ePrePickedVoxelState, VOXEL_LAYER::_STATIC)))
				cout << "Failed Set VoxelID iPrePickedIndex" << endl;

			if (FAILED(m_pGameInstance->Get_VoxelState(iCurPickedIndex, m_ePrePickedVoxelState, VOXEL_LAYER::_STATIC)))
			{
				m_ePrePickedVoxelState = VOXEL_STATE::_DEFAULT;
				cout << "FAILD" << endl;
			}
			if (FAILED(m_pGameInstance->Set_VoxelState(iCurPickedIndex, VOXEL_STATE::_PICKED, VOXEL_LAYER::_STATIC)))
				cout << "Failed Set VoxelID iCurPickedIndex" << endl;


			//vector<_uint>			NeighborIndicesPre = { m_pGameInstance->Get_NeighborIndices_Voxel(iPrePickedIndex) };
			//for (auto iNeighborIndex : NeighborIndicesPre)
			//{
			//	if (FAILED(m_pGameInstance->Set_VoxelState(iNeighborIndex, VOXEL_STATE::_DEFAULT)))
			//		cout << "Failed Set VoxelID iPreNeighborIndex" << endl;
			//}
			//vector<_uint>			NeighborIndicesCur = { m_pGameInstance->Get_NeighborIndices_Voxel(iCurPickedIndex) };
			//for (auto iNeighborIndex : NeighborIndicesCur)
			//{
			//	if (FAILED(m_pGameInstance->Set_VoxelState(iNeighborIndex, VOXEL_STATE::_IS_NEIGHBOR)))
			//		cout << "Failed Set VoxelID iPreNeighborIndex" << endl;
			//}

		}
	}		

	Draw_Current_PickPos();
}

HRESULT CVoxel_Debugger::Render()
{
	if (FAILED(Draw_Current_PickPos()))
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_Debugger::Draw_Current_PickPos()
{
	_float3			vPosFloat3 = { m_pGameInstance->Get_CurPicked_VoxelWorldPos(VOXEL_LAYER::_STATIC) };
	_uint			iVoxelIndex;

	m_pGameInstance->Get_Index_Voxel(vPosFloat3, iVoxelIndex);
	//	_float			fDistanceFromGoal = { m_pGameInstance->Get_Distance_From_Goal_FlowField(iVoxelIndex) };

	_vector			vWorldPos = { XMVectorSetW(XMLoadFloat3(&vPosFloat3), 1.f) };

	_matrix			ViewMatrix = { m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) };
	_matrix			ProjMatrix = { m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ) };
	_matrix			VPMatrix = { ViewMatrix * ProjMatrix };

	_vector			vProjPos = { XMVector3TransformCoord(vWorldPos, VPMatrix) };

	_float			fWinSizeX = { static_cast<_float>(m_pGameInstance->Get_WinSizeX()) };
	_float			fWinSizeY = { static_cast<_float>(m_pGameInstance->Get_WinSizeY()) };

	_vector			vScreenSpacePosition = {
			XMVectorGetX(vProjPos) * fWinSizeX * 0.5f + fWinSizeX * 0.5f,
			(XMVectorGetY(vProjPos) * fWinSizeY * 0.5f - fWinSizeY * 0.5f) * -1.f,
			0.f, 0.f
	};

	_float2			vScreenSpacePositionFloat2 = {};
	XMStoreFloat2(&vScreenSpacePositionFloat2, vScreenSpacePosition);

	//	_float3			vFlowDir;
	//	m_pGameInstance->Get_Direction_FlowField(vPosFloat3, vFlowDir);
	
	wstring			strBuff;
	strBuff += TEXT("x : ") + to_wstring(XMVectorGetX(vWorldPos)) + TEXT("\n");
	strBuff += TEXT("y : ") + to_wstring(XMVectorGetY(vWorldPos)) + TEXT("\n");
	strBuff += TEXT("z : ") + to_wstring(XMVectorGetZ(vWorldPos)) + TEXT("\n");
	//	strBuff += TEXT("Distance : ") + to_wstring(fDistanceFromGoal) + TEXT("\n");
	/*strBuff += TEXT("x : ") + to_wstring(vFlowDir.x) + TEXT("\n");
	strBuff += TEXT("y : ") + to_wstring(vFlowDir.y) + TEXT("\n");
	strBuff += TEXT("z : ") + to_wstring(vFlowDir.z) + TEXT("\n");*/

	m_pGameInstance->Render_Font_Scaled (TEXT("Font_Default"), strBuff, vScreenSpacePositionFloat2, XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.5f);
	//	m_pGameInstance->Render_Font(TEXT("Font_Default"), strBuff.c_str(), _float2(0.f, 0.f), XMVectorSet(1.f, 0.f, 0.f, 1.f), 0.f);

	return S_OK;
}

CVoxel_Debugger* CVoxel_Debugger::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_Debugger* pInstance = { new CVoxel_Debugger{ pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxelDebugger"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Debugger::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
