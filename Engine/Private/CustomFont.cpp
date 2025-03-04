#include "..\Public\CustomFont.h"

CCustomFont::CCustomFont(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCustomFont::Initialize(const wstring & strFontFilePath)
{
	m_pFont = new SpriteFont(m_pDevice, strFontFilePath.c_str());
	m_pBatch = new SpriteBatch(m_pContext);

	return S_OK;
}

HRESULT CCustomFont::Render(const wstring& strText, const _float2& vPosition, _fvector vColor, _float fRadian)
{
	// ���̴� ����
	m_pContext->VSSetShader(nullptr, nullptr, 0);
	m_pContext->PSSetShader(nullptr, nullptr, 0);
	m_pContext->GSSetShader(nullptr, nullptr, 0);
	m_pContext->HSSetShader(nullptr, nullptr, 0);
	m_pContext->DSSetShader(nullptr, nullptr, 0);

	m_pBatch->Begin(
		SpriteSortMode_Deferred,
		nullptr,                          // �⺻ ���� ����
		nullptr,                          // �⺻ ���÷� ����
		nullptr,                          // �⺻ ����-���ٽ� ����
		nullptr,                          // �⺻ �����Ͷ����� ����
		nullptr,                          // Ŀ���� ȿ�� ����
		XMMatrixIdentity()                // �⺻ ��ȯ ���
	);

	m_pFont->DrawString(m_pBatch, strText.c_str(), vPosition, vColor, fRadian);

	m_pBatch->End();

	return S_OK;
}

HRESULT CCustomFont::Render_Scaled(const wstring& strText, const _float2& vPosition, _fvector vColor, _float fScale)
{
	// ���̴� ����
	m_pContext->VSSetShader(nullptr, nullptr, 0);
	m_pContext->PSSetShader(nullptr, nullptr, 0);
	m_pContext->GSSetShader(nullptr, nullptr, 0);
	m_pContext->HSSetShader(nullptr, nullptr, 0);
	m_pContext->DSSetShader(nullptr, nullptr, 0);

	m_pBatch->Begin(
		SpriteSortMode_Deferred,
		nullptr,                          // �⺻ ���� ����
		nullptr,                          // �⺻ ���÷� ����
		nullptr,                          // �⺻ ����-���ٽ� ����
		nullptr,                          // �⺻ �����Ͷ����� ����
		nullptr,                          // Ŀ���� ȿ�� ����
		XMMatrixIdentity()                // �⺻ ��ȯ ���
	);

	m_pFont->DrawString(m_pBatch, strText.c_str(), vPosition, vColor, 0.f, _float2(), fScale);

	m_pBatch->End();

	return S_OK;
}

CCustomFont * CCustomFont::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strFontFilePath)
{
	CCustomFont*		pInstance = new CCustomFont(pDevice, pContext);

	if (FAILED(pInstance->Initialize(strFontFilePath)))
	{
		MSG_BOX(TEXT("Failed to Created : CCustomFont"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCustomFont::Free()
{
	Safe_Delete_Array(m_pBatch);
	Safe_Delete_Array(m_pFont);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
