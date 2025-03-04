#include "..\Public\Texture.h"
#include "Shader.h"

CTexture::CTexture(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{

}

CTexture::CTexture(const CTexture & rhs)
	: CComponent{ rhs }
	, m_Textures{ rhs.m_Textures }
	, m_iNumTextures{ rhs.m_iNumTextures }
{
	for (auto& pSRV : m_Textures)
		Safe_AddRef(pSRV);
}

HRESULT CTexture::Initialize_Prototype(const wstring & strTextureFilePath, _uint iNumTextures)
{
	_tchar		szEXT[MAX_PATH] = TEXT("");

	_wsplitpath_s(strTextureFilePath.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, szEXT, MAX_PATH);

	HRESULT hr = { 0 };

	_tchar		szFileName[MAX_PATH] = { TEXT("") };

	ID3D11ShaderResourceView* pSRV = { nullptr };

	if (false == lstrcmp(szEXT, TEXT(".dds")))
	{
		for (size_t i = 0; i < iNumTextures; i++)
		{
			wsprintf(szFileName, strTextureFilePath.c_str(), i);

			hr = CreateDDSTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Textures.push_back(pSRV);
		}

	}
	else if (false == lstrcmp(szEXT, TEXT(".tga")))
	{
		for (_uint i = 0; i < iNumTextures; ++i)
		{
			wstring strPerfectPath = { TEXT("") };
			size_t iFindIndex_fbx = strTextureFilePath.find(L".tga");
			strPerfectPath = strTextureFilePath.substr(0, iFindIndex_fbx);
			strPerfectPath += L".dds";

			wsprintf(szFileName, strPerfectPath.c_str(), i);

			hr = CreateDDSTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Textures.push_back(pSRV);
		}
	}
	else if (!lstrcmp(szEXT, TEXT(".tex")))
	{
		for (_uint i = 0; i < iNumTextures; ++i)
		{
			_tchar         szDriveName[MAX_PATH] = { TEXT("") };
			_tchar         szDirName[MAX_PATH] = { TEXT("") };
			wstring      strPerfectPath = { TEXT("") };
			_wsplitpath_s(strTextureFilePath.c_str(), szDriveName, MAX_PATH, szDirName, MAX_PATH, nullptr, 0, nullptr, 0);
			strPerfectPath += szDriveName;
			strPerfectPath += szDirName;
			strPerfectPath += TEXT("NullWhite.dds");

			wsprintf(szFileName, strPerfectPath.c_str(), i);

			hr = CreateDDSTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Textures.push_back(pSRV);
		}

	}
	else
	{
		for (size_t i = 0; i < iNumTextures; i++)
		{
			wsprintf(szFileName, strTextureFilePath.c_str(), i);

			hr = CreateWICTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Textures.push_back(pSRV);
		}
	}

	m_iNumTextures = iNumTextures;

	//if (pDesc != nullptr)
	//{
	//	IMG_SIZE Size;
	//	Size.iSizeX = pDesc->iWidth;
	//	Size.iSizeY = pDesc->iHeight;
	//	m_TextureSize = Size;

	//	m_DivideCount.first = pDesc->iCountX;
	//	m_DivideCount.second = pDesc->iCountY;
	//}

	return S_OK;
}

HRESULT CTexture::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CTexture::Bind_ShaderResource(CShader * pShader, const _char * pConstantName, _uint iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_Textures[iTextureIndex]);	
}

HRESULT CTexture::Bind_ShaderResources(CShader * pShader, const _char * pConstantName)
{
	return pShader->Bind_Textures(pConstantName, &m_Textures.front(), m_iNumTextures);
}

CTexture * CTexture::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strTextureFilePath, _uint iNumTexture)
{
	CTexture*		pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strTextureFilePath, iNumTexture)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CTexture::Clone(void * pArg)
{
	CTexture*		pInstance = new CTexture(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTexture"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTexture::Free()
{
	__super::Free();

	for (auto& pSRV : m_Textures)
		Safe_Release(pSRV);

	m_Textures.clear();
}
