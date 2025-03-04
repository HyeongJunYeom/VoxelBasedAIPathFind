#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CVoxel_Scorer :
	public CBase
{
private:
	CVoxel_Scorer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVoxel_Scorer() = default;

public:
	HRESULT Initialize(void* pArg);
	
public:
	_float Copute_VoxelOpennessScore(_uint iVoxelIndex);

public:
	_uint Get_MaxStep() { return m_iMaxStep; }
	void Set_MaxStep(const _uint iMaxStep) { m_iMaxStep = iMaxStep; }
	_float Get_StepScore() { return m_fStepScore; }
	void Set_StepScore(const _float fStepScore) { m_fStepScore = fStepScore; }

private:
	void Clear_Data();
	void Update_Data();
	void Compute_Openness_Score_BFS(const _uint iCurIndex, const _uint iCurStep = 0);

private:
	HRESULT Compute_Dist(const _uint iSrcIndex, const _uint iDstIndex, _float& fDist);


private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

private:
	_uint						m_iMaxStep = { 2 };
	_uint						m_iNumReachedNeighbor = {};
	_float						m_fTotalScore = {};
	_float						m_fStepScore = { 1.f };
	_float						m_fCurVoxelWorldSize = {};
	unordered_set<_uint>		m_VisitedIndices;



public:
	static CVoxel_Scorer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual void Free() override;

};

END