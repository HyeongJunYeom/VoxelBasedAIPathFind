#pragma once

#include "Engine_Defines.h"
#include "Base.h"

BEGIN(Engine)

class CPicking : public CBase
{
private:
	CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPicking() = default;

public:
	HRESULT Initialize(_uint iWinSizeX, _uint iWinSizeY);
	void Tick();
	void Transform_PickingToLocalSpace_Perspective(const class CTransform* pTransform, _Out_ _float3* pRayDir, _Out_ _float4* pRayPos);
	void Transform_PickingToLocalSpace_Perspective(_fmatrix WorldMatrix, _Out_ _float3* pRayDir, _Out_ _float4* pRayPos);
	void Transform_PickingToLocalSpace_Ortho(const class CTransform* pTransform, _Out_ _float3* pRayDir, _Out_ _float4* pRayPos);


private:
	void Update_PerspectiveRay();
public:
	_float4 Get_MouseWorldPos() { return m_vRayPosPerspective; }
	_float3 Get_MouseWorldDirection() { return m_vRayDirPerspective; }

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	_uint					m_iWinSizeX = { 0 };
	_uint					m_iWinSizeY = { 0 };

	_float4					m_vRayPosPerspective = { 0.f, 0.f, 0.f, 1.f };
	_float3					m_vRayDirPerspective = { 0.f, 0.f, 0.f };

	_float4					m_vRayPosOrtho = { 0.f, 0.f, 0.f, 1.f };
	_float3					m_vRayDirOrtho = { 0.f, 0.f, 0.f };

public:
	static CPicking* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void Free() override;
};

END