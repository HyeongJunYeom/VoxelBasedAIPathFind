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
	// 셰이더 해제
	m_pContext->VSSetShader(nullptr, nullptr, 0);
	m_pContext->PSSetShader(nullptr, nullptr, 0);
	m_pContext->GSSetShader(nullptr, nullptr, 0);
	m_pContext->HSSetShader(nullptr, nullptr, 0);
	m_pContext->DSSetShader(nullptr, nullptr, 0);

	m_pBatch->Begin(
		SpriteSortMode_Deferred,
		nullptr,                          // 기본 블렌드 상태
		nullptr,                          // 기본 샘플러 상태
		nullptr,                          // 기본 깊이-스텐실 상태
		nullptr,                          // 기본 래스터라이저 상태
		nullptr,                          // 커스텀 효과 없음
		XMMatrixIdentity()                // 기본 변환 행렬
	);

	m_pFont->DrawString(m_pBatch, strText.c_str(), vPosition, vColor, fRadian);

	m_pBatch->End();

	return S_OK;
}

HRESULT CCustomFont::Render_Scaled(const wstring& strText, const _float2& vPosition, _fvector vColor, _float fScale)
{
	// 셰이더 해제
	m_pContext->VSSetShader(nullptr, nullptr, 0);
	m_pContext->PSSetShader(nullptr, nullptr, 0);
	m_pContext->GSSetShader(nullptr, nullptr, 0);
	m_pContext->HSSetShader(nullptr, nullptr, 0);
	m_pContext->DSSetShader(nullptr, nullptr, 0);

	m_pBatch->Begin(
		SpriteSortMode_Deferred,
		nullptr,                          // 기본 블렌드 상태
		nullptr,                          // 기본 샘플러 상태
		nullptr,                          // 기본 깊이-스텐실 상태
		nullptr,                          // 기본 래스터라이저 상태
		nullptr,                          // 커스텀 효과 없음
		XMMatrixIdentity()                // 기본 변환 행렬
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
