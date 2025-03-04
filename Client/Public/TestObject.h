#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Cube;
class CShader;
class CCollider;
class CPathFinder_Voxel;
END

BEGIN(Client)

class CTestObject final : public CGameObject
{
private:
	CTestObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestObject(const CTestObject& rhs);
	virtual ~CTestObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CVIBuffer_Cube*					m_pVIBufferCube = { nullptr };
	CVIBuffer_Rect*					m_pVIBufferRect = { nullptr };
	CShader*						m_pCubeShaderCom = { nullptr };
	CNavigation*					m_pNavigation = { nullptr };
	CShader*						m_pModelShaderCom = { nullptr };
	CShader*						m_pRectShaderCom = { nullptr };
	CTexture*						m_pTextureCom = { nullptr };
	vector<CModel*>					m_Models;

	unordered_set<_uint>			m_OpenList;
	unordered_set<_uint>			m_CloseList;
	list<_uint>						m_BestList;
	list<_uint>						m_OptimizedList;

private:	//	For.PathFinder
	_bool				m_isStartChoice = { false };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CTestObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END