#pragma once

#include "Engine_Defines.h"
#include "Gui_Object.h"
#include "VoxelIncludes.h"
#include "PathIncludes.h"

BEGIN(Engine)

class CNavi_Tool final : public CGui_Object
{
public:
	typedef struct tagNaviToolDesc : public GUI_OBJECT_DESC
	{
		vector<class CComponent*>*		pNavigations = { nullptr };
		vector<class CComponent*>*		pTransforms = { nullptr };
		const _int*						pCurObjectIndex = { nullptr };
	}NAVI_TOOL_DESC;

public:
	enum class MODE { _START, _GOAL, _END };

private:
	CNavi_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CNavi_Tool() = default;

public:
	virtual HRESULT Initialize(class CPath_Manager* pPath_Manager, void* pArg);
	virtual void Tick(_float fTimeDelta);

public:
	void Shuffle_All_Agents_FlowField();
	void Shuffle_All_Agents_Floor();

private:
	void Choice_Mode();
	void Choice_HeuristicWeights();				//	테스트 기능중... 
	void Choice_Algorithm();
	void Choice_Heuristic();
	void Choice_Can_Throw_Path();		//	통과 가능 지형
	void Choice_Test_Mode_Optimizer();

private:
	void Move_AllAgent_Make_Flow_Field();

private:
	void Set_AgentParams();
	void Set_BoidParams();

private:
	void Set_GoalIndex_VectorField();
	void Choice_Formation_Type();
	void Make_Formation_Initiate();
	void Make_Formation_Initiate2();
	void Make_Formation_Initiate_Flocking();
	void Make_Move_VectorField_Daijkstra();
	void Make_Move_VectorField_A_Star();
	void Make_Path_Move_A_Star();
	void Clear_VectorField();

	void Show_VectorFieldInfos();

private:
	void Show_CurObjectPathLists();
	void Show_OpenList(class CNavigation* pNavigaion);
	void Show_CloseList(class CNavigation* pNavigaion);
	void Show_BestList(class CNavigation* pNavigaion);

private:
	class CNavigation* Get_CurObjectNavigation();

private:
	void Show_Current_PathInfos();

private:	
	void Show_Current_OpenListInfo();
	void Show_Current_CloseListInfo();
	void Show_Current_BestListInfo();
	void Show_Current_OptimizedListInfo();

private:
	void Show_TotalDistance();
	void Show_TotalTime();

private:
	void Make_Path();
	void Pick_Voxel_Change_State();

private:
	HRESULT Add_Components();
	

private:
	MODE							m_eMode = { MODE::_END };
	
	class CPath_Manager*			m_pPath_Manager = { nullptr };

	vector<class CComponent*>*		m_pNavigations = { nullptr };
	vector<class CComponent*>*		m_pTransforms = { nullptr };
	const _int*						m_pCurObjectIndex = { nullptr };

	vector<_uint>					m_PreOpenList;
	vector<_uint>					m_PreCloseList;
	vector<_uint>					m_PreBestList;

	_bool							m_isShowOpenList = { true };
	_bool							m_isShowCloseList = { true };
	_bool							m_isShowBestList = { true };

	_uint							m_iGoalIndexVectorFiled = {};
	vector<_uint>					m_GoalIndices;


public:
	static CNavi_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, class CPath_Manager* pPath_Manager, void* pArg);
	virtual void Free() override;
};

END