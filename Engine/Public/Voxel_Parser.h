#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Parser final :	public CBase
{
public:
	typedef struct tagVoxelSaveDesc
	{
		void*		pVoxelSectorLayerDesc = { nullptr };
	}VOXEL_SAVE_DESC;

private:
	CVoxel_Parser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_Parser() = default;

public:
	HRESULT Initialize(void* pArg);

public:
	HRESULT Load_Data(const string& strFilePath, vector<class CVoxel_SectorLayer*>& VoxelLayers,VOXEL_SAVE_DESC& Desc);
	HRESULT Save_Data(const string& strFilePath, const vector<class CVoxel_SectorLayer*>& VoxelLayers, VOXEL_SAVE_DESC& Desc);

private:
	HRESULT Load_Data_Sectors(unordered_map<_uint, class CVoxel_Sector*>& Voxel_Sectors, void* pSectorDesc);
	HRESULT Save_Data_Sectors(const unordered_map<_uint, class CVoxel_Sector*>& Voxel_Sectors);

private:
	static void			Write_File(void* pValue, _uint iSize);
	static void			Read_File(void* pValue, _uint iSize);
	static HRESULT 		Open_File(const _char* pFIlePath, _bool isWrite);
	static void			Close_File();


private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	class CGameInstance*	m_pGameInstance = { nullptr };

private:
	static ofstream			ms_ofs;
	static ifstream			ms_ifs;


public:
	static CVoxel_Parser* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual void Free() override;

};

END