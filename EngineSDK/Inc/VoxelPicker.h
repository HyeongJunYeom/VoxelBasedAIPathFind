#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Picker :
	public CBase
{
private:
	CVoxel_Picker(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_Picker() = default;

public:
	HRESULT Initialize(void* pArg);
	void Tick();

public:
	_uint Get_CurPickIndex() { return m_iCurPickIndex; }
	_uint Get_PrePickIndex() { return m_iPrePickIndex; }
	_float3 Get_CurPickWorldPos();
	_float3 Get_PrePickWorldPos();

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	class CGameInstance*	m_pGameInstance = { nullptr };

private:
	_uint					m_iCurPickIndex = {};
	_uint					m_iPrePickIndex = {};
	_bool					m_isPick = { false };

public:
	static CVoxel_Picker* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual void Free() override;

};

END