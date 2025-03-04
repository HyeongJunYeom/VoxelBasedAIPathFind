#include "VoxelSector.h"
#include "GameInstance.h"

#include "VIBuffer_Cube.h"
#include "Shader.h"

_float CVoxel_Sector::ms_fCullDist = { 100.f };

CVoxel_Sector::CVoxel_Sector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CVoxel_Sector::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	VOXEL_SECTOR_DESC* pDesc = { static_cast<VOXEL_SECTOR_DESC*>(pArg) };
	m_pVoxelSize = pDesc->pVoxelSize;

	return S_OK;
}

HRESULT CVoxel_Sector::Render()
{
	return S_OK;
}

HRESULT CVoxel_Sector::Add_Voxel(const _uint3& vPos, VOXEL_ID eID, VOXEL_STATE eState)
{
	if (VOXEL_ID::_END <= eID || 
		VOXEL_STATE::_END <= eState ||
		true == Is_Exist(vPos))
		return E_FAIL;

	_uint		iIndex{};
	if (FAILED(Encode_UInt3(vPos, iIndex)))
		return E_FAIL;

	VOXEL		Voxel{};
	Voxel.bID = static_cast<_byte>(eID);
	Voxel.bState = static_cast<_byte>(eState);

	m_Voxels.emplace(iIndex, Voxel);

	return S_OK;
}

HRESULT CVoxel_Sector::Insert_Voxel(_uint iIndex, VOXEL& Voxel)
{
	auto iter{ m_Voxels.find(iIndex) };
	if (iter != m_Voxels.end())
		return E_FAIL;
	
	m_Voxels.insert({ iIndex, Voxel });

	return S_OK;
}

HRESULT CVoxel_Sector::Erase_Voxel(const _uint3& vPos)
{
	auto iter{ Get_Iter(vPos) };
	if (iter == m_Voxels.end())
		return E_FAIL;

	m_Voxels.erase(iter);
	return S_OK;
}

HRESULT CVoxel_Sector::Clear()
{
	m_Voxels.clear();

	return S_OK;
}

_bool CVoxel_Sector::Is_Empty()
{
	return m_Voxels.empty();
}

_bool CVoxel_Sector::Is_Exist(const _uint iIndex)
{
	auto		iter{ m_Voxels.find(iIndex) };
	return iter != m_Voxels.end();
}



bool CVoxel_Sector::Is_Exist(const _uint3& vIndexPos)
{
	_uint		iIndex;
	if (FAILED(Encode_UInt3(vIndexPos, iIndex)))
		return false;

	return Is_Exist(iIndex);
}

HRESULT CVoxel_Sector::Get_ID(const _uint3& vPos, VOXEL_ID& eID)
{
	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	eID = static_cast<VOXEL_ID>(iter->second.bID);
	return S_OK;
}

HRESULT CVoxel_Sector::Set_ID(const _uint3& vPos, VOXEL_ID eID)
{
	if (VOXEL_ID::_END <= eID)
		return E_FAIL;

	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.bID = static_cast<_byte>(eID);
	return S_OK;
}

HRESULT CVoxel_Sector::Get_ID(const _uint iIndex, VOXEL_ID& eID)
{
	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	eID = static_cast<VOXEL_ID>(iter->second.bID);
	return S_OK;
}

HRESULT CVoxel_Sector::Set_ID(const _uint iIndex, VOXEL_ID eID)
{
	if (VOXEL_ID::_END <= eID)
		return E_FAIL;

	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.bID = static_cast<_byte>(eID);
	return S_OK;
}

HRESULT CVoxel_Sector::Get_State(const _uint3& vPos, VOXEL_STATE& eState)
{
	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	eState = static_cast<VOXEL_STATE>(iter->second.bState);
	return S_OK;
}

HRESULT CVoxel_Sector::Set_State(const _uint3& vPos, VOXEL_STATE eState)
{
	if (VOXEL_STATE::_END <= eState)
		return E_FAIL;

	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.bState = static_cast<_byte>(eState);
	return S_OK;
}

HRESULT CVoxel_Sector::Get_State(const _uint iIndex, VOXEL_STATE& eState)
{
	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	eState = static_cast<VOXEL_STATE>(iter->second.bState);
	return S_OK;
}

HRESULT CVoxel_Sector::Set_State(const _uint iIndex, VOXEL_STATE eState)
{
	if (VOXEL_STATE::_END <= eState)
		return E_FAIL;

	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.bState = static_cast<_byte>(eState);
	return S_OK;
}

HRESULT CVoxel_Sector::Get_OpenessScore(const _uint3 vPos, _byte& bOpenessScore)
{
	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	bOpenessScore = iter->second.bOpenessScore;
	return S_OK;
}

HRESULT CVoxel_Sector::Set_OpenessScore(const _uint3 vPos, const _byte bOpenessScore)
{
	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.bOpenessScore = bOpenessScore;
	return S_OK;
}

HRESULT CVoxel_Sector::Get_OpenessScore(const _uint iIndex, _byte& bOpenessScore)
{
	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	bOpenessScore = iter->second.bOpenessScore;
	return S_OK;
}

HRESULT CVoxel_Sector::Set_OpenessScore(const _uint iIndex, const _byte bOpenessScore)
{
	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.bOpenessScore = bOpenessScore;
	return S_OK;
}

HRESULT CVoxel_Sector::Get_NeighborFlag(const _uint3& vPos, _uint& iNeighborFlag)
{
	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iNeighborFlag = iter->second.iNeighborFlag;
	return S_OK;
}

HRESULT CVoxel_Sector::Get_NeighborFlag(const _uint iIndex, _uint& iNeighborFlag)
{
	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iNeighborFlag = iter->second.iNeighborFlag;
	return S_OK;
}

HRESULT CVoxel_Sector::Add_Neighbor(const _uint3& vPos, const _uint iNeighborFlag)
{
	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.iNeighborFlag |= iNeighborFlag;
	return S_OK;
}

HRESULT CVoxel_Sector::Add_Neighbor(const _uint iIndex, const _uint iNeighborFlag)
{
	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.iNeighborFlag |= iNeighborFlag;
	return S_OK;
}

HRESULT CVoxel_Sector::Erase_Neighbor(const _uint3& vPos, const _uint iNeighborFlag)
{
	auto		iter{ Get_Iter(vPos) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.iNeighborFlag &= ~iNeighborFlag;
	return S_OK;
}

HRESULT CVoxel_Sector::Erase_Neighbor(const _uint iIndex, const _uint iNeighborFlag)
{
	auto		iter{ Get_Iter(iIndex) };
	if (m_Voxels.end() == iter)
		return E_FAIL;

	iter->second.iNeighborFlag &= ~iNeighborFlag;
	return S_OK;
}

HRESULT CVoxel_Sector::Get_IndexSectorLocal(const _uint3& vPos, _uint& iIndex)
{
	return Encode_UInt3(vPos, iIndex);
}

HRESULT CVoxel_Sector::Get_Position_Local(_uint iIndex, _uint3& vPos)
{
	return Decode_UInt3(iIndex, vPos);
}

vector<_uint> CVoxel_Sector::Get_VoxelIndices()
{
	vector<_uint>			Indices;
	for (auto& Pair : m_Voxels)
	{
		Indices.push_back((Pair.first));
	}

	return Indices;
}

_uint CVoxel_Sector::Get_NumVoxel()
{
	return static_cast<_uint>(m_Voxels.size());
}

vector<_uint> CVoxel_Sector::Get_Indices()
{
	vector<_uint>			Indicies;
	for (auto& Pair : m_Voxels)
	{
		Indicies.push_back(Pair.first);
	}

	return Indicies;
}

HRESULT CVoxel_Sector::Bind_InstanceBuffers(vector<_uint>& IndexBuffer, vector<_byte>& StateBuffer, vector<_byte>& IDBuffer, vector<_int>& NeighborBuffer)
{
	for (auto& Pair : m_Voxels)
	{
		//_uint3		vIndexPos;
		//Decode_UInt3(Pair.first, vIndexPos);

		/*	if (Is_Culled(vIndexPos))
			continue;*/

		IndexBuffer.push_back(Pair.first);
		IDBuffer.push_back(Pair.second.bID);
		StateBuffer.push_back(Pair.second.bState);
		NeighborBuffer.push_back(Pair.second.iNeighborFlag);
	}

	return S_OK;
}

unordered_map<_uint, VOXEL>::iterator CVoxel_Sector::Get_Iter(const _uint3& iPos)
{
	_uint iIndex{};
	if (FAILED(Encode_UInt3(iPos, iIndex)))
		return m_Voxels.end();

	return m_Voxels.find(iIndex);
}

unordered_map<_uint, VOXEL>::iterator CVoxel_Sector::Get_Iter(const _uint& iIndex)
{
	return m_Voxels.find(iIndex);
}

_bool CVoxel_Sector::Is_Culled(const _uint3& vPos)
{
	_vector				vLocalPos = { XMLoadUInt3(&vPos) * (*m_pVoxelSize)};

	if (true == Is_Culled_Frustum(vLocalPos))
		return true;

	return false;
}

_bool CVoxel_Sector::Is_Culled_Frustum(_fvector vPos)
{
	return !(m_pGameInstance->isInFrustum_LocalSpace(vPos, powf((*m_pVoxelSize) * 0.5f, 3.f)));
}

_bool CVoxel_Sector::Is_Culled_ViewDist(_fvector vPos)
{
	_vector			vCamPos = { m_pGameInstance->Get_CamPosition_Vector() };
	_float			fDistToCam = { XMVectorGetX(XMVector3Length(vPos - vCamPos)) };

	return ms_fCullDist < fDistToCam;
}

HRESULT CVoxel_Sector::Encode_UInt3(const _uint3& iPos, _uint& iIndexSector)
{
	if ((max(iPos.x, max(iPos.y, iPos.z))) > g_iVoxelPosLimit)
		return E_FAIL;

	iIndexSector = (iPos.x << g_iShift_X) 
		+ (iPos.y << g_iShift_Y) 
		+ (iPos.z << g_iShift_Z);

	return S_OK;
}

HRESULT CVoxel_Sector::Decode_UInt3(const _uint iIndexSector, _uint3& iPos)
{
	if (iIndexSector > g_iVoxelIndexLimit)
		return E_FAIL;

	iPos.x = iIndexSector & g_iConvertFlagX;
	iPos.y = (iIndexSector & g_iConvertFlagY) >> g_iShift_Y;
	iPos.z = (iIndexSector & g_iConvertFlagZ) >> g_iShift_Z;

	return S_OK;
}

void CVoxel_Sector::Set_CullDist(_float fDist)
{
	ms_fCullDist = max(fDist, 0.f);
}

CVoxel_Sector* CVoxel_Sector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CVoxel_Sector* pInstance = { new CVoxel_Sector{ pDevice, pContext } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxelSector"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel_Sector::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
