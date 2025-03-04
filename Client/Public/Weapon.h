#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CBone;
class CModel;
class CShader;
class CCollider;
END

BEGIN(Client)

class CWeapon final : public CPartObject
{
public:
	typedef struct tagWeaponDesc: public CPartObject::PARTOBJECT_DESC
	{
		CBone*		pSocket = { nullptr };
	}WEAPON_DESC;

private:
	CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWeapon(const CWeapon& rhs);
	virtual ~CWeapon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;

private:
	CModel*					m_pModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };	
	CBone*					m_pSocket = { nullptr };
	CCollider*				m_pColliderCom = { nullptr };
	
private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END