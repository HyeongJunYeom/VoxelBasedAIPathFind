#pragma once

#include "Base.h"

BEGIN(Engine)

class CBoid_Manager final : public CBase
{
private:
	CBoid_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBoid_Manager() = default;

public:
	HRESULT Initialize();

public:
	HRESULT Add_Boid(class CBoid* pBoid);
	HRESULT Erase_Boid(class CBoid* pBoid);

public:
	HRESULT Get_Neighbor_Position_Boid(const _uint iNeighborIndex, _float3& vNeighborPos);
	HRESULT Get_Neighbor_Indices_Boid(class CBoid* pBoid, const _float fNeighborRange, vector<_uint>& NeighborIndices);
	HRESULT Get_Neighbor_FlowFieldIndex_Boid(const _uint iNeighborIndex, _uint& iFlowFieldIndexTag);

private:
	HRESULT Find_BoidIndex(class CBoid* pBoid, _uint& iBoidIndex);


private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	class CGameInstance*	m_pGameInstance = { nullptr };

private:
	vector<class CBoid*>					m_Boids;

public:
	static CBoid_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END