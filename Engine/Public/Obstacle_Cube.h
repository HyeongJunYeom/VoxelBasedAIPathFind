#pragma once

#include "Obstacle.h"

BEGIN(Engine)

class ENGINE_DLL CObstacle_Cube final : public CObstacle
{
private:
	CObstacle_Cube(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CObstacle_Cube(const CObstacle_Cube& rhs);
	virtual ~CObstacle_Cube() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	void Tick(_float fTimeDelta);

private:
	virtual HRESULT Cull_Point_Cloud_Shape() override;

public:
	static CObstacle_Cube* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END