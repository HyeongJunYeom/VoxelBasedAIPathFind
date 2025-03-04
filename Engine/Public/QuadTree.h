#pragma once

#include "Base.h"

BEGIN(Engine)

class CQuadTree final : public CBase
{
public:
	enum CORNER { CORNER_LT, CORNER_RT, CORNER_RB, CORNER_LB, CORNER_END };
	enum NEIGHBOR { NEIGHBOR_LEFT, NEIGHBOR_TOP, NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM, NEIGHBOR_END };

private:
	CQuadTree();
	virtual ~CQuadTree() = default;

public:
	HRESULT Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	HRESULT SetUp_Neighbors();
	void Culling(class CGameInstance * pGameInstance, const _float3 * pVerticesPos, _uint * pIndices, _uint * pNumIndices);
	_bool isDraw(class CGameInstance * pGameInstance, const _float3 * pVerticesPos);

private:
	_uint				m_iCenter = {};

	_uint				m_iCorners[CORNER_END] = {};

	class CQuadTree*	m_pChildren[CORNER_END] = { nullptr, nullptr, nullptr ,nullptr };
	class CQuadTree*	m_pNeighbors[NEIGHBOR_END] = { nullptr, nullptr, nullptr, nullptr };

public:
	static CQuadTree* Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	virtual void Free() override;
};

END