#pragma once

#include "Base.h"
#include "Formation_Includes.h"

BEGIN(Engine)

class CFormation_Manager final : public CBase
{
private:
	const static _uint			ms_iNumFormationGroupInfos = { 100 };
	const static _uint			ms_iMaxDiffAquired = { 1 };

private:
	typedef struct tagFormationGroupInfo
	{
		vector<class CFormation*>		Formations;
		vector<_float>					PathTotalLengthes;
		vector<_float>					RatioOffsets;

		_float							fMinRange;
		_float							fMaxRange;

		list<_uint>						VirtualPath;
		_float							fVirtualPathTotalLength = {};
		_float							fRatio = {};

		vector<list<_uint>>				ColsPathes;
		vector<unordered_map<_uint, _float>>			ColsRatios;

		_float							fMinSpeed = {};
		_float							fMaxSpeed = {};
		_uint							iFlowFieldIndexTag = {};

		_bool							isActive = { false };
	}FORMATION_GROUP_INFO;

private:
	CFormation_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CFormation_Manager() = default;

public:
	HRESULT Initialize();
	void Tick(_float fTimeDelta);

public:
	HRESULT Add_Group(_uint& iGroupIndex);
	HRESULT Erase_Group(const _uint iGroupIndex);

	HRESULT Add_Formation(class CFormation* pFormation, const _uint iGroupIndex);
	HRESULT Erase_Formation(class CFormation* pFormation, const _uint iGroupIndex);

public:
	HRESULT Update_Formations(_float fTimeDelta, FORMATION_GROUP_INFO* pGroupInfo);

public:
	HRESULT Initiate_Formation(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex);
	HRESULT Initiate_Formation_For_Flocking(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex);

private:
	HRESULT Initiate_Default_Param(const _uint iGroupIndexTag);
	HRESULT Initiate_Formation_Pathes(const _uint iGroupIndexTag);
	HRESULT Initiate_Formation_Virtual_Path(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex);
	HRESULT Initiate_Formation_Virtual_Path_FlowField(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex);
	HRESULT Initiate_Formation_Path_Lengthes(const _uint iGroupIndexTag);
	HRESULT Initiate_Formation_Rollback_Virtual_Path(const _uint iGroupIndexTag);

private:
	HRESULT Get_Formation_Offsets(vector<_int>& Offset_Width, vector<_int>& Offset_Depth, const _uint iNumAgent) const;

private: 
	HRESULT Compute_Aquired_Range(const _int iMinOffsetWidth, const _int iMaxOffsetWidth, list<_uint>& VirtualPathIndices, vector<vector< _float3>>& AquiredPoses, vector<vector< _bool>>& AquiredSucceed);
	HRESULT Compute_Formation_Offsets(const vector<_int>& Offset_Width, const vector<_int>& Offset_Depth, const _uint iNumAgent, _int& iMinOffsetWidth, _int& iMaxOffsetWidth, _int& iMinOffsetDepth, _int& iMaxOffsetDepth);
	HRESULT Compute_TotalLength_Path(const list<_uint>& PathIndices, _float& fTotalDist);

private:
	HRESULT Optimize_Aquired_Voxels(vector<vector<_float3>>& AquiredPoses, vector<vector<_bool>>& AquiredSucceed, const _uint iCurIndex = 0);
	HRESULT Optimize_Path_Duplication(list<_uint>& PathIndices);

private:
	HRESULT Initiate_Formation_Virtual_Path_Flocking(const _uint iGroupIndexTag, const _uint iStartIndex, const _uint iGoalIndex);

public:
	FORMATION_TYPE Get_Formation_Type() const { return m_eFormationType; }
	void Set_Formation_Type(const FORMATION_TYPE eType) { if (eType >= FORMATION_TYPE::_END) return; m_eFormationType = eType; }


private:
	_float Compute_Path_TotalDist(const list<_uint>& PathIndices);

private:
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*						m_pContext = { nullptr };
	class CGameInstance*						m_pGameInstance = { nullptr };

private:
	vector<FORMATION_GROUP_INFO*>				m_GroupInfos;

private:
	_uint										m_iNumAgent = {};
	vector<_int>								m_Offset_Width;
	vector<_int>								m_Offset_Depth;
	_int										m_iMinOffsetWidth = {};
	_int										m_iMaxOffsetWidth = {};
	_int										m_iMinOffsetDepth = {};
	_int										m_iMaxOffsetDepth = {};

	_float										m_fMinRange = {};
	_float										m_fMaxRange = {};

	FORMATION_TYPE								m_eFormationType = { FORMATION_TYPE::_TWO_LINE };

private:
	//	TEST 
	_bool				m_isFourLine = { false };

public:
	static CFormation_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END