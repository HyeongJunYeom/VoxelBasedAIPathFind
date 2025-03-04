#pragma once

#include "Base.h"

BEGIN(Engine)

class CPath_Rounder final : public CBase
{
private:
	CPath_Rounder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPath_Rounder(const CPath_Rounder& rhs) = delete;
	virtual ~CPath_Rounder() = default;

public:
	HRESULT Initialize(void* pArg);
	HRESULT Render();

public:
	const vector<_float3>& Get_SmoothPath() { return m_SmoothPath; }

public:
	void Compute_Smooth_Path(const list<_float3>& PathList);
	void Compute_Smooth_Path_Bazier(const list<_float3>& PathList);

private:
	vector<_float3> Make_SmoothPath(_fvector vStart, _fvector vEnd, _fvector vDir, _float fRadius, _uint iNumSample);
	vector<_float3> Make_SmoothPath_Bezier(const _uint3& vStartIndexPos, const _uint3& vEndIndexPos);
	vector<_float3> Make_SmoothPath_Bezier(_fvector vStart, _fvector vEnd, _fvector vControllPos, _uint iNumSample);


private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

private:
	vector<_float3>				m_SmoothPath;
	_uint						m_iNumSample = { 8 };

public:
	static CPath_Rounder* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual void Free() override;
};

END