#include "Engine_Defines.h"
#include "Voxel.h"
#include "VIBuffer_Cube.h"
#include "Shader.h"

//class CImpl_Voxel
//{
//public:
//	
//
//private:
//	CVIBuffer_Cube*			m_pVIBuffer_Cube = { nullptr };
//	CShader*				m_pShader = { nullptr };
//};

CVoxel::CVoxel()
{
}

CVoxel::CVoxel(const CVoxel& rhs)
	: m_iID{ rhs.m_iID }
	, m_vPos{ rhs.m_vPos }
{
}

HRESULT CVoxel::Initialize_Prototype()
{


	return S_OK;
}

HRESULT CVoxel::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CVoxel::Tick(_float fTimeDelta)
{
}

void CVoxel::Late_Tick(_float fTimeDelta)
{
}

HRESULT CVoxel::Render()
{
	return S_OK;
}

HRESULT CVoxel::Set_ID(_int iID)
{
	m_iID = iID;
	return S_OK;
}

HRESULT CVoxel::Set_Pos(const _float3& vPos)
{
	m_vPos = vPos;
	return S_OK;
}

_int CVoxel::Get_ID()
{
	return m_iID;
}

_float3 CVoxel::Get_Pos()
{
	return m_vPos;
}

CVoxel* CVoxel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVoxel*			pInstance = { new CVoxel{} };

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CVoxel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CVoxel* CVoxel::Clone(void* pArg)
{
	CVoxel* pInstance = { new CVoxel{ *this } };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CVoxel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVoxel::Free()
{
	__super::Free();
}
