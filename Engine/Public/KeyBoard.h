#pragma once

#include "Base.h"

BEGIN(Engine)

class CKeyboard final : public CBase
{
public:
	enum WHEEL { WH_DOWN, WH_UP, WH_END };
private:
	CKeyboard();
	~CKeyboard() = default;

public:
	HRESULT			Initialize();
	void			Tick(_float fTimeDelta);

	EKeyState		Get_KeyState(_int iKey) { return m_KeyStates[iKey]; }
	_float			Get_Key_Press_Time(_int iKey) { return m_fPressTime[iKey]; }

	void			Clear();

public:
	static CKeyboard* Create();
	virtual void Free() override;

private:
	EKeyState		m_KeyStates[VK_MAX];
	_bool			m_isPrePushed[VK_MAX];
	_float			m_fPressTime[VK_MAX] = { 0.f };

	_bool			m_isMouseWHEEL[WH_END] = {};
};

END