#include "Dummy_Object.h"
#include "GameInstance.h"
#include "VIBuffer.h"
#include "Shader.h"
#include "Renderer.h"

CDummy_Object::CDummy_Object(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

HRESULT CDummy_Object::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
    

    return S_OK;
}

void CDummy_Object::Priority_Tick(_float fTimeDelta)
{
}

void CDummy_Object::Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	CModel* pAnimModel = { static_cast<CModel*>(Get_Component(g_strAnimModelTag)) };
	if (nullptr != pAnimModel)
	{
		pAnimModel->Play_Animation(fTimeDelta);
	}
}

void CDummy_Object::Late_Tick(_float fTimeDelta)
{
}

HRESULT CDummy_Object::Render()
{
	CShader*		pModelShaderCom = { static_cast<CShader*>(Get_Component(g_strShaderModelTag)) };
	CModel*			pModel = { static_cast<CModel*>(Get_Component(g_strModelTag)) };
	if (nullptr != pModelShaderCom &&
		nullptr != pModel)
	{
		if (FAILED(Bind_ShaderResources(pModelShaderCom)))
			return E_FAIL;

		_uint           iNumMeshes = { pModel->Get_NumMeshes() };

		for (_uint i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(pModel->Bind_ShaderResource(pModelShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
				return E_FAIL;

			if (FAILED(pModel->Bind_ShaderResource(pModelShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS)))
			{

			}

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(pModelShaderCom->Begin(0)))
				return E_FAIL;

			pModel->Render(i);
		}
	}

	CShader*		pAnimModelShaderCom = { static_cast<CShader*>(Get_Component(g_strShaderAnimModelTag)) };
	CModel*			pAnimModel = { static_cast<CModel*>(Get_Component(g_strAnimModelTag)) };
	if (nullptr != pAnimModelShaderCom &&
		nullptr != pAnimModel)
	{
		if (FAILED(Bind_ShaderResources(pAnimModelShaderCom)))
			return E_FAIL;

		_uint           iNumMeshes = { pAnimModel->Get_NumMeshes() };

		for (_uint i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(pAnimModel->Bind_ShaderResource(pAnimModelShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
				return E_FAIL;

			if (FAILED(pAnimModel->Bind_ShaderResource(pAnimModelShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS)))
			{

			}

			if (FAILED(pAnimModel->Bind_BoneMatrices(pAnimModelShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(pAnimModelShaderCom->Begin(0)))
				return E_FAIL;

			pAnimModel->Render(i);
		}
	}
    
    return S_OK;
}

_uint CDummy_Object::Get_NumAnims()
{
	CModel* pModel = { static_cast<CModel*>(Get_Component(g_strAnimModelTag)) };
	if (nullptr == pModel)
		return 0;

	return pModel->Get_NumAnims();
}

void CDummy_Object::Set_Animation(const _uint iAnimIndex, const _bool isLoop)
{
	CModel* pModel = { static_cast<CModel*>(Get_Component(g_strAnimModelTag)) };
	if (nullptr == pModel)
		return;

	if (iAnimIndex >= Get_NumAnims())
		return;

	pModel->Set_Animation(iAnimIndex, isLoop);
}

HRESULT CDummy_Object::Add_Component(CComponent* pComponent, const wstring& strComponentTag)
{
    if (nullptr == pComponent)
        return E_FAIL;

    auto	iter = m_Components.find(strComponentTag);
    if (iter != m_Components.end())
        return E_FAIL;

    m_Components.emplace(strComponentTag, pComponent);

    return S_OK;
}

HRESULT CDummy_Object::Bind_ShaderResources(CShader* pShader)
{
	if (nullptr == pShader)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CDummy_Object* CDummy_Object::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
    CDummy_Object* pInstance = { new CDummy_Object(pDevice, pContext) };
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Created : CDummy Object "));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CDummy_Object* CDummy_Object::Clone(void* pArg)
{
    MSG_BOX(TEXT("CDummy_Object Can't Clone"));

    return nullptr;
}

void CDummy_Object::Free()
{
    __super::Free();
}
