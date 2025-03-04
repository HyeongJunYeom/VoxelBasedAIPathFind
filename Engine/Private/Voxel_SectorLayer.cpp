#include "GameInstance.h"
#include "Voxel_SectorLayer.h"
#include "Shader.h"
#include "VIBuffer_Cube.h"
#include "VIBuffer_Instance_Point.h"
#include "Transform.h"
#include "Model.h"
#include "VoxelSector.h"
#include "VoxelRenderCube.h"
#include "VoxelDebugger.h"
#include "VoxelPicker.h"
#include "Voxel_Scorer.h"
#include "Voxel_Parser.h"
#include "Voxel_Convertor.h"

CVoxel_SectorLayer::CVoxel_SectorLayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_SectorLayer::Initialize(void *pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	VOXEL_SECTOR_LAYER_DESC*		pSectorLayerDesc = { static_cast<VOXEL_SECTOR_LAYER_DESC*>(pArg) };
	m_pNumVoxelWorldDim = pSectorLayerDesc->pNumVoxelWorldDim;
	m_pNumVoxelSectorDim = pSectorLayerDesc->pNumVoxelSectorDim;
	m_pNumSectorWorldDim = pSectorLayerDesc->pNumSectorWorldDim;
	m_pTempVoxelSize = pSectorLayerDesc->pTempVoxelSize;
	m_pVoxelSize = pSectorLayerDesc->pVoxelSize;

	m_pTransform = pSectorLayerDesc->pTransform;
	m_pVoxelConvertor = pSectorLayerDesc->pVoxelConvertor;
	m_pVoxelScorer = pSectorLayerDesc->pVoxelScorer;
	m_pVoxelPicker= pSectorLayerDesc->pVoxelPicker;

	Safe_AddRef(m_pTransform);
	Safe_AddRef(m_pVoxelConvertor);
	Safe_AddRef(m_pVoxelScorer);
	Safe_AddRef(m_pVoxelPicker);

	if (nullptr == m_pNumVoxelWorldDim ||
		nullptr == m_pNumVoxelSectorDim ||
		nullptr == m_pNumSectorWorldDim ||
		nullptr == m_pTempVoxelSize ||
		nullptr == m_pVoxelSize ||
		nullptr == m_pTempVoxelSize ||
		nullptr == m_pVoxelConvertor ||
		nullptr == m_pVoxelScorer ||
		nullptr == m_pVoxelPicker)
		return E_FAIL;

	CVoxel_Sector::VOXEL_SECTOR_DESC			SectorDesc{};
	SectorDesc.pVoxelSize = m_pVoxelSize;

	if (FAILED(Add_Components()))
		return E_FAIL;

	if (FAILED(Add_Supports()))
		return E_FAIL;

	return S_OK;
}

void CVoxel_SectorLayer::Priority_Tick(_float fTimeDelta)
{
}

void CVoxel_SectorLayer::Tick(_float fTimeDelta)
{
}

HRESULT CVoxel_SectorLayer::Render(const unordered_set<_uint>& ActiveSectorIndices)
{
	_uint				iNumMaxInstance = { m_VIBufferInstanceCubes.front()->Get_MaxInstance() };

	_uint				iVIBufferIndex{};
	 
	Clear_Instance_Buffers();

	//	Instance
	for (auto& iIndex : ActiveSectorIndices)
	{
		if (false == Is_Exist_Sector(iIndex))
			continue;

		if (true == Is_Culled_Sector(iIndex))
			continue;

		vector<_uint>			Indices;
		_uint					iNumVoxel = { m_Sectors[iIndex]->Get_NumVoxel() };
		Indices.reserve(iNumVoxel);
		m_Sectors[iIndex]->Bind_InstanceBuffers(Indices, m_Instance_State_Buffer, m_Instance_ID_Buffer, m_Instance_Neighbor_Buffer);

		for (auto& iVoxelIndex : Indices)
		{
			_float3			vWorldPos;
			if (FAILED(m_pVoxelConvertor->Convert_WorldPos_From_Index(iVoxelIndex, vWorldPos)))
			{
				cout << "Failed To Convert WorldPos From Index : Render VoxelWorld " << endl;
				return E_FAIL;
			}

			if (m_Instance_Position_Buffer.size() >= iNumMaxInstance)
			{
				Bind_Instance_Buffers(iVIBufferIndex);
				Render_Current_Instance_Buffers(iVIBufferIndex);
				Clear_Instance_Buffers();

				++iVIBufferIndex;
			}

			m_Instance_Position_Buffer.push_back(vWorldPos);
		}

	}

	Bind_Instance_Buffers(iVIBufferIndex);
	Render_Current_Instance_Buffers(iVIBufferIndex);
	Clear_Instance_Buffers();

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Add_Voxel(const _uint3& vIndexPos, VOXEL_ID eID, VOXEL_STATE eState)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	if (false == Is_Exist_Sector(iSectorIndex))
	{
		if (FAILED(Add_Sector(iSectorIndex)))
			return E_FAIL;
	}

	CVoxel_Sector*		pVoxelSector = { m_Sectors[iSectorIndex] };
	if (FAILED(pVoxelSector->Add_Voxel(vIndexPos, eID, eState)))
		return E_FAIL;

	m_Sectors[iSectorIndex] = pVoxelSector;


	if (FAILED(SetUp_Neighbor(vIndexPos)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Erase_Voxel(const _uint3& vIndexPos)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;
	
	if (false == Is_Exist_Sector(iSectorIndex))
		return E_FAIL;

	CVoxel_Sector* pVoxelSector = { m_Sectors[iSectorIndex] };

	if (FAILED(Erase_Neighbors(vIndexPos)))
		return E_FAIL;

	if (FAILED(pVoxelSector->Erase_Voxel(vIndexPos)))
		return E_FAIL;

	if (true == pVoxelSector->Is_Empty())
	{
		pVoxelSector->Clear();

		if (FAILED(Erase_Sector(iSectorIndex)))
			return E_FAIL;
	}

	return S_OK;
}

_bool CVoxel_SectorLayer::Is_Exist_Voxel(const _uint3& vIndexPos)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return false;

	if (false == Is_Exist_Sector(iSectorIndex))
		return false;

	return m_Sectors[iSectorIndex]->Is_Exist(vIndexPos);
}

CVoxel_Sector* CVoxel_SectorLayer::Get_Sector(const _uint iSectorIndex)
{
	auto iter{ m_Sectors.find(iSectorIndex) };
	if (m_Sectors.end() == iter)
		return nullptr;

	return iter->second;
}

HRESULT CVoxel_SectorLayer::Add_Sector(const _uint iSectorIndex)
{
	if (m_Sectors.end() != m_Sectors.find(iSectorIndex))
		return E_FAIL;

	CVoxel_Sector::VOXEL_SECTOR_DESC			SectorDesc;
	SectorDesc.pVoxelSize = m_pVoxelSize;

	CVoxel_Sector*		pVoxelSector = { CVoxel_Sector::Create(m_pDevice, m_pContext, &SectorDesc) };
	if (nullptr == pVoxelSector)
		return E_FAIL;

	m_Sectors[iSectorIndex] = pVoxelSector;

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Add_Sector(const _uint3& vSectorIndexPos)
{
	_uint			iNumSectorWorldDim = { *m_pNumSectorWorldDim };
	_uint			iSectorIndex = { 
		vSectorIndexPos.x 
		+ (vSectorIndexPos.y * iNumSectorWorldDim) 
		+ (vSectorIndexPos.z * iNumSectorWorldDim * iNumSectorWorldDim) };
	return Add_Sector(iSectorIndex);
}

HRESULT CVoxel_SectorLayer::Erase_Sector(const _uint iSectorIndex)
{
	auto iter{ m_Sectors.find(iSectorIndex) };
	if (m_Sectors.end() == iter)
		return E_FAIL;

	Safe_Release(iter->second);
	m_Sectors.erase(iter);

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Erase_Sector(const _uint3& vSectorIndexPos)
{
	_uint			iNumSectorWorldDim = { *m_pNumSectorWorldDim };
	_uint			iSectorIndex = { 
		vSectorIndexPos.x 
		+ (vSectorIndexPos.y * iNumSectorWorldDim) 
		+ (vSectorIndexPos.z * iNumSectorWorldDim * iNumSectorWorldDim) };

	return Erase_Sector(iSectorIndex);
}

_bool CVoxel_SectorLayer::Is_Exist_Sector(const _uint iSectorIndex)
{
	auto iter{ m_Sectors.find(iSectorIndex) };
	return iter != m_Sectors.end();
}

_bool CVoxel_SectorLayer::Is_Exist_Sector(const _uint3& vSectorIndexPos)
{
	_uint			iNumSectorWorldDim = { *m_pNumSectorWorldDim };
	_uint			iSectorIndex = { 
		vSectorIndexPos.x 
		+ (vSectorIndexPos.y * iNumSectorWorldDim) 
		+ (vSectorIndexPos.z * iNumSectorWorldDim * iNumSectorWorldDim) };

	return Is_Exist_Sector(iSectorIndex);
}

HRESULT CVoxel_SectorLayer::SetUp_Neighbor(const _uint3& vIndexPos)
{
	if (false == Is_Exist_Voxel(vIndexPos))
		return E_FAIL;

	_int Offsets[] = { -1, 0, 1 };
	for (auto iOffsetZ : Offsets)
	{
		for (auto iOffsetY : Offsets)
		{
			for (auto iOffsetX : Offsets)
			{
				_int		iNewX = { static_cast<_int>(vIndexPos.x) + iOffsetX };
				_int		iNewY = { static_cast<_int>(vIndexPos.y) + iOffsetY };
				_int		iNewZ = { static_cast<_int>(vIndexPos.z) + iOffsetZ };

				if (0 > static_cast<_int>(min(iNewX, min(iNewY, iNewZ))))
					continue;

				if (static_cast<_int>(*m_pNumVoxelWorldDim) < max(iNewX, max(iNewY, iNewZ)))
					continue;

				_uint3		vDstIndexPos = {
					static_cast<_uint>(iNewX),
					static_cast<_uint>(iNewY),
					static_cast<_uint>(iNewZ)
				};

				if (false == Is_Exist_Voxel(vDstIndexPos))
					continue;

				_uint		iSrcNeighborFlag = {
					static_cast<_uint>(powf(2.f, (iOffsetX + 1) + (iOffsetY + 1) * 3.f + (iOffsetZ + 1) * 9.f) + EPSILON)
				};

				_int		iDstDirX = { iOffsetX * -1 };
				_int		iDstDirY = { iOffsetY * -1 };
				_int		iDstDirZ = { iOffsetZ * -1 };

				_uint		iDstNeighborFlag = {
					static_cast<_uint>(powf(2.f, (iDstDirX + 1) + (iDstDirY + 1) * 3.f + (iDstDirZ + 1) * 9.f) + EPSILON)
				};

				if (FAILED(Add_Neighbor(vIndexPos, iSrcNeighborFlag)))
					return E_FAIL;
				if (FAILED(Add_Neighbor(vDstIndexPos, iDstNeighborFlag)))
					return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Cooking_Terrain_Static_Voxels(CVIBuffer_Terrain* pTerrainBuffer, CTransform* pTerrainTransform)
{

	Clear_Sectors();

	{
		auto start = std::chrono::high_resolution_clock::now();

		if (FAILED(Cooking_TerrainBuffer(pTerrainBuffer, pTerrainTransform)))
			return E_FAIL;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Cooking_TerrainBuffer() Duration : " << duration.count() << endl;;
	}


	{
		auto start = std::chrono::high_resolution_clock::now();

		if (FAILED(Update_OpenessScores_All()))
			return E_FAIL;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Update_OpenessScores_All() Duration : " << duration.count() << endl;;
	}

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Cooking_Model_Static_Voxels(CModel* pModel, CTransform* pModelTransform)
{
	Clear_Sectors();

	{
		auto start = std::chrono::high_resolution_clock::now();

		if (FAILED(pModel->Cooking_StaticVoxel(pModelTransform)))
			return E_FAIL;

		if (FAILED(Convert_Wall_BottomNeighbors()))
			return E_FAIL;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Cooking_StaticVoxel() Duration : " << duration.count() << endl;;
	}

	{
		auto start = std::chrono::high_resolution_clock::now();

		if (FAILED(Update_OpenessScores_All()))
			return E_FAIL;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		cout << "Update_OpenessScores_All() Duration : " << duration.count() << endl;;
	}
	
	return S_OK;
}

HRESULT CVoxel_SectorLayer::Convert_Wall_BottomNeighbors()
{
	for (auto Pair : m_Sectors)
	{
		auto		pSector{ Pair.second };
		auto		Indicies{ pSector->Get_Indices() };

		for (auto iIndex : Indicies)
		{
			_uint			iNeighborFlag;
			if (FAILED(pSector->Get_NeighborFlag(iIndex, iNeighborFlag)))
				return E_FAIL;
			
			if (0 != (iNeighborFlag & g_iCTC))
			{
				pSector->Set_ID(iIndex, VOXEL_ID::_WALL);
			}
		}
	}

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Update_OpenessScores_All()
{
	for (auto& Pair : m_Sectors)
	{
		auto pSector = Pair.second;

		vector<_uint>			VoxelIndices = { pSector->Get_Indices() };

		for (auto iVoxelIndex : VoxelIndices)
		{
			_float		fOpenessScore{};
			if (FAILED(Compute_OpenessScore(iVoxelIndex, fOpenessScore)))
			{
				MSG_BOX(TEXT("Faield To Compute OpnessScore => Update_OpenessScore"));
				return E_FAIL;
			}

			_byte		bScore = { static_cast<_byte>(fOpenessScore) };
			if (FAILED(pSector->Set_OpenessScore(iVoxelIndex, bScore)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Update_OpenessScores_NearArea(const _uint3& vIndexPos)
{
	_uint		iMaxStep = { m_pVoxelScorer->Get_MaxStep() };
	_uint		iVoxelIndex;

	if (FAILED(m_pVoxelConvertor->Convert_VoxelIndex_From_IndexPos(vIndexPos, iVoxelIndex)))
		return E_FAIL;

	for (_int iOffsetX = static_cast<_int>(iMaxStep) * -1; iOffsetX <= static_cast<_int>(iMaxStep); ++iOffsetX)
	{
		for (_int iOffsetY = static_cast<_int>(iMaxStep) * -1; iOffsetY <= static_cast<_int>(iMaxStep); ++iOffsetY)
		{
			for (_int iOffsetZ = static_cast<_int>(iMaxStep) * -1; iOffsetZ <= static_cast<_int>(iMaxStep); ++iOffsetZ)
			{
				_int		iNewX = { static_cast<_int>(vIndexPos.x) + iOffsetX };
				_int		iNewY = { static_cast<_int>(vIndexPos.y) + iOffsetY };
				_int		iNewZ = { static_cast<_int>(vIndexPos.z) + iOffsetZ };

				if (0 > min(iNewX, min(iNewY, iNewZ)))
					continue;

				_uint		iNewIndex;
				_uint3		vNewIndexPos = {
					static_cast<_uint>(iNewX),
				static_cast<_uint>(iNewY),
				static_cast<_uint>(iNewZ) };

				if (FAILED(m_pVoxelConvertor->Convert_VoxelIndex_From_IndexPos(vNewIndexPos, iNewIndex)))
					continue;

				if (false == Is_Exist_Voxel(vNewIndexPos))
					continue;

				_float		fOpenessScore{};
				if (FAILED(Compute_OpenessScore(iNewIndex, fOpenessScore)))
					return E_FAIL;

				_uint		iSectorIndex;
				if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vNewIndexPos, iSectorIndex)))
					return E_FAIL;

				_byte		bScore = { static_cast<_byte>(fOpenessScore) };

				auto		pSector = Get_Sector(iSectorIndex);
				if (nullptr == pSector)
					continue; 

				if (FAILED(pSector->Set_OpenessScore(iNewIndex, bScore)))
					return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Compute_OpenessScore(const _uint iVoxelIndex, _float& fOpeneesScore)
{
	fOpeneesScore = m_pVoxelScorer->Copute_VoxelOpennessScore(iVoxelIndex);

	return S_OK;
}

vector<_uint> CVoxel_SectorLayer::Get_CurrentActive_SectorIndices(const unordered_set<_uint>& ActiveSectorIndicies)
{
	vector<_uint>			Indices;

	for (auto& iIndex : ActiveSectorIndicies)
	{
		if (false == Is_Exist_Sector(iIndex))
			continue;

		Indices.push_back(iIndex);
	}

	return Indices;
}

vector<_uint3> CVoxel_SectorLayer::Get_CurrentActive_SectorIndexPoses(const unordered_set<_uint>& ActiveSectorIndicies)
{
	vector<_uint3>			IndexPoses;

	_uint		iNumSectorWorldDim = { *m_pNumSectorWorldDim };

	for (auto& iIndex : ActiveSectorIndicies)
	{
		if (false == Is_Exist_Sector(iIndex))
			continue;

		_uint3			vIndexPos;
		vIndexPos.x = iIndex % iNumSectorWorldDim;
		vIndexPos.y = iIndex % (iNumSectorWorldDim * iNumSectorWorldDim);
		vIndexPos.z = iIndex % (iNumSectorWorldDim * iNumSectorWorldDim * iNumSectorWorldDim);

		IndexPoses.push_back(vIndexPos);
	}

	return IndexPoses;
}

_uint CVoxel_SectorLayer::Get_NumVoxel_Total()
{
	_uint			iNumVoxel = {};
	for (auto& Pair : m_Sectors)
	{
		CVoxel_Sector* pSector = { Pair.second };

		iNumVoxel += pSector->Get_NumVoxel();
	}

	return iNumVoxel;
}

_uint CVoxel_SectorLayer::Get_NumSector_Total()
{
	return static_cast<_uint>(m_Sectors.size());
}

_uint CVoxel_SectorLayer::Get_NumVoxel_Active(const unordered_set<_uint>& ActiveSectorIndices)
{
	_uint			iNumVoxel = {};
	for (auto& iIndex : ActiveSectorIndices)
	{
		if (false == Is_Exist_Sector(iIndex))
			continue;

		iNumVoxel += m_Sectors[iIndex]->Get_NumVoxel();
	}

	return iNumVoxel;
}

_float CVoxel_SectorLayer::Get_WorldSize_Voxel()
{
	return (*m_pVoxelSize) * m_pTransform->Get_Scaled().x;
}

vector<_uint> CVoxel_SectorLayer::Get_NeighborIndices(_uint iIndex)
{
	vector<_uint>			NeighborIndicies;

	_uint					iSectorIndex, iNeighborFlag;
	_uint3					vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_VoxelIndex(iIndex, iSectorIndex)) ||
		false == Is_Exist_Sector(iSectorIndex) ||
		FAILED(m_Sectors[iSectorIndex]->Get_NeighborFlag(iIndex, iNeighborFlag)) ||
		FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iIndex, vIndexPos)))
		return NeighborIndicies;

	static const _uint			s_iNumBit = { 27 };
	NeighborIndicies.reserve(s_iNumBit);
	for (_uint iBit = 0; iBit < s_iNumBit; ++iBit)
	{
		_uint			iCurBit = { static_cast<_uint>(1) << iBit };
		if (0 == (iCurBit & iNeighborFlag))
			continue;

		_uint			iTempBit = { iBit };
		_int			iOffsetX = { iTempBit % 3 - 1 };
		iTempBit /= 3;
		_int			iOffsetY = { iTempBit % 3 - 1 };
		iTempBit /= 3;
		_int			iOffsetZ = { iTempBit % 3 - 1 };

		_uint3			vNeighborIndexPos = { vIndexPos.x + iOffsetX, vIndexPos.y + iOffsetY, vIndexPos.z + iOffsetZ };
		_uint			iNeighborIndex;
		if (FAILED(m_pVoxelConvertor->Convert_VoxelIndex_From_IndexPos(vNeighborIndexPos, iNeighborIndex)))
			continue;

		NeighborIndicies.push_back(iNeighborIndex);
	}

	auto& iter{ find(NeighborIndicies.begin(), NeighborIndicies.end(),iIndex) };

	if (iter != NeighborIndicies.end())
		NeighborIndicies.erase(iter);

	return NeighborIndicies;
}

vector<_uint> CVoxel_SectorLayer::Get_NeighborIndices_All(_uint iIndex)
{
	vector<_uint>			NeighborIndicies;
	_uint3					vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iIndex, vIndexPos)))
		return NeighborIndicies;

	static const _uint			s_iNumBit = { 27 };
	NeighborIndicies.reserve(s_iNumBit);
	for (_uint iBit = 0; iBit < s_iNumBit; ++iBit)
	{
		_uint			iCurBit = { static_cast<_uint>(1) << iBit };

		_uint			iTempBit = { iBit };
		_int			iOffsetX = { iTempBit % 3 - 1 };		iTempBit /= 3;
		_int			iOffsetY = { iTempBit % 3 - 1 };		iTempBit /= 3;
		_int			iOffsetZ = { iTempBit % 3 - 1 };

		_uint3			vNeighborIndexPos = { vIndexPos.x + iOffsetX, vIndexPos.y + iOffsetY, vIndexPos.z + iOffsetZ };
		_uint			iNeighborIndex;
		if (FAILED(m_pVoxelConvertor->Convert_VoxelIndex_From_IndexPos(vNeighborIndexPos, iNeighborIndex)))
			continue;

		NeighborIndicies.push_back(iNeighborIndex);
	}

	auto& iter{ find(NeighborIndicies.begin(), NeighborIndicies.end(),iIndex) };

	if (iter != NeighborIndicies.end())
		NeighborIndicies.erase(iter);

	return NeighborIndicies;
}

HRESULT CVoxel_SectorLayer::Get_VoxelID(const _uint3& vIndexPos, VOXEL_ID& eID)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Get_ID(vIndexPos, eID);
}

HRESULT CVoxel_SectorLayer::Set_VoxelID(const _uint3& vIndexPos, VOXEL_ID eID)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Set_ID(vIndexPos, eID);
}

HRESULT CVoxel_SectorLayer::Get_VoxelState(const _uint3& vIndexPos, VOXEL_STATE& eState)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Get_State(vIndexPos, eState);
}

HRESULT CVoxel_SectorLayer::Set_VoxelState(const _uint3& vIndexPos, VOXEL_STATE eState)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Set_State(vIndexPos, eState);
}

HRESULT CVoxel_SectorLayer::Get_VoxelOpenessScore(const _uint3& vIndexPos, _byte& bOpenessScore)
{
	if (false == Is_Exist_Voxel(vIndexPos))
		return E_FAIL;

	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Get_OpenessScore(vIndexPos, bOpenessScore);
}

HRESULT CVoxel_SectorLayer::Get_NeighborFlag(const _uint3& vIndexPos, _uint& iNeighborFlag)
{
	_uint			iSectorIndex;
	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Get_NeighborFlag(vIndexPos, iNeighborFlag);
}

HRESULT CVoxel_SectorLayer::Add_Neighbor(const _uint3& vIndexPos, const _uint iNeighborFlag)
{
	_uint		iSectorIndex{};

	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Add_Neighbor(vIndexPos, iNeighborFlag);
}

HRESULT CVoxel_SectorLayer::Add_Neighbor(const _uint iIndex, const _uint iNeighborFlag)
{
	_uint3		vIndexPos{};
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iIndex, vIndexPos)))
		return E_FAIL;

	return Add_Neighbor(vIndexPos, iNeighborFlag);
}

HRESULT CVoxel_SectorLayer::Erase_Neighbor(const _uint3& vIndexPos, const _uint iNeighborFlag)
{
	_uint		iSectorIndex{};

	if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_IndexPos(vIndexPos, iSectorIndex)))
		return E_FAIL;

	auto		pSector = Get_Sector(iSectorIndex);
	if (nullptr == pSector)
		return E_FAIL;

	return pSector->Erase_Neighbor(vIndexPos, iNeighborFlag);
}

HRESULT CVoxel_SectorLayer::Erase_Neighbor(const _uint iIndex, const _uint iNeighborFlag)
{
	_uint3		vIndexPos{};
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iIndex, vIndexPos)))
		return E_FAIL;

	return Erase_Neighbor(vIndexPos, iNeighborFlag);
}

_uint CVoxel_SectorLayer::Get_CurPickedIndex()
{
	return m_pVoxelPicker->Get_CurPickIndex();
}

_uint CVoxel_SectorLayer::Get_PrePickedIndex()
{
	return m_pVoxelPicker->Get_PrePickIndex();
}

_float3 CVoxel_SectorLayer::Get_CurPickedWorldPos()
{
	return m_pVoxelPicker->Get_CurPickWorldPos();
}

_float3 CVoxel_SectorLayer::Get_PrePickedWorldPos()
{
	return m_pVoxelPicker->Get_PrePickWorldPos();
}

HRESULT CVoxel_SectorLayer::Add_Supports()
{
	return S_OK;
}

HRESULT CVoxel_SectorLayer::Add_Components()
{
	m_VIBufferInstanceCubes.clear();
	for (_uint i = 0; i < g_iNumVoxelInstancePool; ++i)
	{
		CVIBuffer_Instance_Cube* pVIBufferCube = { CVIBuffer_Instance_Cube::Create(m_pDevice, m_pContext) };
		if (nullptr == pVIBufferCube)
			return E_FAIL;

		m_VIBufferInstanceCubes.push_back(pVIBufferCube);
	}

	m_pShaderInstanceCube = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Cube.hlsl"), VTXINSTANCE_CUBE::Elements, VTXINSTANCE_CUBE::iNumElements);
	if (nullptr == m_pShaderInstanceCube)
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Cooking_TerrainBuffer(CVIBuffer_Terrain* pTerrainBuffer, CTransform* pTerrainTransform)
{
	const _uint		iWidth = { pTerrainBuffer->Get_Width() };
	const _uint		iDepth = { pTerrainBuffer->Get_Depth() };
	const _float	fMaxX = { static_cast<_float>(iWidth) * pTerrainTransform->Get_Scaled().x };
	const _float	fMaxZ = { static_cast<_float>(iDepth) * pTerrainTransform->Get_Scaled().z };

	vector<_uint>		CurRowHeights;
	vector<_uint>		PreRowHeights;

	_matrix			WorldMatrix = { m_pTransform->Get_WorldMatrix() };

	_uint			iNumVoxelWorldDim = { *m_pNumVoxelWorldDim };
	_float			fVoxelSize = { *m_pVoxelSize };

	for (_uint iZ = 0; iZ < iNumVoxelWorldDim; ++iZ)
	{
		_float		fZ = { static_cast<_float>(iZ) };

		PreRowHeights = CurRowHeights;
		CurRowHeights.clear();

		for (_uint iX = 0; iX < iNumVoxelWorldDim; ++iX)
		{
			_float		fX = { static_cast<_float>(iX) };

			_float		fLocalX = { (fVoxelSize)*iX };
			_float		fLocalZ = { (fVoxelSize)*iZ };

			if (fMaxX < fLocalX ||
				fMaxZ < fLocalZ)
				continue;

			_vector		vLocalPos = { XMVectorSet(fLocalX, 0.f, fLocalZ, 1.f) };
			_vector		vWorldPos = { XMVector3TransformCoord(vLocalPos, WorldMatrix) };

			_float4		vResultPos = {};
			pTerrainBuffer->Compute_Height(pTerrainTransform, vWorldPos, &vResultPos);

			/*vResultPos.y /= static_cast<_float>(g_iTerrainMaxHeight);
			vResultPos.y *= g_iTerrainMaxHeight;*/
			vResultPos.y = max(0.f, vResultPos.y);
			vResultPos.y = min(static_cast<_float>(g_iVoxelPosLimit * fVoxelSize), vResultPos.y);

			_uint3		vIndexPos;
			if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(*(_float3*)&vResultPos, vIndexPos)))
				return E_FAIL;

			if (FAILED(Add_Voxel(vIndexPos, VOXEL_ID::_FLOOR, VOXEL_STATE::_DEFAULT)))
				return E_FAIL;


			_uint		iCurHeight = { vIndexPos.y };
			if (1 < CurRowHeights.size())
			{
				_uint		iLeftHeight = { CurRowHeights[vIndexPos.x - 1] };
				_uint		iTempCurHeight = { iCurHeight };

				while (1 < abs(static_cast<_int>(iLeftHeight) - static_cast<_int>(iTempCurHeight)))
				{
					if (iLeftHeight > iTempCurHeight)
					{
						--iLeftHeight;
						_uint3 vTempPos = { vIndexPos.x - 1, iLeftHeight, vIndexPos.z };

						if (true == Is_Exist_Voxel(vTempPos))
							continue;

						if (FAILED(Add_Voxel(vTempPos, VOXEL_ID::_WALL)))
							return E_FAIL;
					}
					else
					{
						--iTempCurHeight;
						_uint3 vTempPos = { vIndexPos.x, iTempCurHeight, vIndexPos.z };

						if (true == Is_Exist_Voxel(vTempPos))
							continue;

						if (FAILED(Add_Voxel(vTempPos, VOXEL_ID::_WALL)))
							return E_FAIL;
					}
				}
			}

			if (false == PreRowHeights.empty())
			{
				_uint		iBackHeight = { PreRowHeights[vIndexPos.x] };

				_uint		iTempCurHeight = { iCurHeight };

				while (1 < abs(static_cast<_int>(iBackHeight) - static_cast<_int>(iTempCurHeight)))
				{
					if (iBackHeight > iTempCurHeight)
					{
						--iBackHeight;
						_uint3 vTempPos = { vIndexPos.x, iBackHeight, vIndexPos.z - 1 };

						if (true == Is_Exist_Voxel(vTempPos))
							continue;

						if (FAILED(Add_Voxel(vTempPos, VOXEL_ID::_WALL)))
							return E_FAIL;
					}
					else
					{
						--iTempCurHeight;
						_uint3 vTempPos = { vIndexPos.x, iTempCurHeight, vIndexPos.z };

						if (true == Is_Exist_Voxel(vTempPos))
							continue;

						if (FAILED(Add_Voxel(vTempPos, VOXEL_ID::_WALL)))
							return E_FAIL;
					}
				}

				if (1 < CurRowHeights.size())
				{
					iTempCurHeight = iCurHeight;
					_uint		iLeftBackHeight = { PreRowHeights[vIndexPos.x - 1] };
					while (1 < abs(static_cast<_int>(iLeftBackHeight) - static_cast<_int>(iTempCurHeight)))
					{
						if (iLeftBackHeight > iTempCurHeight)
						{
							--iLeftBackHeight;
							_uint3 vTempPos = { vIndexPos.x - 1, iLeftBackHeight, vIndexPos.z - 1 };

							if (true == Is_Exist_Voxel(vTempPos))
								continue;

							if (FAILED(Add_Voxel(vTempPos, VOXEL_ID::_WALL)))
								return E_FAIL;
						}
						else
						{
							--iTempCurHeight;
							_uint3 vTempPos = { vIndexPos.x, iTempCurHeight, vIndexPos.z };

							if (true == Is_Exist_Voxel(vTempPos))
								continue;

							if (FAILED(Add_Voxel(vTempPos, VOXEL_ID::_WALL)))
								return E_FAIL;
						}
					}
				}

			}

			CurRowHeights.push_back(iCurHeight);
		}
	}

	return S_OK;
}

void CVoxel_SectorLayer::Clear_Sectors()
{
	for (auto& Pair : m_Sectors)
		Safe_Release(Pair.second);

	m_Sectors.clear();
}

_bool CVoxel_SectorLayer::Is_Culled_Sector(const _uint iSectorIndex)
{
	_uint3			vSectorIndexPos;
	_uint			iNumSectorWorldDim = { *m_pNumSectorWorldDim };
	_uint			iTempIndex = { iSectorIndex };
	vSectorIndexPos.x = iTempIndex % iNumSectorWorldDim;
	iTempIndex /= iNumSectorWorldDim;
	vSectorIndexPos.y = iTempIndex % iNumSectorWorldDim;
	iTempIndex /= iNumSectorWorldDim;
	vSectorIndexPos.z = iTempIndex % iNumSectorWorldDim;

	_float				fVoxelSize = { *m_pVoxelSize };
	_uint				iNumVoxelSectorDim = { *m_pNumVoxelSectorDim };

	_float				fSectorSize = { fVoxelSize * iNumVoxelSectorDim };
	_vector				vLocalPos = { (XMLoadUInt3(&vSectorIndexPos) * fSectorSize + (XMLoadUInt3(&vSectorIndexPos) + XMVectorSet(1.f, 1.f, 1.f, 0.f)) * fSectorSize) * 0.5f };

	if (true == Is_Culled_Frustum_Sector(vLocalPos))
		return true;

	return false;
}

_bool CVoxel_SectorLayer::Is_Culled_Frustum_Sector(_fvector vPos)
{
	_uint		iNumVoxelSectorDim = { *m_pNumVoxelSectorDim };
	_float		fVoxelSize = { *m_pVoxelSize };

	return !(m_pGameInstance->isInFrustum_LocalSpace(vPos, iNumVoxelSectorDim * fVoxelSize * sqrtf(3.f) * 0.5f));
}

HRESULT CVoxel_SectorLayer::Erase_Neighbors(const _uint iVoxelIndex)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Erase_Neighbors(vIndexPos);
}

HRESULT CVoxel_SectorLayer::Erase_Neighbors(const _uint3& vIndexPos)
{
	if (false == Is_Exist_Voxel(vIndexPos))
		return E_FAIL;

	_int		Offsets[] = { -1, 0, 1 };
	for (auto iOffsetZ : Offsets)
	{
		for (auto iOffsetY : Offsets)
		{
			for (auto iOffsetX : Offsets)
			{
				_int		iNewX = { static_cast<_int>(vIndexPos.x) + iOffsetX };
				_int		iNewY = { static_cast<_int>(vIndexPos.y) + iOffsetY };
				_int		iNewZ = { static_cast<_int>(vIndexPos.z) + iOffsetZ };

				if (0 > static_cast<_int>(min(iNewX, min(iNewY, iNewZ))))
					continue;

				_uint3		vDstIndexPos = {
					static_cast<_uint>(iNewX),
					static_cast<_uint>(iNewY),
					static_cast<_uint>(iNewZ)
				};

				if (false == Is_Exist_Voxel(vDstIndexPos))
					continue;

				_uint		iSrcNeighborFlag = {
					static_cast<_uint>(powf(2.f, static_cast<_float>(iOffsetX + 1) + static_cast<_float>(iOffsetY + 1) * 3.f + static_cast<_float>(iOffsetZ + 1) * 9.f) + EPSILON)
				};

				_int		iDstDirX = { iOffsetX * -1 };
				_int		iDstDirY = { iOffsetY * -1 };
				_int		iDstDirZ = { iOffsetZ * -1 };

				_uint		iDstNeighborFlag = {
					static_cast<_uint>(powf(2.f, static_cast<_float>(iDstDirX + 1) + static_cast<_float>(iDstDirY + 1) * 3 + static_cast<_float>(iDstDirZ + 1) * 9) + EPSILON)
				};

				if (FAILED(Erase_Neighbor(vIndexPos, iSrcNeighborFlag)))
					return E_FAIL;
				if (FAILED(Erase_Neighbor(vDstIndexPos, iDstNeighborFlag)))
					return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CVoxel_SectorLayer::Erase_Neighbors(const _float3& vWorldPos)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Erase_Neighbors(vIndexPos);
}

HRESULT CVoxel_SectorLayer::Compute_Picking(_uint& PickIndex)
{
	_float4			vRayOriginPosFloat4 = { m_pGameInstance->Get_MouseWorldPos() };
	_float3			vRayDirFloat3 = { m_pGameInstance->Get_MouseWorldDirection() };
	_vector			vRayOriginPos = { XMLoadFloat4(&vRayOriginPosFloat4) };

	_vector			vRayDir = { XMVector3Normalize(XMLoadFloat3(&vRayDirFloat3)) };
	_float			fMax = { max(fabsf(XMVectorGetX(vRayDir)), max(fabsf(XMVectorGetY(vRayDir)), fabsf(XMVectorGetZ(vRayDir)))) };

	vRayDir /= fMax;
	vRayDir *= (*m_pVoxelSize);

	_float3			vTheta = { (XMVectorGetX(vRayOriginPos) < 0) ? -XMVectorGetX(vRayOriginPos) / XMVectorGetX(vRayDir) : 0,
	(XMVectorGetY(vRayOriginPos) < 0) ? -XMVectorGetY(vRayOriginPos) / XMVectorGetY(vRayDir) : 0,
	(XMVectorGetZ(vRayOriginPos) < 0) ? -XMVectorGetZ(vRayOriginPos) / XMVectorGetZ(vRayDir) : 0 };

	_float			fMinTheta = max(vTheta.x, max(vTheta.y, vTheta.z));
	vRayOriginPos = vRayOriginPos + fMinTheta * vRayDir;

	// 초기화
	_vector			vNextPos = { vRayOriginPos };

	// DDA 루프
	while (true) {
		// 다음 위치 계산
		vNextPos = vNextPos + vRayDir;

		if (XMVectorGetX(vNextPos) > static_cast<_float>(g_iWorldLength) ||
			XMVectorGetY(vNextPos) > static_cast<_float>(g_iWorldLength) ||
			XMVectorGetZ(vNextPos) > static_cast<_float>(g_iWorldLength))
			break;

		_float3			vNextPosFloat3;
		XMStoreFloat3(&vNextPosFloat3, vNextPos);

		_uint3			vNextIndexPos;
		if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vNextPosFloat3, vNextIndexPos)))
			return E_FAIL;

		// 인덱스 범위 및 존재 여부 확인
		if (!Is_Exist_Voxel(vNextIndexPos))
			continue;

		return m_pVoxelConvertor->Convert_VoxelIndex_From_IndexPos(vNextIndexPos, PickIndex);
	}

	return E_FAIL;
}

struct CMP
{
	_bool operator()(const _uint3& Src, const _uint3& Dst) const
	{
		if (Src.x != Dst.x)
			return Src.x < Dst.x;
		if (Src.y != Dst.y)
			return Src.y < Dst.y;

		return Src.z < Dst.z;
	}
};

_bool CVoxel_SectorLayer::Is_Reach_TargetIndex(const _uint iStartIndex, const _uint3 &vStartIndexPosUInt3, const _uint iTargetIndex, const _uint3& vTargetIndexPosUInt3)
{
	if (iStartIndex == iTargetIndex)
		return true;

	_vector				vDirectionToTarget = { XMLoadUInt3(&vTargetIndexPosUInt3) - XMLoadUInt3(&vStartIndexPosUInt3) };
	const _float		fMaxDist = { XMVectorGetX(XMVector3Length(vDirectionToTarget)) };

	_vector				vNormarlizeDir = { XMVector3Normalize(vDirectionToTarget) };


	_vector			vStepX = { vNormarlizeDir / fmaxf(fabsf(XMVectorGetX(vNormarlizeDir)), EPSILON) * 0.5f };
	_vector			vStepY = { vNormarlizeDir / fmaxf(fabsf(XMVectorGetY(vNormarlizeDir)), EPSILON) * 0.5f };
	_vector			vStepZ = { vNormarlizeDir / fmaxf(fabsf(XMVectorGetZ(vNormarlizeDir)), EPSILON) * 0.5f };

	_float			fStepLengthX = { XMVectorGetX(XMVector3Length(vStepX)) };
	_float			fStepLengthY = { XMVectorGetX(XMVector3Length(vStepY)) };
	_float			fStepLengthZ = { XMVectorGetX(XMVector3Length(vStepZ)) };

	_vector			vOffset = { XMVectorSet(0.5f, 0.5f, 0.5f, 0.f) };
	_vector			vStartPos = { XMLoadUInt3(&vStartIndexPosUInt3) };
	_vector			vNextPos = { vStartPos + vOffset };
	_vector			vNextPosX = { vNextPos };
	_vector			vNextPosY = { vNextPos };
	_vector			vNextPosZ = { vNextPos };
	_float			fCurDist = {};
	_float			fCurDistX = {};
	_float			fCurDistY = {};
	_float			fCurDistZ = {};

	_uint			iMaxStep = { static_cast<_uint>(fMaxDist / fminf(fStepLengthX, fminf(fStepLengthY, fStepLengthZ))) };

	while (fMaxDist - fCurDist > EPSILON)
	{
		_float		fNextDistX = { fCurDistX + fStepLengthX };
		_float		fNextDistY = { fCurDistY + fStepLengthY };
		_float		fNextDistZ = { fCurDistZ + fStepLengthZ };

		if (fNextDistX < fNextDistY &&
			fNextDistX < fNextDistZ)
		{
			vNextPos = vNextPosX += vStepX;
			fCurDist = fCurDistX += fStepLengthX;
		}

		else if (fNextDistY < fNextDistZ)
		{
			vNextPos = vNextPosY += vStepY;
			fCurDist = fCurDistY += fStepLengthY;
		}

		else
		{
			vNextPos = vNextPosZ += vStepZ;
			fCurDist = fCurDistZ += fStepLengthZ;
		}

		_uint3		vNextIndexPosUInt3 = {
			static_cast<_uint>(XMVectorGetX(vNextPos)),
			static_cast<_uint>(XMVectorGetY(vNextPos)),
			static_cast<_uint>(XMVectorGetZ(vNextPos))
		};

		map<_uint3, VOXEL_ID, CMP> CurCheckedPoses;

		if (XMVectorGetX(vNextPos) == static_cast<_float>(vNextIndexPosUInt3.x))
		{
			_uint3		vNextIndexPosUInt3_Lower = vNextIndexPosUInt3;
			vNextIndexPosUInt3_Lower.x -= 1;

			VOXEL_ID	eID;
			Get_VoxelID(vNextIndexPosUInt3_Lower, eID);
			CurCheckedPoses.emplace(vNextIndexPosUInt3_Lower, eID);
		}

		if (XMVectorGetY(vNextPos) == static_cast<_float>(vNextIndexPosUInt3.y))
		{
			_uint3		vNextIndexPosUInt3_Lower = vNextIndexPosUInt3;
			vNextIndexPosUInt3_Lower.y -= 1;

			VOXEL_ID	eID;
			Get_VoxelID(vNextIndexPosUInt3_Lower, eID);
			CurCheckedPoses.emplace(vNextIndexPosUInt3_Lower, eID);
		}

		if (XMVectorGetZ(vNextPos) == static_cast<_float>(vNextIndexPosUInt3.z))
		{
			_uint3		vNextIndexPosUInt3_Lower = vNextIndexPosUInt3;
			vNextIndexPosUInt3_Lower.z -= 1;

			VOXEL_ID	eID;
			Get_VoxelID(vNextIndexPosUInt3_Lower, eID);
			CurCheckedPoses.emplace(vNextIndexPosUInt3_Lower, eID);
		}

		VOXEL_ID	eID;
		// 월드 좌표 -> 인덱스 변환
		Get_VoxelID(vNextIndexPosUInt3, eID);
		CurCheckedPoses.emplace(vNextIndexPosUInt3, eID);

		_uint iFloorCnt = {};
		for (auto& Pair : CurCheckedPoses)
		{
			eID = Pair.second;
			if (VOXEL_ID::_WALL == eID)
				return false;

			if (VOXEL_ID::_FLOOR == eID)
				++iFloorCnt;
		}

		if (CurCheckedPoses.size() / 2 + CurCheckedPoses.size() % 2 > iFloorCnt)
			return false;
	}

	return true;
}

void CVoxel_SectorLayer::Clear_Instance_Buffers()
{
	_uint		iNumMaxInstance = { m_VIBufferInstanceCubes.front()->Get_MaxInstance() };
	_uint		iPositionBufferSize = { static_cast<_uint>(m_Instance_Position_Buffer.size()) };
	m_Instance_Position_Buffer.clear();

	if (iPositionBufferSize < m_Instance_State_Buffer.size())
	{
		vector<_byte>		TempStateBuffer(m_Instance_State_Buffer.begin() + iPositionBufferSize, m_Instance_State_Buffer.end());
		vector<_byte>		TempIDBuffer(m_Instance_ID_Buffer.begin() + iPositionBufferSize, m_Instance_ID_Buffer.end());
		vector<_int>		TempNeighborBuffer(m_Instance_Neighbor_Buffer.begin() + iPositionBufferSize, m_Instance_Neighbor_Buffer.end());

		m_Instance_ID_Buffer = move(TempIDBuffer);
		m_Instance_State_Buffer = move(TempStateBuffer);
		m_Instance_Neighbor_Buffer = move(TempNeighborBuffer);
	}
	else
	{
		m_Instance_ID_Buffer.clear();
		m_Instance_State_Buffer.clear();
		m_Instance_Neighbor_Buffer.clear();
		m_Instance_ID_Buffer.reserve(iNumMaxInstance);
		m_Instance_State_Buffer.reserve(iNumMaxInstance);
		m_Instance_Neighbor_Buffer.reserve(iNumMaxInstance);
	}

	m_Instance_Position_Buffer.reserve(iNumMaxInstance);
}

void CVoxel_SectorLayer::Bind_Instance_Buffers(const _uint iCurVIBufferIndex)
{
	CVIBuffer_Instance_Cube* pVIBuffer = { m_VIBufferInstanceCubes[iCurVIBufferIndex % g_iNumVoxelInstancePool] };

	pVIBuffer->Set_Positions_States_IDs(m_Instance_Position_Buffer, m_Instance_State_Buffer, m_Instance_ID_Buffer, m_Instance_Neighbor_Buffer, (*m_pVoxelSize));
}

void CVoxel_SectorLayer::Render_Current_Instance_Buffers(const _uint iCurVIBufferIndex)
{
	CVIBuffer_Instance_Cube*		pVIBuffer = { m_VIBufferInstanceCubes[iCurVIBufferIndex % g_iNumVoxelInstancePool] };
	_float							fVoxelSize = { *m_pVoxelSize };

	if (FAILED(m_pShaderInstanceCube->Bind_RawValue("g_fVoxelSize", &fVoxelSize, sizeof(_float))))
		return;

	if (FAILED(m_pShaderInstanceCube->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return;

	if (FAILED(m_pShaderInstanceCube->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return;

	if (FAILED(m_pTransform->Bind_ShaderResource(m_pShaderInstanceCube, "g_WorldMatrix")))
		return;

	m_pShaderInstanceCube->Begin(0);
	pVIBuffer->Bind_Buffers();
	pVIBuffer->Render();
}

CVoxel_SectorLayer* CVoxel_SectorLayer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_SectorLayer* pInstance = { new CVoxel_SectorLayer{pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxel_SectorLayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_SectorLayer::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pShaderInstanceCube);

	Safe_Release(m_pTransform);
	Safe_Release(m_pVoxelConvertor);
	Safe_Release(m_pVoxelScorer);
	Safe_Release(m_pVoxelPicker);

	for (auto& pVIBuffer : m_VIBufferInstanceCubes)
		Safe_Release(pVIBuffer);
	m_VIBufferInstanceCubes.clear();

	Clear_Sectors();
}
