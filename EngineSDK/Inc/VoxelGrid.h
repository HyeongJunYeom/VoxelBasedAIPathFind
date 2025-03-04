#pragma once
#include "Base.h"

BEGIN(Engine)

const _float			g_fDefaultVoxelGridLength = { 128.f };
//	const _float			g_fDefaultVoxelGridLength = { 1024.f };
const _float			g_fDefaultVoxelLength = { 1.f };

typedef struct tagVoxel
{
	_byte bID = { -1 };
} VOXEL;

class CVoxelGrid : public CBase
{
private:
	CVoxelGrid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVoxelGrid(const CVoxelGrid& rhs) = delete;
	virtual ~CVoxelGrid() = default;

public:
	HRESULT Initialize(void* pArg);
	HRESULT Render(class CVIBuffer_Cube* pVIBuffer, class CShader* pShader, class CTransform* pTransform);

	HRESULT Add_Voxel(const _float3& vPos, _byte bID);
	HRESULT Erase_Voxel(const _float3& vPos);
	HRESULT Clear();
	
public:
	HRESULT Get_ID(const _float3& vPos, _byte& bID);
	HRESULT Set_ID(const _float3& vPos, _byte bID);
	HRESULT Get_ID(_uint iIndex, _byte& bID);
	HRESULT Set_ID(_uint iIndex, _byte bID);

	HRESULT Get_Index(const _float3& vPos, _uint& iIndex);
	HRESULT Get_Position(_uint iIndex, _float3& vPos);

public:
	//	인덱스 초과 미 고려 이므로 외부에서 확인하고 사용
	vector<_uint> Get_NeighborIndices_Unsafe(_uint iIndex);
	vector<_uint> Get_NeighborIndices_Safe(_uint iIndex);
	vector<_uint> Get_NeighborIndices_Safe(const _float3& vPos);

private:
	vector<_uint> Get_NeighborIndices_Base(_uint iIndex);

private:
	bool Is_Culled_Frustum(const _float3& vPos);
	bool Is_Culled_ViewDist(const _float3& vPos, const _float4& vCamPos);

private:
	bool Is_Exist(const _float3& vPos);
	bool Is_Exist(const _uint3& iPos);
	unordered_map<_uint, VOXEL>::iterator Get_Iter(const _float3& vPos);
	unordered_map<_uint, VOXEL>::iterator Get_Iter(const _uint3& iPos);

private:
	HRESULT Encode_Float3(const _float3& vPos, _uint& iIndex);
	HRESULT Encode_UInt3(const _uint3& iPos, _uint& iIndex);
	HRESULT Decode_Float3(_uint iIndex, _float3& vPos);

	HRESULT Convert_UInt3_From_Index(_uint iIndex, _uint3& iPos);

private:
	unordered_map<_uint, VOXEL>				m_Voxels;

	/* for. Render */
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*						m_pContext = { nullptr };
	class CShader*								m_pShader = { nullptr };
	class CVIBuffer_Cube*						m_pVIBufferCube = { nullptr };
	//	class CVoxelWorld	=> 추 후 추가해야함 별도의 그리드 집합을 관리하는 클래스


public:
	static CVoxelGrid* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END