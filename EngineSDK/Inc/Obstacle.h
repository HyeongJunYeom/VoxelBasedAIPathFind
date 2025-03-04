#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CObstacle abstract : public CComponent
{
public:
	typedef struct tagObstacleDesc
	{
		class CTransform*		pTransform = { nullptr };
	}OBSTACLE_DESC;

protected:
	CObstacle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CObstacle(const CObstacle& rhs);
	virtual ~CObstacle() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	virtual HRESULT Sample_Point_Cloud();
	virtual HRESULT Cull_Point_Cloud_Shape() = 0;
	virtual HRESULT Update_VoxelWorld();

protected:
	class CTransform*			m_pTransformCom = { nullptr };

	_bool						m_isDynamic = { false };

	_float						m_fRadius = { 1.f };
	_float						m_fHeight = { 1.f };

	vector<_float3>				m_LocalPointCloud;
	vector<_uint3>				m_ActiveIndexPoses;
	unordered_set<_uint>		m_ActiveIndices;

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END