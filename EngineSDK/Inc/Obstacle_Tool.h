#pragma once

#include "Engine_Defines.h"
#include "Gui_Object.h"

BEGIN(Engine)

class CObstacle_Tool final : public CGui_Object
{
public:
	typedef struct tagObstacleToolDesc : public GUI_OBJECT_DESC
	{
		vector<class CComponent*>*		pObstacles = { nullptr };
		const _int*						pCurObjectIndex = { nullptr };
	}OBSTACLE_TOOL_DESC;

private:
	CObstacle_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CObstacle_Tool() = default;

public:
	virtual HRESULT Initialize( void* pArg);
	virtual void Tick(_float fTimeDelta);

private:
	vector<class CComponent*>*			m_pObstacles = { nullptr };
	const _int*							m_pCurObjectIndex = { nullptr };

public:
	static CObstacle_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual void Free() override;
};

END