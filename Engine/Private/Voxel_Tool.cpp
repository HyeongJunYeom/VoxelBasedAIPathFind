#include "Voxel_Tool.h"
#include "GameInstance.h"
#include "Voxel_Manager.h"

CVoxel_Tool::CVoxel_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGui_Object{ pDevice, pContext}
{
}

HRESULT CVoxel_Tool::Initialize(CVoxel_Manager* pVoxel_Manager, void* pArg)
{
	if (nullptr == pVoxel_Manager)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pVoxel_Manager = pVoxel_Manager;
	Safe_AddRef(m_pVoxel_Manager);

	return S_OK;
}

void CVoxel_Tool::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	ImGui::Begin("Voxel Tool");

	if (ImGui::IsWindowFocused())
		*m_pFocus = true;
	if (ImGui::IsWindowHovered())
		*m_pHovered = true;

	Update_VoxelData();

	Show_Current_PickVoxel_Info();
	//	Setup_CookingInfo();
	Show_Voxels_Info();
	Show_Sectors_Info();

	Choice_Mode();

	Save_Data();
	Load_Data();

	static		_float			fCoolDown = {};
	fCoolDown = fmaxf(0.f, fCoolDown - fTimeDelta);

	if (MODE::_SELECT == m_eMode)
	{
		Choice_VoxelState();
		if (false == *m_pHovered &&
			false == *m_pFocus &&
			DOWN == m_pGameInstance->Get_KeyState(VK_LBUTTON))
		{
		}
	}

	else if (MODE::_CREATE == m_eMode)
	{
		Choice_Dir();
		Choice_VoxelID();


		if (false == *m_pHovered &&
			false == *m_pFocus &&
			PRESSING == m_pGameInstance->Get_KeyState(VK_LBUTTON) &&
			fCoolDown <= 0.f)
		{
			fCoolDown = 0.25f;
			Create_Voxel();
		}
	}

	else if (MODE::_ERASE == m_eMode)
	{
		if (false == *m_pHovered &&
			false == *m_pFocus &&
			DOWN == m_pGameInstance->Get_KeyState(VK_LBUTTON))
		{
			Erase_Voxel();
		}
	}

	else if (MODE::_CHANGE == m_eMode)
	{
		if (false == *m_pHovered &&
			false == *m_pFocus &&
			DOWN == m_pGameInstance->Get_KeyState(VK_LBUTTON))
		{
			Choice_VoxelID();
		}
	}

	ImGui::End();
}

void CVoxel_Tool::Update_VoxelData()
{
	m_iCurPickedVoxelIndex = { m_pGameInstance->Get_CurPicked_VoxelIndex(VOXEL_LAYER::_STATIC) };
	m_iPrePickedVoxelIndex = { m_pGameInstance->Get_PrePicked_VoxelIndex() };

	VOXEL_STATE		eState;
	if(FAILED(m_pGameInstance->Get_VoxelState(m_iCurPickedVoxelIndex, eState, VOXEL_LAYER::_STATIC)))
		return;
}

void CVoxel_Tool::Choice_VoxelID()
{
	if (ImGui::CollapsingHeader("Voxel ID"))
	{
		if (ImGui::RadioButton("Floor", m_eSetupVoxelID == VOXEL_ID::_FLOOR))
			m_eSetupVoxelID = VOXEL_ID::_FLOOR;
		ImGui::SameLine();

		if (ImGui::RadioButton("Wall", m_eSetupVoxelID == VOXEL_ID::_WALL))
			m_eSetupVoxelID = VOXEL_ID::_WALL;
		ImGui::SameLine();
	}

	ImGui::NewLine();
}

void CVoxel_Tool::Choice_VoxelState()
{
	if (ImGui::CollapsingHeader("Voxel Type"))
	{
		//if (ImGui::RadioButton("Start", m_eSetupVoxelState == VOXEL_ID::_START))
		//	m_eSetupVoxelState = VOXEL_STATE::_START;
		//ImGui::SameLine();

		//if (ImGui::RadioButton("Goal", m_eSetupVoxelState == VOXEL_STATE::_GOAL))
		//	m_eSetupVoxelState = VOXEL_STATE::_GOAL;
		//ImGui::SameLine();

		//if (ImGui::RadioButton("Blocked", m_eSetupVoxelState == VOXEL_STATE::_BLOCKED))
		//	m_eSetupVoxelState = VOXEL_STATE::_BLOCKED;
	}

	ImGui::NewLine();
}

void CVoxel_Tool::Choice_Mode()
{
	if (ImGui::CollapsingHeader("Mode ##VoxelTool"))
	{
		if (ImGui::RadioButton("Select", m_eMode == MODE::_SELECT))
			m_eMode = MODE::_SELECT;
		ImGui::SameLine();

		if (ImGui::RadioButton("Create", m_eMode == MODE::_CREATE))
			m_eMode = MODE::_CREATE;
		ImGui::SameLine();

		if (ImGui::RadioButton("Erase ##02314", m_eMode == MODE::_ERASE))
			m_eMode = MODE::_ERASE;
		ImGui::SameLine();

		if (ImGui::RadioButton("Change ##012312904587", m_eMode == MODE::_CHANGE))
			m_eMode = MODE::_CHANGE;
	}

	ImGui::NewLine();
}

void CVoxel_Tool::Choice_Dir()
{
	if (ImGui::CollapsingHeader("Selector Direction"))
	{
		if (ImGui::RadioButton("X+", m_eDir == DIR::_X_UP))
			m_eDir = DIR::_X_UP;
		ImGui::SameLine();

		if (ImGui::RadioButton("X-", m_eDir == DIR::_X_DOWN))
			m_eDir = DIR::_X_DOWN;
		ImGui::SameLine();

		if (ImGui::RadioButton("Y+", m_eDir == DIR::_Y_UP))
			m_eDir = DIR::_Y_UP;
		ImGui::SameLine();

		if (ImGui::RadioButton("Y-", m_eDir == DIR::_Y_DOWN))
			m_eDir = DIR::_Y_DOWN;
		ImGui::SameLine();

		if (ImGui::RadioButton("Z+", m_eDir == DIR::_Z_UP))
			m_eDir = DIR::_Z_UP;
		ImGui::SameLine();

		if (ImGui::RadioButton("Z-", m_eDir == DIR::_Z_DOWN))
			m_eDir = DIR::_Z_DOWN;
	}

	ImGui::NewLine();
}

void CVoxel_Tool::Save_Data()
{
	if (ImGui::CollapsingHeader("Save Data ##CVoxel_Tool::Save_Data()"))
	{
		_int			iTempSlotNum = { static_cast<_int>(m_iSaveSlot_Save) };
		if (ImGui::InputInt("Slot Num ##void CVoxel_Tool::Save_Data()", &iTempSlotNum))
		{
			if(-1 < iTempSlotNum &&
				INT_MAX >= iTempSlotNum)
				m_iSaveSlot_Save = iTempSlotNum;
		}

		if(ImGui::Button("Save ##CVoxel_Tool::Save_Data()"))
		{
			m_pVoxel_Manager->Save_Data(m_iSaveSlot_Save);
		}
	}
}

void CVoxel_Tool::Load_Data()
{
	if (ImGui::CollapsingHeader("Load Data ##CVoxel_Tool::Load_Data()"))
	{
		_int			iTempSlotNum = { static_cast<_int>(m_iSaveSlot_Load) };
		if (ImGui::InputInt("Slot Num ##void CVoxel_Tool::Load_Data()", &iTempSlotNum))
		{
			if (-1 < iTempSlotNum &&
				INT_MAX >= iTempSlotNum)
				m_iSaveSlot_Load = iTempSlotNum;
		}

		if (ImGui::Button("Load ##CVoxel_Tool::Load_Data()"))
		{
			m_pVoxel_Manager->Load_Data(m_iSaveSlot_Load);
		}
	}
}

void CVoxel_Tool::Setup_CookingInfo()
{
	if (nullptr == m_pVoxel_Manager)
		return;

	_float		fCookingMAxHeight, fTempCookingMaxHeight, fVoxelSize, fTempVoxelSize;
	
	if (FAILED(m_pVoxel_Manager->Get_CookingMaxHeight(fCookingMAxHeight)) ||
		FAILED(m_pVoxel_Manager->Get_TempCookingMaxHeight(fTempCookingMaxHeight)) ||
		FAILED(m_pVoxel_Manager->Get_VoxelSize(fVoxelSize)) ||
		FAILED(m_pVoxel_Manager->Get_TempVoxelSize(fTempVoxelSize)))
		return;

	ImGui::Text("================ Cooking Info ==============");

	ImGui::Text("Current Max Height : %f ##CVoxel_Tool:Setup_CookingInfo", fCookingMAxHeight);
	ImGui::Text("Current Voxel Size : %f ##CVoxel_Tool:Setup_CookingInfo", fVoxelSize);

	if (ImGui::SliderFloat("Max Height", &fTempCookingMaxHeight, 1.f, g_iTerrainMaxHeight))
	{		

	}
	if (ImGui::SliderFloat("Voxel Size", &fTempVoxelSize, g_fVoxelSizeMinLimit, static_cast<_float>(g_iSectorLength) * 0.5f))
	{
		fTempVoxelSize = floor(fTempVoxelSize);

		if (fTempVoxelSize < 1.f)
		{
			fTempVoxelSize = 0.5f;
		}
		else if (fTempVoxelSize < 2.f)
		{
			fTempVoxelSize = 1.f;
		}
		else
		{
			while (static_cast<_uint>(fTempVoxelSize) % 2 != 0)
			{
				fTempVoxelSize = fTempVoxelSize - 1;
				fTempVoxelSize = floor(fTempVoxelSize);
			}
		}

	}
	m_pVoxel_Manager->Set_TempCookingMaxHeight(fTempCookingMaxHeight);
	m_pVoxel_Manager->Set_TempVoxelSize(fTempVoxelSize);

	if (ImGui::Button("Cooking ##CVoxel_Tool::Setup_CookingInfo()"))
	{
		//	m_pVoxel_Manager->Cooking();
	}

	ImGui::NewLine();
}

void CVoxel_Tool::Show_Current_PickVoxel_Info()
{

	_float3			vCurPickedVoxePositionFromIndexFloat3;
	m_pGameInstance->Get_WorldPosition_Voxel(m_iCurPickedVoxelIndex, vCurPickedVoxePositionFromIndexFloat3);
	_vector			vCurPickedVoxePositionFromIndex = { XMLoadFloat3(&vCurPickedVoxePositionFromIndexFloat3) };

	_uint3			vIndexPosFromIndexUInt3;
	m_pGameInstance->Get_IndexPosition_Voxel(m_iCurPickedVoxelIndex, vIndexPosFromIndexUInt3);

	VOXEL_ID		eVoxelID;
	m_pGameInstance->Get_VoxelID(m_iCurPickedVoxelIndex, eVoxelID, VOXEL_LAYER::_STATIC);

	_byte			bVoxelOpenessScore;
	m_pVoxel_Manager->Get_VoxelOpenessScore(m_iCurPickedVoxelIndex, bVoxelOpenessScore, VOXEL_LAYER::_STATIC);

	_uint			iNeighborFlag;
	m_pVoxel_Manager->Get_NeighborFlag(m_iCurPickedVoxelIndex, iNeighborFlag, VOXEL_LAYER::_STATIC);

	ImGui::Text("================ Cur Picked Voxel Info By Index ==============");

	ImGui::Text("Index : %d", m_iCurPickedVoxelIndex);

	ImGui::Text("World X : %f", XMVectorGetX(vCurPickedVoxePositionFromIndex) );
	ImGui::Text("World Y : %f", XMVectorGetY(vCurPickedVoxePositionFromIndex) );
	ImGui::Text("World Z : %f", XMVectorGetZ(vCurPickedVoxePositionFromIndex) );

	ImGui::Text("Index X : %d", vIndexPosFromIndexUInt3.x);
	ImGui::Text("Index Y : %d", vIndexPosFromIndexUInt3.y);
	ImGui::Text("Index Z : %d", vIndexPosFromIndexUInt3.z);

	ImGui::Text("Openess Score : %d", bVoxelOpenessScore);

	string		strFlag = {};
	for (_uint i = 0; i < 32; ++i)
	{
		if (0 != (iNeighborFlag & 1))
			strFlag += '1';
		else
			strFlag += '0';

		iNeighborFlag = iNeighborFlag >> 1;
	}
	
	reverse(strFlag.begin(), strFlag.end());
	string		strResult = { "Neighbor Flag : " + strFlag };

	ImGui::Text(strResult.c_str(), iNeighborFlag);


	switch (eVoxelID)
	{
	case Engine::VOXEL_ID::_FLOOR:
		ImGui::Text("ID FLOOR");
		break;
	case Engine::VOXEL_ID::_WALL:
		ImGui::Text("ID WALL");
		break;
	case Engine::VOXEL_ID::_END:
		ImGui::Text("ID END");
		break;
	default:
		ImGui::Text("ID Error");
		break;
	}
}

void CVoxel_Tool::Show_Sectors_Info()
{
	_uint			iNumActiveSector = { m_pGameInstance->Get_NumSector_Active() };
	_uint			iNumTotalSector = { m_pGameInstance->Get_NumSector(VOXEL_LAYER::_STATIC) };

	ImGui::Text("================ Cur Sector Info ==============");

	ImGui::Text("Active Sector : %d Ea", iNumActiveSector);
	ImGui::Text("Static Total Sector : %d Ea", iNumTotalSector);
}

void CVoxel_Tool::Show_Voxels_Info()
{
	_uint			iNumActiveVoxel = { m_pGameInstance->Get_NumVoxel_Active_All_Layer() };
	_uint			iNumTotalVoxel = { m_pGameInstance->Get_NumVoxel_All_Layer() };

	ImGui::Text("================ Cur Sector Info ==============");

	ImGui::Text("Active Voxel : %d Ea", iNumActiveVoxel);
	ImGui::Text("Total Voxel : %d Ea", iNumTotalVoxel);
}

void CVoxel_Tool::Create_Voxel()
{
	if (DIR::_END == m_eDir)
		return;

	_uint3			vIndexPosUInt3;

	if (!(FAILED(m_pGameInstance->Get_IndexPosition_Voxel(m_iCurPickedVoxelIndex, vIndexPosUInt3))))
	{
		_vector			vIndexPos = { XMLoadUInt3(&vIndexPosUInt3) };
		_vector			vNextPos = { vIndexPos };
		_vector			vMoveDir = {
		XMVectorSet(
			static_cast<_float>(iMoveX[static_cast<_uint>(m_eDir)]),
			static_cast<_float>(iMoveY[static_cast<_uint>(m_eDir)]),
			static_cast<_float>(iMoveZ[static_cast<_uint>(m_eDir)]),
			0.f) };

		while (true)
		{
			vNextPos += vMoveDir;

			if (0.f > min(XMVectorGetX(vNextPos), min(XMVectorGetY(vNextPos), XMVectorGetZ(vNextPos))))
				break;

			if (m_pVoxel_Manager->Get_NumVoxelWorldDim() <= max(XMVectorGetX(vNextPos), max(XMVectorGetY(vNextPos), XMVectorGetZ(vNextPos))))
				break;

			_uint3			vNextIndexPosUInt3;
			XMStoreUInt3(&vNextIndexPosUInt3, vNextPos);

			if (!FAILED(m_pGameInstance->Add_Voxel(vNextIndexPosUInt3, VOXEL_LAYER::_STATIC, m_eSetupVoxelID)))
				break;
		}
	}
}

void CVoxel_Tool::Erase_Voxel()
{
	_uint3			vIndexPosUInt3;

	if (!(FAILED(m_pGameInstance->Get_IndexPosition_Voxel(m_iCurPickedVoxelIndex, vIndexPosUInt3))))
	{
		m_pVoxel_Manager->Erase_Voxel(vIndexPosUInt3, VOXEL_LAYER::_STATIC);
	}
}

void CVoxel_Tool::Change_Voxel()
{
	_uint3			vIndexPosUInt3;

	if (!(FAILED(m_pGameInstance->Get_IndexPosition_Voxel(m_iCurPickedVoxelIndex, vIndexPosUInt3))))
	{
		m_pVoxel_Manager->Set_VoxelID(vIndexPosUInt3, m_eSetupVoxelID, VOXEL_LAYER::_STATIC);
	}
}

CVoxel_Tool* CVoxel_Tool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CVoxel_Manager* pVoxel_Manager, void* pArg)
{
	CVoxel_Tool* pInstance = { new CVoxel_Tool(pDevice, pContext) };
	if (FAILED(pInstance->Initialize(pVoxel_Manager, pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CVoxel_Tool"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Tool::Free()
{
	__super::Free();

	Safe_Release(m_pVoxel_Manager);
}
