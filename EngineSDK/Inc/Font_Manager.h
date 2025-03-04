#pragma once

#include "Base.h"

BEGIN(Engine)

class CFont_Manager final : public CBase
{
private:
	CFont_Manager();
	virtual ~CFont_Manager() = default;

public:
	HRESULT Add_Font(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strFontTag, const wstring& strFontFilePath);
	HRESULT Render(const wstring& strFontTag, const wstring & strText, const _float2 & vPosition, _fvector vColor, _float fRadian);
	HRESULT Render_Scaled(const wstring& strFontTag, const wstring & strText, const _float2 & vPosition, _fvector vColor, _float fScale);

private:
	map<const wstring, class CCustomFont*>		m_Fonts;

private:
	class CCustomFont* Find_Font(const wstring& strFontTag);

public:
	static CFont_Manager* Create();
	virtual void Free() override;
};

END