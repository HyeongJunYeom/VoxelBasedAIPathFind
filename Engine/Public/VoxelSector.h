#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Sector : public CBase
{
public:
	typedef struct tagVoxelSectorDesc
	{
		const _float*						pVoxelSize = { nullptr };
	}VOXEL_SECTOR_DESC;

private:
	CVoxel_Sector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVoxel_Sector(const CVoxel_Sector& rhs) = delete;
	virtual ~CVoxel_Sector() = default;

public:
	HRESULT Initialize(void* pArg);
	HRESULT Render();

	HRESULT Add_Voxel(const _uint3& vPos, VOXEL_ID eID, VOXEL_STATE eState = VOXEL_STATE::_DEFAULT);
	HRESULT Insert_Voxel(_uint iIndex, VOXEL& Voxel);

	HRESULT Erase_Voxel(const _uint3& vPos);
	HRESULT Clear();

	_bool	Is_Empty();
	_bool	Is_Exist(const _uint iIndex);
	_bool	Is_Exist(const _uint3& vIndexPos);
	
public:
	HRESULT Get_ID(const _uint3& vPos, VOXEL_ID& eID);
	HRESULT Set_ID(const _uint3& vPos, VOXEL_ID eID);
	HRESULT Get_ID(const _uint iIndex, VOXEL_ID& eID);
	HRESULT Set_ID(const _uint iIndex, VOXEL_ID eID);

	HRESULT Get_State(const _uint3& vPos, VOXEL_STATE& eState);
	HRESULT Set_State(const _uint3& vPos, VOXEL_STATE eState);
	HRESULT Get_State(const _uint iIndex, VOXEL_STATE& eState);
	HRESULT Set_State(const _uint iIndex, VOXEL_STATE eState);

	HRESULT Get_OpenessScore(const _uint3 vPos, _byte& bOpenessScore);
	HRESULT Set_OpenessScore(const _uint3 vPos, const _byte bOpenessScore);
	HRESULT Get_OpenessScore(const _uint iIndex, _byte& bOpenessScore);
	HRESULT Set_OpenessScore(const _uint iIndex, const _byte bOpenessScore);

	HRESULT Get_NeighborFlag(const _uint3& vIndexPos, _uint& iNeighborFlag);
	HRESULT Get_NeighborFlag(const _uint iIndex, _uint& iNeighborFlag);

	HRESULT Add_Neighbor(const _uint3& vIndexPos, const _uint iNeighborFlag);
	HRESULT Add_Neighbor(const _uint iIndex, const _uint iNeighborFlag);

	HRESULT Erase_Neighbor(const _uint3& vIndexPos, const _uint iNeighborFlag);
	HRESULT Erase_Neighbor(const _uint iIndex, const _uint iNeighborFlag);

	HRESULT Get_IndexSectorLocal(const _uint3& vPos, _uint& iIndex);
	HRESULT Get_Position_Local(_uint iIndex, _uint3& vPos);

	vector<_uint> Get_VoxelIndices();

	_uint Get_NumVoxel();
	vector<_uint>	Get_Indices();

public:
	HRESULT Bind_InstanceBuffers(vector<_uint>& IndexBuffer, vector<_byte>& StateBuffer, vector<_byte>& IDBuffer, vector<_int>& NeighborBuffer);

private:
	//	HRESULT Bind_ShaderResources(_uint3 vPos);

public:
	_bool Is_Culled(const _uint3& vPos);

private:
	_bool Is_Culled_Frustum(_fvector vPos);
	_bool Is_Culled_ViewDist(_fvector vPos);

private:
	unordered_map<_uint, VOXEL>::iterator Get_Iter(const _uint3& iPos);
	unordered_map<_uint, VOXEL>::iterator Get_Iter(const _uint& iIndex);

private:
	//	vPos => 3차원 배열상의 인덱스 값 [x][y][z] 
	HRESULT Encode_UInt3(const _uint3& vPos, _uint& iIndexSector);
	//	iIndex => 1차원 배열상의 인덱스 값으로 치환
	HRESULT Decode_UInt3(const _uint iIndexSector, _uint3& vPos);

private:
	unordered_map<_uint, VOXEL>					m_Voxels;
	const _float*								m_pVoxelSize = {};

	class CGameInstance*						m_pGameInstance = { nullptr };

	/* for. Render */
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*						m_pContext = { nullptr };

public:
	static void Set_CullDist(_float fDist);

private:
	static _float								ms_fCullDist;

public:
	static CVoxel_Sector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END