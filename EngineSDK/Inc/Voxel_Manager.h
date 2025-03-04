#pragma once
#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Manager final : public CBase
{
public:
	CVoxel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVoxel_Manager(const CVoxel_Manager& rhs) = delete;
	~CVoxel_Manager() = default;

public:
	HRESULT Initialize();
	void Priority_Tick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	HRESULT Render();

private:
	void Update_Cur_Active_Sectors();

private:
	HRESULT Add_Supports();
	HRESULT Add_Components();
	HRESULT Add_Layers();

	class CVoxel_SectorLayer* Get_Layer(const VOXEL_LAYER eLayer);

public:
	_uint Get_NumVoxel_Active_In_Layer(const VOXEL_LAYER eLayer);
	_uint Get_NumVoxel_Active_All_Layer();
	_uint Get_NumVoxel_In_Layer(const VOXEL_LAYER eLayer);
	_uint Get_NumVoxel_All_Layer();
	_uint Get_NumSector(const VOXEL_LAYER eLayer);
	_uint Get_NumSector_Active();

	_uint Get_NumVoxelWorldDim(); 
	_float Get_WorldSize_Voxel();

	vector<_uint> Get_CurrentActive_SectorIndices(const VOXEL_LAYER eLayer);
	vector<_uint3> Get_CurrentActive_SectorIndexPoses(const VOXEL_LAYER eLayer);

public:
	HRESULT Add_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Add_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Add_Voxel(const _float3& vWorldPos, const VOXEL_LAYER eLayer, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Erase_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer);
	HRESULT Erase_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer);
	HRESULT Erase_Voxel(const _float3& vWorldPos, const VOXEL_LAYER eLayer);
	_bool Is_Exist_Voxel(const _uint iVoxelIndex, const VOXEL_LAYER eLayer);
	_bool Is_Exist_Voxel(const _uint3& vIndexPos, const VOXEL_LAYER eLayer);

public:
	vector<_uint> Get_NeighborIndices(const _uint iIndex, VOXEL_LAYER eLayer);
	vector<_uint> Get_NeighborIndices(const _float3& vPos, VOXEL_LAYER eLayer);
	vector<_uint> Get_NeighborIndices_All(const _uint iIndex, VOXEL_LAYER eLayer);
	vector<_uint> Get_NeighborIndices_All(const _float3& vPos, VOXEL_LAYER eLayer);

	HRESULT Get_Index(const _float3& vPos, _uint& iIndex);
	HRESULT Get_Index(const _uint3& vIndexPos, _uint& iIndex);
	HRESULT Get_WorldPosition(const _uint iIndex, _float3& vPos);
	HRESULT Get_WorldPosition(const _uint3& vIndexPos, _float3& vPos);
	HRESULT Get_IndexPosition(const _uint iIndex, _uint3& vIndexPos);
	HRESULT Get_IndexPosition(const _float3& vPos, _uint3& vIndexPos);

public:
	void Set_Render(const VOXEL_LAYER eLayer, _bool isRender);
	void Set_Render_All(_bool isRender);

public:
	HRESULT Get_VoxelID(const _uint3& vIndexPos, VOXEL_ID& eID, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelID(const _uint3& vIndexPos, VOXEL_ID eID, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelID(const _float3& vWorldPos, VOXEL_ID& eID, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelID(const _float3& vWorldPos, VOXEL_ID eID, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelID(const _uint iVoxelIndex, VOXEL_ID& eID, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelID(const _uint iVoxelIndex, VOXEL_ID eID, const VOXEL_LAYER eLayer);

	HRESULT Get_VoxelState(const _uint3& vIndexPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelState(const _uint3& vIndexPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelState(const _float3& vWorldPos, VOXEL_STATE& eState, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelState(const _float3& vWorldPos, VOXEL_STATE eState, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelState(const _uint iVoxelIndex, VOXEL_STATE& eState, const VOXEL_LAYER eLayer);
	HRESULT Set_VoxelState(const _uint iVoxelIndex, VOXEL_STATE eState, const VOXEL_LAYER eLayer);

	HRESULT Get_VoxelOpenessScore(const _float3& vWorldPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelOpenessScore(const _uint3& vIndexPos, _byte& bOpenessScore, const VOXEL_LAYER eLayer);
	HRESULT Get_VoxelOpenessScore(const _uint iVoxelIndex, _byte& bOpenessScore, const VOXEL_LAYER eLayer);

	HRESULT Get_NeighborFlag(const _uint3& vIndexPos, _uint& iNeighborFlag, const VOXEL_LAYER eLayer);
	HRESULT Get_NeighborFlag(const _uint iIndex, _uint& iNeighborFlag, const VOXEL_LAYER eLayer);

public:
	HRESULT Cooking_Terrain_Static_Voxels(class CVIBuffer_Terrain* pTerrainBuffer, class CTransform* pTerrainTransform);
	HRESULT Cooking_Model_Static_Voxels(class CModel* pModel, class CTransform* pModelTransform);

public:
	HRESULT Resizing_Voxel(_float fVoxelSize, _float fMaxHeight = g_iTerrainMaxHeight);

	HRESULT Set_TempVoxelSize(const _float fVoxelSize);
	HRESULT Get_TempVoxelSize(_float& fVoxelSize);
	HRESULT Get_VoxelSize(_float& fVoxelSize);
	HRESULT Set_TempCookingMaxHeight(const _float fMaxHeight);
	HRESULT Get_TempCookingMaxHeight(_float& fMaxHeight);
	HRESULT Get_CookingMaxHeight(_float& fMaxHeight);

public:
	HRESULT Save_Data(const _uint iSaveSlot);
	HRESULT Load_Data(const _uint iSaveSlot);

public:
	HRESULT Compute_Picking_Voxel(_uint & iPickIndex, const VOXEL_LAYER eLayer);
	_bool Is_Reach_TargetIndex(const _uint iStartIndex, const _uint iTargetIndex, const VOXEL_LAYER eLayer);

	_uint Get_CurPicked_VoxelIndex(const VOXEL_LAYER eLayer);
	_uint Get_PrePicked_VoxelIndex();
	_float3 Get_CurPicked_VoxelWorldPos(const VOXEL_LAYER eLayer);
	_float3 Get_PrePicked_VoxelWorldPos();

public:
	HRESULT Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint iVoxelIndex, _float3& vNearestPosFloat3);
	HRESULT Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _uint3& vVoxelIndexPos, _float3& vNearestPosFloat3);
	HRESULT Compute_Nearest_Position_VoxelBounding(const _float3& vStartPos, const _float3& vVoxelWorldPos, _float3& vNearestPosFloat3);

private:
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*						m_pContext= { nullptr };
	class CGameInstance*						m_pGameInstance = { nullptr };

	class CTransform*							m_pTransformCom = { nullptr };

private:
	vector<class CVoxel_SectorLayer*>			m_VoxelLayers;
	vector<_bool>								m_RenderStates;

	unordered_set<_uint>						m_ActiveSectors;

private:
	class CVoxel_Debugger*						m_pVoxelDebugger = { nullptr };
	class CVoxel_Picker*						m_pVoxelPicker = { nullptr };
	class CVoxel_Scorer*						m_pVoxelScorer = { nullptr };
	class CVoxel_Parser*						m_pVoxelParser = { nullptr };
	class CVoxel_Convertor*						m_pVoxelConvertor = { nullptr };

private:
	_uint										m_iNumVoxelWorldDim = {};
	_uint										m_iNumVoxelSectorDim = {};
	_uint										m_iNumSectorWorldDim = {};
	_float										m_fTempVoxelSize = {};
	_float										m_fVoxelSize = {};

	/* For.Cooking Terrain */
	_float										m_fTempMaxCookingHeight = { static_cast<_float>(10.f) };
	_float										m_fMaxCookingHeight = { static_cast<_float>(10.f) };
	wstring										m_strHeightMapPass = { TEXT("") };

public:
	static CVoxel_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END