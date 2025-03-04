#pragma once
#include "Base.h"

BEGIN(Engine)

class CPathFinder_FlowField final : public CBase
{
private:
	const static _uint			ms_iNumFlowFieldInfos = { 100 };

private:
	typedef struct tagFlowFieldInfo
	{
		unordered_map<_uint, _float>			DistanceMap;
		unordered_map<_uint, _float3>			FlowField;
		unordered_map<_uint, _uint>				ParentMap;
		_int									iRefCnt = {};
	}FLOW_FIELD_INFO;

private:
	CPathFinder_FlowField(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPathFinder_FlowField(const CPathFinder_FlowField& rhs) = delete;
	virtual ~CPathFinder_FlowField() = default;

public:
	HRESULT Initialize();
	HRESULT Render();

public:		//	TEST VECTOR FIELD
	HRESULT Make_VectorFiled_Daijkstra(const vector<_uint>& GoalIndices, const _uint iNumStartAgent, _uint& iIndexTag);
	HRESULT Make_VectorFiled_A_Star(const vector<_uint>& StartIndices, const _uint iGoalIndex, const NODE_INFOS& NodeInfos, const _uint iNumStartAgent, _uint& iIndexTag);
	HRESULT Get_Direction_FlowField(const _float3& vCurPos, const _uint iIndexTag, _float3& vDirection);
	HRESULT Get_ParentIndex_FlowField(const _float3& vCurPos, const _uint iIndexTag, _uint& iParentIndex);
	HRESULT Get_ParentIndex_FlowField(const _uint iVoxelIndex, const _uint iIndexTag, _uint& iParentIndex);

public:
	HRESULT Add_Distances(const _uint iIndexTag, const unordered_map<_uint, _float>& AdditionalDists);

private:
	HRESULT Make_DistanceMap_Daijkstra(const vector<_uint>& GoalIndices, unordered_map<_uint, _float>& DistanceMap);
	HRESULT Make_FlowFieldMap(const unordered_map<_uint, _float>& DistanceMap, unordered_map<_uint, _float3>& FlowFiledMap);
	HRESULT Make_ParentMap(const unordered_map<_uint, _float>& DistanceMap, unordered_map<_uint, _uint>& ParentMap);

public:
	_float Get_DistanceFromGoal(const _uint iVoxelIndex, const _uint iIndexTag);
	const unordered_map<_uint, _float3>& Get_FlowField(const _uint iIndexTag);

public:
	HRESULT AddRef_FlowField(const _uint iIndexTag);
	HRESULT Release_FlowField(const _uint iIndexTag);

public:
	vector<_uint> Get_Active_FlowField_RefCnt();
	vector<_uint> Get_Active_FlowField_Indices();

private:
	_bool Is_Active_Field(const _uint iIndexTag);
	HRESULT Find_Non_Active_IndexTag(_uint& iIndexTag);
	HRESULT Add_FlowField(FLOW_FIELD_INFO& FlowFieldInfo, const _uint iStartRefCnt, _uint& iIndexTag);


private:
	ID3D11Device*							m_pDevice = { nullptr };
	ID3D11DeviceContext*					m_pContext = { nullptr };
	class CGameInstance*					m_pGameInstance = { nullptr };

private:
	vector<FLOW_FIELD_INFO*>				m_FlowFieldInfos;
	_uint									m_iActiveCnt = {};

private:	//	For.A_Star

public:
	static CPathFinder_FlowField* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END