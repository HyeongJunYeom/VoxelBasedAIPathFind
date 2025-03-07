#pragma once

#include "Base.h"

/* 클랑이언트개발자가 만드는 모든 레벨 클래스들의 부모가 되는 클래스다. */
BEGIN(Engine)

class ENGINE_DLL CLevel abstract : public CBase
{
protected:
	CLevel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel() = default;

public:
	virtual HRESULT Initialize();
	virtual void Tick(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

public:
	virtual void Free() override;
};

END