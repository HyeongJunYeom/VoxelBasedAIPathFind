#pragma once

#include "Engine_Defines.h"
#include "Base.h"

#include "Keyboard.h"
#include "Mouse.h"

BEGIN(Engine)

class CInput_Device final : public CBase
{
private:
	CInput_Device();
	virtual ~CInput_Device() = default;

public:
	HRESULT Initialize(HWND hWnd);
	void Tick(_float fTimeDelta);

public: /* For.Keyboard */
	EKeyState		Get_KeyState(_int iKey) 
	{
		return m_pKeyboard->Get_KeyState(iKey);
	}

	_float			Get_Key_Press_Time(_int iKey)
	{
		return m_pKeyboard->Get_Key_Press_Time(iKey);
	}

public: /* For.Mouse */
	_bool			Check_Wheel_Down()
	{
		return m_pMouse->Check_Wheel_Down();
	}

	_bool			Check_Wheel_Up()
	{
		return m_pMouse->Check_Wheel_Up();
	}

	_int			Check_Wheel_Delta() 
	{
		return m_pMouse->Check_Wheel_Delta();
	}

	void			Wheel_Clear() 
	{
		m_pMouse->Wheel_Clear();
	}

	void			Update_WHEEL(_int iDelta) 
	{
		m_pMouse->Update_WHEEL(iDelta);
	}

	POINT			Get_MouseCurPos() 
	{
		return m_pMouse->Get_MouseCurPos();
	}
	POINT			Get_MouseDeltaPos() 
	{
		return m_pMouse->Get_MouseDeltaPos();
	}
	void			Set_MouseCurPos(POINT ptPos) 
	{
		m_pMouse->Set_MouseCurPos(ptPos);
	}

private:
	CMouse*			m_pMouse = { nullptr };
	CKeyboard*		m_pKeyboard = { nullptr };

	HWND			m_hWnd = { NULL };

public:
	static CInput_Device* Create(HWND hWnd);
	virtual void Free() override;
};

END