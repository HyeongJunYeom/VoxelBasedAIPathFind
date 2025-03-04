#pragma once

#include "Transform.h"
#include "ImGui_Includes.h"

BEGIN(Engine)

class ENGINE_DLL CGui_Object abstract : public CBase
{
public:
	typedef struct tagGuiObjectDesc
	{
		_bool*		pFocus = { nullptr};
		_bool*		pHovered = { nullptr};
	}GUI_OBJECT_DESC;

protected:
	CGui_Object(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGui_Object(const CGui_Object& rhs) = delete;
	virtual ~CGui_Object() = default;

public:
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	class CGameInstance*	m_pGameInstance = { nullptr };

	_bool*					m_pFocus = { false };
	_bool*					m_pHovered = { false };

public:
	virtual void Free() override;
};

END