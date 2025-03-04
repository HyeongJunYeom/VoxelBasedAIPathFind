#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CParticle_Red final : public CGameObject
{
private:
	CParticle_Red(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle_Red(const CParticle_Red& rhs);
	virtual ~CParticle_Red() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CVIBuffer_Instance_Point*	m_pVIBufferCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CShader*					m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CParticle_Red* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END