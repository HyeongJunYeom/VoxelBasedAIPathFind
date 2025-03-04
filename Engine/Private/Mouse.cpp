#include "../Public/Mouse.h"

CMouse::CMouse()
{
}

HRESULT CMouse::Initialize(HWND hWnd)
{
	if (NULL == hWnd)
		return E_FAIL;

	m_hWnd = hWnd;

	return S_OK;
}

void CMouse::Tick(_float fTimeDelta)
{
	Update_MousePosition(fTimeDelta);
}

void CMouse::Wheel_Clear()
{
	m_isMouseWHEEL[WH_UP] = false;
	m_isMouseWHEEL[WH_DOWN] = false;
}

void CMouse::Update_WHEEL(_int iDelta)
{
	m_iDeltaWeeel = iDelta;

	if (iDelta > 0)
	{
		// 휠 업 이벤트 처리
		// 여기에 코드 추가
		m_isMouseWHEEL[WH_UP] = true;
		m_isMouseWHEEL[WH_DOWN] = false;
	}
	else if (iDelta < 0)
	{
		// 휠 다운 이벤트 처리
		// 여기에 코드 추가
		m_isMouseWHEEL[WH_UP] = false;
		m_isMouseWHEEL[WH_DOWN] = true;
	}
	else
	{
		m_isMouseWHEEL[WH_UP] = false;
		m_isMouseWHEEL[WH_DOWN] = false;
	}
}

void CMouse::Clear()
{
	for (auto& isMouseWHEEL : m_isMouseWHEEL)
	{
		isMouseWHEEL = false;
	}

	m_iDeltaWeeel = { 0 };
	m_ptDeltaPos = { 0, 0 };
}

void CMouse::Update_MousePosition(_float fTimeDelta)
{
	POINT ptMouse = {};
	GetCursorPos(&ptMouse);
	ScreenToClient(m_hWnd, &ptMouse);

	m_ptDeltaPos.x = ptMouse.x - m_ptCurPos.x;
	m_ptDeltaPos.y = ptMouse.y - m_ptCurPos.y;

	m_ptCurPos = ptMouse;
}

CMouse* CMouse::Create(HWND hWnd)
{
	CMouse* pInstance = new CMouse();

	if (FAILED(pInstance->Initialize(hWnd)))
	{
		MSG_BOX(TEXT("Failed To Ctrated : CMouse"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMouse::Free()
{
}
