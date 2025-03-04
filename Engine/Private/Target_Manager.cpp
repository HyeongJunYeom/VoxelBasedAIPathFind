#include "..\Public\Target_Manager.h"
#include "RenderTarget.h"

#include "GameInstance.h"

CTarget_Manager::CTarget_Manager(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CTarget_Manager::Initialize()
{
	return S_OK;
}

HRESULT CTarget_Manager::Add_RenderTarget(const wstring & strRenderTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT ePixelFormat, const _float4 & vClearColor)
{
	if (nullptr != Find_RenderTarget(strRenderTargetTag))
		return E_FAIL;

	CRenderTarget*		pRenderTarget = CRenderTarget::Create(m_pDevice, m_pContext, iSizeX, iSizeY, ePixelFormat, vClearColor);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	m_RenderTargets.emplace(strRenderTargetTag, pRenderTarget);

	return S_OK;
}

HRESULT CTarget_Manager::Add_MRT(const wstring & strMRTTag, const wstring & strRenderTargetTag)
{
	CRenderTarget*		pRenderTarget = Find_RenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	list<CRenderTarget*>*		pTargetList = Find_MRT(strMRTTag);
	if (nullptr == pTargetList)
	{
		list<CRenderTarget*>		TargetList;
		TargetList.emplace_back(pRenderTarget);
		m_MRTs.emplace(strMRTTag, TargetList);
	}
	else
		pTargetList->emplace_back(pRenderTarget);

	Safe_AddRef(pRenderTarget);

	return S_OK;
}

HRESULT CTarget_Manager::Begin_MRT(const wstring & strMRTTag, ID3D11DepthStencilView* pDSV)
{
	ID3D11ShaderResourceView*			pSRV[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {
		nullptr
	};

	m_pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pSRV);


	list<CRenderTarget*>*		pTargetList = Find_MRT(strMRTTag);
	if (nullptr == pTargetList)
		return E_FAIL;

	m_pContext->OMGetRenderTargets(1, &m_pBackBufferRTV, &m_pDSV);

	_uint		iNumRenderTargets = {0};

	ID3D11RenderTargetView* pRenderTargets[8] = { nullptr };

	for (auto& pRenderTarget : *pTargetList)
	{
		pRenderTarget->Clear();
		pRenderTargets[iNumRenderTargets++] = pRenderTarget->Get_RTV();
	}

	if(nullptr != pDSV)
		m_pContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	m_pContext->OMSetRenderTargets(iNumRenderTargets, pRenderTargets, pDSV == nullptr ? m_pDSV : pDSV);

	return S_OK;
}

HRESULT CTarget_Manager::End_MRT()
{
	m_pContext->OMSetRenderTargets(1, &m_pBackBufferRTV, m_pDSV);

	Safe_Release(m_pBackBufferRTV);
	Safe_Release(m_pDSV);

	return S_OK;
}

HRESULT CTarget_Manager::Bind_ShaderResource(CShader * pShader, const wstring & strRenderTargetTag, const _char * pConstantName)
{
	CRenderTarget*		pRenderTarget = Find_RenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Bind_ShaderResource(pShader, pConstantName);	
}

HRESULT CTarget_Manager::Copy_Resource(const wstring & strRenderTargetTag, ID3D11Texture2D ** ppTextureHub)
{
	CRenderTarget*		pRenderTarget = Find_RenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Copy_Resource(ppTextureHub);
}


HRESULT CTarget_Manager::Ready_Debug(const wstring & strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	CRenderTarget*		pRenderTarget = Find_RenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Ready_Debug(fX, fY, fSizeX, fSizeY);	
}

HRESULT CTarget_Manager::Render_Debug(const wstring & strMRTTag, CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	list<CRenderTarget*>*	pRenderTargetList = Find_MRT(strMRTTag);
	if (nullptr == pRenderTargetList)
		return E_FAIL;

	for (auto& pRenderTarget : *pRenderTargetList)
	{
		pRenderTarget->Render_Debug(pShader, pVIBuffer);
	}

	return S_OK;
}


CRenderTarget * CTarget_Manager::Find_RenderTarget(const wstring & strRenderTargetTag)
{
	auto	iter = m_RenderTargets.find(strRenderTargetTag);

	if (iter == m_RenderTargets.end())
		return nullptr;

	return iter->second;	
}

list<class CRenderTarget*>* CTarget_Manager::Find_MRT(const wstring & strMRTTag)
{
	auto	iter = m_MRTs.find(strMRTTag);
	if (iter == m_MRTs.end())
		return nullptr;

	return &iter->second;	
}

CTarget_Manager * CTarget_Manager::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTarget_Manager*		pInstance = new CTarget_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CTarget_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTarget_Manager::Free()
{
	for (auto& Pair : m_MRTs)
	{
		for (auto& pRenderTarget : Pair.second)
			Safe_Release(pRenderTarget);
		Pair.second.clear();
	}		
	m_MRTs.clear();

	for (auto& Pair : m_RenderTargets)
		Safe_Release(Pair.second);
	m_RenderTargets.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
