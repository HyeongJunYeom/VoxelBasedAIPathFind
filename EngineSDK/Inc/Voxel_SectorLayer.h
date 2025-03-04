#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_SectorLayer final : public CBase
{
public:
	typedef struct tagVoxelSectorLayerDesc
	{
		_uint*			pNumVoxelWorldDim = { nullptr };
		_uint*			pNumVoxelSectorDim = { nullptr };
		_uint*			pNumSectorWorldDim = { nullptr };
		_float*			pTempVoxelSize = { nullptr };
		_float*			pVoxelSize = { nullptr };

		class CTransform*				pTransform = { nullptr };
		class CVoxel_Convertor*			pVoxelConvertor = { nullptr };
		class CVoxel_Scorer*			pVoxelScorer = { nullptr };
		class CVoxel_Picker*			pVoxelPicker = { nullptr };

	}VOXEL_SECTOR_LAYER_DESC;

private:
	CVoxel_SectorLayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_SectorLayer() = default;

public:
	HRESULT Initialize(void* pArg);
	void Priority_Tick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	HRESULT Render(const unordered_set<_uint>& ActiveSectorIndices);		//	현재 카메라 좌표로부터 인접한 복셀 그리드들을 혼합하여 ( 그리드와 카메라 절두체간의 충돌확인)

public:
	HRESULT Add_Voxel(const _uint3& vIndexPos, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Erase_Voxel(const _uint3& vIndexPos);
	_bool Is_Exist_Voxel(const _uint3& vIndexPos);

private:
	class CVoxel_Sector* Get_Sector(const _uint iSectorIndex);

private:
	HRESULT Add_Sector(const _uint iSectorIndex);
	HRESULT Add_Sector(const _uint3& vSectorIndexPos);
	HRESULT Erase_Sector(const _uint iSectorIndex);
	HRESULT Erase_Sector(const _uint3& vSectorIndexPos);
	_bool Is_Exist_Sector(const _uint iSectorIndex);
	_bool Is_Exist_Sector(const _uint3& vSectorIndexPos);

private:
	HRESULT SetUp_Neighbor(const _uint3& vIndexPos);

public:
	HRESULT Cooking_Terrain_Static_Voxels(class CVIBuffer_Terrain* pTerrainBuffer, class CTransform* pTerrainTransform);
	HRESULT Cooking_Model_Static_Voxels(class CModel* pModel, class CTransform* pModelTransform);

	HRESULT Convert_Wall_BottomNeighbors();

private:
	HRESULT Update_OpenessScores_All();
	HRESULT Update_OpenessScores_NearArea(const _uint3& vIndexPos);
	HRESULT Compute_OpenessScore(const _uint iVoxelIndex, _float& fOpeneesScore);

public:
	vector<_uint> Get_CurrentActive_SectorIndices(const unordered_set<_uint>& ActiveSectorIndicies);
	vector<_uint3> Get_CurrentActive_SectorIndexPoses(const unordered_set<_uint>& ActiveSectorIndicies);

	_uint Get_NumVoxel_Active(const unordered_set<_uint>& ActiveSectorIndices);
	_uint Get_NumVoxel_Total();
	_uint Get_NumSector_Total();
	_float Get_WorldSize_Voxel();

public:
	vector<_uint> Get_NeighborIndices(_uint iIndex);
	vector<_uint> Get_NeighborIndices_All(_uint iIndex);

public:
	HRESULT Get_VoxelID(const _uint3& vIndexPos, VOXEL_ID& eID);
	HRESULT Set_VoxelID(const _uint3& vIndexPos, VOXEL_ID eID);

	HRESULT Get_VoxelState(const _uint3& vIndexPos, VOXEL_STATE& eState);
	HRESULT Set_VoxelState(const _uint3& vIndexPos, VOXEL_STATE eState);

	HRESULT Get_VoxelOpenessScore(const _uint3& vIndexPos, _byte& bOpenessScore);

	HRESULT Get_NeighborFlag(const _uint3& vIndexPos, _uint& iNeighborFlag);


	HRESULT Add_Neighbor(const _uint3& vPos, const _uint iNeighborFlag);
	HRESULT Add_Neighbor(const _uint iIndex, const _uint iNeighborFlag);

	HRESULT Erase_Neighbor(const _uint3& vPos, const _uint iNeighborFlag);
	HRESULT Erase_Neighbor(const _uint iIndex, const _uint iNeighborFlag);

public:
	_uint Get_CurPickedIndex();
	_uint Get_PrePickedIndex();
	_float3 Get_CurPickedWorldPos();
	_float3 Get_PrePickedWorldPos();

private:
	HRESULT Add_Supports();
	HRESULT Add_Components();
	HRESULT Cooking_TerrainBuffer(CVIBuffer_Terrain * pTerrainBuffer, CTransform* pTerrainTransform);

private:
	void Clear_Sectors();

private:
	_bool Is_Culled_Sector(const _uint iSectorIndex);
	_bool Is_Culled_Frustum_Sector(_fvector vPos);

private:
	HRESULT Erase_Neighbors(const _uint iVoxelIndex);
	HRESULT Erase_Neighbors(const _uint3& vIndexPos);
	HRESULT Erase_Neighbors(const _float3& vWorldPos);

public:
	HRESULT Compute_Picking(_uint& iPickIndex);
	_bool Is_Reach_TargetIndex(const _uint iStartIndex, const _uint3& vStartIndexPosUInt3, const _uint iTargetIndex, const _uint3& vTargetIndexPosUInt3);

public:
	unordered_map<_uint, class CVoxel_Sector*>& Get_Sectors() { return m_Sectors; }

private:
	void Clear_Instance_Buffers();
	void Bind_Instance_Buffers(const _uint iCurVIBufferIndex);
	void Render_Current_Instance_Buffers(const _uint iCurVIBufferIndex);
	

private:
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*						m_pContext = { nullptr };
	class CGameInstance*						m_pGameInstance = { nullptr };

	class CTransform*							m_pTransform = { nullptr };
	class CVoxel_Convertor*						m_pVoxelConvertor = { nullptr };
	class CVoxel_Scorer*						m_pVoxelScorer = { nullptr };
	class CVoxel_Picker*						m_pVoxelPicker = { nullptr };

private:
	vector<class CVIBuffer_Instance_Cube*>		m_VIBufferInstanceCubes = { nullptr };
	class CShader*								m_pShaderInstanceCube = { nullptr };

	unordered_map<_uint, class CVoxel_Sector*>	m_Sectors;

private:
	const _uint*								m_pNumVoxelWorldDim = {};
	const _uint*								m_pNumVoxelSectorDim = {};
	const _uint*								m_pNumSectorWorldDim = {};
	const _float*								m_pTempVoxelSize = {};
	const _float*								m_pVoxelSize = {};

	vector<_float3>								m_Instance_Position_Buffer;
	vector<_byte>								m_Instance_State_Buffer;
	vector<_byte>								m_Instance_ID_Buffer;
	vector<_int>								m_Instance_Neighbor_Buffer;

public:
	static CVoxel_SectorLayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END