#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Manager final : public CBase
{
private:
	CVoxel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_Manager() = default;

public:
	HRESULT Initialize();
	void Priority_Tick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	HRESULT Render();		//	현재 카메라 좌표로부터 인접한 복셀 그리드들을 혼합하여 ( 그리드와 카메라 절두체간의 충돌확인)

private:
	void Update_Cur_Active_Sectors();

public:
	HRESULT Add_Voxel(const _uint3& vIndexPos, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Add_Voxel(const _float3& vWorldPos, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Erase_Voxel(const _uint3& vIndexPos);
	HRESULT Erase_Voxel(const _float3& vWorldPos);

	HRESULT Add_Voxel_NonUpdateOpenessScore(const _uint3& vIndexPos, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Add_Voxel_NonUpdateOpenessScore(const _float3& vWorldPos, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Erase_Voxel_NonUpdateOpenessScore(const _uint3& vIndexPos);
	HRESULT Erase_Voxel_NonUpdateOpenessScore(const _float3& vWorldPos);

private:
	HRESULT Add_Sector(const _uint iSectorIndex);
	HRESULT Add_Sector(const _uint3& vSectorIndexPos);
	HRESULT Erase_Sector(const _uint iSectorIndex);
	HRESULT Erase_Sector(const _uint3& vSectorIndexPos);

	_bool Is_Exist_Sector(const _uint iSectorIndex);
	_bool Is_Exist_Sector(const _uint3& vSectorIndexPos);

public:
	_bool Is_Exist_Voxel(const _uint iVoxelIndex);
	_bool Is_Exist_Voxel(const _uint3& vIndexPos);

private:
	HRESULT SetUp_Neighbor(const _uint3& vIndexPos);

public:
	HRESULT SetUp_Terrain(class CVIBuffer_Terrain* pTerrainBuffer, class CTransform* pTransform);
	HRESULT Resizing(_float fVoxelSize, _float fMaxHeight = g_iTerrainMaxHeight);

	HRESULT Set_TempVoxelSize(const _float fVoxelSize);
	HRESULT Get_TempVoxelSize(_float& fVoxelSize);
	HRESULT Get_VoxelSize(_float& fVoxelSize);
	HRESULT Set_TempCookingMaxHeight(const _float fMaxHeight);
	HRESULT Get_TempCookingMaxHeight(_float& fMaxHeight);
	HRESULT Get_CookingMaxHeight(_float& fMaxHeight);

	HRESULT Cooking();
	HRESULT Cooking_Model_StaticVoxels(class CModel* pModel, class CTransform* pModelTransform);

	HRESULT Convert_Wall_BottomNeighbors();

	HRESULT Save_Data(const _uint iSaveSlot);
	HRESULT Load_Data(const _uint iSaveSlot);

private:
	HRESULT Update_OpenessScores_All();
	HRESULT Update_OpenessScores_NearArea(const _uint3& vIndexPos);
	HRESULT Compute_OpenessSocre(const _uint iVoxelIndex, _float& fOpeneesScore);

public:
	_uint Get_NumVoxelWorldDim();

	vector<_uint> Get_CurrentActive_SectorIndices();
	vector<_uint3> Get_CurrentActive_SectorIndexPoses();

	_uint Get_NumVoxel_Total();
	_uint Get_NumVoxel_Active();
	_uint Get_NumSector_Total();
	_uint Get_NumSector_Active();
	_float Get_VoxelWorldSize();

public:
	vector<_uint> Get_NeighborIndices(_uint iIndex);
	vector<_uint> Get_NeighborIndices(const _float3& vPos);

	HRESULT Get_Index(const _float3& vPos, _uint& iIndex);
	HRESULT Get_Index(const _uint3& vIndexPos, _uint& iIndex);
	HRESULT Get_WorldPosition(const _uint iIndex, _float3& vPos);
	HRESULT Get_IndexPosition(const _uint iIndex, _uint3& vIndexPos);


public:
	HRESULT Get_VoxelID(const _float3& vWorldPos, VOXEL_ID& eID);
	HRESULT Set_VoxelID(const _float3& vWorldPos, VOXEL_ID eID);
	HRESULT Get_VoxelID(const _uint3& vIndexPos, VOXEL_ID& eID);
	HRESULT Set_VoxelID(const _uint3& vIndexPos, VOXEL_ID eID);
	HRESULT Get_VoxelID(const _uint iVoxelIndex, VOXEL_ID& eID);
	HRESULT Set_VoxelID(const _uint iVoxelIndex, VOXEL_ID eID);

	HRESULT Get_VoxelState(const _float3& vWorldPos, VOXEL_STATE& eState);
	HRESULT Set_VoxelState(const _float3& vWorldPos, VOXEL_STATE eState);
	HRESULT Get_VoxelState(const _uint3& vIndexPos, VOXEL_STATE& eState);
	HRESULT Set_VoxelState(const _uint3& vIndexPos, VOXEL_STATE eState);
	HRESULT Get_VoxelState(const _uint iVoxelIndex, VOXEL_STATE& eState);
	HRESULT Set_VoxelState(const _uint iVoxelIndex, VOXEL_STATE eState);

	HRESULT Get_VoxelOpenessScore(const _float3& vWorldPos, _byte& bOpenessScore);
	HRESULT Get_VoxelOpenessScore(const _uint3& vIndexPos, _byte& bOpenessScore);
	HRESULT Get_VoxelOpenessScore(const _uint iVoxelIndex, _byte& bOpenessScore);

	HRESULT Get_NeighborFlag(const _uint3& vPos, _uint& iNeighborFlag);
	HRESULT Get_NeighborFlag(const _uint iIndex, _uint& iNeighborFlag);

	HRESULT Add_Neighbor(const _uint3& vPos, const _uint iNeighborFlag);
	HRESULT Add_Neighbor(const _uint iIndex, const _uint iNeighborFlag);

	HRESULT Erase_Neighbor(const _uint3& vPos, const _uint iNeighborFlag);
	HRESULT Erase_Neighbor(const _uint iIndex, const _uint iNeighborFlag);

public:
	_uint Get_CurPickedIndex();
	_uint Get_PrePickedIndex();
	_float3 Get_CurPickedWorldPos();
	_float3 Get_PrePickedWorldPos();

public:
	void Set_Render(_bool isRender);

private:
	HRESULT Add_Supports();
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	HRESULT Cooking_TerrainBuffer();

private:
	void Clear_StaticSectors();
	void Clear_ObstcleSectors();

private:
	_bool Is_Culled_Sector(const _uint iSectorIndex);
	_bool Is_Culled_Frustum_Sector(_fvector vPos);

private:	
	HRESULT Convert_WorldPos_From_IndexPos(const _uint3& vIndexPos, _float3& vWorldPos);
	HRESULT Convert_IndexPos_From_WorldPos(const _float3& vWorldPos, _uint3& vIndexPos);
	HRESULT Convert_VoxelIndex_From_IndexPos(const _uint3& vIndexPos, _uint& iIndex);
	HRESULT Convert_VoxelIndex_From_WorldPos(const _float3& vWorldPos, _uint& iIndex);
	HRESULT Convert_SectorIndex_From_IndexPos(const _uint3& vIndexPos, _uint& iIndex);
	HRESULT Convert_SectorIndex_From_WorldPos(const _float3& vWorldPos, _uint& iIndex);
	HRESULT Convert_SectorIndex_From_VoxelIndex(const _uint iVoxelIndex, _uint& iSectorIndex);
	HRESULT Convert_IndexPos_From_Index(const _uint iIndex, _uint3& vIndexPos);
	HRESULT Convert_WorldPos_From_Index(const _uint iIndex, _float3& vWorldPos);

private:
	HRESULT Erase_Neighbors(const _uint iVoxelIndex);
	HRESULT Erase_Neighbors(const _uint3& vIndexPos);
	HRESULT Erase_Neighbors(const _float3& vWorldPos);

private:
	//	완전히 월드화된 Float3
	_bool Is_Out_Of_Range_WorldPos(const _float3& vWorldPos);
	//	IndexPos기반의 Float3
	_bool Is_Out_Of_Range_LocalPos(const _float3& vLocalPos);		

	_bool Is_Out_Of_Range_Voxel(const _uint3& vIndexPos);
	_bool Is_Out_Of_Range_Voxel(const _uint iIndex);

	_bool Is_Out_Of_Range_Sector(const _uint3& vIndexPos);
	_bool Is_Out_Of_Range_Sector(const _uint iIndex);

public:
	HRESULT Compute_Picking(_uint& iPickIndex);
	_bool Is_Reach_TargetIndex_IgnoreWall(const _uint iStartIndex, const _uint iTargetIndex);

private:
	_bool Is_Culled(const _uint iVoxelIndex);


private:
	void Clear_Instance_Buffers();
	void Bind_Instance_Buffers(const _uint iCurVIBufferIndex);
	void Render_Current_Instance_Buffers(const _uint iCurVIBufferIndex);
	

private:
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*						m_pContext = { nullptr };
	class CGameInstance*						m_pGameInstance = { nullptr };

private:
	class CVoxel_Debugger*						m_pVoxelDebugger = { nullptr };
	class CVoxel_Picker*						m_pVoxelPicker = { nullptr };
	class CVoxel_Scorer*						m_pVoxelScorer = { nullptr };
	class CVoxel_Parser*						m_pVoxelParser = { nullptr };

private:
	class CVIBuffer_Cube*						m_pVIBufferCube = { nullptr };
	vector<class CVIBuffer_Instance_Cube*>		m_VIBufferInstanceCubes = { nullptr };
	class CShader*								m_pShaderCube = { nullptr };
	class CShader*								m_pShaderInstanceCube = { nullptr };
	class CTransform*							m_pTransformCom = { nullptr };
	class CVIBuffer_Terrain*					m_pTerrainBuffer = { nullptr };
	class CTransform*							m_pTerrainTransform = { nullptr };
	class CVoxelRenderCube*						m_pRenderCube = { nullptr };

	unordered_map<_uint, class CVoxel_Sector*>	m_StaticSectors;
	unordered_map<_uint, class CVoxel_Sector*>	m_ObstacleSectors;

	_bool										m_isRender = { false };

	unordered_set<_uint>						m_ActiveSectors;

private:
	_uint										m_iNumVoxelWorldDim = {};
	_uint										m_iNumVoxelSectorDim = {};
	_uint										m_iNumSectorWorldDim = {};
	_float										m_fTempVoxelSize = {};
	_float										m_fVoxelSize = {};

	_float										m_fTempMaxCookingHeight = { static_cast<_float>(10.f) };
	_float										m_fMaxCookingHeight = { static_cast<_float>(10.f)  };

	wstring										m_strHeightMapPass = { TEXT("") };

	vector<_float3>								m_Instance_Position_Buffer;
	vector<_byte>								m_Instance_State_Buffer;
	vector<_byte>								m_Instance_ID_Buffer;

public:
	static CVoxel_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END