#include "stdafx.h"
#include "TestObject.h"
#include "PathFinder_Voxel.h"

CTestObject::CTestObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTestObject::CTestObject(const CTestObject& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CTestObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTestObject::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scaled(1.f, 1.f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(64.f, 5.f, 48.f, 1.f));

	m_pGameInstance->Cooking_Model_Static_Voxels(m_Models[0], m_pTransformCom);

	return S_OK;
}

void CTestObject::Priority_Tick(_float fTimeDelta)
{
}

void CTestObject::Tick(_float fTimeDelta)
{
}

void CTestObject::Late_Tick(_float fTimeDelta)
{
	static _bool		isRender = {};
	if (m_pGameInstance->Get_KeyState('K') == DOWN)
		isRender = !isRender;

	if(isRender)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	//	m_pPathFinder->Render();
}

HRESULT CTestObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (auto pModel : m_Models)
	{
		if (nullptr == pModel)
			continue;

		_uint iNumMeshes = pModel->Get_NumMeshes();

		for (_uint i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(pModel->Bind_ShaderResource(m_pModelShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
				return E_FAIL;

			if (FAILED(pModel->Bind_ShaderResource(m_pModelShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS)))
			{

			}

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(m_pModelShaderCom->Begin(0)))
				return E_FAIL;

			pModel->Render(i);
		}
	}

	return S_OK;
}

HRESULT CTestObject::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxCube"),
		TEXT("Com_Shader"), (CComponent**)&m_pCubeShaderCom)))
		return E_FAIL;

	/* For.Com_VIBufferCube */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Cube"),
		TEXT("Com_VIBufferCube"), (CComponent**)&m_pVIBufferCube)))
		return E_FAIL;



	CNavigation::NAVIGATION_DESC		NaviDesc;
	NaviDesc.pTransform = m_pTransformCom;

	/* For.Com_Navigation */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), (CComponent**)&m_pNavigation, &NaviDesc)))
		return E_FAIL;

	/* For.Com_Shader_Model*/
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader_Model"), (CComponent**)&m_pModelShaderCom)))
		return E_FAIL;

	/* For.Com_VIBufferRect */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBufferRect"), (CComponent**)&m_pVIBufferRect)))
		return E_FAIL;

	/* For.Com_Shader_Rect*/
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader_Rect"), (CComponent**)&m_pRectShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Logo"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Model */

	CModel* pModel = { nullptr };

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_MergePolice"),
		TEXT("Com_Model"), (CComponent**)&pModel)))
		return E_FAIL;
	m_Models.push_back(pModel);

	/*if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"),
		TEXT("Com_Model2"), (CComponent**)&pModel)))
		return E_FAIL;
	m_Models.push_back(pModel);*/

	return S_OK;
}

HRESULT CTestObject::Bind_ShaderResources()
{
	if (nullptr == m_pModelShaderCom)
		return E_FAIL;

	//if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
	//	return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pModelShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pModelShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pModelShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CTestObject* CTestObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTestObject* pInstance = new CTestObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTestObject"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTestObject::Clone(void* pArg)
{
	CTestObject* pInstance = new CTestObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CTestObject"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTestObject::Free()
{
	__super::Free();

	Safe_Release(m_pCubeShaderCom);
	Safe_Release(m_pVIBufferCube);
	Safe_Release(m_pVIBufferRect);
	Safe_Release(m_pNavigation);
	Safe_Release(m_pModelShaderCom);
	Safe_Release(m_pRectShaderCom);
	Safe_Release(m_pTextureCom);

	for (auto& pModel : m_Models)
	{
		Safe_Release(pModel);
		pModel = nullptr;
	}
	m_Models.clear();
}
