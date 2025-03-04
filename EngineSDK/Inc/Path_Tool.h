#pragma once

#include "Engine_Defines.h"
#include "Gui_Object.h"
#include "VoxelIncludes.h"
#include "PathIncludes.h"

BEGIN(Engine)

class CPath_Tool final : public CGui_Object
{
public:
	typedef struct tagPathToolDesc : public GUI_OBJECT_DESC
	{
		vector<class CComponent*>*		pNavigations = { nullptr };
		const _int*						pCurObjectIndex = { nullptr };
	}PATH_TOOL_DESC;

public:
	enum class MODE { _START, _GOAL, _END };

private:
	CPath_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPath_Tool() = default;

public:
	virtual HRESULT Initialize(class CPath_Manager* pPath_Manager, void* pArg);
	virtual void Tick(_float fTimeDelta);

private:
	void Choice_Mode();
	void Choice_HeuristicWeights();				//	테스트 기능중... 
	void Choice_Algorithm();
	void Choice_Heuristic();
	void Choice_Can_Throw_Path();		//	통과 가능 지형
	void Choice_Test_Mode_Optimizer();
	void Clear_PathList();
	void Update_PathList();

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

	list<_uint>						m_OpenList;
	list<_uint>						m_CloseList;
	list<_uint>						m_BestList;
	list<_uint>						m_OptimizedList;
	
	class CPath_Manager*			m_pPath_Manager = { nullptr };

	vector<class CComponent*>*		m_pNavigations;
	const _int*						m_pCurObjectIndex = { nullptr };


public:
	static CPath_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, class CPath_Manager* pPath_Manager, void* pArg);
	virtual void Free() override;
};

END