#pragma once

#include "Base.h"

/* 1. ȭ�鿡 �׷������� ��ü�鸸 �׷����� ������� �����ϴ� Ŭ�����̴�.*/
/* 2. �����ϰ� �ִ� ������� ��ü���� Draw��(�����Լ���ȣ���Ѵ�.)�� �����Ѵ�.*/

BEGIN(Engine)

class CRenderer final : public CBase
{
public:
	enum RENDERGROUP { RENDER_PRIORITY, RENDER_SHADOW, RENDER_FIELD, RENDER_NONBLEND, RENDER_NONLIGHT,  RENDER_BLEND, RENDER_UI, RENDER_END };
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;
public:
	HRESULT Initialize();
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT Render();

#ifdef _DEBUG
public:
	HRESULT Add_DebugComponents(class CComponent* pRenderObject);

#endif

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	class CGameInstance*				m_pGameInstance = { nullptr };
	list<class CGameObject*>			m_RenderObjects[RENDER_END];



private:
	class CVIBuffer_Rect*				m_pVIBuffer = { nullptr };
	class CShader*						m_pShader = { nullptr };
	_float4x4							m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	ID3D11DepthStencilView*				m_pLightDepthDSV = { nullptr };

#ifdef _DEBUG
private:
	list<class CComponent*>				m_DebugComponents;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend();
	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Render_UI();
	HRESULT Render_SSAO();
	HRESULT Render_SSAO_Blur();

private:
	HRESULT Render_Lights();
	HRESULT Render_Result();

private:
	HRESULT Render_Debug();

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;


};

END