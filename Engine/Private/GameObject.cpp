#include "..\Public\GameObject.h"
#include "GameInstance.h"



CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CGameObject::CGameObject(const CGameObject & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
	, m_pGameInstance(rhs.m_pGameInstance)
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CComponent * CGameObject::Get_Component(const wstring & strComTag)
{
	auto	iter = m_Components.find(strComTag);

	if (iter == m_Components.end())
		return nullptr;

	return iter->second;	
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

/* ���� ���ӳ��� ���Ǵ� ��ü�� ȣ���ϴ� �Լ���. */
HRESULT CGameObject::Initialize(void* pArg)
{
	if(nullptr != pArg)
	{
		GAMEOBJECT_DESC*		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;		
	}

	if (FAILED(Add_Component(CTransform::Create(m_pDevice, m_pContext), g_strTransformTag, (CComponent**)(&m_pTransformCom))))
		return E_FAIL;

	if (FAILED(m_pTransformCom->Initialize(pArg)))
		return E_FAIL;
	
	return S_OK;
}

void CGameObject::Priority_Tick(_float fTimeDelta)
{
}

void CGameObject::Tick(_float fTimeDelta)
{
}

void CGameObject::Late_Tick(_float fTimeDelta)
{

}

HRESULT CGameObject::Render()
{
	return S_OK;
}


HRESULT CGameObject::Add_Component(_uint iLevelIndex, const wstring & strPrototypeTag, const wstring & strComponentTag, CComponent** ppOut, void * pArg)
{
	CComponent*		pComponent = m_pGameInstance->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
	if (nullptr == pComponent)
		return E_FAIL;

	auto	iter = m_Components.find(strComponentTag);
	if (iter != m_Components.end())
		return E_FAIL;

	m_Components.emplace(strComponentTag, pComponent);
	
	*ppOut = pComponent;

	Safe_AddRef(pComponent);

	return S_OK;
}

HRESULT CGameObject::Add_Component(CComponent* pComponent, const wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	if (nullptr == pComponent)
		return E_FAIL;

	auto	iter = m_Components.find(strComponentTag);
	if (iter != m_Components.end())
		return E_FAIL;

	m_Components.emplace(strComponentTag, static_cast<CComponent*>(pComponent));
	*ppOut = pComponent;

	Safe_AddRef(pComponent);

	return S_OK;
}

void CGameObject::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);

	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);

	m_Components.clear();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);	
}
