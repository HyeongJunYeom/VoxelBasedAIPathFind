#include "Obstacle_Cylinder.h"
#include "Transform.h"
#include "GameInstance.h"
#include "Bounding.h"

CObstacle_Cylinder::CObstacle_Cylinder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CObstacle{ pDevice, pContext }
{
}

CObstacle_Cylinder::CObstacle_Cylinder(const CObstacle_Cylinder& rhs)
	: CObstacle{ rhs }
{
}

HRESULT CObstacle_Cylinder::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CObstacle_Cylinder::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CObstacle_Cylinder::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

HRESULT CObstacle_Cylinder::Cull_Point_Cloud_Shape()
{
	for (auto iter = m_LocalPointCloud.begin(); iter != m_LocalPointCloud.end(); )
	{
		const _float3&		vLocalPosFloat3 = { *iter };
		_vector				vLocalPos = { XMLoadFloat3(&vLocalPosFloat3) };

		_vector				vLocalPosXZPlane = { XMVectorSetY(vLocalPos, 0.f) };
		_float				fRadius = { XMVectorGetX(XMVector3Length(vLocalPosXZPlane)) };
		_float				fLocalHeight = { XMVectorGetY(vLocalPos) };

		if (fLocalHeight > m_fHeight ||
			fRadius > m_fRadius)
		{
			iter = m_LocalPointCloud.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	return S_OK;
}

CObstacle_Cylinder* CObstacle_Cylinder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CObstacle_Cylinder*			pInstance = { new CObstacle_Cylinder(pDevice, pContext) };

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CObstacle_Cylinder"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CObstacle_Cylinder::Clone(void* pArg)
{
	CObstacle_Cylinder*			pInstance = { new CObstacle_Cylinder(*this) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CObstacle_Cylinder"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObstacle_Cylinder::Free()
{
	__super::Free();
}
