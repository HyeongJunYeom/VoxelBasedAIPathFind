#include "..\Public\Obstacle_Mesh.h"
#include "Model.h"
#include "Transform.h"
#include "GameInstance.h"
#include "Shader.h"

CObstacle_Mesh::CObstacle_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CObstacle{ pDevice, pContext }
{
}

CObstacle_Mesh::CObstacle_Mesh(const CObstacle_Mesh& rhs)
	: CObstacle{ rhs }
{
}

HRESULT CObstacle_Mesh::Initialize_Prototype(CModel* pModel)
{
	if (nullptr == pModel)
		return E_FAIL;

	m_LocalPointCloud.clear();
	if (FAILED(pModel->Cooking_ObstacleVoxel_Local(m_LocalPointCloud)))
		return E_FAIL;

	cout << "메시 로컬 버텍스 수 " << endl;
	cout << m_LocalPointCloud.size() << endl;

	_float		fVoxelSize = { m_pGameInstance->Get_WorldSize_Voxel() };

	auto iterSrc{ m_LocalPointCloud.begin() };
	while (iterSrc != m_LocalPointCloud.end())
	{
		auto iterDst{ iterSrc + 1 };
		
		_float3			vSrcPosFloat3 = { *iterSrc };
		_vector			vSrcPos = { XMLoadFloat3(&vSrcPosFloat3) };

		while (iterDst != m_LocalPointCloud.end())
		{
			_float3			vDstPosFloat3 = { *iterDst };
			_vector			vDstPos = { XMLoadFloat3(&vDstPosFloat3) };

			_float			fDist = { XMVectorGetX(XMVector3Length(vSrcPos - vDstPos)) };

			if (fDist < EPSILON)
				iterDst = m_LocalPointCloud.erase(iterDst);
			else 
				++iterDst;
		}

		++iterSrc;
	}
	cout << "메시 로컬 버텍스 수 " << endl;
	cout << m_LocalPointCloud.size() << endl;


	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CObstacle_Mesh::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CObstacle_Mesh::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

HRESULT CObstacle_Mesh::Cull_Point_Cloud_Shape()
{
	return S_OK;
}

CObstacle_Mesh* CObstacle_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CModel* pModel)
{
	CObstacle_Mesh*			pInstance = { new CObstacle_Mesh(pDevice, pContext) };

	if (FAILED(pInstance->Initialize_Prototype(pModel)))
	{
		MSG_BOX(TEXT("Failed To Created : CObstacle_Mesh"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CObstacle_Mesh::Clone(void* pArg)
{
	CObstacle_Mesh*			pInstance = { new CObstacle_Mesh(*this) };

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CObstacle_Mesh"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObstacle_Mesh::Free()
{
	__super::Free();
}
