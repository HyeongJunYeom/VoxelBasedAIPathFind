#pragma once

#include "Base.h"
#include "VoxelIncludes.h"
#include "Formation_Includes.h"

BEGIN(Engine)

class CFormation final : public CBase
{
public:
	typedef struct tagFormationDesc
	{
		class CTransform*		pTransform = { nullptr };
		_float*					pSpeed = { nullptr };
		_float*					pMinSpeed = { nullptr };
		_float*					pMaxSpeed = { nullptr };
		_float*					pMaxWaitTime = { nullptr };
	} FORMATION_DESC;

private:
	CFormation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFormation(const CFormation& rhs) = delete;
	virtual ~CFormation() = default;

public:
	HRESULT Initialize(void* pArg);
	void Tick(_float fTimeDelta);

public:
	void Set_Offset_Width(const _int iOffset) { m_iOffset_Width = iOffset; }
	void Set_Offset_Depth(const _int iOffset) { m_iOffset_Depth = iOffset; }
	void Set_BestPathIndices(list<_uint>& BestPathIndices) { m_BestPathIndices = move(BestPathIndices); }
	void Set_PathRatios(unordered_map<_uint, _float> PathRatios) { m_PathRatios = move(PathRatios); }

public:
	const list<_uint>& Get_BestPathIndices() { return m_BestPathIndices; }

public:
	HRESULT Add_FormationGroup(const _uint iGroupIndex);
	HRESULT Erase_FormationGroup(const _uint iGroupIndex);

public:
	CTransform* Get_Transform() { return m_pTransform; }

public:
	HRESULT Update_CurrentRatio();
	HRESULT Update_Speed(const _float fMainRatio, const _float fMinRatio, const _float fMaxRatio);

public:
	_float Get_LastRatio() const { return m_fLastRatio; }
	_float Get_Speed() const { if (nullptr == m_pSpeed) return 0.f;  return *m_pSpeed; }
	_float Get_MinSpeed() const { if (nullptr == m_pMinSpeed) return 0.f; return *m_pMinSpeed; }
	_float Get_MaxSpeed() const { if (nullptr == m_pMaxSpeed) return 0.f; return *m_pMaxSpeed; }
	_float Get_MaxWaitTime() const { return *m_pMaxWaitTime; }
	_float3 Get_TargetPos() const { return m_vTargetPos; }

	void Set_LastRatio(const _float fRatio) { if (fRatio > 1.f || fRatio < 0.f) return;  m_fLastRatio = fmaxf(fRatio, m_fLastRatio); }
	void Set_Speed(const _float fSpeed) { if (fSpeed < 0.f) return; *m_pSpeed = fSpeed; }
	void Set_MaxWaitTime(const _float fMaxWaitTime) { if (fMaxWaitTime < 0.f) return; *m_pMaxWaitTime = fMaxWaitTime; }
	void Set_TargetPos(const _float3& vTargetPos) { m_vTargetPos = vTargetPos; }

	void Reset_LastRatio() { m_fLastRatio = 0.f; }

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

private:
	class CTransform*			m_pTransform = { nullptr };

private:
	_int						m_iOffset_Depth = {};
	_int						m_iOffset_Width = {};

	_float3						m_vTargetPos = {};
	_float*						m_pSpeed = { nullptr };
	_float*						m_pMinSpeed = { nullptr };
	_float*						m_pMaxSpeed = { nullptr };
	_float*						m_pMaxWaitTime = { nullptr };
	_float						m_fLastRatio = { 0.f };

	list<_uint>							m_BestPathIndices;
	unordered_map<_uint, _float>		m_PathRatios;

public:
	static CFormation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END