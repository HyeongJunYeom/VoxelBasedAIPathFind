#pragma once

#include "Obstacle.h"

BEGIN(Engine)

class ENGINE_DLL CObstacle_Mesh final : public CObstacle
{
private:
	CObstacle_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CObstacle_Mesh(const CObstacle_Mesh& rhs);
	virtual ~CObstacle_Mesh() = default;

public:
	virtual HRESULT Initialize_Prototype(class CModel* pModel);
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;

private:
	virtual HRESULT Cull_Point_Cloud_Shape() override;

public:
	static CObstacle_Mesh* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, class CModel* pModel);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END