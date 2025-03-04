#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Convertor final : public CBase
{
public:
	typedef struct tagVoxelConvertorDesc
	{
		_uint*					pNumVoxelWorldDim = { nullptr };
		_uint*					pNumVoxelSectorDim = { nullptr };
		_uint*					pNumSectorWorldDim = { nullptr };
		_float*					pVoxelSize = { nullptr };
		class CTransform*		pTransform = { nullptr };
	}VOXEL_CONVERTOR_DESC;

private:
	CVoxel_Convertor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_Convertor() = default;

public:
	HRESULT Initialize(void* pArg);

public:
	HRESULT Convert_WorldPos_From_IndexPos(const _uint3& vIndexPos, _float3& vWorldPos);
	HRESULT Convert_LocalPos_From_IndexPos(const _uint3& vIndexPos, _float3& vLocalPos);
	HRESULT Convert_IndexPos_From_WorldPos(const _float3& vWorldPos, _uint3& vIndexPos);
	HRESULT Convert_VoxelIndex_From_IndexPos(const _uint3& vIndexPos, _uint& iIndex);
	HRESULT Convert_VoxelIndex_From_WorldPos(const _float3& vWorldPos, _uint& iIndex);
	HRESULT Convert_SectorIndex_From_IndexPos(const _uint3& vIndexPos, _uint& iIndex);
	HRESULT Convert_SectorIndex_From_WorldPos(const _float3& vWorldPos, _uint& iIndex);
	HRESULT Convert_SectorIndex_From_VoxelIndex(const _uint iVoxelIndex, _uint& iSectorIndex);
	HRESULT Convert_IndexPos_From_Index(const _uint iIndex, _uint3& vIndexPos);
	HRESULT Convert_LocalPos_From_Index(const _uint iIndex, _float3& vLocalPos);
	HRESULT Convert_WorldPos_From_Index(const _uint iIndex, _float3& vWorldPos);

private:
	_bool Is_Out_Of_Range_WorldPos(const _float3& vWorldPos);		//	월드 좌표
	_bool Is_Out_Of_Range_LocalPos(const _float3& vLocalPos);		//	복셀월드상의 로컬상의 좌표 IndexPos * fVoxelSize

	_bool Is_Out_Of_Range_Voxel(const _uint3& vIndexPos);
	_bool Is_Out_Of_Range_Voxel(const _uint iIndex);

	_bool Is_Out_Of_Range_Sector(const _uint3& vIndexPos);
	_bool Is_Out_Of_Range_Sector(const _uint iIndex);


private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

private:
	const _uint*				m_pNumVoxelWorldDim = { nullptr };
	const _uint*				m_pNumVoxelSectorDim = { nullptr };
	const _uint*				m_pNumSectorWorldDim = { nullptr };
	const _float*				m_pVoxelSize = { nullptr };
	class CTransform*			m_pTransformCom = { nullptr };

public:
	static CVoxel_Convertor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END