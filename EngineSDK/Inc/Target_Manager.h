#pragma once

#include "Base.h"

/* 여러개의 렌더타겟을 저장한다. */

BEGIN(Engine)

class CTarget_Manager final : public CBase
{
private:
	CTarget_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CTarget_Manager() = default;

public:
	HRESULT Initialize();
	HRESULT Add_RenderTarget(const wstring& strRenderTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const wstring& strMRTTag, const wstring& strRenderTargetTag);
	HRESULT Begin_MRT(const wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr);
	HRESULT End_MRT();
	HRESULT Bind_ShaderResource(class CShader* pShader, const wstring& strRenderTargetTag, const _char* pConstantName);
	HRESULT Copy_Resource(const wstring& strRenderTargetTag, ID3D11Texture2D** ppTextureHub);

public:
	HRESULT Ready_Debug(const wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_Debug(const wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

private:
	ID3D11Device*											m_pDevice = { nullptr };
	ID3D11DeviceContext*									m_pContext = { nullptr };
	map<const wstring, class CRenderTarget*>				m_RenderTargets;
	map<const wstring, list<class CRenderTarget*>>			m_MRTs;

	ID3D11RenderTargetView*									m_pBackBufferRTV = { nullptr };
	ID3D11DepthStencilView*									m_pDSV = { nullptr };

private:
	class CRenderTarget* Find_RenderTarget(const wstring& strRenderTargetTag);
	list<class CRenderTarget*>* Find_MRT(const wstring& strMRTTag);

public:
	static CTarget_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END