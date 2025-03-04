#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CCollider;
class CPartObject;
class CNavigation;
END

BEGIN(Client)

class CPlayer final : public CGameObject
{
public:
	enum STATE {
		STATE_IDLE = 0x01,
		STATE_RUN = 0x02,
		STATE_ATT = 0x04,
	};
private:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& rhs);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	map<const wstring, CPartObject*>			m_PartObjects;
	_ubyte										m_eState = {};
	CCollider*									m_pColliderCom = { nullptr };
	CObstacle_Cube*								m_pObstacle = { nullptr };

	vector<_float3>								m_PathPositions;


private:
	HRESULT Add_Components();
	HRESULT Add_PartObjects();

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END