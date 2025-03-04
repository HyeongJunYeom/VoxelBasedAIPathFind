#include "../Public/Keyboard.h"

CKeyboard::CKeyboard()
{
}

HRESULT CKeyboard::Initialize()
{
	ZeroMemory(m_KeyStates, sizeof(m_KeyStates));
	ZeroMemory(m_isPrePushed, sizeof(m_isPrePushed));

	return S_OK;
}

void CKeyboard::Tick(_float fTimeDelta)
{
	for (int iKey = 0; iKey < VK_MAX; ++iKey)
	{
		if (GetAsyncKeyState(iKey) & 0x8000)
		{
			if (m_isPrePushed[iKey])
			{
				m_KeyStates[iKey] = PRESSING;
				m_fPressTime[iKey] += fTimeDelta;
			}
			else
			{
				m_KeyStates[iKey] = DOWN;
				m_fPressTime[iKey] = 0.f;
			}
			m_isPrePushed[iKey] = true;
		}
		else
		{
			if (m_isPrePushed[iKey])
			{
				m_KeyStates[iKey] = UP;
				m_fPressTime[iKey] = 0.f;
			}
			else
			{
				m_KeyStates[iKey] = NONE;
			}
			m_isPrePushed[iKey] = false;
		}
	}
}

void CKeyboard::Clear()
{
	for (auto& KeyState : m_KeyStates)
	{
		KeyState = NONE;
	}

	for (auto& isPrePused : m_isPrePushed)
	{
		isPrePused = false;
	}

	for (auto& fPressTime : m_fPressTime)
	{
		fPressTime = 0.f;
	}

	for (auto& isMouseWheel : m_isMouseWHEEL)
	{
		isMouseWheel = false;
	}

}

CKeyboard* CKeyboard::Create()
{
	CKeyboard* pInstance = new CKeyboard();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Ctrated : CKeyboard"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKeyboard::Free()
{
}
