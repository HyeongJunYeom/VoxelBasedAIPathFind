#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Rect;
class CTexture;
class CShader;
END

BEGIN(Client)

class CParticle_Blue final : public CPartObject
{
private:
	CParticle_Blue(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle_Blue(const CParticle_Blue& rhs);
	virtual ~CParticle_Blue() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CVIBuffer_Instance_Rect*	m_pVIBufferCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CShader*					m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CParticle_Blue* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END