#pragma once

#include "Base.h"

BEGIN(Engine)

class CPath_Detour final : public CBase
{
private:
	CPath_Detour(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPath_Detour(const CPath_Detour& rhs) = delete;
	virtual ~CPath_Detour() = default;

public:
	HRESULT Initialize(void* pArg);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

	

public:
	static CPath_Detour* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END