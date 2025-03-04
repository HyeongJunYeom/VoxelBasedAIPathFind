#include "VoxelRenderCube.h"
#include "VIBuffer_Cube.h"
#include "Shader.h"

CVoxelRenderCube::CVoxelRenderCube(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CVoxelRenderCube::CVoxelRenderCube(const CVoxelRenderCube& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CVoxelRenderCube::Initialize_Prototype()
{
    if (FAILED(Add_Components()))
        return E_FAIL;

    return S_OK;
}

HRESULT CVoxelRenderCube::Render()
{

    return S_OK;
}

HRESULT CVoxelRenderCube::Add_Components()
{
	if (FAILED(Add_Component(CVIBuffer_Cube::Create(m_pDevice, m_pContext), TEXT("Com_VIBuffer"), (CComponent**)(&m_pParentVIBufferCube))))
		return E_FAIL;
	if (FAILED(Add_Component(CVIBuffer_Cube::Create(m_pDevice, m_pContext), TEXT("Com_Shader"), (CComponent**)(&m_pShaderCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CVoxelRenderCube::Bind_ShaderResources()
{
    return E_NOTIMPL;
}

CVoxelRenderCube* CVoxelRenderCube::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVoxelRenderCube*       pInstance = { new CVoxelRenderCube(pDevice, pContext) };

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CVoxelRenderCube"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CVoxelRenderCube::Clone(void* pArg)
{
    return nullptr;
}

void CVoxelRenderCube::Free()
{
    __super::Free();

    Safe_Release(m_pParentVIBufferCube);
    Safe_Release(m_pShaderCom);
}
