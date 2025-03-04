#include "Voxel_Manager.h"
#include "GameInstance.h"

#include "VoxelSector.h"
#include "Voxel_SectorLayer.h"
#include "VoxelDebugger.h"
#include "VoxelPicker.h"
#include "Voxel_Scorer.h"
#include "Voxel_Parser.h"
#include "Voxel_Convertor.h"

#include "Transform.h"

CVoxel_Manager::CVoxel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CVoxel_Manager::Initialize()
{
	m_fTempVoxelSize = m_fVoxelSize = g_fDefaultVoxelLength;
	m_fTempMaxCookingHeight = m_fMaxCookingHeight;
	m_iNumSectorWorldDim = g_iWorldLength / g_iSectorLength;
	m_iNumVoxelWorldDim =
		static_cast<_uint>(min(g_iWorldLength,
			static_cast<_uint>(static_cast<_float>(g_iWorldLength) / m_fVoxelSize)));
	m_iNumVoxelSectorDim = static_cast<_uint>(static_cast<_float>(g_iSectorLength) / m_fVoxelSize);

	if (FAILED(Add_Components()))
		return E_FAIL;

	if (FAILED(Add_Supports()))
		return E_FAIL;

	if (FAILED(Add_Layers()))
		return E_FAIL;

	m_RenderStates.resize(static_cast<_uint>(VOXEL_LAYER::_END));

	return S_OK;
}

void CVoxel_Manager::Priority_Tick(_float fTimeDelta)
{
	Update_Cur_Active_Sectors();
}

void CVoxel_Manager::Tick(_float fTimeDelta)
{
	m_pVoxelPicker->Tick();
	m_pVoxelDebugger->Tick();

	for (auto& pVoxelSectorLayer : m_VoxelLayers)
		pVoxelSectorLayer->Tick(fTimeDelta);
}

HRESULT CVoxel_Manager::Render()
{
	////	컬링 이전 프러스텀을 내 로컬로 가져온다
	//_matrix			WorldMatrixInv = { m_pTransformCom->Get_WorldMatrix_Inverse() };
	//m_pGameInstance->TransformFrustum_LocalSpace(WorldMatrixInv);

	for (_uint i = 0; i < static_cast<_uint>(VOXEL_LAYER::_END); ++i)
	{
		if (false == m_RenderStates[i])
			continue;

		if (FAILED(m_VoxelLayers[i]->Render(m_ActiveSectors)))
			return E_FAIL;
	}

	if (FAILED(m_pVoxelDebugger->Render()))
		return E_FAIL;

	return S_OK;
}

void CVoxel_Manager::Update_Cur_Active_Sectors()
{
	m_ActiveSectors.clear();

	_vector					vCamPos = { m_pGameInstance->Get_CamPosition_Vector() };
	_int					Offsets[] = { -2, -1, 0, 1, 2 };
	_float					fSectorSize = { m_iNumVoxelSectorDim * m_fVoxelSize };
	_matrix					WorldMatrix = { m_pTransformCom->Get_WorldMatrix() };

	_matrix					WorldMatrixInv = { m_pTransformCom->Get_WorldMatrix_Inverse() };
	_vector					vCamPosLocal = { XMVector3TransformCoord(vCamPos, WorldMatrixInv) };

	for (auto& iOffsetZ : Offsets)
	{
		for (auto& iOffsetY : Offsets)
		{
			for (auto& iOffsetX : Offsets)
			{
				_uint		iCurSectorIndex = {};
				_vector		vDirection = { XMVectorSet(fSectorSize * iOffsetX , fSectorSize * iOffsetY, fSectorSize * iOffsetZ, 0.f) };
				_vector		vResultPos = { vCamPosLocal + vDirection };
				_vector		vResultWorldPos = { XMVector3TransformCoord(vResultPos, WorldMatrix) };
				_float3		vResultWorldPosFloat3;

				XMStoreFloat3(&vResultWorldPosFloat3, vResultWorldPos);

				if (FAILED(m_pVoxelConvertor->Convert_SectorIndex_From_WorldPos(vResultWorldPosFloat3, iCurSectorIndex)))
					continue;

				if (m_iNumSectorWorldDim * m_iNumSectorWorldDim * m_iNumSectorWorldDim <= iCurSectorIndex)
					continue;

				m_ActiveSectors.insert(iCurSectorIndex);
			}
		}
	}
}

HRESULT CVoxel_Manager::Add_Supports()
{
	m_pVoxelDebugger = CVoxel_Debugger::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVoxelDebugger)
		return E_FAIL;

	m_pVoxelPicker = CVoxel_Picker::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVoxelPicker)
		return E_FAIL;

	m_pVoxelScorer = CVoxel_Scorer::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVoxelScorer)
		return E_FAIL;

	m_pVoxelParser = CVoxel_Parser::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVoxelParser)
		return E_FAIL;

	CVoxel_Convertor::VOXEL_CONVERTOR_DESC			ConvertorDesc;
	ConvertorDesc.pNumSectorWorldDim = &m_iNumSectorWorldDim;
	ConvertorDesc.pNumVoxelSectorDim = &m_iNumVoxelSectorDim;
	ConvertorDesc.pNumVoxelWorldDim = &m_iNumVoxelWorldDim;
	ConvertorDesc.pTransform = m_pTransformCom;
	ConvertorDesc.pVoxelSize = &m_fVoxelSize;

	m_pVoxelConvertor = CVoxel_Convertor::Create(m_pDevice, m_pContext, &ConvertorDesc);
	if (nullptr == m_pVoxelConvertor)
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_Manager::Add_Components()
{

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxel_Manager::Add_Layers()
{
	m_VoxelLayers.resize(static_cast<_uint>(VOXEL_LAYER::_END));

	CVoxel_SectorLayer::VOXEL_SECTOR_LAYER_DESC			Desc;
	Desc.pNumSectorWorldDim = &m_iNumSectorWorldDim;
	Desc.pNumVoxelSectorDim = &m_iNumVoxelSectorDim;
	Desc.pNumVoxelWorldDim = &m_iNumVoxelWorldDim;
	Desc.pTempVoxelSize = &m_fTempVoxelSize;
	Desc.pVoxelSize = &m_fVoxelSize;
	Desc.pTransform = m_pTransformCom;
	Desc.pVoxelConvertor = m_pVoxelConvertor;
	Desc.pVoxelPicker = m_pVoxelPicker;
	Desc.pVoxelScorer = m_pVoxelScorer;

	for (_uint i = 0; i < static_cast<_uint>(VOXEL_LAYER::_END); ++i)
	{
		m_VoxelLayers[i] = CVoxel_SectorLayer::Create(m_pDevice, m_pContext, &Desc);
	}

	return S_OK;
}

CVoxel_SectorLayer* CVoxel_Manager::Get_Layer(const VOXEL_LAYER eLayer)
{
	if (VOXEL_LAYER::_END <= eLayer)
		return nullptr;

	return m_VoxelLayers[static_cast<_uint>(eLayer)];
}

_uint CVoxel_Manager::Get_NumVoxel_Active_In_Layer(const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer*			pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return 0;

	return pVoxelSectorLayer->Get_NumVoxel_Active(m_ActiveSectors);
}

_uint CVoxel_Manager::Get_NumVoxel_Active_All_Layer()
{
	_uint		iNumActiveVoxel = {};
	for (auto pVoxelSectorLayer : m_VoxelLayers)
	{
		iNumActiveVoxel += pVoxelSectorLayer->Get_NumVoxel_Active(m_ActiveSectors);
	}

	return iNumActiveVoxel;
}

_uint CVoxel_Manager::Get_NumVoxel_In_Layer(const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return 0;

	return pVoxelSectorLayer->Get_NumVoxel_Total();
}

_uint CVoxel_Manager::Get_NumVoxel_All_Layer()
{
	_uint		iNumVoxel = {};
	for (auto pVoxelSectorLayer : m_VoxelLayers)
	{
		iNumVoxel += pVoxelSectorLayer->Get_NumVoxel_Total();
	}

	return iNumVoxel;
}

_uint CVoxel_Manager::Get_NumSector(const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return 0;

	return pVoxelSectorLayer->Get_NumSector_Total();
}

_uint CVoxel_Manager::Get_NumSector_Active()
{
	return static_cast<_uint>(m_ActiveSectors.size());
}

_uint CVoxel_Manager::Get_NumVoxelWorldDim()
{
	return m_iNumVoxelWorldDim;
}

_float CVoxel_Manager::Get_WorldSize_Voxel()
{
	return m_pTransformCom->Get_Scaled().x * m_fVoxelSize;
}

vector<_uint> CVoxel_Manager::Get_CurrentActive_SectorIndices(const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return vector<_uint>();

	return pVoxelSectorLayer->Get_CurrentActive_SectorIndices(m_ActiveSectors);
}

vector<_uint3> CVoxel_Manager::Get_CurrentActive_SectorIndexPoses(const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return vector<_uint3>();

	return pVoxelSectorLayer->Get_CurrentActive_SectorIndexPoses(m_ActiveSectors);
}

HRESULT CVoxel_Manager::Add_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Add_Voxel(vIndexPos, eLayer, eID, eState);
}

HRESULT CVoxel_Manager::Add_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Add_Voxel(vIndexPos, eID, eState);
}

HRESULT CVoxel_Manager::Add_Voxel(const _float3& vWorldPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Add_Voxel(vIndexPos, eLayer, eID, eState);
}

HRESULT CVoxel_Manager::Erase_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Erase_Voxel(vIndexPos, eLayer);
}

HRESULT CVoxel_Manager::Erase_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Erase_Voxel(vIndexPos);
}

HRESULT CVoxel_Manager::Erase_Voxel(const _float3& vWorldPos, const VOXEL_LAYER eLayer)
{
	_uint3				vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Erase_Voxel(vIndexPos, eLayer);
}

_bool CVoxel_Manager::Is_Exist_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return false;

	return Is_Exist_Voxel(vIndexPos, eLayer);
}

_bool CVoxel_Manager::Is_Exist_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return false;

	return pVoxelSectorLayer->Is_Exist_Voxel(vIndexPos);
}

vector<_uint> CVoxel_Manager::Get_NeighborIndices(const _uint iIndex, VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return vector<_uint>();

	return pVoxelSectorLayer->Get_NeighborIndices(iIndex);
}

vector<_uint> CVoxel_Manager::Get_NeighborIndices(const _float3& vPos, VOXEL_LAYER eLayer)
{
	_uint			iVoxelIndex;
	if (FAILED(m_pVoxelConvertor->Convert_VoxelIndex_From_WorldPos(vPos, iVoxelIndex)))
		return vector<_uint>();

	return Get_NeighborIndices(iVoxelIndex, eLayer);
}

vector<_uint> CVoxel_Manager::Get_NeighborIndices_All(const _uint iIndex, VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return vector<_uint>();

	return pVoxelSectorLayer->Get_NeighborIndices_All(iIndex);
}

vector<_uint> CVoxel_Manager::Get_NeighborIndices_All(const _float3& vPos, VOXEL_LAYER eLayer)
{
	_uint			iVoxelIndex;
	if (FAILED(m_pVoxelConvertor->Convert_VoxelIndex_From_WorldPos(vPos, iVoxelIndex)))
		return vector<_uint>();

	return Get_NeighborIndices_All(iVoxelIndex, eLayer);
}

HRESULT CVoxel_Manager::Get_Index(const _float3& vPos, _uint& iIndex)
{
	return m_pVoxelConvertor->Convert_VoxelIndex_From_WorldPos(vPos, iIndex);
}

HRESULT CVoxel_Manager::Get_Index(const _uint3& vIndexPos, _uint& iIndex)
{
	return m_pVoxelConvertor->Convert_VoxelIndex_From_IndexPos(vIndexPos, iIndex);
}

HRESULT CVoxel_Manager::Get_WorldPosition(const _uint iIndex, _float3& vPos)
{
	return m_pVoxelConvertor->Convert_WorldPos_From_Index(iIndex, vPos);
}

HRESULT CVoxel_Manager::Get_WorldPosition(const _uint3& vIndexPos, _float3& vPos)
{
	return m_pVoxelConvertor->Convert_WorldPos_From_IndexPos(vIndexPos, vPos);
}

HRESULT CVoxel_Manager::Get_IndexPosition(const _uint iIndex, _uint3& vIndexPos)
{
	return m_pVoxelConvertor->Convert_IndexPos_From_Index(iIndex, vIndexPos);
}

HRESULT CVoxel_Manager::Get_IndexPosition(const _float3& vPos, _uint3& vIndexPos)
{
	return m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vPos, vIndexPos);
}

void CVoxel_Manager::Set_Render(const VOXEL_LAYER eLayer, _bool isRender)
{
	if (VOXEL_LAYER::_END <= eLayer)
		return;

	m_RenderStates[static_cast<_uint>(eLayer)] = isRender;
}

void CVoxel_Manager::Set_Render_All(_bool isRender)
{
	for (_uint i = 0; i < static_cast<_uint>(VOXEL_LAYER::_END); ++i)
		Set_Render(static_cast<VOXEL_LAYER>(i), isRender);
}

HRESULT CVoxel_Manager::Get_VoxelID(const _uint3& vIndexPos, VOXEL_ID& eID, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer*			pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Get_VoxelID(vIndexPos, eID);
}

HRESULT CVoxel_Manager::Set_VoxelID(const _uint3& vIndexPos, VOXEL_ID eID, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer*			pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Set_VoxelID(vIndexPos, eID);
}

HRESULT CVoxel_Manager::Get_VoxelID(const _float3& vWorldPos, VOXEL_ID& eID, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Get_VoxelID(vIndexPos, eID, eLayer);
}

HRESULT CVoxel_Manager::Set_VoxelID(const _float3& vWorldPos, VOXEL_ID eID, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Set_VoxelID(vIndexPos, eID, eLayer);
}

HRESULT CVoxel_Manager::Get_VoxelID(const _uint iVoxelIndex, VOXEL_ID& eID, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Get_VoxelID(vIndexPos, eID, eLayer);
}

HRESULT CVoxel_Manager::Set_VoxelID(const _uint iVoxelIndex, VOXEL_ID eID, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Set_VoxelID(vIndexPos, eID, eLayer);
}

HRESULT CVoxel_Manager::Get_VoxelState(const _uint3& vIndexPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer*			pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Get_VoxelState(vIndexPos, eState);
}

HRESULT CVoxel_Manager::Set_VoxelState(const _uint3& vIndexPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer*			pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Set_VoxelState(vIndexPos, eState);
}

HRESULT CVoxel_Manager::Get_VoxelState(const _float3& vWorldPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Get_VoxelState(vIndexPos, eState, eLayer);
}

HRESULT CVoxel_Manager::Set_VoxelState(const _float3& vWorldPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Set_VoxelState(vIndexPos, eState, eLayer);
}

HRESULT CVoxel_Manager::Get_VoxelState(const _uint iVoxelIndex, VOXEL_STATE& eState, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Get_VoxelState(vIndexPos, eState, eLayer);
}

HRESULT CVoxel_Manager::Set_VoxelState(const _uint iVoxelIndex, VOXEL_STATE eState, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Set_VoxelState(vIndexPos, eState, eLayer);
}

HRESULT CVoxel_Manager::Get_VoxelOpenessScore(const _uint3& vIndexPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer*			pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Get_VoxelOpenessScore(vIndexPos, bOpenessScore);
}

HRESULT CVoxel_Manager::Get_VoxelOpenessScore(const _float3& vWorldPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_WorldPos(vWorldPos, vIndexPos)))
		return E_FAIL;

	return Get_VoxelOpenessScore(vIndexPos, bOpenessScore, eLayer);
}

HRESULT CVoxel_Manager::Get_VoxelOpenessScore(const _uint iVoxelIndex, _byte& bOpenessScore, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iVoxelIndex, vIndexPos)))
		return E_FAIL;

	return Get_VoxelOpenessScore(vIndexPos, bOpenessScore, eLayer);
}

HRESULT CVoxel_Manager::Get_NeighborFlag(const _uint3& vIndexPos, _uint& iNeighborFlag, const VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer*			pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return E_FAIL;

	return pVoxelSectorLayer->Get_NeighborFlag(vIndexPos, iNeighborFlag);
}

HRESULT CVoxel_Manager::Get_NeighborFlag(const _uint iIndex, _uint& iNeighborFlag, const VOXEL_LAYER eLayer)
{
	_uint3			vIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iIndex, vIndexPos)))
		return E_FAIL;

	return Get_NeighborFlag(vIndexPos, iNeighborFlag, eLayer);
}

HRESULT CVoxel_Manager::Cooking_Terrain_Static_Voxels(CVIBuffer_Terrain* pTerrainBuffer, CTransform* pTerrainTransform)
{
	if (nullptr == pTerrainBuffer ||
		nullptr == pTerrainTransform)
		return E_FAIL;

	if (FAILED(Resizing_Voxel(m_fTempVoxelSize, m_fTempMaxCookingHeight)))
		return E_FAIL;

	return m_VoxelLayers[static_cast<_uint>(VOXEL_LAYER::_STATIC)]->Cooking_Terrain_Static_Voxels(pTerrainBuffer, pTerrainTransform);
}

HRESULT CVoxel_Manager::Cooking_Model_Static_Voxels(CModel* pModel, CTransform* pModelTransform)
{
	if (nullptr == pModel ||
		nullptr == pModelTransform)
		return E_FAIL;

	if (FAILED(Resizing_Voxel(m_fTempVoxelSize, m_fTempMaxCookingHeight)))
		return E_FAIL;

	return m_VoxelLayers[static_cast<_uint>(VOXEL_LAYER::_STATIC)]->Cooking_Model_Static_Voxels(pModel, pModelTransform);
}

HRESULT CVoxel_Manager::Resizing_Voxel(_float fVoxelSize, _float fMaxHeight)
{
	m_fVoxelSize = max(fVoxelSize, g_fVoxelSizeMinLimit);
	m_fVoxelSize = min(fVoxelSize, 64.f);
	m_fMaxCookingHeight = fMaxHeight;

	m_iNumVoxelWorldDim = static_cast<_uint>(min(g_iWorldLength,
		static_cast<_uint>(static_cast<_float>(g_iWorldLength) / m_fVoxelSize)));
	m_iNumVoxelSectorDim = static_cast<_uint>(static_cast<_float>(g_iSectorLength) / m_fVoxelSize);

	return S_OK;
}

HRESULT CVoxel_Manager::Set_TempVoxelSize(const _float fVoxelSize)
{
	if (fVoxelSize < g_fVoxelSizeMinLimit ||
		fVoxelSize > static_cast<_float>(g_iSectorLength) * 0.5f)
		return E_FAIL;

	m_fTempVoxelSize = fVoxelSize;

	return S_OK;
}

HRESULT CVoxel_Manager::Get_TempVoxelSize(_float& fVoxelSize)
{
	fVoxelSize = m_fTempVoxelSize;
	return S_OK;
}

HRESULT CVoxel_Manager::Get_VoxelSize(_float& fVoxelSize)
{
	fVoxelSize = m_fVoxelSize;
	return S_OK;
}

HRESULT CVoxel_Manager::Set_TempCookingMaxHeight(const _float fMaxHeight)
{
	if (fMaxHeight < 1.f)
		return E_FAIL;

	m_fTempMaxCookingHeight = fMaxHeight;
	return S_OK;
}

HRESULT CVoxel_Manager::Get_TempCookingMaxHeight(_float& fMaxHeight)
{
	fMaxHeight = m_fTempMaxCookingHeight;
	return S_OK;
}

HRESULT CVoxel_Manager::Get_CookingMaxHeight(_float& fMaxHeight)
{
	fMaxHeight = m_fMaxCookingHeight;
	return S_OK;
}

_bool CVoxel_Manager::Is_Reach_TargetIndex(const _uint iStartIndex, const _uint iTargetIndex, VOXEL_LAYER eLayer)
{
	CVoxel_SectorLayer* pVoxelSectorLayer = { Get_Layer(eLayer) };
	if (nullptr == pVoxelSectorLayer)
		return false;

	_uint3				vStartIndexPos, vTargetIndexPos;
	if (FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iStartIndex, vStartIndexPos)) ||
		FAILED(m_pVoxelConvertor->Convert_IndexPos_From_Index(iTargetIndex, vTargetIndexPos)))
		return false;

	return pVoxelSectorLayer->Is_Reach_TargetIndex(iStartIndex, vStartIndexPos, iTargetIndex, vTargetIndexPos);
}

HRESULT CVoxel_Manager::Save_Data(const _uint iSaveSlot)
{
	string			strDefualtPass = { "../../VoxelBin/Data" };
	string			strSaveSlot = { to_string(iSaveSlot) };
	string			strExp = { ".bin" };

	string			strPass = { strDefualtPass + strSaveSlot + strExp };

	CVoxel_Parser::VOXEL_SAVE_DESC			VoxelSaveDesc;

	CVoxel_SectorLayer::VOXEL_SECTOR_LAYER_DESC		VoxelSectorDesc;
	VoxelSectorDesc.pNumSectorWorldDim = &m_iNumSectorWorldDim;
	VoxelSectorDesc.pNumVoxelSectorDim = &m_iNumVoxelSectorDim;
	VoxelSectorDesc.pNumVoxelWorldDim = &m_iNumVoxelWorldDim;
	VoxelSectorDesc.pTempVoxelSize = &m_fTempVoxelSize;
	VoxelSectorDesc.pTransform = m_pTransformCom;
	VoxelSectorDesc.pVoxelConvertor = m_pVoxelConvertor;
	VoxelSectorDesc.pVoxelScorer = m_pVoxelScorer;
	VoxelSectorDesc.pVoxelSize = &m_fVoxelSize;

	VoxelSaveDesc.pVoxelSectorLayerDesc = &VoxelSectorDesc;

	return m_pVoxelParser->Save_Data(strPass, m_VoxelLayers, VoxelSaveDesc);
}

HRESULT CVoxel_Manager::Load_Data(const _uint iSaveSlot)
{
	string			strDefualtPass = { "../../VoxelBin/Data" };
	string			strSaveSlot = { to_string(iSaveSlot) };
	string			strExp = { ".bin" };

	string			strPass = { strDefualtPass + strSaveSlot + strExp };

	CVoxel_Parser::VOXEL_SAVE_DESC		VoxelSaveDesc;

	CVoxel_SectorLayer::VOXEL_SECTOR_LAYER_DESC		VoxelSectorDesc;
	VoxelSectorDesc.pNumSectorWorldDim = &m_iNumSectorWorldDim;
	VoxelSectorDesc.pNumVoxelSectorDim = &m_iNumVoxelSectorDim;
	VoxelSectorDesc.pNumVoxelWorldDim = &m_iNumVoxelWorldDim;
	VoxelSectorDesc.pTempVoxelSize = &m_fTempVoxelSize;
	VoxelSectorDesc.pTransform = m_pTransformCom;
	VoxelSectorDesc.pVoxelConvertor = m_pVoxelConvertor;
	VoxelSectorDesc.pVoxelScorer = m_pVoxelScorer;
	VoxelSectorDesc.pVoxelPicker = m_pVoxelPicker;
	VoxelSectorDesc.pVoxelSize = &m_fVoxelSize;

	VoxelSaveDesc.pVoxelSectorLayerDesc = &VoxelSectorDesc;

	if (FAILED(m_pVoxelParser->Load_Data(strPass, m_VoxelLayers, VoxelSaveDesc)))
		return E_FAIL;


	Resizing_Voxel(m_fVoxelSize);

	return S_OK;
}

HRESULT CVoxel_Manager::Compute_Picking_Voxel(_uint& iPickIndex, const VOXEL_LAYER eLayer)
{
	auto pLayer{ Get_Layer(eLayer) };
	if (nullptr == pLayer)
		return E_FAIL;

	return pLayer->Compute_Picking(iPickIndex);
}

_uint CVoxel_Manager::Get_CurPicked_VoxelIndex(const VOXEL_LAYER eLayer)
{
	auto pLayer{ Get_Layer(eLayer) };
	if (nullptr == pLayer)
		return 0;

	return pLayer->Get_CurPickedIndex();
}

_uint CVoxel_Manager::Get_PrePicked_VoxelIndex()
{
	return m_pVoxelPicker->Get_PrePickIndex();
}

_float3 CVoxel_Manager::Get_CurPicked_VoxelWorldPos(const VOXEL_LAYER eLayer)
{
	auto pLayer{ Get_Layer(eLayer) };
	if (nullptr == pLayer)
		return _float3();

	return pLayer->Get_CurPickedWorldPos();
}

_float3 CVoxel_Manager::Get_PrePicked_VoxelWorldPos()
{
	return m_pVoxelPicker->Get_PrePickWorldPos();
}

HRESULT CVoxel_Manager::Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint iVoxelIndex, _float3& vNearestPosFloat3)
{
	_float3			vVoxelWorldPosFloat3;
	if (FAILED(m_pVoxelConvertor->Convert_WorldPos_From_Index(iVoxelIndex, vVoxelWorldPosFloat3)))
		return E_FAIL;

	return Compute_Nearest_Position_VoxelBounding(vStartPos, vVoxelWorldPosFloat3, vNearestPosFloat3);
}

HRESULT CVoxel_Manager::Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint3& vVoxelIndexPos, _float3& vNearestPosFloat3)
{
	_float3			vVoxelWorldPosFloat3;
	if (FAILED(m_pVoxelConvertor->Convert_WorldPos_From_IndexPos(vVoxelIndexPos, vVoxelWorldPosFloat3)))
		return E_FAIL;

	return Compute_Nearest_Position_VoxelBounding(vStartPos, vVoxelWorldPosFloat3, vNearestPosFloat3);
}

HRESULT CVoxel_Manager::Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _float3& vVoxelWorldPos, _float3& vNearestPosFloat3)
{
	const _float3			vLBB = {
	vVoxelWorldPos.x - m_fVoxelSize * 0.5f,
	vVoxelWorldPos.y - m_fVoxelSize * 0.5f,
	vVoxelWorldPos.z - m_fVoxelSize * 0.5f
	};

	const _float3			vRTF = {
		vVoxelWorldPos.x + m_fVoxelSize * 0.5f,
		vVoxelWorldPos.y + m_fVoxelSize * 0.5f,
		vVoxelWorldPos.z + m_fVoxelSize * 0.5f
	};

	vNearestPosFloat3 = move(_float3{
		clamp(vStartPos.x, vLBB.x, vRTF.x),
		clamp(vStartPos.y, vLBB.y, vRTF.y),
		clamp(vStartPos.z, vLBB.z, vRTF.z)
		});

	return S_OK;
}


CVoxel_Manager* CVoxel_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVoxel_Manager* pInstance = { new CVoxel_Manager{pDevice, pContext } };

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxel_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	m_pGameInstance = nullptr;

	Safe_Release(m_pVoxelDebugger);
	Safe_Release(m_pVoxelPicker);
	Safe_Release(m_pVoxelScorer);
	Safe_Release(m_pVoxelParser);
	Safe_Release(m_pVoxelConvertor);

	Safe_Release(m_pTransformCom);

	for (auto& pSectorLayer : m_VoxelLayers)
	{
		Safe_Release(pSectorLayer);
		pSectorLayer = nullptr;
	}
	m_VoxelLayers.clear();
}
