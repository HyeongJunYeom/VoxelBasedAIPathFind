#include "Obstacle.h"

BEGIN(Engine)

class ENGINE_DLL CObstacle_Cylinder final : public CObstacle
{
private:
	CObstacle_Cylinder(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CObstacle_Cylinder(const CObstacle_Cylinder& rhs);
	virtual ~CObstacle_Cylinder() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;

private: 
	virtual HRESULT Cull_Point_Cloud_Shape() override;

public:
	static CObstacle_Cylinder* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END