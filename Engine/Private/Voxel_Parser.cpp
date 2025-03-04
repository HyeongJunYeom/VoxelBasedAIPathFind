#include "..\Public\Voxel_Parser.h"
#include "GameInstance.h"
#include "VoxelSector.h"
#include "Voxel_SectorLayer.h"

ofstream					CVoxel_Parser::ms_ofs;
ifstream					CVoxel_Parser::ms_ifs;


CVoxel_Parser::CVoxel_Parser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_Parser::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CVoxel_Parser::Load_Data(const string& strFilePath, vector<CVoxel_SectorLayer*>& VoxelLayers, VOXEL_SAVE_DESC& Desc)
{
	filesystem::path		FullPath(strFilePath);

	if (FAILED(Open_File(strFilePath.c_str(), false)))
		return E_FAIL;

	vector< CVoxel_SectorLayer*>		TempVoxelLayers;
	CVoxel_SectorLayer::VOXEL_SECTOR_LAYER_DESC* pVoxelLayerDesc = { static_cast<CVoxel_SectorLayer::VOXEL_SECTOR_LAYER_DESC*>(Desc.pVoxelSectorLayerDesc) };
	if (nullptr == pVoxelLayerDesc)
		return E_FAIL;

	_uint iNumLayer = {};
	Read_File(&iNumLayer, sizeof(iNumLayer));
	Read_File((pVoxelLayerDesc->pVoxelSize), sizeof(*(pVoxelLayerDesc->pVoxelSize)));

	CVoxel_Sector::VOXEL_SECTOR_DESC		SectorDesc;
	SectorDesc.pVoxelSize = pVoxelLayerDesc->pVoxelSize;


	for (_uint i = 0; i < iNumLayer; ++i)
	{
		CVoxel_SectorLayer*			pVoxelSectorLayer = { CVoxel_SectorLayer::Create(m_pDevice, m_pContext, pVoxelLayerDesc) };
		auto& Sectors = pVoxelSectorLayer->Get_Sectors();

		if (FAILED(Load_Data_Sectors(Sectors, &SectorDesc)))
			return E_FAIL;

		TempVoxelLayers.push_back(pVoxelSectorLayer);
	}

	for (auto& pVoxelLayer : VoxelLayers)
	{
		Safe_Release(pVoxelLayer);
	}
	VoxelLayers.clear();
	VoxelLayers = move(TempVoxelLayers);

	Close_File();
	return S_OK;
}

HRESULT CVoxel_Parser::Save_Data(const string& strFilePath, const vector<CVoxel_SectorLayer*>& VoxelLayers, VOXEL_SAVE_DESC& Desc)
{
	filesystem::path		FullPath(strFilePath);
	if (FAILED(Open_File(strFilePath.c_str(),true)))
		return E_FAIL;

	_uint		iNumLayer = { static_cast<_uint>(VoxelLayers.size()) };
	_float		fVoxelSize = { *(static_cast<CVoxel_SectorLayer::VOXEL_SECTOR_LAYER_DESC*>(Desc.pVoxelSectorLayerDesc)->pVoxelSize) };
	
	Write_File(&iNumLayer, sizeof(iNumLayer));
	Write_File(&fVoxelSize, sizeof(fVoxelSize));

	for (_uint i = 0; i < iNumLayer; ++i)
	{
		auto& Sectors = VoxelLayers[i]->Get_Sectors();

		if (FAILED(Save_Data_Sectors(Sectors)))
			return E_FAIL;
	}

	Close_File();
	return S_OK;
}

HRESULT CVoxel_Parser::Load_Data_Sectors(unordered_map<_uint, class CVoxel_Sector*>& Voxel_Sectors, void* pSectorDesc)
{
	unordered_map<_uint, CVoxel_Sector*>			Temp_Voxel_Sectors;

	VOXEL_ID			eVoxel_ID = { VOXEL_ID::_END };
	VOXEL_STATE			eVoxel_State = { VOXEL_STATE::_END };
	_uint				iNeighbor_Flag = {};
	_byte				bOpennessScore = {};

	_uint				iNumSector = {};
	_uint				iNumVoxelInSector = {};
	_uint				iVoxelIndex = {};
	_uint				iSectorIndex = {};

	Read_File(&iNumSector, sizeof(_uint));
	for (_uint i = 0; i < iNumSector; ++i)
	{
		Read_File(&iSectorIndex, sizeof(_uint));
		Read_File(&iNumVoxelInSector, sizeof(_uint));

		Temp_Voxel_Sectors.emplace(iSectorIndex, CVoxel_Sector::Create(m_pDevice, m_pContext, pSectorDesc));

		for (_uint j = 0; j < iNumVoxelInSector; ++j)
		{
			Read_File(&iVoxelIndex, sizeof(_uint));
			Read_File(&eVoxel_ID, sizeof(VOXEL_ID));
			Read_File(&eVoxel_State, sizeof(VOXEL_STATE));
			Read_File(&iNeighbor_Flag, sizeof(_uint));
			Read_File(&bOpennessScore, sizeof(_byte));

			VOXEL			Voxel;
			Voxel.bID = static_cast<_byte>(eVoxel_ID);
			Voxel.bState = static_cast<_byte>(eVoxel_State);
			Voxel.iNeighborFlag = iNeighbor_Flag;
			Voxel.bOpenessScore = bOpennessScore;

			if (FAILED(Temp_Voxel_Sectors[iSectorIndex]->Insert_Voxel(iVoxelIndex, Voxel)))
			{
				Close_File();
				return E_FAIL;
			}
		}
	}

	for (auto& Pair : Voxel_Sectors)
	{
		Safe_Release(Pair.second);
	}
	Voxel_Sectors.clear();
	Voxel_Sectors = move(Temp_Voxel_Sectors);

	return S_OK;
}

HRESULT CVoxel_Parser::Save_Data_Sectors(const unordered_map<_uint, class CVoxel_Sector*>& Voxel_Sectors)
{
	VOXEL_ID			eVoxel_ID = { VOXEL_ID::_END };
	VOXEL_STATE			eVoxel_State = { VOXEL_STATE::_END };
	_uint				iNeighbor_Flag = {};
	_byte				bOpennessScore = {};

	_uint				iNumSector = { static_cast<_uint>(Voxel_Sectors.size()) };
	Write_File(&iNumSector, sizeof(_uint));

	for (auto& Pair : Voxel_Sectors)
	{
		_uint				iSectorIndex = { Pair.first };
		CVoxel_Sector* pVoxel_Sector = { Pair.second };

		vector<_uint>		VoxelIndices = { pVoxel_Sector->Get_Indices() };

		_uint				iNumVoxelInSector = { static_cast<_uint>(VoxelIndices.size()) };
		Write_File(&iSectorIndex, sizeof(_uint));
		Write_File(&iNumVoxelInSector, sizeof(_uint));

		for (auto& iVoxelIndex : VoxelIndices)
		{
			if (FAILED(pVoxel_Sector->Get_ID(iVoxelIndex, eVoxel_ID)) ||
				FAILED(pVoxel_Sector->Get_State(iVoxelIndex, eVoxel_State)) ||
				FAILED(pVoxel_Sector->Get_NeighborFlag(iVoxelIndex, iNeighbor_Flag)) ||
				FAILED(pVoxel_Sector->Get_OpenessScore(iVoxelIndex, bOpennessScore)))
			{
				Close_File();
				return E_FAIL;
			}

			Write_File(&iVoxelIndex, sizeof(_uint));
			Write_File(&eVoxel_ID, sizeof(VOXEL_ID));
			Write_File(&eVoxel_State, sizeof(VOXEL_STATE));
			Write_File(&iNeighbor_Flag, sizeof(_uint));
			Write_File(&bOpennessScore, sizeof(_byte));
		}
	}

	return S_OK;
}

void CVoxel_Parser::Write_File(void* pValue, _uint iSize)
{
	ms_ofs.write(reinterpret_cast<_char*>(pValue), iSize);
}

void CVoxel_Parser::Read_File(void* pValue, _uint iSize)
{
	ms_ifs.read(reinterpret_cast<_char*>(pValue), iSize);
}

HRESULT CVoxel_Parser::Open_File(const _char* pFIlePath, _bool isWrite)
{
	std::filesystem::path	FullPath(pFIlePath);

	//	상대경로에서 경로와 파일이름만 분리
	string				strParentsPath = { FullPath.parent_path().string() };
	string				strFileName = { FullPath.stem().string() };

	//	 동일경로에 동일 파일이름에 확장자만 다르게 새로운 경로 생성
	string				strNewPath = { strParentsPath + "/" + strFileName + ".bin" };

	//	바이너리로 데이터를 작성하기위해서 바이너리 플래그를 포함하였다.
	if (false == isWrite)
	{
		ms_ifs = ifstream(strNewPath.c_str(), ios::binary | ios::out);
		if (true == ms_ifs.fail())
		{
			MSG_BOX(TEXT("Failed To OpenFile"));
			return E_FAIL;
		}

	}
	else
	{
		ms_ofs = ofstream(strNewPath.c_str(), ios::binary | ios::out);
		if (true == ms_ofs.fail())
		{
			MSG_BOX(TEXT("Failed To OpenFile"));
			return E_FAIL;
		}
	}


	return S_OK;
}

void CVoxel_Parser::Close_File()
{
	CVoxel_Parser::ms_ofs.close();
	CVoxel_Parser::ms_ifs.close();
}

CVoxel_Parser* CVoxel_Parser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_Parser*		pInstance = { new CVoxel_Parser(pDevice, pContext) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxel_Parser"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Parser::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
