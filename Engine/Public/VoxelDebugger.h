#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Debugger :
    public CBase
{
private:
	CVoxel_Debugger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_Debugger() = default;

public:
	HRESULT Initialize(void* pArg);
	void Tick();
	HRESULT Render();

private:
	HRESULT Draw_Current_PickPos();

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

private:
	VOXEL_STATE					m_ePrePickedVoxelState = { VOXEL_STATE::_DEFAULT };

public:
	static CVoxel_Debugger* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, void* pArg = nullptr);
	virtual void Free() override;

};

END