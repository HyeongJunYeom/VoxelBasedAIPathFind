#include "Input_Device.h"
#include "Mouse.h"
#include "Keyboard.h"

#include "Windows.h"

CInput_Device::CInput_Device()
{
}

HRESULT CInput_Device::Initialize(HWND hWnd)
{
	m_pKeyboard = CKeyboard::Create();
	if (nullptr == m_pKeyboard)
		return E_FAIL;

	m_pMouse = CMouse::Create(hWnd);
	if (nullptr == m_pMouse)
		return E_FAIL;
	
	m_hWnd = hWnd;

	return S_OK;
}

void CInput_Device::Tick(_float fTimeDelta)
{
	m_pKeyboard->Tick(fTimeDelta);
	m_pMouse->Tick(fTimeDelta);

	if (m_hWnd != GetForegroundWindow())
	{
		m_pKeyboard->Clear();
		m_pMouse->Clear();
	}
}

CInput_Device* CInput_Device::Create(HWND hWnd)
{
	CInput_Device* pInstance = new CInput_Device();
	if (FAILED(pInstance->Initialize(hWnd)))
	{
		MSG_BOX(TEXT("Failed To Created : CInput_Device"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInput_Device::Free()
{
	Safe_Release(m_pKeyboard);
	Safe_Release(m_pMouse);
}
