#pragma once

#include "Engine_Defines.h"
#include "Gui_Object.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Tool final : public CGui_Object
{
public:
	enum class MODE { _CREATE, _SELECT, _ERASE, _CHANGE, _END };
	enum class DIR { _X_UP, _X_DOWN, _Y_UP, _Y_DOWN, _Z_UP, _Z_DOWN, _END };

	const _int iMoveX[6] = { 1, -1, 0, 0, 0, 0 };
	const _int iMoveY[6] = { 0, 0, 1, -1, 0, 0 };
	const _int iMoveZ[6] = { 0, 0, 0, 0, 1, -1 };

private:
	CVoxel_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_Tool() = default;

public:
	virtual HRESULT Initialize(class CVoxel_Manager* pVoxel_Manager, void* pArg);
	virtual void Tick(_float fTimeDelta);

private:
	void Update_VoxelData();
	void Choice_VoxelID();
	void Choice_VoxelState();
	void Choice_Mode();
	void Choice_Dir();

private:
	void Save_Data();
	void Load_Data();

private:
	void Setup_CookingInfo();

private:
	void Show_Current_PickVoxel_Info();
	void Show_Sectors_Info();
	void Show_Voxels_Info();

private:
	void Create_Voxel();
	void Erase_Voxel();
	void Change_Voxel();

private:
	MODE			m_eMode = { MODE::_END };

	_uint			m_iCurPickedVoxelIndex = {};
	_uint			m_iPrePickedVoxelIndex = {};

	//	For.Select_Mode
	_uint			m_iLastChooseVoxelIndex = {};
	VOXEL_ID		m_eLastVoxelID = {};
	VOXEL_ID		m_ePickedVoxelID = {};
	VOXEL_ID		m_eSetupVoxelID = {};

	//	For.Create_Mode
	DIR				m_eDir = { DIR::_END };

	//	For.SaveSlot
	_uint			m_iSaveSlot_Save = { 0 };
	_uint			m_iSaveSlot_Load = { 0 };

	class CVoxel_Manager*		m_pVoxel_Manager = { nullptr };


public:
	static CVoxel_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, class CVoxel_Manager* pVoxel_Manager, void* pArg);
	virtual void Free() override;
};

END