#pragma once

#include "Base.h"
#include "VoxelIncludes.h"

BEGIN(Engine)

class CBoid final : public CBase
{
public:
	typedef struct tagBoidDesc
	{
		class CTransform*		pTransform = { nullptr };
		_bool*					pMove = { nullptr };
		_bool*					pArrived = { nullptr };
	} BOID_DESC;
private:
	CBoid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBoid(const CBoid& rhs) = delete;
	virtual ~CBoid() = default;

public:
	HRESULT Initialize(void* pArg);
	void Tick(_float fTimeDelta);

public:
	HRESULT Compute_ResultDirection_BoidMove(_fvector vSrcPos, _float3& vResultDirection);

public:
	class CTransform* Get_Transform() { return m_pTransform; }

	_float3 Get_NextPos() { return m_vNextPos; }
	_uint Get_CurFlowFieldIndex() { return m_iCurFlowFieldIndex; }
	void Set_NextPos(const _float3& vNextPos) { m_vNextPos = vNextPos; }
	void Set_CurFlowFieldIndex(const _uint iFlowFieldIndex) { m_iCurFlowFieldIndex = iFlowFieldIndex; }

private:
	vector<_uint> Get_NeighborIndices();

private:
	HRESULT Compute_Alignment_Direction(const vector<_uint>& NeighborIndices, _float3& vResultDir);		//	정렬
	HRESULT Compute_Cohesion_Direction(const vector<_uint>& NeighborIndices, _fvector vSrcPos, _float3& vResultDir);		//	응집
	HRESULT Compute_Separation_Direction(const vector<_uint>& NeighborIndices, _fvector vSrcPos, _float3& vResultDir);	//	분리
	HRESULT Compute_Avoidence_Direction(const vector<_uint>& NeighborIndices, _fvector vSrcPos, _float3& vResultDir);		//	회피

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

private:
	class CTransform*			m_pTransform = { nullptr };

private:
	_float						m_fMinDist = { 1.f };
	_float						m_fMaxDist = { 2.f };

	_float3						m_vNextPos = {};
	_uint						m_iCurFlowFieldIndex = {};

	_bool*						m_pMove = { nullptr };
	_bool*						m_pArrived = { nullptr };

public:
	static _float Get_FlowField_Ratio() { return ms_fRatio_FlowField; }
	static _float Get_Alignment_Ratio() { return ms_fRatio_Alignment; }
	static _float Get_Cohesion_Ratio() { return ms_fRatio_Cohesion; }
	static _float Get_Separation_Ratio() { return ms_fRatio_Sepration; }
	static _float Get_Avoidence_Ratio() { return ms_fRatio_Avoidence; }

	static void Set_FlowField_Ratio(const _float fRatio) { if(fRatio < 0.f) return; ms_fRatio_FlowField = fRatio; }
	static void Set_Alignment_Ratio(const _float fRatio) { if(fRatio < 0.f) return; ms_fRatio_Alignment = fRatio; }
	static void Set_Cohesion_Ratio(const _float fRatio) { if(fRatio < 0.f) return; ms_fRatio_Cohesion = fRatio; }
	static void Set_Separation_Ratio(const _float fRatio) { if(fRatio < 0.f) return; ms_fRatio_Sepration = fRatio; }
	static void Set_Avoidence_Ratio(const _float fRatio) { if (fRatio < 0.f) return; ms_fRatio_Avoidence = fRatio; }

private:
	static _float				ms_fRatio_FlowField;
	static _float				ms_fRatio_Alignment;
	static _float				ms_fRatio_Cohesion;
	static _float				ms_fRatio_Sepration;
	static _float				ms_fRatio_Avoidence;

private:
	static _float				ms_fArriveRangeFromGoal;

public:
	static CBoid* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END