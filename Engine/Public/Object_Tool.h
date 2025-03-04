#pragma once

/// 오브젝트 툴 활용하여 
//  1. 오브젝트 생성
//  2. 오브젝트에 컴포넌트 부착, 및 세부 조정 가능하게
//  3.  ex) Navigation 부착하여 에이전트 속성등 설정 가능
//      ex) Obstacle 부착하여 동적 장애물, 정적 장애물로 활용가능

//  4.  추 후 계획 
//      장애물 생성, 및 동적장애물의 로밍 활동
//      

#pragma once

#include "Engine_Defines.h"
#include "Gui_Object.h"
#include "VoxelIncludes.h"
#include "PathIncludes.h"

BEGIN(Engine)

class CObject_Tool final : public CGui_Object
{
public:
	typedef struct tagObjectToolDesc : public GUI_OBJECT_DESC
	{
		vector<vector<class CComponent*>>*		pComponents = { nullptr };
		vector<class CDummy_Object*>*			pDummyObjects = { nullptr };
		_int*									pCurObjectIndex = { nullptr };
	}OBJECT_TOOL_DESC;

private:
	CObject_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CObject_Tool() = default;

public:
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);

private:
	void Select_ComponentType();
	void Select_ObstacleType();
	void Select_ObjectIndex();

private:
	void Show_CurObject_ComponentInfos();

	void Show_NavigationInfo(const _int iObjectIdx);
	void Show_ObstacleInfo(const _int iObjectIdx);
	void Show_TransformInfo(const _int iObjectIdx);
	void Show_AnimationInfo(const _int iObjectIdx);

private:
	HRESULT Create_Object();
	HRESULT Erase_Objest(const _int iObjectIdx);
	HRESULT Add_Component_To_Object(const _int iObjectIdx, const COMPONENT_TYPE eComoponnent);
	HRESULT Erase_Component();

private:
	_bool Is_OutOfRange_Objects(const _int iObjectIdx);

private:
	HRESULT Add_Components();

private:
	vector<class CDummy_Object*>*			m_pObjects = { nullptr };
	vector<vector<class CComponent*>>*		m_pComponents = { nullptr };
	_int*									m_pCurObjectIdx = { nullptr };
	COMPONENT_TYPE							m_eComponentType = { COMPONENT_TYPE::_END };
	OBSTACLE_TYPE							m_eObstacleType = { OBSTACLE_TYPE::_MESH };

private:
	class CModel* m_pPrototypeModel = { nullptr };

public:
	static CObject_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END