#include "Picking.h"
#include "Transform.h"
#include "GameInstance.h"
#include "Collider.h"

CPicking::CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPicking::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	return S_OK;
}

void CPicking::Tick()
{
	Update_PerspectiveRay();
}

void CPicking::Transform_PickingToLocalSpace_Perspective(const CTransform* pTransform, _Out_ _float3* pRayDir, _Out_ _float4* pRayPos)
{
	if (nullptr == pRayDir ||
		nullptr == pRayPos)
		return;

	_matrix	WorldMatrixInv = pTransform->Get_WorldMatrix_Inverse();

	_vector vRayDir, vRayPos;
	vRayDir = XMLoadFloat3(&m_vRayDirPerspective);
	vRayPos = XMLoadFloat4(&m_vRayPosPerspective);

	vRayDir = XMVector3TransformNormal(vRayDir, WorldMatrixInv);
	vRayPos = XMVector4Transform(vRayPos, WorldMatrixInv);

	XMStoreFloat3(pRayDir, vRayDir);
	XMStoreFloat4(pRayPos, vRayPos);
}

void CPicking::Transform_PickingToLocalSpace_Perspective(_fmatrix WorldMatrix, _float3* pRayDir, _float4* pRayPos)
{
	if (nullptr == pRayDir ||
		nullptr == pRayPos)
		return;

	_matrix	WorldMatrixInv = XMMatrixInverse(nullptr, WorldMatrix);

	_vector vRayDir, vRayPos;
	vRayDir = XMLoadFloat3(&m_vRayDirOrtho);
	vRayPos = XMLoadFloat4(&m_vRayPosOrtho);

	vRayDir = XMVector3TransformNormal(vRayDir, WorldMatrixInv);
	vRayPos = XMVector4Transform(vRayPos, WorldMatrixInv);

	XMStoreFloat3(pRayDir, vRayDir);
	XMStoreFloat4(pRayPos, vRayPos);
}

void CPicking::Transform_PickingToLocalSpace_Ortho(const CTransform* pTransform, _float3* pRayDir, _float4* pRayPos)
{
	if (nullptr == pRayDir ||
		nullptr == pRayPos)
		return;

	_matrix	WorldMatrixInv = pTransform->Get_WorldMatrix_Inverse();

	_vector vRayDir, vRayPos;
	vRayDir = XMLoadFloat3(&m_vRayDirOrtho);
	vRayPos = XMLoadFloat4(&m_vRayPosOrtho);

	vRayDir = XMVector3TransformNormal(vRayDir, WorldMatrixInv);
	vRayPos = XMVector4Transform(vRayPos, WorldMatrixInv);

	XMStoreFloat3(pRayDir, vRayDir);
	XMStoreFloat4(pRayPos, vRayPos);
}

void CPicking::Update_PerspectiveRay()
{
	POINT ptMouse = m_pGameInstance->Get_MouseCurPos();

	_vector		vMousePos;

	vMousePos = XMVectorSet(
		(_float)ptMouse.x / (m_iWinSizeX * 0.5f) - 1.f,
		(_float)ptMouse.y / -(m_iWinSizeY * 0.5f) + 1.f,
		0.f, 1.f);

	_matrix	ProjMatrixInv;
	ProjMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ);

	//	마우스 위치 뷰변환
	vMousePos = XMVector4Transform(vMousePos, ProjMatrixInv);

	XMStoreFloat3(&m_vRayDirPerspective, vMousePos - XMVectorSet(0.f, 0.f, 0.f, 1.f));
	XMStoreFloat4(&m_vRayPosPerspective, XMVectorSet(0.f, 0.f, 0.f, 1.f));

	_matrix	ViewMatrixInv;
	ViewMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);

	_vector vRayDir, vRayPos;
	vRayDir = XMLoadFloat3(&m_vRayDirPerspective);
	vRayPos = XMLoadFloat4(&m_vRayPosPerspective);

	vRayDir = XMVector3TransformNormal(vRayDir, ViewMatrixInv);
	vRayPos = XMVector4Transform(vRayPos, ViewMatrixInv);

	XMStoreFloat3(&m_vRayDirPerspective, vRayDir);
	XMStoreFloat4(&m_vRayPosPerspective, vRayPos);
}

CPicking* CPicking::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CPicking* pInstance = new CPicking(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX(TEXT("Failed To Created : CPicking"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPicking::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
