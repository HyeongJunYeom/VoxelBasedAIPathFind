#pragma once

#include "Base.h"

BEGIN(Engine)

class CObstacle_Manager final : public CBase
{
private:
	CObstacle_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CObstacle_Manager() = default;

public:
	HRESULT Initialize();
	void Priority_Tick(_float fTimeDelta);
	void Tick(_float fTimeDelta);
	void Late_Tick(_float fTimeDelta);

public:
	HRESULT Add_Obstacle(class CObstacle* pObstacle);
	HRESULT Erase_Obstacle(class CObstacle* pObstacle);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

private:
	vector<class CObstacle*>	m_Static_Obstacles;
	vector<class CObstacle*>	m_Dynamic_Obstacles;

public:
	static CObstacle_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END