#pragma once

#include "Engine_Defines.h"
#include "Base.h"

BEGIN(Engine)

class CMouse final : public CBase
{
public:
	enum WHEEL { WH_DOWN, WH_UP, WH_END };

private:
	CMouse();
	~CMouse() = default;

public:
	HRESULT			Initialize(HWND hWnd);
	void			Tick(_float fTimeDelta);

	_bool			Check_Wheel_Down() { return m_isMouseWHEEL[WH_DOWN]; }
	_bool			Check_Wheel_Up() { return m_isMouseWHEEL[WH_UP]; }
	_int			Check_Wheel_Delta() { return m_iDeltaWeeel; }
	void			Wheel_Clear();

public:
	void			Update_WHEEL(_int iDelta);

	POINT			Get_MouseCurPos()
	{
		return m_ptCurPos;
	}
	POINT			Get_MouseDeltaPos()
	{
		return m_ptDeltaPos;
	}
	void			Set_MouseCurPos(POINT ptPos) 
	{
		m_ptCurPos = ptPos;
	}

	void			Clear();

private:
	void			Update_MousePosition(_float fTimeDelta);

private:
	POINT			m_ptCurPos = {};
	POINT			m_ptDeltaPos = {};
	_bool			m_isMouseWHEEL[WH_END] = {};
	_int			m_iDeltaWeeel = {};

	HWND			m_hWnd = { NULL };

public:
	static CMouse* Create(HWND hWnd);
	virtual void Free() override;
};

END