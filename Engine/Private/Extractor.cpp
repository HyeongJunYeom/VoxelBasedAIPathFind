#include "..\Public\Extractor.h"
#include "GameInstance.h"

CExtractor::CExtractor(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance { CGameInstance::Get_Instance()}
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CExtractor::Initialize()
{
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	m_iViewportWidth = static_cast<_uint>(ViewportDesc.Width);
	m_iViewportHeight = static_cast<_uint>(ViewportDesc.Height);

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	TextureDesc.Width = static_cast<_uint>(ViewportDesc.Width);
	TextureDesc.Height = static_cast<_uint>(ViewportDesc.Height);
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_STAGING;
	TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTextureHub)))
		return E_FAIL;

	return S_OK;
}

_vector CExtractor::Compute_WorldPos(const _float2 & vViewportPos, const wstring & strZRenderTargetTag, _uint iOffset)
{
	if (FAILED(m_pGameInstance->Copy_Resource(strZRenderTargetTag, &m_pTextureHub)))
		return XMVectorZero();


	//for (size_t i = 0; i < Height; i++)
	//{
	//	for (size_t j = 0; j < Width; j++)
	//	{
	//		_uint iIndex = i * Width + j;

	//	}

	//}

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pTextureHub, 0, D3D11_MAP_READ, 0, &SubResource);

	_uint			iPixelIndex = static_cast<_uint>(vViewportPos.y * m_iViewportWidth + vViewportPos.x);
	
	_ubyte*			pAddress = ((_ubyte*)(((_float4*)SubResource.pData) + iPixelIndex));

	_float			fZ = *((_float*)(pAddress + iOffset));


	m_pContext->Unmap(m_pTextureHub, 0);

	_float3		vProjPos;

	vProjPos.x = vViewportPos.x / (m_iViewportWidth * 0.5f) - 1.f;
	vProjPos.y = vViewportPos.y / -(m_iViewportHeight * 0.5f) + 1.f;
	vProjPos.z = fZ;

	_matrix		ProjMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ);
	_matrix		ViewMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);

	_vector		vWorldPos = XMVector3TransformCoord(XMLoadFloat3(&vProjPos), ProjMatrixInv);
	vWorldPos = XMVector3TransformCoord(vWorldPos, ViewMatrixInv);

	return vWorldPos;
}

CExtractor * CExtractor::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CExtractor*		pInstance = new CExtractor(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CExtractor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CExtractor::Free()
{
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pTextureHub);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
