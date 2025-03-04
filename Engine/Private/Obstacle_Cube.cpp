#include "Obstacle_Cube.h"
#include "Transform.h"
#include "GameInstance.h"
#include "Bounding.h"

CObstacle_Cube::CObstacle_Cube(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CObstacle{ pDevice, pContext }
{
}

CObstacle_Cube::CObstacle_Cube(const CObstacle_Cube& rhs)
	: CObstacle{ rhs }
{
}

HRESULT CObstacle_Cube::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CObstacle_Cube::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CObstacle_Cube::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);


}

HRESULT CObstacle_Cube::Cull_Point_Cloud_Shape()
{
	for (auto iter = m_LocalPointCloud.begin(); iter != m_LocalPointCloud.end(); )
	{
		const _float3& vLocalPosFloat3 = { *iter };
		_vector				vLocalPos = { XMLoadFloat3(&vLocalPosFloat3) };

		_float				fRadius = { fmaxf(XMVectorGetX(vLocalPos), XMVectorGetZ(vLocalPos)) };
		_float				fLocalHeight = { XMVectorGetY(vLocalPos) };

		if (fLocalHeight > m_fHeight ||
			fRadius > m_fRadius)
			iter = m_LocalPointCloud.erase(iter);
		else
			++iter;
	}

	return S_OK;
}

CObstacle_Cube* CObstacle_Cube::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CObstacle_Cube*			pInstance = { new CObstacle_Cube(pDevice, pContext) };

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CObstacle_Cube"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CObstacle_Cube::Clone(void* pArg)
{
	CObstacle_Cube* pInstance = { new CObstacle_Cube(*this) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CObstacle_Cube"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObstacle_Cube::Free()
{
	__super::Free();
}
